#include <iostream>
#include <sstream>

#include "Decode.h"
#include "Event.h"

int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::cout << std::endl;
    std::cout << "Please provide a data file as an argument, e.g.: " << std::endl << std::endl;
    std::cout << "./AnalysisChain neutron.dat" << std::endl << std::endl;
    return 0;
  } else {
    Decode* DataSet1 = new Decode(argv[1], 100);
    DataSet1->SetFilter(7, 3);
    DataSet1->Run();
    Event* EventDataSet1 = new Event(DataSet1->GetROOTFileName());
    EventDataSet1->Loop();
  }

  return 0;
}
