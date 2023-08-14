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
  int res = flock(fd, LOCK_SH);

  cout << "res = " << res << endl;

  res = flock(fd, LOCK_EX);
  cout << "res = " << res << endl;

  res = flock(fd, LOCK_EX);
  cout << "res = " << res << endl;

  return 0;
}