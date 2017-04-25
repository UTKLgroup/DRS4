#include <iostream>
#include <cmath>
#include <cstring>
#include <vector>
#include <sys/stat.h>

#include "DRS4Decode.h"

int main(int argc, char *argv[]) {
  struct stat statbuf;

  if (argc < 2) {
    std::cout << "Please provide a data file as an argument, e.g.: " << std::endl << std::endl;
    std::cout << "./AnalysisChain datafile1.dat " << std::endl << std::endl;
    return 0;
  } else {
    if (stat(argv[1], &statbuf) == -1) {
      std::cout << "The input data file is not found." << std::endl << std::endl;
      return 0;
    } else {
      DRS4Decode* DataSet1 = new DRS4Decode(argv[1]);
      DataSet1->PrintFileName();
    }
  }

  return 0;
}
