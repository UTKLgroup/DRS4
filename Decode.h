/*
  Original version created by:      Stefan Ritt <stefan.ritt@psi.ch>
  Current version developed by:     UTKL HEP group <The University of Texas at Austin>
*/

#ifndef Decode_H
#define Decode_H

#include <iostream>
#include <cstring>
#include <array>
#include <sys/stat.h>

#include <TFile.h>
#include <TTree.h>

struct TIMEHEADER {
  char           TimeHeaderLabel[4];
  char           BoardSerialLabel[2];
  unsigned short BoardSerialNumber;
};

struct EVENTHEADER {
  char           EventHeaderLabel[4];
  unsigned int   EventSerialNumberValue;
  unsigned short YearValue;
  unsigned short MonthValue;
  unsigned short DayValue;
  unsigned short HourValue;
  unsigned short MinuteValue;
  unsigned short SecondValue;
  unsigned short MillisecondValue;
  unsigned short ReservedCharValue;
  char           BoardSerialNumberLabel[2];
  unsigned short BoardSerialNumberValue;
  char           TriggerCellLabel[2];
  unsigned short TriggerCellValue;
};

struct WAVEFORM {
  double Waveform[4][1024];
  double Time[4][1024];
};

class Decode {
public:
  Decode(std::string DataFileName, int userNumberOfEvents);
  virtual ~Decode();
  void Run();
  std::string GetROOTFileName();

private:
  void AccessTimeHeader();
  void AccessEventHeader();

  TFile*          ROOTFile;
  FILE*           DataFile;
  std::string     ROOTFileName;

  unsigned int    NumberOfEvents;
  bool            DecodeAllEventsFlag;
  TIMEHEADER      TimeHeader;
  EVENTHEADER     EventHeader;

  char            Header[4];
  unsigned short  Voltage[1024];
  double          Waveform[4][1024];
  double          Time[4][1024];
  float           TimeBinWidth[4][1024];
};

#endif