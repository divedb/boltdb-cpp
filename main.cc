#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <vector>

using namespace std;

int main() {
  int fd = open("a", O_CREAT);

  return 0;
}