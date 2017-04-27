#include "GSFilter.h"


GSFilter::GSFilter(unsigned int userNumberOfPoints, unsigned int userExtrapolationDegree) {
  SetExtrapolationDegree(userExtrapolationDegree);
  SetNumberOfPoints(userNumberOfPoints);
  CalculateJMatrix();
  CalculateCMatrix();
  CalculateFuncSmoothingCoeffs();
}

GSFilter::~GSFilter() {
}

void GSFilter::PrintMatrices() {

  std::cout << "\t The J matrix: " << std::endl;
  JMatrix->Print();
  std::cout << std::endl;

  std::cout << "\t The J-transposed matrix: " << std::endl;
  JTransposeMatrix->Print();
  std::cout << std::endl;

  std::cout << "\t The product of J and J-transposed matrices: " << std::endl;
  JJTransposeMatrix->Print();
  std::cout << std::endl;

  std::cout << "The C matrix: " << std::endl;
  CMatrix->Print();
  std::cout << std::endl;

  std::cout << "The function smoothing coefficents using Golay-Savitzky filter are: " << std::endl;
  for (unsigned int i = 0; i < NumberOfPoints; i++) {
    std::cout << "a[" << i << "] = " << std::setw(12) << *(FuncSmoothingCoeffs + i) << "." << std::endl;
  }
  std::cout << std::endl;

  return;
}

void GSFilter::SetNumberOfPoints(unsigned int userNumberOfPoints) {
  if (userNumberOfPoints % 2 == 1) {
    NumberOfPoints = userNumberOfPoints;
  } else {
    std::cout << "\t WARNING: Number of interpolation points has to be an odd integer." << std::endl;
    std::cout << "\t WARNING: Filter algorithm automatically sets number of points to " << userNumberOfPoints + 1 << "." << std::endl;
    NumberOfPoints = userNumberOfPoints + 1;
  }

  return;
}

void GSFilter::SetExtrapolationDegree(unsigned int userExtrapolationDegree) {
  ExtrapolationDegree = userExtrapolationDegree;
  return;
}

void GSFilter::CalculateJMatrix() {
  double* JArray = (double*) malloc(NumberOfPoints * ExtrapolationDegree * sizeof(double));
  for (int i = 0; i < NumberOfPoints; i++) {
    for (int j = 0; j < ExtrapolationDegree; j++) {
      if ((- ((NumberOfPoints - 1) / 2) + i == 0) && (j == 0)) {
        *(JArray + ExtrapolationDegree * i + j) = 1;
      } else {
        *(JArray + ExtrapolationDegree * i + j) = TMath::Power(- (((double) NumberOfPoints - 1) / 2) + (double) i, (double) j);
      }
    }
  }

  JMatrix = new TMatrixD(NumberOfPoints, ExtrapolationDegree, JArray);

  JTransposeMatrix = new TMatrixD(ExtrapolationDegree, NumberOfPoints);
  JTransposeMatrix->Transpose(*JMatrix);

  JJTransposeMatrix = new TMatrixD(ExtrapolationDegree, ExtrapolationDegree);
  JJTransposeMatrix->Mult(*JTransposeMatrix, *JMatrix);

  return;
}

void GSFilter::CalculateCMatrix() {
  double Determinant;
  JJTransposeMatrix->InvertFast(&Determinant);

  CMatrix = new TMatrixD(ExtrapolationDegree, NumberOfPoints);
  CMatrix->Mult(*JJTransposeMatrix, *JTransposeMatrix);

  return;
}

void GSFilter::CalculateFuncSmoothingCoeffs() {
  FuncSmoothingCoeffs = (double*) malloc(NumberOfPoints * sizeof(double));
  CMatrix->ExtractRow(0, 0, FuncSmoothingCoeffs);

  return;
}

void GSFilter::Filter(double* Waveform, double* FilteredWaveform) {
  *(FilteredWaveform + 0) = *(Waveform + 0);
  *(FilteredWaveform + 1) = *(Waveform + 1);
  *(FilteredWaveform + 2) = *(Waveform + 2);
  *(FilteredWaveform + 1021) = *(Waveform + 1021);
  *(FilteredWaveform + 1022) = *(Waveform + 1022);
  *(FilteredWaveform + 1023) = *(Waveform + 1023);
  
  for (unsigned int i = 3; i < 1021; i++) {
    *(FilteredWaveform + i) = 0;
    for (int j = -3; j < 4; j++) {
      *(FilteredWaveform + i) += *(Waveform + i + j) * *(FuncSmoothingCoeffs + j + 3);
    }
  }
  return;
}
