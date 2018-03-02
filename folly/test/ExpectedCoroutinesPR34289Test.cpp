/*
 * Copyright 2017-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <folly/Expected.h>
#include <folly/Portability.h>
#include <folly/ScopeGuard.h>
#include <folly/portability/GTest.h>

using namespace folly;

namespace {

struct Exn {};

// not default-constructible, thereby preventing Expected<T, Err> from being
// default-constructible, forcing our implementation to handle such cases
class Err {
 private:
  enum class Type { Bad, Badder, Baddest };

  Type type_;

  constexpr Err(Type type) : type_(type) {}

 public:
  Err(Err const&) = default;
  Err(Err&&) = default;
  Err& operator=(Err const&) = default;
  Err& operator=(Err&&) = default;

  friend bool operator==(Err a, Err b) {
    return a.type_ == b.type_;
  }
  friend bool operator!=(Err a, Err b) {
    return a.type_ != b.type_;
  }

  static constexpr Err bad() {
    return Type::Bad;
  }
  static constexpr Err badder() {
    return Type::Badder;
  }
  static constexpr Err baddest() {
    return Type::Baddest;
  }
};

Expected<int, Err> f1() {
  return 7;
}

Expected<double, Err> f2(int x) {
  return 2.0 * x;
}

// move-only type
Expected<std::unique_ptr<int>, Err> f3(int x, double y) {
  return std::make_unique<int>(int(x + y));
}

} // namespace

#if FOLLY_HAS_COROUTINES

TEST(Expected, CoroutineSuccess) {
  auto r0 = []() -> Expected<int, Err> {
    auto x = co_await f1();
    EXPECT_EQ(7, x);
    auto y = co_await f2(x);
    EXPECT_EQ(2.0 * 7, y);
    auto z = co_await f3(x, y);
    EXPECT_EQ(int(2.0 * 7 + 7), *z);
    co_return* z;
  }();
  EXPECT_TRUE(r0.hasValue());
  EXPECT_EQ(21, *r0);
}

#endif