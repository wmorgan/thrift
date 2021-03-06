// Copyright (c) 2006- Facebook
// Distributed under the Thrift Software License
//
// See accompanying file LICENSE or visit the Thrift site at:
// http://developers.facebook.com/thrift/

#include "PosixThreadFactory.h"
#include "Exception.h"

#include <assert.h>
#include <pthread.h>

#include <iostream>

#include <boost/weak_ptr.hpp>

namespace facebook { namespace thrift { namespace concurrency {

using boost::shared_ptr;
using boost::weak_ptr;

/**
 * The POSIX thread class.
 *
 * @author marc
 * @version $Id:$
 */
class PthreadThread: public Thread {
 public:

  enum STATE {
    uninitialized,
    starting,
    started,
    stopping,
    stopped
  };

  static const int MB = 1024 * 1024;

  static void* threadMain(void* arg);

 private:
  pthread_t pthread_;
  STATE state_;
  int policy_;
  int priority_;
  int stackSize_;
  weak_ptr<PthreadThread> self_;
  bool detached_;

 public:

  PthreadThread(int policy, int priority, int stackSize, bool detached, shared_ptr<Runnable> runnable) :
    pthread_(0),
    state_(uninitialized),
    policy_(policy),
    priority_(priority),
    stackSize_(stackSize),
    detached_(detached) {

    this->Thread::runnable(runnable);
  }

  ~PthreadThread() {
    /* Nothing references this thread, if is is not detached, do a join
       now, otherwise the thread-id and, possibly, other resources will
       be leaked. */
    if(!detached_) {
      try {
        join();
      } catch(...) {
        // We're really hosed.
      }
    }
  }

  void start() {
    if (state_ != uninitialized) {
      return;
    }

    pthread_attr_t thread_attr;
    if (pthread_attr_init(&thread_attr) != 0) {
        throw SystemResourceException("pthread_attr_init failed");
    }

    if(pthread_attr_setdetachstate(&thread_attr,
                                   detached_ ?
                                   PTHREAD_CREATE_DETACHED :
                                   PTHREAD_CREATE_JOINABLE) != 0) {
        throw SystemResourceException("pthread_attr_setdetachstate failed");
    }

    // Set thread stack size
    if (pthread_attr_setstacksize(&thread_attr, MB * stackSize_) != 0) {
      throw SystemResourceException("pthread_attr_setstacksize failed");
    }

    // Set thread policy
    if (pthread_attr_setschedpolicy(&thread_attr, policy_) != 0) {
      throw SystemResourceException("pthread_attr_setschedpolicy failed");
    }

    struct sched_param sched_param;
    sched_param.sched_priority = priority_;

    // Set thread priority
    if (pthread_attr_setschedparam(&thread_attr, &sched_param) != 0) {
      throw SystemResourceException("pthread_attr_setschedparam failed");
    }

    // Create reference
    shared_ptr<PthreadThread>* selfRef = new shared_ptr<PthreadThread>();
    *selfRef = self_.lock();

    state_ = starting;

    if (pthread_create(&pthread_, &thread_attr, threadMain, (void*)selfRef) != 0) {
      throw SystemResourceException("pthread_create failed");
    }
  }

  void join() {
    if (!detached_ && state_ != uninitialized) {
      void* ignore;
      /* XXX
         If join fails it is most likely due to the fact
         that the last reference was the thread itself and cannot
         join.  This results in leaked threads and will eventually
         cause the process to run out of thread resources.
         We're beyond the point of throwing an exception.  Not clear how
         best to handle this. */
      detached_ = pthread_join(pthread_, &ignore) == 0;
    }
  }

  id_t getId() {
    // TODO(dreiss): Stop using C-style casts.
    return (id_t)pthread_;
  }

  shared_ptr<Runnable> runnable() const { return Thread::runnable(); }

  void runnable(shared_ptr<Runnable> value) { Thread::runnable(value); }

