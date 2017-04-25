#include <iostream>

#include "DRS4Decode.h"

int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::cout << "Please provide a data file as an argument, e.g.: " << std::endl << std::endl;
    std::cout << "./AnalysisChain neutron.dat " << std::endl << std::endl;
    return 0;
  } else {
    DRS4Decode* DataSet1 = new DRS4Decode(argv[1]);
  }

  return 0;
}
