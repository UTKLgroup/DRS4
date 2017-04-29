#include "GSFilter.h"


GSFilter::GSFilter(unsigned int userNumberOfPoints, unsigned int userExtrapolationDegree) {
  SetExtrapolationDegree(userExtrapolationDegree);
  SetNumberOfPoints(userNumberOfPoints);
  CalculateJMatrix();
  CalculateAEvenMatrix();
  CalculateAOddMatrix();
  CalculateCMatrix();
  CalculateFuncSmoothingCoeffs();
  CalculateFirstDerivativeSmoothingCoeffs();
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
    std::cout << "A[" << i << "] = " << std::setw(12) << *(FuncSmoothingCoeffs + i) << "." << std::endl;
  }
  std::cout << std::endl;

  std::cout << "The first derivative smoothing coefficents using Golay-Savitzky filter are: " << std::endl;
  for (unsigned int i = 0; i < NumberOfPoints; i++) {
    std::cout << "B[" << i << "] = " << std::setw(12) << *(FirstDerivativeSmoothingCoeffs + i) << "." << std::endl;
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
  JArray = (double*) malloc(NumberOfPoints * ExtrapolationDegree * sizeof(double));
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
  JJTransposeArray = JJTransposeMatrix->GetMatrixArray();

  return;
}

void GSFilter::CalculateAEvenMatrix() {
  unsigned int NumberOfColumns = (int)(ExtrapolationDegree / 2);
  unsigned int NumberOfRows    = NumberOfPoints;

  double* JEvenArray = (double*) malloc(NumberOfRows * NumberOfColumns * sizeof(double));
  for (unsigned int i = 0; i < NumberOfRows; i++) {
    for (unsigned int j = 0; j < ExtrapolationDegree; j++) {
      if (j % 2 == 0) {
        *(JEvenArray + i*NumberOfColumns + (int)(j/2)) = *(JArray + i*ExtrapolationDegree + j);
      }
    }
  }
  TMatrixD* JEvenMatrix = new TMatrixD(NumberOfRows, NumberOfColumns, JEvenArray);

  double* JJTransposeEvenArray = (double*) malloc(NumberOfColumns * NumberOfColumns * sizeof(double));
  for (unsigned int i = 0; i < ExtrapolationDegree; i++) {
    for (unsigned int j = 0; j < ExtrapolationDegree; j++) {
      if (j % 2 == 0 && i % 2 == 0) {
        *(JJTransposeEvenArray + (int)(i/2)*NumberOfColumns + (int)(j/2)) = *(JJTransposeArray + i*ExtrapolationDegree + j);
      }
    }
  }
  TMatrixD* JJTransposeEvenMatrix = new TMatrixD(NumberOfColumns, NumberOfColumns, JJTransposeEvenArray);

  // Calculate AEven
  double Determinant;
  JJTransposeEvenMatrix->InvertFast(&Determinant);
  // Important notice
  // - NumberOfRows happens to be the number of columns of the AEven matrix;
  // - NumberOfColumns happens to be the number of rows of the AEven matrix;
  AEvenMatrix = new TMatrixD(NumberOfColumns, NumberOfRows);
  AEvenMatrix->Mult(*JJTransposeEvenMatrix, *JEvenMatrix);

  return;
}

void GSFilter::CalculateAOddMatrix() {
  unsigned int NumberOfColumns;
  if (ExtrapolationDegree % 2 == 0) {
    NumberOfColumns = (int)(ExtrapolationDegree / 2);
  } else {
    NumberOfColumns = (int)(ExtrapolationDegree / 2) - 1;
  }
  unsigned int NumberOfRows    = NumberOfPoints;

  double* JOddArray = (double*) malloc(NumberOfRows * NumberOfColumns * sizeof(double));
  for (unsigned int i = 0; i < NumberOfRows; i++) {
    for (unsigned int j = 0; j < ExtrapolationDegree; j++) {
      if (j % 2 == 1) {
        *(JOddArray + i*NumberOfColumns + (int)(j/2)) = *(JArray + i*ExtrapolationDegree + j);
      }
    }
  }
  TMatrixD* JOddMatrix = new TMatrixD(NumberOfRows, NumberOfColumns, JOddArray);

  double* JJTransposeOddArray = (double*) malloc(NumberOfColumns * NumberOfColumns * sizeof(double));
  for (unsigned int i = 0; i < ExtrapolationDegree; i++) {
    for (unsigned int j = 0; j < ExtrapolationDegree; j++) {
      if (j % 2 == 1 && i % 2 == 1) {
        *(JJTransposeOddArray + (int)(i/2)*NumberOfColumns + (int)(j/2)) = *(JJTransposeArray + i*ExtrapolationDegree + j);
      }
    }
  }
  TMatrixD* JJTransposeOddMatrix = new TMatrixD(NumberOfColumns, NumberOfColumns, JJTransposeOddArray);

  // Calculate AOdd
  double Determinant;
  JJTransposeOddMatrix->InvertFast(&Determinant);
  // Important notice
  // - NumberOfRows happens to be the number of columns of the AOdd matrix;
  // - NumberOfColumns happens to be the number of rows of the AOdd matrix;
  AOddMatrix = new TMatrixD(NumberOfColumns, NumberOfRows);
  AOddMatrix->Mult(*JJTransposeOddMatrix, *JOddMatrix);

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
  AEvenMatrix->ExtractRow(0, 0, FuncSmoothingCoeffs);

  return;
}

void GSFilter::CalculateFirstDerivativeSmoothingCoeffs() {
  FirstDerivativeSmoothingCoeffs = (double*) malloc(NumberOfPoints * sizeof(double));
  AOddMatrix->ExtractRow(0, 0, FirstDerivativeSmoothingCoeffs);

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
