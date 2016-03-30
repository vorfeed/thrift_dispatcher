// Copyright 2015, Xiaojie Chen (swly@live.com). All rights reserved.
// https://github.com/vorfeed/thrift_dispatcher
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

#pragma once

#include <future>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "function_traits.h"
#include "thrift_client.h"

namespace td {

// since C++14
//template <class T, class... Args>
//inline std::unique_ptr<T> make_unique(Args&&... args) {
//  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
//}

template <class RPCClient, class DispatchExecutor, class WaitExecutor>
class ThriftClients {
 public:
  ThriftClients(DispatchExecutor& dispatch_executor, WaitExecutor& wait_executor,
      const std::unordered_set<NetPoint>& servers, int timeout_ms = 10) :
        dispatch_executor_(dispatch_executor), wait_executor_(wait_executor),
        timeout_ms_(timeout_ms) {
    clients_.reserve(servers.size());
    for (const NetPoint& net_point : servers) {
      clients_.emplace_back(std::make_unique<ThriftClient<RPCClient>>(
          net_point.host, net_point.port, timeout_ms_));
    }
  }

  // no response
  template <class F, class... Args>
  typename std::enable_if<
    !function_tratis<typename std::decay<F>::type>::have_return,
    std::future<std::unordered_set<NetPoint>>
  >::type Dispatch(F f, Args... args) {
    typedef std::unordered_set<NetPoint> Rs;
    typedef std::packaged_task<Rs()> Task;
    // [args...] https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55914
    std::shared_ptr<Task> task = std::make_shared<Task>(
        std::bind([this](F f, Args... args) {
      typedef std::packaged_task<bool()> InnerTask;
      std::unordered_map<NetPoint, std::unique_ptr<std::future<bool>>> futures;
      futures.reserve(clients_.size());
      for (auto& client : clients_) {
        std::shared_ptr<InnerTask> inner_task = std::make_shared<InnerTask>(
            std::bind([this, &client](F f, Args... args) {
          return client->Call(std::move(f), std::move(args)...);
        }, f, args...));
        futures.emplace(client->net_point(),
            std::make_unique<std::future<bool>>(std::move(inner_task->get_future())));
        dispatch_executor_(std::bind(&InnerTask::operator(), inner_task));
      }
      Rs rs;
      for (auto& client_future : futures) {
        client_future.second->wait();
        if (client_future.second->get()) {
          rs.emplace(client_future.first);
        }
      }
      return std::move(rs);
    }, std::move(f), std::move(args)...));
    std::future<Rs> ret(task->get_future());
    wait_executor_(std::bind(&Task::operator(), task));
    return ret;
  }

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

 private:
  DispatchExecutor& dispatch_executor_;
  WaitExecutor& wait_executor_;
  int timeout_ms_;
  std::vector<std::unique_ptr<ThriftClient<RPCClient>>> clients_;
};

}
