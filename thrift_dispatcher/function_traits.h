// Copyright 2015, Xiaojie Chen (swly@live.com). All rights reserved.
// https://github.com/vorfeed/thrift_dispatcher
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

#pragma once

#include <functional>
#include <tuple>

namespace td {

template <class> class function_tratis;

template <class T>
class function_tratis<T*> : function_tratis<T> {};

template <typename F>
struct function_tratis : function_tratis<decltype(&F::operator())> {};

template <class R, class... Args>
struct function_tratis<R(*)(Args...)> {
  typedef R result_type;
  typedef typename std::tuple_element<0, std::tuple<Args...>>::type first_argument_type;
  static constexpr bool have_return =
      std::is_lvalue_reference<first_argument_type>::value &&
      !std::is_const<typename std::remove_reference<first_argument_type>::type>::value;
};

template <class R>
struct function_tratis<R(*)(void)> {
  typedef R result_type;
  typedef void first_argument_type;
  static constexpr bool have_return = false;
};

template <class R, class C, class... Args>
struct function_tratis<R(C::*)(Args...)> {
  typedef R result_type;
  typedef typename std::tuple_element<0, std::tuple<Args...>>::type first_argument_type;
  static constexpr bool have_return =
      std::is_lvalue_reference<first_argument_type>::value &&
      !std::is_const<typename std::remove_reference<first_argument_type>::type>::value;
};

template <class R, class C>
struct function_tratis<R(C::*)(void)> {
  typedef R result_type;
  typedef void first_argument_type;
  static constexpr bool have_return = false;
};

}
