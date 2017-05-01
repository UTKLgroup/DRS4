#ifndef Event_h
#define Event_h

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

struct CHANNEL {
  std::string TimeBranchName;
  std::string WaveformBranchName;
  std::string RawWaveformBranchName;
  std::string DerivativeWaveformBranchName;
  TBranch*    b_TimeChannel;
  TBranch*    b_WaveformChannel;
  TBranch*    b_RawWaveformChannel;
  TBranch*    b_DerivativeWaveformChannel;
  Double_t    Time[1024];
  Double_t    Waveform[1024];
  Double_t    RawWaveform[1024];
  Double_t    DerivativeWaveform[1024];
  TH1D*       hVoltageSampleHistogram;
  double      Baseline;
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

  virtual void    EndOfRun();

  virtual void    InitiateChannelContainer();
  virtual void    InitiateBranches(unsigned int ChannelID);
  TTree *fChain;
  Int_t fCurrent;
  Long64_t TotalNumberOfEvents;
  std::vector<CHANNEL>  ChannelContainer;
  CHANNEL*        GetChannel(unsigned int ChannelID);

  Long64_t         CurrentEventIndex;

  // Calculate high-level variables
  virtual void     FillVoltageSampleHistogram();
  //virtual void     FindBaseline();

  virtual void          MakeValidationPlots();
  virtual TMultiGraph*  DrawFilterValidationPlots(unsigned int ChannelID);
  virtual TH1D*         DrawVoltageSampleHistogram(unsigned int ChannelID);
};

#endif // ifndef Event_h
