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
    GetChannel(ChannelID)->hVoltageSampleHistogram = new TH1D(hName.c_str(), "Voltage samples histogram", 200, -.006, .002);
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

void Event::CheckWaveformCutoff() {
  for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
    if (DataFoundInChannel[ChannelID]) {
      int nLocationsAtMaxVoltage = 0;
      for (unsigned int i = 0; i < NADC; i++) {
        if ((GetChannel(ChannelID)->Waveform)[i] <= -0.5) {
          nLocationsAtMaxVoltage++;
        }
      }
      if (nLocationsAtMaxVoltage >= maxNADCToBeConsiderCutoff) {
        *(GetChannel(ChannelID)->WaveformCutoff) = true;
      } else {
        *(GetChannel(ChannelID)->WaveformCutoff) = false;
      }
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

    CheckWaveformCutoff();
    // Analysis steps start here
    FindTimeInfo();
    FindBaselineInfo();
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
    if (DataFoundInChannel[ChannelID] && !(*(GetChannel(ChannelID)->WaveformCutoff))) {
      GetChannel(ChannelID)->hVoltageSampleHistogram->Reset();
      for (unsigned int i = 0; i < GetChannel(ChannelID)->LocationOfVoltageDrop; i++) {
        GetChannel(ChannelID)->hVoltageSampleHistogram->Fill(*(ChannelContainer.at(ChannelID).Waveform + i));
      }
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
    if (DataFoundInChannel[ChannelID] && !(*(GetChannel(ChannelID)->WaveformCutoff))) {
      TCanvas* cValidation = new TCanvas();
      cValidation->Divide(1, 2);
      cValidation->cd(1);
      { // First pad
        DrawFilterValidationPlots(ChannelID)->Draw("APL");
        // Draw a line to indicate the baseline of the waveform
        double yBaseline      = GetChannel(ChannelID)->Baseline;
        double yBaselinePlus  = GetChannel(ChannelID)->Baseline + (2 * GetChannel(ChannelID)->BaselineRMS);
        double yBaselineMinus = GetChannel(ChannelID)->Baseline - (2 * GetChannel(ChannelID)->BaselineRMS);
        double xBaselineMin     = GetChannel(ChannelID)->Time[0];
        double xBaselineMax     = GetChannel(ChannelID)->Time[NADC - 1];
        TLine *theBaseline      = new TLine(xBaselineMin, yBaseline, xBaselineMax, yBaseline);
        TLine *theBaselinePlus  = new TLine(xBaselineMin, yBaselinePlus , xBaselineMax, yBaselinePlus);
        TLine *theBaselineMinus = new TLine(xBaselineMin, yBaselineMinus, xBaselineMax, yBaselineMinus);
        theBaseline->SetLineColor(kGreen);
        theBaselinePlus->SetLineColor(kGreen);
        theBaselineMinus->SetLineColor(kGreen);

        // Draw a line to indicate the peak of the waveforms
        double yPeakMin     = gPad->GetUymin();
        double yPeakMax     = gPad->GetUymax();
        double xPeak        = GetChannel(ChannelID)->TimeOfWaveformPeak;
        double xDrop        = (GetChannel(ChannelID)->Time)[GetChannel(ChannelID)->LocationOfVoltageDrop];
        double xRecover     = (GetChannel(ChannelID)->Time)[GetChannel(ChannelID)->LocationOfVoltageRecover];
        TLine *thePeakLine     = new TLine(xPeak, yPeakMin, xPeak, yPeakMax);
        TLine *theDropLine     = new TLine(xDrop, yPeakMin, xDrop, yPeakMax);
        TLine *theRecoverLine  = new TLine(xRecover, yPeakMin, xRecover, yPeakMax);
        thePeakLine->SetLineColor(kCyan);
        theDropLine->SetLineColor(kCyan);
        theRecoverLine->SetLineColor(kCyan);

        theBaseline->Draw();
        theBaselinePlus->Draw();
        theBaselineMinus->Draw();
        thePeakLine->Draw();
        theDropLine->Draw();
        theRecoverLine->Draw();
      }
      cValidation->cd(2);
      { // Second pad
        DrawVoltageSampleHistogram(ChannelID)->Draw();
      }
      cValidation->SaveAs(Form("./ValidationPlots/Evt%i_Ch%i.eps", CurrentEventIndex, ChannelID + 1));
    }
  }

  return;
}

TMultiGraph* Event::DrawFilterValidationPlots(unsigned int ChannelID) {
  TGraph* grWaveform     = new TGraph(NADC, GetChannel(ChannelID)->Time, GetChannel(ChannelID)->Waveform);
  TGraph* grRawWaveform  = new TGraph(NADC, GetChannel(ChannelID)->Time, GetChannel(ChannelID)->RawWaveform);
  TGraph* grDerivative   = new TGraph(NADC, GetChannel(ChannelID)->Time, GetChannel(ChannelID)->DerivativeWaveform);

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
  gStyle->SetOptStat(0);
  GetChannel(ChannelID)->hVoltageSampleHistogram->SetTitle("Histogram of voltage samples");
  GetChannel(ChannelID)->hVoltageSampleHistogram->GetXaxis()->SetTitle("Voltage [V]");
  GetChannel(ChannelID)->hVoltageSampleHistogram->GetYaxis()->SetTitle("Entries");

  return GetChannel(ChannelID)->hVoltageSampleHistogram;
}

CHANNEL* Event::GetChannel(unsigned int ChannelID) {
  return &ChannelContainer.at(ChannelID);
}

void Event::FindBaselineInfo() {
  FillVoltageSampleHistogram();
  FindBaseline();
  FindBaselineRMS();

  return;
}

void Event::FindBaseline() {
  for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
    if (DataFoundInChannel[ChannelID] && !(*(GetChannel(ChannelID)->WaveformCutoff))) {
      TH1D* h = GetChannel(ChannelID)->hVoltageSampleHistogram;
      GetChannel(ChannelID)->Baseline = h->GetBinCenter(h->GetMaximumBin());
    }
  }

  return;
}

