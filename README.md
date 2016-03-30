# Thrift Dispatcher

A client library dispatch thrift protocol requests to a batch of servers and merge their results

All online services provide an interface based on thrift RPC, for external access to the internal state of the engines' information



## Description

A client dispatch requests to query the status of all engines, put the RPC process into a dispatch executor

These RPC calls are asynchronously sent to the peer in multithreads

The client creates a merge task and put it into a wait executor to wait for all RPC calls to complete

The task keeps all the future of each RPC Call, and give its own future to the client

When all RPC calls finished, the wait task merge their results and return it back to the client through the merge future


## Features

This library support all forms of thrift rpc functions, no matter whether a RPC call has parameters or a return

So you can just define your custom RPC function and use ThriftClients<...>::Dispatch() to send all the requests


## Example

Client

* define a thrift RPC class

```c++
service Example
{
  void NoParamsNoReturn(),
  void HasParamsNoReturn(1: string thing, 2: Xtruct xtruct, 3: Insanity insanity),
  Xtruct NoParamsHasReturn(),
  Insanity HasParamsHasReturn(1: string thing, 2: Xtruct xtruct, 3: Insanity insanity)
}
```

* define a thrift RPC Client 

```c++
typedef ThriftClients<ExampleClient, Executors, Executors> DispatchClient;
```

* call Dispatch() to send request and add RPC parameters

```c++
std::unordered_set<NetPoint> results1 = client.Dispatch(&ExampleClient::NoParamsNoReturn).get();
std::unordered_set<NetPoint> results2 = client.Dispatch(&ExampleClient::HasParamsNoReturn, "HasParamsNoReturn", param, insane).get();
std::unordered_map<NetPoint, Xtruct> results3 = client.Dispatch(&ExampleClient::NoParamsHasReturn).get();
std::unordered_map<NetPoint, Insanity> results4 = client.Dispatch(&ExampleClient::HasParamsHasReturn, "HasParamsHasReturn", param, insane).get();
```

Server

* define a thrift RPC Server

```c++
typedef ThriftServer<ServerHandler, ExampleProcessor> Server;
```

* run multiple servers to process request from client

```c++
  vector<unique_ptr<Server>> servers(8);
  vector<thread> threads;
  threads.reserve(8);
  int i = 0;
  for (auto& server : servers) {
    server.reset(new Server(++i+2000, 2));
    threads.emplace_back([&server] { server->Start(); });
  }
```


## Design

* Take has response specialization for example:

* Traits the first parameter of the RPC call as the result type, then define a packaged_task type based on the result type

* Wrapper the wait process as a outter task, put it into the wait executor

* Wrapper each RPC call as a inner task inside the wait task, put them into the dispatch executor

* When a RPC call finished, it return the result to the outside task

* The outside task wait for all RPC calls to finish and merges all the results and return them back to the caller

```c++
  // has response
  template <class F, class... Args>
  typename std::enable_if<
    function_tratis<typename std::decay<F>::type>::have_return,
    std::future<std::unordered_map<NetPoint, typename std::decay<
        typename function_tratis<typename std::decay<F>::type>::first_argument_type>::type>>
  >::type Dispatch(F f, Args... args) {
    typedef typename std::decay<typename function_tratis<
        typename std::decay<F>::type>::first_argument_type>::type P;
    typedef std::unordered_map<NetPoint, P> Rs;
    typedef std::packaged_task<Rs()> Task;
    std::shared_ptr<Task> task = std::make_shared<Task>(
        std::bind([this](F f, Args... args) {
      typedef std::pair<bool, P> InnerR;
      std::unordered_map<NetPoint, std::unique_ptr<std::future<InnerR>>> futures;
      futures.reserve(clients_.size());
      for (auto& client : clients_) {
        typedef std::packaged_task<InnerR()> InnerTask;
        std::shared_ptr<InnerTask> inner_task = std::make_shared<InnerTask>(
            std::bind([this, &client](F f, Args... args) {
          P p;
          bool suc = client->Call(std::move(f), p, std::move(args)...);
          return std::make_pair(suc, std::move(p));
        }, f, args...));
        futures.emplace(client->net_point(),
            std::make_unique<std::future<InnerR>>(std::move(inner_task->get_future())));
        dispatch_executor_(std::bind(&InnerTask::operator(), inner_task));
      }
      Rs rs;
      for (auto& client_future : futures) {
        client_future.second->wait();
        InnerR inner_ret(std::move(client_future.second->get()));
        if (inner_ret.first) {
          rs.emplace(client_future.first, std::move(inner_ret.second));
        }
      }
      return std::move(rs);
    }, std::move(f), std::move(args)...));
    std::future<Rs> ret(task->get_future());
    wait_executor_(std::bind(&Task::operator(), task));
    return ret;
  }
```


## Dependings

* cmake

* boost

* thrift (my version 1.0.0-dev)

* C++14 (std::make_unique)


================================
by Xiaojie Chen (swly@live.com)

