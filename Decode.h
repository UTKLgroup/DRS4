/*
  Original version created by:      Stefan Ritt <stefan.ritt@psi.ch>
  Current version developed by:     UTKL HEP group <The University of Texas at Austin>
*/

#ifndef Decode_H
#define Decode_H

#ifndef Event_H
const unsigned int NADC = 1024;
#endif

#include <iostream>
#include <cstring>
#include <array>
#include <sys/stat.h>

#include <TFile.h>
#include <TTree.h>

#include "GSFilter.h"

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
  double Waveform[4][NADC];
  double Time[4][NADC];
};

class Decode {
public:
  Decode(std::string DataFileName, int userNumberOfEvents);
  virtual ~Decode();
  void Run();
  std::string GetROOTFileName();
  void        SaveROOTFile();
  void        SetFilter(unsigned int userNumberOfFilteringPoints, unsigned int userFilteringDegree);
  void        SetSaveRawWaveform();

private:
  void AccessTimeHeader();
  void AccessEventHeader();

  TFile*          ROOTFile;
  FILE*           DataFile;
  std::string     ROOTFileName;

  GSFilter*       WaveformFilter;
  bool            FilterFlag = false;
  bool            SaveRawWaveform = false;
  unsigned int    NumberOfFilteringPoints;
  unsigned int    FilteringDegree;

  unsigned int    NumberOfEvents;
  bool            DecodeAllEventsFlag;
  TIMEHEADER      TimeHeader;
  EVENTHEADER     EventHeader;

  char            Header[4];
  unsigned short  Voltage[NADC];
  double          Waveform[4][NADC];
  double          FilteredWaveform[4][NADC];
  double          FirstDerivativeOfWaveform[4][NADC];
  double          Time[4][NADC];
  float           TimeBinWidth[4][NADC];

  bool            DataFoundInChannel[4] = {false, false, false, false};
};

#endif
