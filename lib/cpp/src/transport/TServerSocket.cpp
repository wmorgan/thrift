// Copyright (c) 2006- Facebook
// Distributed under the Thrift Software License
//
// See accompanying file LICENSE or visit the Thrift site at:
// http://developers.facebook.com/thrift/

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <errno.h>

#include "TSocket.h"
#include "TServerSocket.h"
#include <boost/shared_ptr.hpp>

namespace facebook { namespace thrift { namespace transport { 

using namespace boost;

TServerSocket::TServerSocket(int port) :
  port_(port),
  serverSocket_(-1),
  acceptBacklog_(1024),
  sendTimeout_(0),
  recvTimeout_(0),
  intSock1_(-1),
  intSock2_(-1) {}

TServerSocket::TServerSocket(int port, int sendTimeout, int recvTimeout) :
  port_(port),
  serverSocket_(-1),
  acceptBacklog_(1024),
  sendTimeout_(sendTimeout),
  recvTimeout_(recvTimeout),
  intSock1_(-1),
  intSock2_(-1) {}

TServerSocket::~TServerSocket() {
  close();
}

void TServerSocket::setSendTimeout(int sendTimeout) {
  sendTimeout_ = sendTimeout;
}

void TServerSocket::setRecvTimeout(int recvTimeout) {
  recvTimeout_ = recvTimeout;
}

void TServerSocket::listen() {
  int sv[2];
  if (-1 == socketpair(AF_LOCAL, SOCK_STREAM, 0, sv)) {
    perror("TServerSocket::init()");
    intSock1_ = -1;
    intSock2_ = -1;
  } else {
    intSock1_ = sv[0];
    intSock2_ = sv[1];
  }

  serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket_ == -1) {
    perror("TServerSocket::listen() socket");
    close();
    throw TTransportException(TTransportException::NOT_OPEN, "Could not create server socket.");
  }

  // Set reusaddress to prevent 2MSL delay on accept
  int one = 1;
  if (-1 == setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR,
                       &one, sizeof(one))) {
    perror("TServerSocket::listen() SO_REUSEADDR");
    close();
    throw TTransportException(TTransportException::NOT_OPEN, "Could not set SO_REUSEADDR");
  }

  // Defer accept
  #ifdef TCP_DEFER_ACCEPT
  if (-1 == setsockopt(serverSocket_, SOL_SOCKET, TCP_DEFER_ACCEPT,
                       &one, sizeof(one))) {
    perror("TServerSocket::listen() TCP_DEFER_ACCEPT");
    close();
    throw TTransportException(TTransportException::NOT_OPEN, "Could not set TCP_DEFER_ACCEPT");
  }
  #endif // #ifdef TCP_DEFER_ACCEPT

  // Turn linger off, don't want to block on calls to close
  struct linger ling = {0, 0};
  if (-1 == setsockopt(serverSocket_, SOL_SOCKET, SO_LINGER,
                       &ling, sizeof(ling))) {
    close();
    perror("TServerSocket::listen() SO_LINGER");
    throw TTransportException(TTransportException::NOT_OPEN, "Could not set SO_LINGER");
  }

  // TCP Nodelay, speed over bandwidth
  if (-1 == setsockopt(serverSocket_, IPPROTO_TCP, TCP_NODELAY,
                       &one, sizeof(one))) {
    close();
    perror("setsockopt TCP_NODELAY");
    throw TTransportException(TTransportException::NOT_OPEN, "Could not set TCP_NODELAY");
  }

  // Set NONBLOCK on the accept socket
  int flags = fcntl(serverSocket_, F_GETFL, 0);
  if (flags == -1) {
    throw TTransportException(TTransportException::NOT_OPEN, "fcntl() failed");
  }

  if (-1 == fcntl(serverSocket_, F_SETFL, flags | O_NONBLOCK)) {
    throw TTransportException(TTransportException::NOT_OPEN, "fcntl() failed");
  }

