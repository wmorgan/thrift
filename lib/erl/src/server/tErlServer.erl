%%% Copyright (c) 2007- Facebook
%%% Distributed under the Thrift Software License
%%%
%%% See accompanying file LICENSE or visit the Thrift site at:
%%% http://developers.facebook.com/thrift/

-module(tErlServer).

-include("oop.hrl").

-include("thrift.hrl").
-include("transport/tTransportException.hrl").
-include("server/tErlServer.hrl").

-behavior(oop).

-export([attr/4, super/0, inspect/1]).

-export([new/6, new/5, new/4, effectful_serve/1, effectful_new_acceptor/1]).

%%%
%%% define attributes
%%% 'super' is required unless ?MODULE is a base class
%%%

?DEFINE_ATTR(super);
?DEFINE_ATTR(acceptor);
?DEFINE_ATTR(listenSocket);
?DEFINE_ATTR(port).

%%%
%%% behavior callbacks
%%%

%%% super() -> SuperModule = atom()
%%%             |  none

super() ->
    tServer.

%%% inspect(This) -> string()

inspect(This) ->
    ?FORMAT_ATTR(acceptor) ++ ", " ++
    ?FORMAT_ATTR(listenSocket) ++ ", " ++
    ?FORMAT_ATTR(port).

%%%
%%% class methods
%%%

new(Port, Handler, Processor, ServerTransport, TransportFactory, ProtocolFactory) ->
    Super = (super()):new(Handler, Processor, ServerTransport, TransportFactory, ProtocolFactory),
    #?MODULE{super=Super, port=Port, listenSocket=nil, acceptor=nil}.

new(Port, Handler, Processor, ServerTransport) ->
    new(Port, Handler, Processor, ServerTransport, nil, nil).

new(Port, Handler, Processor, ServerTransport, TransportFactory) ->
    new(Port, Handler, Processor, ServerTransport, TransportFactory, nil).

% listenSocket, acceptor, port

effectful_serve(This) ->
    Port = oop:get(This, port),

    Options = [binary, {packet, 0}, {active, false}],

    %% listen
    case gen_tcp:listen(Port, Options) of
        {ok, ListenSocket} ->
            ?INFO("thrift server listening on port ~p", [Port]),

            This1 = oop:set(This, listenSocket, ListenSocket),

            %% spawn acceptor
            {_Acceptor, This2} = effectful_new_acceptor(This1),

            {ok, This2};

        {error, eaddrinuse} ->
            ?ERROR("thrift couldn't bind port ~p, address in use", [Port]),
            {{error, eaddrinuse}, This} %% state before the accept
    end.

effectful_new_acceptor(This) ->
    ListenSocket = oop:get(This, listenSocket),
    Processor    = oop:get(This, processor), %% cpiro: generated processor, not the "actual" processor
    Handler      = oop:get(This, handler),

    TF = oop:get(This, transportFactory),
    PF = oop:get(This, protocolFactory),

    tErlAcceptor = oop:get(This, serverTransport), %% cpiro: only supported ServerTransport

    ServerPid = self(),
    Acceptor  = oop:start_new(tErlAcceptor, [ServerPid, TF, PF]),
    ?C3(Acceptor, accept, ListenSocket, Processor, Handler),

    This1 = oop:set(This, acceptor, Acceptor),

    {Acceptor, This1}.
