#include <iostream>
#include <sstream>
#include <fstream>

#include "../GSFilter.h"


int main(int argc, char* argv[]) {

  std::stringstream ArgumentParsing1(argv[1]);
  int NumberOfFilteringPoints;
  ArgumentParsing1 >> NumberOfFilteringPoints;

  std::stringstream ArgumentParsing2(argv[2]);
  int FilteringDegree;
  ArgumentParsing2 >> FilteringDegree;

  GSFilter* aFilter = new GSFilter(NumberOfFilteringPoints, FilteringDegree);
  aFilter->PrintMatrices();

  return 0;
}
