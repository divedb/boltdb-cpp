#include <cstdio>
#include <iostream>

int main() {
  int num = 42;
  char buf[128];

  int res = snprintf(buf, sizeof(buf), "%02x", num);

  printf("buf = %s\n", buf);

  return 0;
}