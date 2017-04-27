#ifndef GSFilter_h
#define GSFilter_h

#include <cmath>
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

private:
  virtual void CalculateJMatrix();
  virtual void CalculateCMatrix();
  virtual void SetNumberOfPoints(unsigned int);
  virtual void SetExtrapolationDegree(unsigned int);

  unsigned int NumberOfPoints;
  unsigned int ExtrapolationDegree;
  TMatrixD* JMatrix;
  TMatrixD* JTransposeMatrix;
  TMatrixD* JJTransposeMatrix;
  TMatrixD* CMatrix;
};

#endif
