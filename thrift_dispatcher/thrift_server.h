// Copyright 2015, Xiaojie Chen (swly@live.com). All rights reserved.
// https://github.com/vorfeed/thrift_dispatcher
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

#pragma once

#include <memory>
#include <thread>

#include <boost/make_shared.hpp>
#include <thrift/TProcessor.h>                          // TProcessor
#include <thrift/transport/TServerSocket.h>             // TServerSocket
#include <thrift/transport/TBufferTransports.h>         // TBufferedTransportFactory, TBufferBase
#include <thrift/transport/TTransport.h>                // TTransportFactory
#include <thrift/protocol/TBinaryProtocol.h>            // TBinaryProtocolFactoryT
#include <thrift/protocol/TProtocol.h>                  // TProtocolFactory
#include <thrift/concurrency/PlatformThreadFactory.h>   // PlatformThreadFactory
#include <thrift/concurrency/ThreadManager.h>           // ThreadManager
#include <thrift/server/TThreadPoolServer.h>            // TThreadPoolServer

namespace td {

using apache::thrift::TProcessor;
using apache::thrift::transport::TServerSocket;
using apache::thrift::transport::TBufferedTransportFactory;
using apache::thrift::transport::TTransportFactory;
using apache::thrift::transport::TBufferBase;
using apache::thrift::protocol::TBinaryProtocolFactoryT;
using apache::thrift::protocol::TProtocolFactory;
using apache::thrift::concurrency::PlatformThreadFactory;
using apache::thrift::concurrency::ThreadManager;
using apache::thrift::server::TThreadPoolServer;

template <class Handler, class Processor>
class ThriftServer {
 public:
  ThriftServer(int port, size_t thread_num) : port_(port), thread_num_(thread_num) {}

  void Start() {
    boost::shared_ptr<ThreadManager> thread_manager(ThreadManager::newSimpleThreadManager(thread_num_));
    thread_manager->threadFactory(boost::make_shared<PlatformThreadFactory>());
    thread_manager->start();
    thread_pool_server_.reset(new TThreadPoolServer(
        boost::make_shared<Processor>(boost::make_shared<Handler>()),
        boost::make_shared<TServerSocket>(port_),
        boost::make_shared<TBufferedTransportFactory>(),
        boost::make_shared<TBinaryProtocolFactoryT<TBufferBase>>(),
        thread_manager));
    thread_pool_server_->serve();
  }

  void Stop() {
    if (thread_pool_server_) {
      thread_pool_server_->stop();
    }
  }

 private:
  int port_;
  size_t thread_num_;
  std::unique_ptr<TThreadPoolServer> thread_pool_server_;
};

}