  // Bind to a port
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (-1 == bind(serverSocket_, (struct sockaddr *)&addr, sizeof(addr))) {
    char errbuf[1024];
    sprintf(errbuf, "TServerSocket::listen() BIND %d", port_);
    perror(errbuf);
    close();
    throw TTransportException(TTransportException::NOT_OPEN, "Could not bind");
  }

  // Call listen
  if (-1 == ::listen(serverSocket_, acceptBacklog_)) {
    perror("TServerSocket::listen() LISTEN");
    close();
    throw TTransportException(TTransportException::NOT_OPEN, "Could not listen");
  }

  // The socket is now listening!
}

shared_ptr<TTransport> TServerSocket::acceptImpl() {
  if (serverSocket_ < 0) {
    throw TTransportException(TTransportException::NOT_OPEN, "TServerSocket not listening");
  }

  fd_set fds;

  while (true) {
    FD_ZERO(&fds);
    FD_SET(serverSocket_, &fds);
    if (intSock2_ >= 0) {
      FD_SET(intSock2_, &fds);
    }
    int ret = select(serverSocket_+1, &fds, NULL, NULL, NULL);

    if (ret < 0) {
      perror("TServerSocket::acceptImpl() select -1");
      throw TTransportException(TTransportException::UNKNOWN);
    } else if (ret > 0) {
      // Check for an interrupt signal
      if (intSock2_ >= 0 && FD_ISSET(intSock2_, &fds)) {      
        int8_t buf;
        if (-1 == recv(intSock2_, &buf, sizeof(int8_t), 0)) {
          perror("TServerSocket::acceptImpl() interrupt receive");
        }
        throw TTransportException(TTransportException::INTERRUPTED);
      }
      // Check for the actual server socket being ready
      if (FD_ISSET(serverSocket_, &fds)) {
        break;
      }
    } else {
      perror("TServerSocket::acceptImpl() select 0");
      throw TTransportException(TTransportException::UNKNOWN);      
    }
  }

  struct sockaddr_in clientAddress;
  int size = sizeof(clientAddress);
  int clientSocket = ::accept(serverSocket_,
                              (struct sockaddr *) &clientAddress,
                              (socklen_t *) &size);
    
  if (clientSocket < 0) {
    perror("TServerSocket::accept()");
    throw TTransportException(TTransportException::UNKNOWN, "ERROR:" + errno);
  }

  // Make sure client socket is blocking
  int flags = fcntl(clientSocket, F_GETFL, 0);
  if (flags == -1) {
    perror("TServerSocket::select() fcntl GETFL");
    throw TTransportException(TTransportException::UNKNOWN, "ERROR:" + errno);
  }
  if (-1 == fcntl(clientSocket, F_SETFL, flags & ~O_NONBLOCK)) {
    perror("TServerSocket::select() fcntl SETFL");
    throw TTransportException(TTransportException::UNKNOWN, "ERROR:" + errno);
  }
  
  shared_ptr<TSocket> client(new TSocket(clientSocket));
  if (sendTimeout_ > 0) {
    client->setSendTimeout(sendTimeout_);
  }
  if (recvTimeout_ > 0) {
    client->setRecvTimeout(recvTimeout_);
  }
  
  return client;
}

void TServerSocket::interrupt() {
  if (intSock1_ >= 0) {
    int8_t byte = 0;
    if (-1 == send(intSock1_, &byte, sizeof(int8_t), 0)) {
      perror("TServerSocket::interrupt()");
    }
  }
}

void TServerSocket::close() {
  if (serverSocket_ >= 0) {
    shutdown(serverSocket_, SHUT_RDWR);
    ::close(serverSocket_);
  }
  if (intSock1_ >= 0) {
    ::close(intSock1_);
  }
  if (intSock2_ >= 0) {
    ::close(intSock2_);
  }
  serverSocket_ = -1;
  intSock1_ = -1;
  intSock2_ = -1;
}

}}} // facebook::thrift::transport