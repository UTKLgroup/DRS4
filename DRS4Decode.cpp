/*
   Original version created by:      Stefan Ritt <stefan.ritt@psi.ch>
   Current version developed by:     UTKL HEP group <The University of Texas at Austin>
 */

#include "DRS4Decode.h"

DRS4Decode::DRS4Decode() {
}

DRS4Decode::DRS4Decode(std::string aDataFileName) {
  DataFileName = aDataFileName;

  std::string::size_type dotPosition;
  std::string::size_type fileExtensionLength = 1;
  dotPosition = aDataFileName.find(".", aDataFileName.length() - fileExtensionLength);
  while (dotPosition == std::string::npos) {
    fileExtensionLength += 1;
    dotPosition = aDataFileName.find(".", aDataFileName.length() - fileExtensionLength);
  }
  aDataFileName.erase(dotPosition, aDataFileName.length());
  ROOTFileName = aDataFileName + ".root";
}

DRS4Decode::~DRS4Decode() {
}

void DRS4Decode::PrintFileName() {
  std::cout << this->DataFileName << std::endl;
  std::cout << this->ROOTFileName << std::endl;

  return;
}
