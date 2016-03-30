// Copyright 2015, Xiaojie Chen (swly@live.com). All rights reserved.
// https://github.com/vorfeed/thrift_dispatcher
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

#include <thrift_dispatcher/thrift_clients.h>
#include <thrift_dispatcher/executors.h>
#include "gen-cpp/Example.h"

using namespace std;
using namespace boost;
using namespace td;
using namespace td::test;

typedef ThriftClients<ExampleClient, Executors, Executors> DispatchClient;

template <class T>
static bool SameServers(const string& method, const unordered_set<NetPoint>& lhs,
    const T& rhs) {
  if (lhs.size() != rhs.size()) {
    return false;
  }
  for (const NetPoint& address : lhs) {
    if (!rhs.count(address)) {
      cerr << method << " test failed: not all address matched!" << endl;
      return false;
    }
  }
  return true;
}

template <class T>
static bool SameValues(const string& method, const T& value,
    const unordered_map<NetPoint, T>& result) {
  for (const auto& server_value : result) {
    if (value != server_value.second) {
      cerr << method << " test failed: not all values matched!" << endl;
      return false;
    }
  }
  return true;
}

static bool CheckResult(const string& method, const unordered_set<NetPoint>& addresses,
    const unordered_set<NetPoint>& result) {
  if (!SameServers(method, addresses, result)) {
    return false;
  }
  cout << method << " test success." << endl;
  return true;
}

template <class T>
static bool CheckResult(const string& method, const unordered_set<NetPoint>& addresses,
    const unordered_map<NetPoint, T>& result, const T& value) {
  if (!SameServers(method, addresses, result)) {
    return false;
  }
  if (!SameValues(method, value, result)) {
    return false;
  }
  cout << method << " test success." << endl;
  return true;
}

static bool operator==(const Xtruct& lhs, const Xtruct& rhs) {
  return lhs.string_thing == rhs.string_thing;
}

static bool operator==(const Insanity& lhs, const Insanity& rhs) {
  return lhs.userMap == rhs.userMap &&
      lhs.xtructs == rhs.xtructs;
}

int main(int argc, char* argv[]) {
  unordered_set<NetPoint> addresses {
    { "127.0.0.1", 2001 },
    { "127.0.0.1", 2002 },
    { "127.0.0.1", 2003 },
    { "127.0.0.1", 2004 },
    { "127.0.0.1", 2005 },
    { "127.0.0.1", 2006 },
    { "127.0.0.1", 2007 },
    { "127.0.0.1", 2008 },
  };
  Executors dispatcher, waiter;
  DispatchClient client(dispatcher, waiter, addresses);
  dispatcher.Start();
  waiter.Start();
  Xtruct param;
  param.string_thing = "param";
  Insanity insane;
  insane.userMap.insert(make_pair(Numberz::TWO, 5));
  Xtruct inner;
  inner.string_thing = "inner";
  insane.xtructs.push_back(inner);
  CheckResult("NoParamsNoReturn", addresses, client.Dispatch(&ExampleClient::NoParamsNoReturn).get());
  CheckResult("HasParamsNoReturn", addresses, client.Dispatch(&ExampleClient::HasParamsNoReturn, "HasParamsNoReturn", param, insane).get());
  CheckResult("NoParamsHasReturn", addresses, client.Dispatch(&ExampleClient::NoParamsHasReturn).get(), param);
  CheckResult("HasParamsHasReturn", addresses, client.Dispatch(&ExampleClient::HasParamsHasReturn, "HasParamsHasReturn", param, insane).get(), insane);
  waiter.Stop();
  dispatcher.Stop();
}
