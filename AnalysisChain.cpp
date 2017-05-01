#include <iostream>
#include <sstream>

#include "Decode.h"
#include "Event.h"

int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::cout << std::endl;
    std::cout << "Please provide arguments as: " << std::endl << std::endl;
    std::cout << "\t./AnalysisChain <Raw data file> <Number of events>" << std::endl << std::endl;
    std::cout << "Example: " << std::endl << std::endl;
    std::cout << "\t./AnalysisChain neutron.dat 100" << std::endl << std::endl;
    return 0;
  } else {
    std::stringstream ss(argv[2]);
    int NumberOfEvents;
    ss >> NumberOfEvents;
    Decode* DataSet1 = new Decode(argv[1], NumberOfEvents);
    DataSet1->SetFilter(31, 5);
    DataSet1->SetSaveRawWaveform();
    DataSet1->Run();
    Event* EventDataSet1 = new Event(DataSet1->GetROOTFileName());
    EventDataSet1->SetValidation(10);
    EventDataSet1->Run();
  }

  return 0;
}
