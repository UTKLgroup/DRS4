/*
   Original version created by:      Stefan Ritt <stefan.ritt@psi.ch>
   Current version developed by:     UTKL HEP group <The University of Texas at Austin>
 */

#include "Decode.h"

struct stat statbuf;

Decode::Decode(std::string DataFileName, int userNumberOfEvents) {
  std::cout << std::endl;

  if (stat(DataFileName.c_str(), &statbuf) == -1) {
    std::cout << "The data file " << DataFileName.c_str() << " is not found." << std::endl << std::endl;
  } else {
    DataFile = fopen(DataFileName.c_str(), "r");
    std::cout << "The data file " << DataFileName.c_str() << " is found." << std::endl << std::endl;

    std::string::size_type dotPosition;
    std::string::size_type fileExtensionLength = 1;
    dotPosition = DataFileName.find(".", DataFileName.length() - fileExtensionLength);
    while (dotPosition == std::string::npos) {
      fileExtensionLength += 1;
      dotPosition = DataFileName.find(".", DataFileName.length() - fileExtensionLength);
    }
    DataFileName.erase(dotPosition, DataFileName.length());
    DataFileName = DataFileName + ".root";
    ROOTFileName = DataFileName;
  }

  if (userNumberOfEvents == -1) {
    NumberOfEvents = 1;
    DecodeAllEventsFlag = true;
  } else {
    NumberOfEvents = userNumberOfEvents;
    DecodeAllEventsFlag = false;
  }
}

Decode::~Decode() {
  if (DataFile != 0) {
    fclose(DataFile);
  }
}

void Decode::SetFilter(unsigned int userNumberOfFilteringPoints, unsigned int userFilteringDegree) {
  FilterFlag = true;
  NumberOfFilteringPoints = userNumberOfFilteringPoints;
  FilteringDegree = userFilteringDegree;

  ROOTFileName.erase(ROOTFileName.length() - 5, ROOTFileName.length());
  ROOTFileName = ROOTFileName + ".FilteredWaveform.root";

  return;
}

void Decode::SetSaveRawWaveform() {
  SaveRawWaveform = true;
  return;
}

std::string Decode::GetROOTFileName() {
  return ROOTFileName;
}

void Decode::Run() {
  if (DataFile != 0 && NumberOfEvents != 0) {
    if (DecodeAllEventsFlag) {
      std::cout << "You chose to decode all events." << std::endl;
    } else {
      std::cout << "You chose to decode " << NumberOfEvents <<  " events." << std::endl;
    }
    std::cout << std::endl;

    if (FilterFlag) {
      WaveformFilter = new GSFilter(NumberOfFilteringPoints, FilteringDegree);
      #ifdef VerboseMode
        WaveformFilter->PrintMatrices();
      #endif
    }

    AccessTimeHeader();
    AccessEventHeader();
  }

  return;
}

void Decode::AccessTimeHeader() {
  fread(&TimeHeader, sizeof(TimeHeader), 1, DataFile);
  std::cout << "Found data for board number " << TimeHeader.BoardSerialNumber << "." << std::endl << std::endl;

  // Read time bin widths
  for (int ChannelID = 0; ChannelID < 5; ChannelID++) {
    fread(Header, sizeof(Header), 1, DataFile);
    if (Header[0] != 'C') {
      // Event header found
      fseek(DataFile, -4, SEEK_CUR);
      break;
    }
    int i = Header[3] - '0' - 1;
    std::cout << "\t - Found timing calibration for channel " << i + 1 << "." << std::endl;
    DataFoundInChannel[i] = true;
    fread(&TimeBinWidth[i][0], sizeof(float), 1024, DataFile);
  }
  std::cout << std::endl;

  return;
}

