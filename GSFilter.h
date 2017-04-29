#ifndef GSFilter_h
#define GSFilter_h

#include <cmath>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TMatrixD.h>
#include <TMath.h>

class GSFilter {
public:
  GSFilter(unsigned int userNumberOfPoints, unsigned int userExtrapolationDegree);
  virtual  ~GSFilter();
  virtual void PrintMatrices();
  virtual void Filter(double* Waveform, double* FilteredWaveform);

private:
  virtual void CalculateJMatrix();
  virtual void CalculateAEvenMatrix();
  virtual void CalculateAOddMatrix();
  virtual void CalculateCMatrix();
  virtual void CalculateFuncSmoothingCoeffs();
  virtual void CalculateFirstDerivativeSmoothingCoeffs();
  virtual void SetNumberOfPoints(unsigned int);
  virtual void SetExtrapolationDegree(unsigned int);

  unsigned int NumberOfPoints;
  unsigned int ExtrapolationDegree;

  double*   JArray;
  double*   JJTransposeArray;
  TMatrixD* JMatrix;
  TMatrixD* JEvenMatrix;
  TMatrixD* JOddMatrix;
  TMatrixD* JTransposeMatrix;
  TMatrixD* JJTransposeMatrix;
  TMatrixD* AEvenMatrix;
  TMatrixD* AOddMatrix;
  TMatrixD* CMatrix;
  double*   FuncSmoothingCoeffs;
  double*   FirstDerivativeSmoothingCoeffs;
};

#endif
