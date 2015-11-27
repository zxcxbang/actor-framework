/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2015                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#ifndef CAF_DETAIL_DEEP_TO_STRING_HPP
#define CAF_DETAIL_DEEP_TO_STRING_HPP

#include <array>
#include <tuple>
#include <string>
#include <utility>
#include <type_traits>

#include "caf/atom.hpp"
#include "caf/detail/apply_args.hpp"
#include "caf/detail/type_traits.hpp"

namespace caf {

class deep_to_string_t {
public:
  constexpr deep_to_string_t() {
    // nop
  }

  inline std::string operator()(const char* cstr) const {
    if (cstr == nullptr || *cstr == '\0')
      return "\"\"";
    std::string result = "\"";
    char c;
    while ((c = *cstr++) != '\0') {
      switch (c) {
        case '\\':
          result += "\\\\";
          break;
        case '"':
          result += "\\\"";
          break;
        default:
          result += c;
      }
    }
    result += "\"";
    return result;
  }

  inline std::string operator()(const std::string& str) const {
    return (*this)(str.c_str());
  }

  inline std::string operator()(const atom_value& x) const {
    std::string result = "'";
    result += to_string(x);
    result += "'";
    return result;
  }

  constexpr const char* operator()(const bool& x) const {
    return x ? "true" : "false";
  }

  template <class T>
  typename std::enable_if<
    ! detail::is_iterable<T>::value,
    std::string
  >::type
  operator()(const T& x) const {
    std::string res = impl(&x);
    if (res == "<unprintable>") {
      res = "<unprintable:";
      res += typeid(T).name();
      res += ">";
    }
    return res;
  }

  template <class F, class S>
  std::string operator()(const std::pair<F, S>& x) const {
    std::string result = "(";
    result += (*this)(x.first);
    result += ", ";
    result += (*this)(x.second);
    result += ")";
    return result;
  }

  template <class T, class U, class... Us>
  std::string operator()(const T& x, const U& y, const Us&... ys) const {
    std::string result = (*this)(x);
    result += ", ";
    result += (*this)(y, ys...);
    return result;
  }

  template <class... Ts>
  std::string operator()(const std::tuple<Ts...>& xs) const {
    std::string result = "(";
    result += detail::apply_args(*this, detail::get_indices(xs), xs);
    result += ")";
    return result;
  }

  template <class T>
  typename std::enable_if<
    detail::is_iterable<T>::value,
    std::string
  >::type
  operator()(const T& x) const {
    auto i = x.begin();
    auto e = x.end();
    if (i == e)
      return "[]";
    std::string result = "[";
    result += (*this)(*i);
    for (++i; i != e; ++i) {
      result += ", ";
      result += (*this)(*i);
    }
    result += "]";
    return result;
  }

  template <class T>
  std::string decompose_array(const T& xs, size_t n) const {
    if (n == 0)
      return "()";
    std::string result = "(";
    result += (*this)(xs[0]);
    for (size_t i = 1; i < n; ++i) {
      result += ", ";
      result += (*this)(xs[i]);
    }
    result += ")";
    return result;
  }

  template <class T, size_t S>
  std::string operator()(const std::array<T, S>& xs) const {
    return decompose_array(xs, S);
  }

  template <class T, size_t S>
  std::string operator()(const T (&xs)[S]) const {
    return decompose_array(xs, S);
  }

private:
  template <class T>
  auto impl(const T* x) const -> decltype(to_string(*x)) {
    return to_string(*x);
  }

  template <class T>
  typename std::enable_if<
    std::is_arithmetic<T>::value,
    std::string
  >::type
  impl(const T* x) const {
    return std::to_string(*x);
  }

  constexpr const char* impl(const void*) const {
    return "<unprintable>";
  }
};

/// Unrolls collections such as vectors/maps, decomposes
/// tuples/pairs/arrays, auto-escapes strings and calls
/// `to_string` for user-defined types via argument-dependent
/// loopkup (ADL). Any user-defined type that does not
/// provide a `to_string` is mapped to `<unprintable>`.
constexpr deep_to_string_t deep_to_string = deep_to_string_t{};

} // namespace caf

#endif // CAF_DETAIL_DEEP_TO_STRING_HPP