  void weakRef(shared_ptr<PthreadThread> self) {
    assert(self.get() == this);
    self_ = weak_ptr<PthreadThread>(self);
  }
};

void* PthreadThread::threadMain(void* arg) {
  shared_ptr<PthreadThread> thread = *(shared_ptr<PthreadThread>*)arg;
  delete reinterpret_cast<shared_ptr<PthreadThread>*>(arg);

  if (thread == NULL) {
    return (void*)0;
  }

  if (thread->state_ != starting) {
    return (void*)0;
  }

  thread->state_ = starting;
  thread->runnable()->run();
  if (thread->state_ != stopping && thread->state_ != stopped) {
    thread->state_ = stopping;
  }

  return (void*)0;
}

/**
 * POSIX Thread factory implementation
 */
class PosixThreadFactory::Impl {

 private:
  POLICY policy_;
  PRIORITY priority_;
  int stackSize_;
  bool detached_;

  /**
   * Converts generic posix thread schedule policy enums into pthread
   * API values.
   */
  static int toPthreadPolicy(POLICY policy) {
    switch (policy) {
    case OTHER:
      return SCHED_OTHER;
    case FIFO:
      return SCHED_FIFO;
    case ROUND_ROBIN:
      return SCHED_RR;
    }
    return SCHED_OTHER;
  }

  /**
   * Converts relative thread priorities to absolute value based on posix
   * thread scheduler policy
   *
   *  The idea is simply to divide up the priority range for the given policy
   * into the correpsonding relative priority level (lowest..highest) and
   * then pro-rate accordingly.
   */
  static int toPthreadPriority(POLICY policy, PRIORITY priority) {
    int pthread_policy = toPthreadPolicy(policy);
    int min_priority = sched_get_priority_min(pthread_policy);
    int max_priority = sched_get_priority_max(pthread_policy);
    int quanta = (HIGHEST - LOWEST) + 1;
    float stepsperquanta = (max_priority - min_priority) / quanta;

    if (priority <= HIGHEST) {
      return (int)(min_priority + stepsperquanta * priority);
    } else {
      // should never get here for priority increments.
      assert(false);
      return (int)(min_priority + stepsperquanta * NORMAL);
    }
  }

 public:

  Impl(POLICY policy, PRIORITY priority, int stackSize, bool detached) :
    policy_(policy),
    priority_(priority),
    stackSize_(stackSize),
    detached_(detached) {}

  /**
   * Creates a new POSIX thread to run the runnable object
   *
   * @param runnable A runnable object
   */
  shared_ptr<Thread> newThread(shared_ptr<Runnable> runnable) const {
    shared_ptr<PthreadThread> result = shared_ptr<PthreadThread>(new PthreadThread(toPthreadPolicy(policy_), toPthreadPriority(policy_, priority_), stackSize_, detached_, runnable));
    result->weakRef(result);
    runnable->thread(result);
    return result;
  }

  int getStackSize() const { return stackSize_; }

  void setStackSize(int value) { stackSize_ = value; }

  PRIORITY getPriority() const { return priority_; }

  /**
   * Sets priority.
   *
   *  XXX
   *  Need to handle incremental priorities properly.
   */
  void setPriority(PRIORITY value) { priority_ = value; }

  bool isDetached() const { return detached_; }

  void setDetached(bool value) { detached_ = value; }

  Thread::id_t getCurrentThreadId() const {
    // TODO(dreiss): Stop using C-style casts.
    return (id_t)pthread_self();
  }

};

PosixThreadFactory::PosixThreadFactory(POLICY policy, PRIORITY priority, int stackSize, bool detached) :
  impl_(new PosixThreadFactory::Impl(policy, priority, stackSize, detached)) {}

shared_ptr<Thread> PosixThreadFactory::newThread(shared_ptr<Runnable> runnable) const { return impl_->newThread(runnable); }

int PosixThreadFactory::getStackSize() const { return impl_->getStackSize(); }

void PosixThreadFactory::setStackSize(int value) { impl_->setStackSize(value); }

PosixThreadFactory::PRIORITY PosixThreadFactory::getPriority() const { return impl_->getPriority(); }

void PosixThreadFactory::setPriority(PosixThreadFactory::PRIORITY value) { impl_->setPriority(value); }

bool PosixThreadFactory::isDetached() const { return impl_->isDetached(); }

void PosixThreadFactory::setDetached(bool value) { impl_->setDetached(value); }

Thread::id_t PosixThreadFactory::getCurrentThreadId() const { return impl_->getCurrentThreadId(); }

}}} // facebook::thrift::concurrency
