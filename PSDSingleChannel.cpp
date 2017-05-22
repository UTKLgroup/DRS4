#define PSDSingleChannel_cxx
#include "PSDSingleChannel.h"

#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

PSDSingleChannel::PSDSingleChannel(std::string ROOTFile) : fChain(0) {
  TTree *tree;

  TFile *f = (TFile *)gROOT->GetListOfFiles()->FindObject(ROOTFile.c_str());
  if (!f || !f->IsOpen()) {
    f = new TFile(ROOTFile.c_str());
  }
  f->GetObject("DataTree", tree);

  Init(tree);
}

PSDSingleChannel::~PSDSingleChannel() {
  if (!fChain) return;
  delete fChain->GetCurrentFile();
}

Int_t PSDSingleChannel::GetEntry(Long64_t entry) {
  if (!fChain) return 0;
  return fChain->GetEntry(entry);
}

Long64_t PSDSingleChannel::LoadTree(Long64_t entry) {
  if (!fChain) return -5;

  Long64_t centry = fChain->LoadTree(entry);
  if (centry < 0) return centry;
  if (fChain->GetTreeNumber() != fCurrent) {
    fCurrent = fChain->GetTreeNumber();
    Notify();
  }
  return centry;
}

void PSDSingleChannel::Init(TTree *tree) {
  if (!tree) return;

  fChain   = tree;
  fCurrent = -1;
  fChain->SetMakeClass(1);
  fChain->SetBranchAddress("TimeChannel1",TimeChannel1,&b_TimeChannel1);
  fChain->SetBranchAddress("WaveformChannel1",WaveformChannel1,&b_WaveformChannel1);
  fChain->SetBranchAddress("RawWaveformChannel1",RawWaveformChannel1,&b_RawWaveformChannel1);
  fChain->SetBranchAddress("DerivativeWaveformChannel1",DerivativeWaveformChannel1,&b_DerivativeWaveformChannel1);

	return;
}

void PSDSingleChannel::Loop() {
  if (fChain == 0) return;
  Long64_t nentries = fChain->GetEntriesFast();
  Long64_t nbytes = 0, nb = 0;
  for (Long64_t jentry = 0; jentry < nentries; jentry++) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;
  }

	return;
}
