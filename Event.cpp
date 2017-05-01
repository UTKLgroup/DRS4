#include "Event.h"

Event::Event(std::string ROOTFile) : fChain(0) {
  TTree *tree;
  TFile *f = (TFile *)gROOT->GetListOfFiles()->FindObject(ROOTFile.c_str());
  if (!f || !f->IsOpen()) {
    f = new TFile(ROOTFile.c_str());
  }
  f->GetObject("DataTree", tree);

  InitiateChannelContainer();
  Init(tree);
}

Event::~Event() {
  if (!fChain) return;
  delete fChain->GetCurrentFile();
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

void Event::InitiateChannelContainer() {
  for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
    CHANNEL tmpChannel;
    ChannelContainer.push_back(tmpChannel);
  }

  for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
    std::stringstream ChannelStringStream;
    ChannelStringStream << (ChannelID + 1);

    GetChannel(ChannelID)->TimeBranchName = "TimeChannel" + ChannelStringStream.str();
    GetChannel(ChannelID)->WaveformBranchName = "WaveformChannel" + ChannelStringStream.str();
    GetChannel(ChannelID)->RawWaveformBranchName = "RawWaveformChannel" + ChannelStringStream.str();
    GetChannel(ChannelID)->DerivativeWaveformBranchName = "DerivativeWaveformChannel" + ChannelStringStream.str();

    std::string hName = "hVoltageSampleHistogramChannel" + ChannelStringStream.str();
    GetChannel(ChannelID)->hVoltageSampleHistogram = new TH1D(hName.c_str(), "Voltage samples histogram", 6000, -.52, .02);
  }

  return;
}

void Event::InitiateBranches(unsigned int ChannelID) {
  DataFoundInChannel[ChannelID] = true;
  fChain->SetBranchAddress(GetChannel(ChannelID)->TimeBranchName.c_str(), GetChannel(ChannelID)->Time, &(GetChannel(ChannelID)->b_TimeChannel));
  fChain->SetBranchAddress(GetChannel(ChannelID)->WaveformBranchName.c_str(), GetChannel(ChannelID)->Waveform, &(GetChannel(ChannelID)->b_WaveformChannel));
  fChain->SetBranchAddress(GetChannel(ChannelID)->DerivativeWaveformBranchName.c_str(), GetChannel(ChannelID)->DerivativeWaveform, &(GetChannel(ChannelID)->b_DerivativeWaveformChannel));
  if (fChain->FindBranch(GetChannel(ChannelID)->RawWaveformBranchName.c_str())) {
    RawWaveformExistFlag = true;
    fChain->SetBranchAddress(GetChannel(ChannelID)->RawWaveformBranchName.c_str(), GetChannel(ChannelID)->RawWaveform, &(GetChannel(ChannelID)->b_RawWaveformChannel));
  }

  return;
}

void Event::Init(TTree *tree) {
  if (!tree) return;
  fChain   = tree;
  fCurrent = -1;
  fChain->SetMakeClass(1);
  for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
    if (fChain->FindBranch(ChannelContainer.at(ChannelID).TimeBranchName.c_str())) {
      InitiateBranches(ChannelID);
    }
  }
  return;
}

void Event::Run() {
  if (fChain == 0) return;
  TotalNumberOfEvents = fChain->GetEntriesFast();
  Long64_t nbytes = 0, nb = 0;

  for (CurrentEventIndex = 0; CurrentEventIndex < TotalNumberOfEvents; CurrentEventIndex++) {
    Long64_t ientry = LoadTree(CurrentEventIndex);
    if (ientry < 0) break;
    nb = fChain->GetEntry(CurrentEventIndex);   nbytes += nb;

    // Analysis steps start here
    FillVoltageSampleHistogram();
    if (CurrentEventIndex < NumberOfEventForValidation) {
      if (ValidationFlag) {
        const int CreateDirectoryError = mkdir("./ValidationPlots", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        MakeValidationPlots();
      }
    }
  }

  EndOfRun();
  return;
}

void:: Event::EndOfRun() {
  // delete to avoid memory leak
  for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
    delete GetChannel(ChannelID)->hVoltageSampleHistogram;
  }

  return;
}

void Event::FillVoltageSampleHistogram() {
  for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
    GetChannel(ChannelID)->hVoltageSampleHistogram->Reset();
    for (unsigned int i = 0; i < 1024; i++) {
      GetChannel(ChannelID)->hVoltageSampleHistogram->Fill(*(ChannelContainer.at(ChannelID).Waveform + i));
    }
  }

  return;
}

void Event::SetValidation(unsigned int userNumberOfEventForValidation) {
  if (RawWaveformExistFlag) {
    ValidationFlag = true;
    NumberOfEventForValidation = userNumberOfEventForValidation;
    return;
  } else {
    std::cout << "\t WARNING: Raw waveforms are not saved in the decoding process." << std::endl;
    std::cout << "\t Waveform filter validation needs raw waveforms as an input. " << std::endl;
    std::cout << "\t Turning off the <ValidationFlag>. " << std::endl;
    std::cout << "\t Please re-run the decoding and call <SetSaveRawWaveform()>. " << std::endl << std::endl;
    return;
  }
}

void Event::MakeValidationPlots() {
  for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
    if (DataFoundInChannel[ChannelID]) {
      TCanvas* cValidation = new TCanvas();
      cValidation->Divide(1, 2);
      cValidation->cd(1);
      DrawFilterValidationPlots(ChannelID)->Draw("APL");
      cValidation->cd(2);
      DrawVoltageSampleHistogram(ChannelID)->Draw();
      cValidation->SaveAs(Form("./ValidationPlots/Evt%i_Ch%i.eps", CurrentEventIndex, ChannelID + 1));
    }
  }

  return;
}

TMultiGraph* Event::DrawFilterValidationPlots(unsigned int ChannelID) {
  TGraph* grWaveform     = new TGraph(1024, GetChannel(ChannelID)->Time, GetChannel(ChannelID)->Waveform);
  TGraph* grRawWaveform  = new TGraph(1024, GetChannel(ChannelID)->Time, GetChannel(ChannelID)->RawWaveform);
  TGraph* grDerivative   = new TGraph(1024, GetChannel(ChannelID)->Time, GetChannel(ChannelID)->DerivativeWaveform);

  grWaveform->SetLineColor(kRed);
  grDerivative->SetLineColor(kBlue);
  TMultiGraph* mgrMultiWaveform = new TMultiGraph();
  mgrMultiWaveform->Add(grDerivative,  "l");
  mgrMultiWaveform->Add(grRawWaveform, "l");
  mgrMultiWaveform->Add(grWaveform,    "l");
  mgrMultiWaveform->Draw("APL");
  mgrMultiWaveform->SetTitle("Waveform display");
  mgrMultiWaveform->GetXaxis()->SetTitle("Time [ns]");
  mgrMultiWaveform->GetYaxis()->SetTitle("Voltage [V]");

  return mgrMultiWaveform;
}

TH1D* Event::DrawVoltageSampleHistogram(unsigned int ChannelID) {
  GetChannel(ChannelID)->hVoltageSampleHistogram->SetTitle("Histogram of voltage samples");
  GetChannel(ChannelID)->hVoltageSampleHistogram->GetXaxis()->SetTitle("Voltage [V]");
  GetChannel(ChannelID)->hVoltageSampleHistogram->GetYaxis()->SetTitle("Entries");

  return GetChannel(ChannelID)->hVoltageSampleHistogram;
}

CHANNEL* Event::GetChannel(unsigned int ChannelID) {
  return &ChannelContainer.at(ChannelID);
}
