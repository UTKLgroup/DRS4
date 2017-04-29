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

  std::cout << "\t The J-even matrices: " << std::endl;
  JEvenMatrix->Print();
  std::cout << std::endl;

  std::cout << "\t The JJTranspose-even matrices: " << std::endl;
  JJTransposeEvenMatrix->Print();
  std::cout << std::endl;

  std::cout << "\t The A-even matrices: " << std::endl;
  AEvenMatrix->Print();
  std::cout << std::endl;

  std::cout << "\t The J-odd matrices: " << std::endl;
  JOddMatrix->Print();
  std::cout << std::endl;

  std::cout << "\t The JJTranspose-odd matrices: " << std::endl;
  JJTransposeOddMatrix->Print();
  std::cout << std::endl;

  std::cout << "\t The A-odd matrices: " << std::endl;
  AOddMatrix->Print();
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
  unsigned int NumberOfColumns;
  if (ExtrapolationDegree % 2 == 0) {
    NumberOfColumns = (int)(ExtrapolationDegree / 2);
  } else {
    NumberOfColumns = (int)(ExtrapolationDegree / 2) + 1;
  }
  unsigned int NumberOfRows    = NumberOfPoints;

  double* JEvenArray = (double*) malloc(NumberOfRows * NumberOfColumns * sizeof(double));
  for (unsigned int i = 0; i < NumberOfRows; i++) {
    for (unsigned int j = 0; j < ExtrapolationDegree; j++) {
      if (j % 2 == 0) {
        *(JEvenArray + i*NumberOfColumns + (int)(j/2)) = *(JArray + i*ExtrapolationDegree + j);
      }
    }
  }
  JEvenMatrix = new TMatrixD(NumberOfRows, NumberOfColumns, JEvenArray);

  double* JJTransposeEvenArray = (double*) malloc(NumberOfColumns * NumberOfColumns * sizeof(double));
  for (unsigned int i = 0; i < ExtrapolationDegree; i++) {
    for (unsigned int j = 0; j < ExtrapolationDegree; j++) {
      if (j % 2 == 0 && i % 2 == 0) {
        *(JJTransposeEvenArray + (int)(i/2)*NumberOfColumns + (int)(j/2)) = *(JJTransposeArray + i*ExtrapolationDegree + j);
      }
    }
  }
  JJTransposeEvenMatrix = new TMatrixD(NumberOfColumns, NumberOfColumns, JJTransposeEvenArray);

  /* Calculate AEven */
  // Inverse JJTransposeEven matrix
  double Determinant;
  JJTransposeEvenMatrix->InvertFast(&Determinant);
  // Transposer JEven matrix
  TMatrixD* JTransposeEvenMatrix = new TMatrixD(NumberOfColumns, NumberOfRows);
  JTransposeEvenMatrix->Transpose(*JEvenMatrix);
  // Important notice
  // - NumberOfRows happens to be the number of columns of the AEven matrix;
  // - NumberOfColumns happens to be the number of rows of the AEven matrix;
  // - AEven matrix is the product of Inversed-JJTransposeEven and JTransposeEven
  AEvenMatrix = new TMatrixD(NumberOfColumns, NumberOfRows);
  AEvenMatrix->Mult(*JJTransposeEvenMatrix, *JTransposeEvenMatrix);

  return;
}

void GSFilter::CalculateAOddMatrix() {
  unsigned int NumberOfColumns;
  if (ExtrapolationDegree % 2 == 0) {
    NumberOfColumns = (int)(ExtrapolationDegree / 2);
  } else {
    NumberOfColumns = (int)(ExtrapolationDegree / 2);
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
  JOddMatrix = new TMatrixD(NumberOfRows, NumberOfColumns, JOddArray);

  double* JJTransposeOddArray = (double*) malloc(NumberOfColumns * NumberOfColumns * sizeof(double));
  for (unsigned int i = 0; i < ExtrapolationDegree; i++) {
    for (unsigned int j = 0; j < ExtrapolationDegree; j++) {
      if (j % 2 == 1 && i % 2 == 1) {
        *(JJTransposeOddArray + (int)(i/2)*NumberOfColumns + (int)(j/2)) = *(JJTransposeArray + i*ExtrapolationDegree + j);
      }
    }
  }
  JJTransposeOddMatrix = new TMatrixD(NumberOfColumns, NumberOfColumns, JJTransposeOddArray);

  /* Calculate AOdd */
  // Inverse JJTransposeOdd matrix
  double Determinant;
  JJTransposeOddMatrix->InvertFast(&Determinant);
  // Transposer JOdd matrix
  TMatrixD* JTransposeOddMatrix = new TMatrixD(NumberOfColumns, NumberOfRows);
  JTransposeOddMatrix->Transpose(*JOddMatrix);
  // Important notice
  // - NumberOfRows happens to be the number of columns of the AOdd matrix;
  // - NumberOfColumns happens to be the number of rows of the AOdd matrix;
  // - AOdd matrix is the product of Inversed-JJTransposeOdd and JTransposeOdd
  AOddMatrix = new TMatrixD(NumberOfColumns, NumberOfRows);
  AOddMatrix->Mult(*JJTransposeOddMatrix, *JTransposeOddMatrix);

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
  int HalfNumberOfPoints = (int)(NumberOfPoints / 2);
  for (unsigned int i = HalfNumberOfPoints; i < 1024 - HalfNumberOfPoints; i++) {
    *(FilteredWaveform + i) = 0;
    for (int j = -HalfNumberOfPoints; j < HalfNumberOfPoints + 1; j++) {
      *(FilteredWaveform + i) += *(Waveform + i + j) * *(FuncSmoothingCoeffs + j + HalfNumberOfPoints);
    }
  }
  for (unsigned int i = 0; i < HalfNumberOfPoints; i++) {
    *(FilteredWaveform + i) = *(FilteredWaveform + HalfNumberOfPoints);
    *(FilteredWaveform + 1023 - i) = *(FilteredWaveform + 1023 - HalfNumberOfPoints);
  }

  return;
}

void GSFilter::FirstDerivative(double* Waveform, double* FilteredFirstDerivative, double VariableStep) {
  int HalfNumberOfPoints = (int)(NumberOfPoints / 2);
  for (unsigned int i = HalfNumberOfPoints; i < 1024 - HalfNumberOfPoints; i++) {
    *(FilteredFirstDerivative + i) = 0;
    for (int j = -HalfNumberOfPoints; j < HalfNumberOfPoints + 1; j++) {
      *(FilteredFirstDerivative + i) += *(Waveform + i + j) * *(FirstDerivativeSmoothingCoeffs + j + HalfNumberOfPoints);
    }
    *(FilteredFirstDerivative + i) = *(FilteredFirstDerivative + i) / VariableStep;
  }

  for (unsigned int i = 0; i < HalfNumberOfPoints; i++) {
    *(FilteredFirstDerivative + i) = *(FilteredFirstDerivative + HalfNumberOfPoints);
    *(FilteredFirstDerivative + 1023 - i) = *(FilteredFirstDerivative + 1023 - HalfNumberOfPoints);
  }

  return;
}
