// Copyright 2015, Xiaojie Chen (swly@live.com). All rights reserved.
// https://github.com/vorfeed/thrift_dispatcher
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

#pragma once

#include <functional>
#include <string>

namespace td {

struct NetPoint {
  std::string host;
  uint16_t port;

  NetPoint() : port(0) {}
  NetPoint(const std::string& host, uint16_t port) : host(host), port(port) {}

  std::string str() const {
    return host + ":" + std::to_string(port);
  }

  friend bool operator==(const NetPoint& lhs, const NetPoint& rhs) {
    return lhs.host == rhs.host && lhs.port == rhs.port;
  }

  friend bool operator<(const NetPoint& lhs, const NetPoint& rhs) {
    return lhs.host == rhs.host ? lhs.port < rhs.port : lhs.host < rhs.host;
  }
};

}

namespace std {

template<>
struct hash<td::NetPoint> {
  typedef td::NetPoint argument_type;
  typedef std::size_t result_type;
  result_type operator()(const argument_type& net_point) const {
    return std::hash<std::string>()(net_point.str());
  }
};

}
