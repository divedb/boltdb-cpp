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

int main() {
  int fd = open("/tmp/dnscache.json", O_RDONLY);
  struct stat stat;
  if (fstat(fd, &stat) == -1) {
    cerr << "stat fail";
    return 1;
  }

  cout << "size of tmp: " << stat.st_size << endl;

  return 0;
}