void Decode::AccessEventHeader() {
  WAVEFORM tmpWaveform;
  WAVEFORM tmpRawWaveform;
  ROOTFile = new TFile(ROOTFileName.c_str(), "RECREATE");
  TTree* DataTree = new TTree("DataTree", "DataTree");
  if (DataFoundInChannel[0]) {
    DataTree->Branch("TimeChannel1", tmpWaveform.Time[0], "TimeChannel1[1024]/D");
    DataTree->Branch("WaveformChannel1", tmpWaveform.Waveform[0], "WaveformChannel1[1024]/D");
    DataTree->Branch("RawWaveformChannel1", tmpRawWaveform.Waveform[0], "RawWaveformChannel1[1024]/D");
  }
  if (DataFoundInChannel[1]) {
    DataTree->Branch("TimeChannel2", tmpWaveform.Time[1], "TimeChannel2[1024]/D");
    DataTree->Branch("WaveformChannel2", tmpWaveform.Waveform[1], "WaveformChannel2[1024]/D");
    DataTree->Branch("RawWaveformChannel2", tmpRawWaveform.Waveform[1], "RawWaveformChannel2[1024]/D");
  }
  if (DataFoundInChannel[2]) {
    DataTree->Branch("TimeChannel3", tmpWaveform.Time[2], "TimeChannel3[1024]/D");
    DataTree->Branch("WaveformChannel3", tmpWaveform.Waveform[2], "WaveformChannel3[1024]/D");
    DataTree->Branch("RawWaveformChannel3", tmpRawWaveform.Waveform[2], "RawWaveformChannel3[1024]/D");
  }
  if (DataFoundInChannel[3]) {
    DataTree->Branch("TimeChannel4", tmpWaveform.Time[3], "TimeChannel4[1024]/D");
    DataTree->Branch("WaveformChannel4", tmpWaveform.Waveform[3], "WaveformChannel4[1024]/D");
    DataTree->Branch("RawWaveformChannel4", tmpRawWaveform.Waveform[3], "RawWaveformChannel4[1024]/D");
  }

  for (unsigned int EventID = 0; EventID < NumberOfEvents; EventID++) {
    #ifdef VerboseMode
      std::cout << "Event ID: " << EventID << "." << std::endl;
    #endif
    // Read event header
    if (fread(&EventHeader, sizeof(EventHeader), 1, DataFile) < 1) {
      // Check if this is the end of file
      NumberOfEvents -= 1;
      break;
    } else {
      if (DecodeAllEventsFlag) {
        NumberOfEvents += 1;
      }
    }

    // Reach channel data
    for (int ChannelID = 0; ChannelID < 5; ChannelID++) {
      if (fread(Header, sizeof(Header), 1, DataFile) < 1) {
        break;
      }
      if (Header[0] != 'C') {
        // EveCryo100mV1kall.binnt header found
        fseek(DataFile, -4, SEEK_CUR);
        break;
      }
      int ChannelIndex = Header[3] - '0' - 1;
      fread(Voltage, sizeof(short), 1024, DataFile);

      for (int m = 0; m < 1024; m++) {
        // Convert data to volts
        Waveform[ChannelIndex][m] = (Voltage[m] / 65536. - 0.5);
        // Calculate time for this cell
        Time[ChannelIndex][m] = 0;
        for (int j = 0; j < m; j++) {
          Time[ChannelIndex][m] += TimeBinWidth[ChannelIndex][(j + EventHeader.TriggerCellValue) % 1024];
        }
      }

      if (FilterFlag) {
        WaveformFilter->Filter(&Waveform[ChannelID][0], &FilteredWaveform[ChannelID][0]);
      }
    }

    // Align cell #0 of all channels
    double T1 = Time[0][(1024 - EventHeader.TriggerCellValue) % 1024];
    for (int ChannelID = 1; ChannelID < 4; ChannelID++) {
      double T2 = Time[ChannelID][(1024 - EventHeader.TriggerCellValue) % 1024];
      double DT = T1 - T2;
      for (int l = 0; l < 1024; l++) {
        Time[ChannelID][l] += DT;
      }
    }

    for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
      std::copy(std::begin(Time[ChannelID]), std::end(Time[ChannelID]), std::begin(tmpWaveform.Time[ChannelID]));
    }
    if (FilterFlag) {
      for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
        std::copy(std::begin(FilteredWaveform[ChannelID]), std::end(FilteredWaveform[ChannelID]), std::begin(tmpWaveform.Waveform[ChannelID]));
      }
    }
    if (SaveRawWaveform) {
      for (unsigned int ChannelID = 0; ChannelID < 4; ChannelID++) {
        std::copy(std::begin(Waveform[ChannelID]), std::end(Waveform[ChannelID]), std::begin(tmpRawWaveform.Waveform[ChannelID]));
      }
    }

    DataTree->Fill();

  }

  DataTree->Write(0, TObject::kOverwrite);
  ROOTFile->Close();

  return;
}
