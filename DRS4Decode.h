/*
  Original version created by:      Stefan Ritt <stefan.ritt@psi.ch>
  Current version developed by:     UTKL HEP group <The University of Texas at Austin>
*/

#ifndef DRS4DECODE_H
#define DRS4DECODE_H

#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <cstring>

class DRS4Decode {
public:
  DRS4Decode();
  DRS4Decode(std::string aDataFileName);
  virtual ~DRS4Decode();
  void PrintFileName();

private:
  std::string DataFileName;
  std::string ROOTFileName;
};

#endif
