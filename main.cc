#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <vector>

using namespace std;

class Base {
 public:
  void say() { cout << "Base\n"; }
};

template <typename T>
class Proxy {
 public:
  template <typename... Args>
  void foo(Args&&... args) {
    static_cast<T*>(this)->say(std::forward<Args>(args)...);
  }
};

class Derived1 : public Base, public Proxy<Derived1> {
 public:
  void say() { cout << "Derived1\n"; }
};

class Derived2 : public Base, public Proxy<Derived1> {
 public:
  void say(int a) { cout << "Derived2: " << a; }
};

int main() {
  vector<Base*> v;
  v.push_back(new Derived1());
  v.push_back(new Derived2());

  ((Proxy<Derived1>*)v[0])->foo();
  ((Proxy<Derived2*>)v[1])->foo(1);

  return 0;
}