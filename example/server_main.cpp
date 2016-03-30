// Copyright 2015, Xiaojie Chen (swly@live.com). All rights reserved.
// https://github.com/vorfeed/thrift_dispatcher
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

#include <csignal>
#include <future>
#include <vector>

#include <thrift_dispatcher/thrift_server.h>
#include "server_handler.h"

static std::promise<void> event;

using namespace std;
using namespace boost;
using namespace td;
using namespace td::test;

int main(int argc, char* argv[]) {
  typedef ThriftServer<ServerHandler, ExampleProcessor> Server;
  vector<unique_ptr<Server>> servers(8);
  vector<thread> threads;
  threads.reserve(8);
  int i = 0;
  for (auto& server : servers) {
    server.reset(new Server(++i+2000, 2));
    threads.emplace_back([&server] { server->Start(); });
  }
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, [](int signal) { event.set_value(); });
  event.get_future().get();
  for (auto& server : servers) {
    server->Stop();
  }
  for (auto& thread : threads) {
    thread.join();
  }
  return 0;
}
