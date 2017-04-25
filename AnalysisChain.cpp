#include <iostream>
#include <sstream>

#include "Decode.h"

int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::cout << std::endl;
    std::cout << "Please provide a data file as an argument, e.g.: " << std::endl << std::endl;
    std::cout << "./AnalysisChain neutron.dat " << std::endl << std::endl;
    return 0;
  } else {
    std::stringstream ss(argv[2]);
    int NumberOfEvents;
    ss >> NumberOfEvents;
    Decode* DataSet1 = new Decode(argv[1], NumberOfEvents);
    DataSet1->Decode();
    std::string ROOTFile = DataSet1->GetROOTFileName();
    Event* EventDataSet1 = new Event()
  }

  return 0;
}
