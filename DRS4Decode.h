/*
  Original version created by:      Stefan Ritt <stefan.ritt@psi.ch>
  Current version developed by:     UTKL HEP group <The University of Texas at Austin>
*/

#ifndef DRS4DECODE_H
#define DRS4DECODE_H

#include <iostream>
#include <string>
#include <sys/stat.h>

#include <TFile.h>

class DRS4Decode {
public:
  DRS4Decode();
  DRS4Decode(std::string aDataFileName);
  virtual ~DRS4Decode();

private:
  std::string ROOTFile;
  FILE*       DataFile;

};

#endif
