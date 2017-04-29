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
  if (fChain->FindBranch("TimeChannel1")) {
    DataFoundInChannel[0] = true;
    fChain->SetBranchAddress("TimeChannel1", TimeChannel1, &b_TimeChannel1);
    fChain->SetBranchAddress("WaveformChannel1", WaveformChannel1, &b_WaveformChannel1);
    fChain->SetBranchAddress("DerivativeWaveformChannel1", DerivativeWaveformChannel1, &b_DerivativeWaveformChannel1);
    if (fChain->FindBranch("RawWaveformChannel1")) {
      RawWaveformExistFlag = true;
      fChain->SetBranchAddress("RawWaveformChannel1", RawWaveformChannel1, &b_RawWaveformChannel1);
    }
  }
  if (fChain->FindBranch("TimeChannel2")) {
    DataFoundInChannel[1] = true;
    fChain->SetBranchAddress("TimeChannel2", TimeChannel2, &b_TimeChannel2);
    fChain->SetBranchAddress("WaveformChannel2", WaveformChannel2, &b_WaveformChannel2);
    fChain->SetBranchAddress("DerivativeWaveformChannel2", DerivativeWaveformChannel2, &b_DerivativeWaveformChannel2);
    if (fChain->FindBranch("RawWaveformChannel2")) {
      RawWaveformExistFlag = true;
      fChain->SetBranchAddress("RawWaveformChannel2", RawWaveformChannel2, &b_RawWaveformChannel2);
    }
  }
  if (fChain->FindBranch("TimeChannel3")) {
    DataFoundInChannel[2] = true;
    fChain->SetBranchAddress("TimeChannel3", TimeChannel3, &b_TimeChannel3);
    fChain->SetBranchAddress("WaveformChannel3", WaveformChannel3, &b_WaveformChannel3);
    fChain->SetBranchAddress("DerivativeWaveformChannel3", DerivativeWaveformChannel3, &b_DerivativeWaveformChannel3);
    if (fChain->FindBranch("RawWaveformChannel3")) {
      RawWaveformExistFlag = true;
      fChain->SetBranchAddress("RawWaveformChannel3", RawWaveformChannel3, &b_RawWaveformChannel3);
    }
  }
  if (fChain->FindBranch("TimeChannel4")) {
    DataFoundInChannel[3] = true;
    fChain->SetBranchAddress("TimeChannel4", TimeChannel4, &b_TimeChannel4);
    fChain->SetBranchAddress("WaveformChannel4", WaveformChannel4, &b_WaveformChannel4);
    fChain->SetBranchAddress("DerivativeWaveformChannel4", DerivativeWaveformChannel4, &b_DerivativeWaveformChannel4);
    if (fChain->FindBranch("RawWaveformChannel4")) {
      RawWaveformExistFlag = true;
      fChain->SetBranchAddress("RawWaveformChannel4", RawWaveformChannel4, &b_RawWaveformChannel4);
    }
  }
  return;
}

bool Event::Cut(Long64_t entry) {
  return true;
}

void Event::Run() {
  if (fChain == 0) return;
  TotalNumberOfEvents = fChain->GetEntriesFast();
  Long64_t nbytes = 0, nb = 0;

  for (CurrentEventIndex = 0; CurrentEventIndex < TotalNumberOfEvents; CurrentEventIndex++) {
    Long64_t ientry = LoadTree(CurrentEventIndex);
    if (ientry < 0) break;
    nb = fChain->GetEntry(CurrentEventIndex);   nbytes += nb;

    if (!Cut(ientry)) continue;

    // Analysis steps start here
    if (CurrentEventIndex < NumberOfEventForFilterValidation) {
      if (FilterValidationFlag) {
        MakeFilterValidationPlots();
      }
    }
  }

  return;
}

void Event::SetFilterValidation(unsigned int userNumberOfEventForFilterValidation) {
  if (RawWaveformExistFlag) {
    FilterValidationFlag = true;
    NumberOfEventForFilterValidation = userNumberOfEventForFilterValidation;
    return;
  } else {
    std::cout << "\t WARNING: Raw waveforms are not saved in the decoding process." << std::endl;
    std::cout << "\t Waveform filter validation needs raw waveforms as an input. " << std::endl;
    std::cout << "\t Turning off the <FilterValidationFlag>. " << std::endl;
    std::cout << "\t Please re-run the decoding and call <SetSaveRawWaveform()>. " << std::endl << std::endl;
    return;
  }
}

void Event::MakeFilterValidationPlots() {
  for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
    if (DataFoundInChannel[ChannelID]) {
      DrawFilterValidationPlotsForChannel(ChannelID);
    }
  }

  return;
}

void Event::DrawFilterValidationPlotsForChannel(unsigned int ChannelID) {
  TGraph* grWaveform     = new TGraph(1024, GetTimeChannel(ChannelID), GetWaveformChannel(ChannelID));
  TGraph* grRawWaveform  = new TGraph(1024, GetTimeChannel(ChannelID), GetRawWaveformChannel(ChannelID));
  TGraph* grDerivative   = new TGraph(1024, GetTimeChannel(ChannelID), GetDerivativeWaveformChannel(ChannelID));

  grWaveform->SetLineColor(kRed);
  grDerivative->SetLineColor(kBlue);
  TMultiGraph *mgrMultiWaveform = new TMultiGraph();
  mgrMultiWaveform->Add(grDerivative,  "l");
  mgrMultiWaveform->Add(grRawWaveform, "l");
  mgrMultiWaveform->Add(grWaveform,    "l");
  TCanvas* cMultiWaveform = new TCanvas();
  mgrMultiWaveform->Draw("APL");

  const int CreateDirectoryError = mkdir("./ValidationPlots", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  cMultiWaveform->SaveAs(Form("./ValidationPlots/FilterValidation_evt%i_ch%i.eps", CurrentEventIndex, ChannelID + 1));

  return;
}

Double_t* Event::GetTimeChannel(unsigned int ChannelID) {
  switch (ChannelID) {
    case 0: {
      return &TimeChannel1[0];
      break;
    }
    case 1: {
      return &TimeChannel2[0];
      break;
    }
    case 2: {
      return &TimeChannel3[0];
      break;
    }
    case 3: {
      return &TimeChannel4[0];
      break;
    }
  }
}

Double_t* Event::GetWaveformChannel(unsigned int ChannelID) {
  switch (ChannelID) {
    case 0: {
      return &WaveformChannel1[0];
    }
    case 1: {
      return &WaveformChannel2[0];
    }
    case 2: {
      return &WaveformChannel3[0];
    }
    case 3: {
      return &WaveformChannel4[0];
    }
  }
}

Double_t* Event::GetRawWaveformChannel(unsigned int ChannelID) {
  switch (ChannelID) {
    case 0: {
      return &RawWaveformChannel1[0];
    }
    case 1: {
      return &RawWaveformChannel2[0];
    }
    case 2: {
      return &RawWaveformChannel3[0];
    }
    case 3: {
      return &RawWaveformChannel4[0];
    }
  }
}

Double_t* Event::GetDerivativeWaveformChannel(unsigned int ChannelID) {
  switch (ChannelID) {
    case 0: {
      return &DerivativeWaveformChannel1[0];
    }
    case 1: {
      return &DerivativeWaveformChannel2[0];
    }
    case 2: {
      return &DerivativeWaveformChannel3[0];
    }
    case 3: {
      return &DerivativeWaveformChannel4[0];
    }
  }
}
