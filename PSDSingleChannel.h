#ifndef PSDSingleChannel_h
#define PSDSingleChannel_h

class PSDSingleChannel {
public:

  TTree *fChain;  // !pointer to the analyzed TTree or TChain
  Int_t fCurrent; // !current Tree number in a TChain

  // Declaration of leaf types
  Double_t TimeChannel1[1024];
  Double_t WaveformChannel1[1024];
  Double_t RawWaveformChannel1[1024];
  Double_t DerivativeWaveformChannel1[1024];

  // List of branches
  TBranch *b_TimeChannel1;        // !
  TBranch *b_WaveformChannel1;    // !
  TBranch *b_RawWaveformChannel1; // !
  TBranch *b_DerivativeWaveformChannel1;

  PSDSingleChannel(TTree *tree = 0);
  virtual ~PSDSingleChannel();
  virtual Int_t    GetEntry(Long64_t entry);
  virtual Long64_t LoadTree(Long64_t entry);
  virtual void     Init(TTree *tree);
  virtual void     Loop();
};

#endif
