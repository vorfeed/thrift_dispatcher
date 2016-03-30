// Copyright 2015, Xiaojie Chen (swly@live.com). All rights reserved.
// https://github.com/vorfeed/thrift_dispatcher
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

#pragma once

#include <functional>
#include <iostream>
#include <memory>

#include <boost/make_shared.hpp>
#include <google/protobuf/message.h>
#include <thrift/transport/TTransport.h>                // TTransport, TTransportException
#include <thrift/transport/TSocket.h>                   // TSocket
#include <thrift/transport/TBufferTransports.h>         // TBufferedTransport
#include <thrift/protocol/TProtocol.h>                  // TProtocol
#include <thrift/protocol/TBinaryProtocol.h>            // TBinaryProtocol
#include <thrift/TApplicationException.h>               // TApplicationException

#include "net_point.h"

namespace td {

using google::protobuf::Message;
using apache::thrift::transport::TTransport;
using apache::thrift::transport::TSocket;
using apache::thrift::transport::TBufferedTransport;
using apache::thrift::transport::TTransportException;
using apache::thrift::protocol::TProtocol;
using apache::thrift::protocol::TBinaryProtocol;
using apache::thrift::TApplicationException;
using apache::thrift::TException;

template <class RPCClient>
class ThriftClient : public RPCClient {
 public:
  ThriftClient(const std::string& host, int port, int timeout_ms = 10) :
    RPCClient(boost::make_shared<TBinaryProtocol>(
        boost::make_shared<TBufferedTransport>(
            boost::make_shared<TSocket>(host, port)))),
    host_(host), port_(port), timeout_ms_(timeout_ms) {
    boost::shared_ptr<TSocket> s(socket());
    s->setConnTimeout(timeout_ms_);
    s->setSendTimeout(timeout_ms_);
    s->setRecvTimeout(timeout_ms_);
  }

  ~ThriftClient() {
    Close();
  }

  bool Open() {
    boost::shared_ptr<TBufferedTransport> trans(transport());
    assert(trans);
    if (trans->isOpen()) {
      return true;
    }
    try {
      trans->open();
    } catch (const TTransportException& te) {
      std::cerr << "connect to server(" << host_ << ":" << port_ <<
          ") failed: " << te.what() << std::endl;
      return false;
    }
    return true;
  }

  void Close() {
    if (transport()->isOpen()) {
      try {
        transport()->close();
      } catch (const TTransportException& te) {}
    }
  }

  bool Connected() {
    return transport()->isOpen();
  }

  template <class F, class... Args>
  bool Call(F&& f, Args&&... args) {
    std::function<void(ThriftClient*,Args...)> func(std::forward<F>(f));
    int retry = 2;
    while (retry--) {
      if (!Connected() && !Open()) {
        return false;
      }
      try {
        func(this, std::forward<Args>(args)...);
        break;
      } catch (const TException& te) {
        Close();
        if (retry) {
          std::cerr << "rpc error: " << te.what() << ", retry...." << std::endl;
          continue;
        }
        std::cerr << "rpc error: " << te.what() << ", give up!" << std::endl;
        return false;
      }
    }
    return true;
  }

  NetPoint net_point() const {
    return NetPoint(host_, port_);
  }

 private:
  boost::shared_ptr<TBinaryProtocol> protocol() {
    return boost::static_pointer_cast<TBinaryProtocol, TProtocol>(RPCClient::getInputProtocol());
  }

  boost::shared_ptr<TBufferedTransport> transport() {
    return boost::static_pointer_cast<TBufferedTransport, TTransport>(protocol()->getTransport());
  }

  boost::shared_ptr<TSocket> socket() {
    return boost::static_pointer_cast<TSocket, TTransport>(transport()->getUnderlyingTransport());
  }

  std::string host_;
  int port_;
  int timeout_ms_;
};

}
