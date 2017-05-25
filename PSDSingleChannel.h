#ifndef PSDSingleChannel_h
#define PSDSingleChannel_h

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

class PSDSingleChannel {
public:

  TTree *fChain;  // !pointer to the analyzed TTree or TChain
  Int_t fCurrent; // !current Tree number in a TChain

  // Declaration of leaf types
  Double_t TimeChannel1[1024];
  Double_t WaveformChannel1[1024];
  Double_t RawWaveformChannel1[1024];
  Double_t DerivativeWaveformChannel1[1024];

  // List of branches
  TBranch *b_TimeChannel1;        // !
  TBranch *b_WaveformChannel1;    // !
  TBranch *b_RawWaveformChannel1; // !
  TBranch *b_DerivativeWaveformChannel1;

  PSDSingleChannel(std::string ROOTFile);
  virtual ~PSDSingleChannel();

  virtual Int_t    GetEntry(Long64_t entry);
  virtual Long64_t LoadTree(Long64_t entry);
  virtual void     Init(TTree *tree);
	virtual void 		 SetValidation(int aNumberOfEventForValidation);
  virtual void     Loop();

private:
	int 		NumberOfEventForValidation;
	TH1D* 	WaveformHistogram;
	TH1D* 	DerivativeHistogram;
	bool 		WaveformCutoff;
	TF1*		WaveformBaselineFit;
	double  WaveformBaselineMean;
	double  WaveformBaselineRMS;

	virtual void 		 FillWaveformHistogram();
	virtual void 		 FillDerivativeHistogram();
	virtual void 		 CheckWaveformCutoff();
	virtual void 		 ClearEvent();
	virtual void 		 WaveformBaselineMeanInVolts();
	virtual void 		 WaveformBaselineRMSInVolts();
	virtual void 		 MakeValidationPlots(int eventID);
};

#endif
