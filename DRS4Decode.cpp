/*
   Original version created by:      Stefan Ritt <stefan.ritt@psi.ch>
   Current version developed by:     UTKL HEP group <The University of Texas at Austin>
 */

#include "DRS4Decode.h"

struct stat statbuf;

DRS4Decode::DRS4Decode() {
}

DRS4Decode::DRS4Decode(std::string aDataFileName) {
  if (stat(aDataFileName.c_str(), &statbuf) == -1) {
    std::cout << "The data file " << aDataFileName.c_str() << " is not found." << std::endl;
  } else {
    DataFile = fopen(aDataFileName.c_str(), "r");
    std::cout << "The data file is found." << std::endl << std::endl;

    std::string::size_type dotPosition;
    std::string::size_type fileExtensionLength = 1;
    dotPosition = aDataFileName.find(".", aDataFileName.length() - fileExtensionLength);
    while (dotPosition == std::string::npos) {
      fileExtensionLength += 1;
      dotPosition = aDataFileName.find(".", aDataFileName.length() - fileExtensionLength);
    }
    aDataFileName.erase(dotPosition, aDataFileName.length());
    aDataFileName = aDataFileName + ".root";
    TFile * ROOTFile = new TFile(aDataFileName.c_str(), "RECREATE");
  }

}

DRS4Decode::~DRS4Decode() {
}