void Event::FindBaselineRMS() {
  for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
    if (DataFoundInChannel[ChannelID] && !(*(GetChannel(ChannelID)->WaveformCutoff))) {
      TH1D* h = GetChannel(ChannelID)->hVoltageSampleHistogram;
      GetChannel(ChannelID)->BaselineRMS = h->GetRMS();
    }
  }
}

void Event::FindLocationOfWeakformPeak() {
  for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
    if (DataFoundInChannel[ChannelID] && !(*(GetChannel(ChannelID)->WaveformCutoff))) {
      CHANNEL* theChannel = GetChannel(ChannelID);
      unsigned int LocOfMinDev = 0;
      unsigned int LocOfMaxDev = 0;
      double MinDev = 0.;
      double MaxDev = 0.;
      for (unsigned int i = 0; i < NADC; i++) {
        if ((theChannel->DerivativeWaveform)[i] > MaxDev) {
          MaxDev = (theChannel->DerivativeWaveform)[i];
          LocOfMaxDev = i;
        }
        if ((theChannel->DerivativeWaveform)[i] < MinDev) {
          MinDev = (theChannel->DerivativeWaveform)[i];
          LocOfMinDev = i;
        }
      }
      theChannel->LocationOfMinimumDerivative = LocOfMinDev;
      theChannel->LocationOfMaximumDerivative = LocOfMaxDev;
    }
  }

  for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
    if (DataFoundInChannel[ChannelID] && !(*(GetChannel(ChannelID)->WaveformCutoff))) {
      CHANNEL* theChannel = GetChannel(ChannelID);
      for (unsigned int i = theChannel->LocationOfMinimumDerivative; i < theChannel->LocationOfMaximumDerivative; i++) {
        double devSignCheck = (theChannel->DerivativeWaveform)[i] * (theChannel->DerivativeWaveform)[i + 1];
        if (devSignCheck <= 0) {
          theChannel->TimeOfWaveformPeak = (theChannel->Time)[i];
          theChannel->LocationOfWaveformPeak = i;
        }
      }
    }
  }

  return;
}

void Event::FindLocationOfVoltageDrop() {
  for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
    if (DataFoundInChannel[ChannelID] && !(*(GetChannel(ChannelID)->WaveformCutoff))) {
      CHANNEL* theChannel = GetChannel(ChannelID);
      for (int i = theChannel->LocationOfMinimumDerivative; i > 0; i--) {
        if ((theChannel->DerivativeWaveform)[i] >= -0.0001) {
          theChannel->LocationOfVoltageDrop = i;
          i = -1;
        }
      }
    }
  }

  return;
}

void Event::FindLocationOfVoltageRecover() {
  for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
    if (DataFoundInChannel[ChannelID] && !(*(GetChannel(ChannelID)->WaveformCutoff))) {
      CHANNEL* theChannel = GetChannel(ChannelID);
      for (unsigned int i = theChannel->LocationOfMaximumDerivative; i < NADC; i++) {
        if ((theChannel->DerivativeWaveform)[i] <= 0.0) {
          theChannel->LocationOfVoltageRecover = i;
          i = NADC;
        }
      }
    }
  }

  return;
}

void Event::FindTimeInfo() {
  FindLocationOfWeakformPeak();
  FindLocationOfVoltageDrop();
  FindLocationOfVoltageRecover();

  return;
}
