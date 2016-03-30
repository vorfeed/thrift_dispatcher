// Copyright 2015, Xiaojie Chen (swly@live.com). All rights reserved.
// https://github.com/vorfeed/thrift_dispatcher
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

#pragma once

#include <iostream>

#include "gen-cpp/Example.h"

namespace td {

namespace test {

class ServerHandler : public ExampleIf {
public:
  ~ServerHandler() = default;

  void NoParamsNoReturn() override {
    std::cout << "NoParamsNoReturn" << std::endl;
  }

  void HasParamsNoReturn(const std::string& thing, const Xtruct& xtruct, const Insanity& insanity) override {
    std::cout << "HasParamsNoReturn: " << thing << std::endl;
  }

  void NoParamsHasReturn(Xtruct& _return) override {
    std::cout << "NoParamsHasReturn" << std::endl;
    _return.string_thing = "param";
  }

  void HasParamsHasReturn(Insanity& _return, const std::string& thing, const Xtruct& xtruct, const Insanity& insanity) override {
    std::cout << "HasParamsHasReturn: " << thing << std::endl;
    _return.userMap.insert(insanity.userMap.begin(), insanity.userMap.end());
    _return.xtructs.insert(_return.xtructs.end(), insanity.xtructs.begin(), insanity.xtructs.end());
  }
};

}

}
