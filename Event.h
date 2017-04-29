#ifndef Event_h
#define Event_h

#include <iostream>
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

class Event {
public:
  Event(std::string ROOTFile);
  virtual ~Event();
  virtual bool     Cut(Long64_t entry);
  virtual Int_t    GetEntry(Long64_t entry);
  virtual Long64_t LoadTree(Long64_t entry);
  virtual void     Init(TTree *tree);
  virtual void     Run();

  // Validation/Visualization
  virtual void     SetFilterValidation(unsigned int userNumberOfEventForFilterValidation);

private:
  bool            DataFoundInChannel[4] = {false, false, false, false};
  bool            RawWaveformExistFlag = false;
  bool            FilterValidationFlag = false;
  unsigned int    NumberOfEventForFilterValidation = 0;

  TTree *fChain;  // !pointer to the analyzed TTree or TChain
  Int_t fCurrent; // !current Tree number in a TChain
  Long64_t TotalNumberOfEvents;

  // Declaration of leaf types
  Double_t TimeChannel1[1024];
  Double_t TimeChannel2[1024];
  Double_t TimeChannel3[1024];
  Double_t TimeChannel4[1024];
  Double_t WaveformChannel1[1024];
  Double_t WaveformChannel2[1024];
  Double_t WaveformChannel3[1024];
  Double_t WaveformChannel4[1024];
  Double_t RawWaveformChannel1[1024];
  Double_t RawWaveformChannel2[1024];
  Double_t RawWaveformChannel3[1024];
  Double_t RawWaveformChannel4[1024];
  Double_t DerivativeWaveformChannel1[1024];
  Double_t DerivativeWaveformChannel2[1024];
  Double_t DerivativeWaveformChannel3[1024];
  Double_t DerivativeWaveformChannel4[1024];

  // List of branches
  TBranch *b_TimeChannel1;     // !
  TBranch *b_TimeChannel2;     // !
  TBranch *b_TimeChannel3;     // !
  TBranch *b_TimeChannel4;     // !
  TBranch *b_WaveformChannel1; // !
  TBranch *b_WaveformChannel2; // !
  TBranch *b_WaveformChannel3; // !
  TBranch *b_WaveformChannel4; // !
  TBranch *b_RawWaveformChannel1; // !
  TBranch *b_RawWaveformChannel2; // !
  TBranch *b_RawWaveformChannel3; // !
  TBranch *b_RawWaveformChannel4; // !
  TBranch *b_DerivativeWaveformChannel1; // !
  TBranch *b_DerivativeWaveformChannel2; // !
  TBranch *b_DerivativeWaveformChannel3; // !
  TBranch *b_DerivativeWaveformChannel4; // !

  Long64_t         CurrentEventIndex;
  Double_t*        GetTimeChannel(unsigned int ChannelID);
  Double_t*        GetWaveformChannel(unsigned int ChannelID);
  Double_t*        GetRawWaveformChannel(unsigned int ChannelID);
  Double_t*        GetDerivativeWaveformChannel(unsigned int ChannelID);

  // Data quality control
  // These functions will be used in Cut()
  //virtual bool     RogueWaveformCheck();
  //virtual bool     CutoffWaveformCheck();

  virtual void     MakeFilterValidationPlots();
  virtual void     DrawFilterValidationPlotsForChannel(unsigned int ChannelID);
};

#endif // ifndef Event_h
