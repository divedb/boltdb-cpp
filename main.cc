#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

using namespace std;

// class Base {
//  public:
//   void say() { cout << "Base\n"; }
// };

// template <typename T>
// class Proxy {
//  public:
//   template <typename... Args>
//   void foo(Args&&... args) {
//     static_cast<T*>(this)->say(std::forward<Args>(args)...);
//   }
// };

// class Derived1 : public Base, public Proxy<Derived1> {
//  public:
//   void say() { cout << "Derived1\n"; }
// };

// class Derived2 : public Base, public Proxy<Derived1> {
//  public:
//   void say(int a) { cout << "Derived2: " << a; }
// };

template <typename T, typename U, bool = is_empty_v<T> && !std::is_final_v<T>,
          bool = is_empty_v<U> && !std::is_final_v<U>>
struct CompressedPair;

template <typename T, typename U>
struct CompressedPair<T, U, true, false> : public T {
  CompressedPair(const T& t, const U& u) : T(), u_(u) {}

  T first() const { return T{}; }
  U second() const { return u_; }

  U u_;
};

struct Empty {};
struct NonEmpty {
  NonEmpty(int a) : age(a) {}
  int age;
};

class Person {
 public:
  void foo();
};

int main() {
  function<int()> a;

  cout << is_function_v<decltype(a)> << endl;
  cout << is_function_v<int()> << endl;
  cout << is_function_v<int (*)()> << endl;
  cout << is_function_v<decltype(&Person::foo)> << endl;

  allocator<int()> p;
  p.allocate(10);

  // cout << cp.first() << endl;

  //   shared_ptr<vector<int>> sp1 = make_shared<vector<int>>({1, 2, 3});
  //   shared_ptr<vector<int>> sp2 = sp1;

  //   sp1->push_back(1);
  //   sp1->push_back(2);
  //   sp1->push_back(3);

  //   cout << sp1->size() << endl;
  //   cout << sp2->size() << endl;

  return 0;
}