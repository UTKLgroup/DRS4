#ifndef Event_h
#define Event_h

#include <iostream>

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

class Event {
public:

  TTree *fChain;  // !pointer to the analyzed TTree or TChain
  Int_t fCurrent; // !current Tree number in a TChain

  // Declaration of leaf types
  Double_t TimeChannel1[1024];
  Double_t TimeChannel2[1024];
  Double_t TimeChannel3[1024];
  Double_t TimeChannel4[1024];
  Double_t WaveformChannel1[1024];
  Double_t WaveformChannel2[1024];
  Double_t WaveformChannel3[1024];
  Double_t WaveformChannel4[1024];

  // List of branches
  TBranch *b_TimeChannel1;     // !
  TBranch *b_TimeChannel2;     // !
  TBranch *b_TimeChannel3;     // !
  TBranch *b_TimeChannel4;     // !
  TBranch *b_WaveformChannel1; // !
  TBranch *b_WaveformChannel2; // !
  TBranch *b_WaveformChannel3; // !
  TBranch *b_WaveformChannel4; // !

  Event(std::string ROOTFile);
  virtual ~Event();
  virtual bool     Cut(Long64_t entry);
  virtual Int_t    GetEntry(Long64_t entry);
  virtual Long64_t LoadTree(Long64_t entry);
  virtual void     Init(TTree *tree);
  virtual void     Loop();

private:

};

#endif // ifndef Event_h
