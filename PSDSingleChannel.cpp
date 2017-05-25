#ifndef PSDSingleChannel_cxx
#define PSDSingleChannel_cxx

#include "PSDSingleChannel.h"

PSDSingleChannel::PSDSingleChannel(std::string ROOTFile) : fChain(0) {
  TTree *tree;
  TFile *f = (TFile *)gROOT->GetListOfFiles()->FindObject(ROOTFile.c_str());
  if (!f || !f->IsOpen()) {
    f = new TFile(ROOTFile.c_str());
  }
  f->GetObject("DataTree", tree);
  Init(tree);

	NumberOfEventForValidation = 0;
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

void PSDSingleChannel::SetValidation(int aNumberOfEventForValidation) {
	NumberOfEventForValidation = aNumberOfEventForValidation;
	if (NumberOfEventForValidation > 0) {
		const int CreateDirectoryError = mkdir("./ValidationPlots", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}
	return;
}

void PSDSingleChannel::Loop() {
	gStyle->SetOptStat(0);
  if (fChain == 0) return;
  Long64_t nentries = fChain->GetEntriesFast();
  Long64_t nbytes = 0, nb = 0;
  for (Long64_t jentry = 0; jentry < nentries; jentry++) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;

		FillDerivativeHistogram();
		FillWaveformHistogram();
		CheckWaveformCutoff();
		////CheckWaveformGoRogue();
		WaveformBaselineMeanInVolts();
		//WaveformBaselineRMSInVolts();
		//WaveformRiseTimeInNADC();
		/*
		WaveformDecayTimeInNADC();
		WaveformPeakTimeInNADC();
		DerivativePeakTimeInNADC();
		DerivativeTroughTimeInNADC();
		DerivativeCenterZeroTimeInNADC();
		DerivativeBaselineMeanInVolts();
		DerivativeBaselineRMSInVolts();
		WaveformPeakHeightInVolts(); // PeakHeight
		WaveformAThirdPeakHeightIntersectionFromPeakInNADC(); // aHeight
		CalculateWaveformArea();
		CalculateWaveformTailArea();
		CalculateChargeRatio(); // TailArea/WaveformArea
		CalculateDecayShapeIndex(); // aHeight/LogE(PeakHeight)
		*/
		MakeValidationPlots(jentry);
		ClearEvent();
  }

	return;
}

void PSDSingleChannel::FillWaveformHistogram() {
	WaveformHistogram = new TH1D("WaveformHistogram", "Waveform Histogram", 1000, -0.5, 0);
	for (int i = 0; i < NADC; i++) {
		WaveformHistogram->Fill(WaveformChannel1[i]);
	}
	return;
}

void PSDSingleChannel::FillDerivativeHistogram() {
	DerivativeHistogram = new TH1D("DerivativeHistogram", "Derivative Histogram", 1000, -0.05, 0.05);
	for (int i = 0; i < NADC; i++) {
		DerivativeHistogram->Fill(DerivativeWaveformChannel1[i]);
	}
	return;
}

void PSDSingleChannel::CheckWaveformCutoff() {
	WaveformCutoff = false;
	if (WaveformHistogram == NULL) {
		return;
	}
	if (WaveformHistogram->Integral(0, 3) >= 10) {
		WaveformCutoff = true;
	}
	return;
}

void PSDSingleChannel::WaveformBaselineMeanInVolts() {
	int aMaxBin = WaveformHistogram->GetMaximumBin();
	int aHalfMaxBin = 1000;
	while (WaveformHistogram->GetBinContent(aHalfMaxBin) < 0.5*WaveformHistogram->GetBinContent(aMaxBin)) {
		aHalfMaxBin += -1;
	}

	double peak = WaveformHistogram->GetBinCenter(aMaxBin);
	double fwhm = TMath::Abs(WaveformHistogram->GetBinCenter(aMaxBin) - WaveformHistogram->GetBinCenter(aHalfMaxBin));
	WaveformBaselineFit = new TF1("WaveformBaselineFit", "gaus", peak - fwhm, peak + 3*fwhm);
	WaveformHistogram->Fit("WaveformBaselineFit", "QR");

	WaveformBaselineMean = WaveformBaselineFit->GetParameter(1);

	return;
}

void PSDSingleChannel::WaveformBaselineRMSInVolts() {
	WaveformBaselineRMS = WaveformBaselineFit->GetParameter(2);

	return;
}

void PSDSingleChannel::MakeValidationPlots(int eventID) {
	const int CreateDirectoryError = mkdir("./ValidationPlots", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	TCanvas* cValidation = new TCanvas();
	cValidation->Divide(1, 2);
	cValidation->cd(1);
	{ // First pad
		TGraph* grWaveform     = new TGraph(NADC, TimeChannel1, WaveformChannel1);
	  TGraph* grRawWaveform  = new TGraph(NADC, TimeChannel1, RawWaveformChannel1);
	  TGraph* grDerivative   = new TGraph(NADC, TimeChannel1, DerivativeWaveformChannel1);

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

		// Draw a line to indicate the baseline of the waveform
		double yBaseline      	= WaveformBaselineMean;
		double yBaselinePlus  	= WaveformBaselineMean + (2 * WaveformBaselineRMS);
		double yBaselineMinus 	= WaveformBaselineMean - (2 * WaveformBaselineRMS);
		double xBaselineMin     = TimeChannel1[0];
		double xBaselineMax     = TimeChannel1[NADC - 1];
		TLine *theBaseline      = new TLine(xBaselineMin, yBaseline, xBaselineMax, yBaseline);
		TLine *theBaselinePlus  = new TLine(xBaselineMin, yBaselinePlus , xBaselineMax, yBaselinePlus);
		TLine *theBaselineMinus = new TLine(xBaselineMin, yBaselineMinus, xBaselineMax, yBaselineMinus);
		theBaseline->SetLineColor(kGreen);
		theBaselinePlus->SetLineColor(kGreen);
		theBaselineMinus->SetLineColor(kGreen);

		// Draw a line to indicate the peak of the waveforms
		/*
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
		*/

		theBaseline->Draw();
		theBaselinePlus->Draw();
		theBaselineMinus->Draw();
		//thePeakLine->Draw();
		//theDropLine->Draw();
		//theRecoverLine->Draw();
	}
	cValidation->cd(2);
	{ // Second pad
	  WaveformHistogram->SetTitle("Histogram of voltage samples");
	  WaveformHistogram->GetXaxis()->SetTitle("Voltage [V]");
	  WaveformHistogram->GetYaxis()->SetTitle("Entries");
		WaveformHistogram->Draw();
		WaveformBaselineFit->Draw("same");
	}
	cValidation->SaveAs(Form("./ValidationPlots/Evt%i.eps", eventID));

	return;
}

void PSDSingleChannel::ClearEvent() {
	if (WaveformHistogram != NULL) {
		delete WaveformHistogram;
	}
	if (DerivativeHistogram != NULL) {
		delete DerivativeHistogram;
	}
	if (WaveformBaselineFit != NULL) {
		delete WaveformBaselineFit;
	}
	return;
}

#endif
