#include "Event.h"

Event::Event(std::string ROOTFile) : fChain(0) {
  TTree *tree;
  TFile *f = (TFile *)gROOT->GetListOfFiles()->FindObject(ROOTFile.c_str());
  if (!f || !f->IsOpen()) {
    f = new TFile(ROOTFile.c_str());
  }
  f->GetObject("DataTree", tree);

  Init(tree);
}

Event::~Event() {
  if (!fChain) return;
  delete fChain->GetCurrentFile();
}

Int_t Event::GetEntry(Long64_t entry) {
  // Read contents of entry.
  if (!fChain) return 0;
  return fChain->GetEntry(entry);
}

Long64_t Event::LoadTree(Long64_t entry) {
  // Set the environment to read one entry
  if (!fChain) return -5;
  Long64_t centry = fChain->LoadTree(entry);
  if (centry < 0) return centry;
  if (fChain->GetTreeNumber() != fCurrent) {
    fCurrent = fChain->GetTreeNumber();
  }
  return centry;
}

void Event::Init(TTree *tree) {
  // Set branch addresses and branch pointers
  if (!tree) return;
  fChain   = tree;
  fCurrent = -1;
  fChain->SetMakeClass(1);
  fChain->SetBranchAddress("TimeChannel1", TimeChannel1, &b_TimeChannel1);
  fChain->SetBranchAddress("TimeChannel2", TimeChannel2, &b_TimeChannel2);
  fChain->SetBranchAddress("TimeChannel3", TimeChannel3, &b_TimeChannel3);
  fChain->SetBranchAddress("TimeChannel4", TimeChannel4, &b_TimeChannel4);
  fChain->SetBranchAddress("WaveformChannel1", WaveformChannel1, &b_WaveformChannel1);
  fChain->SetBranchAddress("WaveformChannel2", WaveformChannel2, &b_WaveformChannel2);
  fChain->SetBranchAddress("WaveformChannel3", WaveformChannel3, &b_WaveformChannel3);
  fChain->SetBranchAddress("WaveformChannel4", WaveformChannel4, &b_WaveformChannel4);
}

bool Event::Cut(Long64_t entry) {
  return true;
}

void Event::Loop() {

  if (fChain == 0) return;
  Long64_t nentries = fChain->GetEntriesFast();
  Long64_t nbytes = 0, nb = 0;

  for (Long64_t jentry = 0; jentry < nentries; jentry++) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;

    if (Cut(ientry)) continue;
  }
}
