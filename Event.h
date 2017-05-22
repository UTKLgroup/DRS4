#ifndef Event_h
#define Event_h

#include "Decode.h"

#include <iostream>
#include <sstream>
#include <sys/stat.h>

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TH2.h>
#include <TStyle.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TCanvas.h>
#include <TString.h>
#include <TLine.h>
#include <TMath.h>

struct CHANNEL {
  std::string TimeBranchName;
  std::string WaveformBranchName;
  std::string RawWaveformBranchName;
  std::string DerivativeWaveformBranchName;
  TBranch*    b_TimeChannel;
  TBranch*    b_WaveformChannel;
  TBranch*    b_RawWaveformChannel;
  TBranch*    b_DerivativeWaveformChannel;
  Double_t    Time[NADC];
  Double_t    Waveform[NADC];
  Double_t    RawWaveform[NADC];
  Double_t    DerivativeWaveform[NADC];
  TH1D*       hVoltageSampleHistogram;
  double      Baseline;
  double      BaselineRMS;
	double 			WaveformArea;
	double 			WaveformTailArea;
	double 			ChargeRatio;
  unsigned int      LocationOfMinimumDerivative;
  unsigned int      LocationOfMaximumDerivative;
  unsigned int      LocationOfWaveformPeak;
  double            TimeOfWaveformPeak;
  unsigned int      LocationOfVoltageDrop;
  unsigned int      LocationOfVoltageRecover;
  bool*             WaveformCutoff;
};

class Event {
public:
  Event(std::string ROOTFile);
  virtual ~Event();
  virtual Long64_t LoadTree(Long64_t entry);
  virtual void     Init(TTree *tree);
  virtual void     Run();

  // Validation/Visualization
  virtual void     SetValidation(unsigned int userNumberOfEventForValidation);

private:
  bool            DataFoundInChannel[4] = {false, false, false, false};
  bool            RawWaveformExistFlag = false;
  bool            ValidationFlag = false;
  unsigned int    NumberOfEventForValidation;

  int maxNADCToBeConsiderCutoff = (int)(NADC / 100) + 1;

  virtual void    EndOfRun();

  virtual void    InitiateChannelContainer();
  virtual void    InitiateBranches(unsigned int ChannelID);
  TTree *fChain;
  Int_t fCurrent;
  Long64_t TotalNumberOfEvents;
  std::vector<CHANNEL>  ChannelContainer;
  CHANNEL*        GetChannel(unsigned int ChannelID);

  Long64_t         CurrentEventIndex;

  virtual void     CheckWaveformCutoff();

  // Calculate high-level variables
  virtual void     FindBaselineInfo();
  virtual void     FillVoltageSampleHistogram(); // to find baseline
  virtual void     FindBaseline();
  virtual void     FindBaselineRMS();

  virtual void     FindTimeInfo();
  virtual void     FindLocationOfWeakformPeak();
  virtual void     FindLocationOfVoltageDrop();
  virtual void     FindLocationOfVoltageRecover();

	virtual void 		 FindChargeInfo();
	virtual void 		 FindWaveformArea();
	virtual void 		 FindWaveformTailArea();
	virtual void 		 FindChargeRatio();

  virtual void          MakeValidationPlots();
  virtual TMultiGraph*  DrawFilterValidationPlots(unsigned int ChannelID);
  virtual TH1D*         DrawVoltageSampleHistogram(unsigned int ChannelID);
};

#endif // ifndef Event_h
