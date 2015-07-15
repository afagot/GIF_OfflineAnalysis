#ifndef ClusterSize_h
#define ClusterSize_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TH1.h>

// Header file for the classes stored in the TTree if any.
#include <vector>

using namespace std;

// Fixed size dimensions of array or collections stored in the TTree if any.

class ClusterSize {

public :

  TTree          *fChain;   //!pointer to the analyzed TTree or TChain
  Int_t           fCurrent; //!current Tree number in a TChain

  // Declaration of leaf types
  Int_t                  EventNumber;
  Int_t                  number_of_hits;
  std::vector<int>       *TDC_channel;
  std::vector<float>     *TDC_TimeStamp;  // For DAQ's ROOT files
  //std::vector<Long64_t>  *TDC_TimeStamp;  // For Isabel's ROOT files

  // List of branches
  TBranch        *b_EventNumber;   //!
  TBranch        *b_number_of_hits;   //!
  TBranch        *b_TDC_channel;   //!
  TBranch        *b_TDC_TimeStamp;   //!

  ClusterSize(TTree *tree=0);
  virtual ~ClusterSize();
  virtual Int_t    Cut(Long64_t entry);
  virtual Int_t    GetEntry(Long64_t entry);
  virtual Long64_t LoadTree(Long64_t entry);
  virtual void     Init(TTree* tree);
  virtual void     Loop(map<int,int> ChannelMap);
  virtual Bool_t   Notify();
  virtual void     Show(Long64_t entry = -1);
  void Initialize();
  void Finalize(string outputFile);

  // Declaration and initialization of global variables.
  float clustering_timeWindow = 30.0;  // Time window used to build candidadte clusters [ns].
  static const int nc = 1;
  static const int np = 3;

  // Declaration of histograms.
  TH1F* h_nHits;
  TH1F* h_TDCChannel;
  TH1F* h_TDCTimeStamp;
  TH1F* h_nClusters[nc][np];
  TH1F* h_ClusterSize[nc][np];
  TH1F* h_nClustersChamber[nc];
  TH1F* h_ClusterSizeChamber[nc];

};

#endif


#ifdef ClusterSize_cxx

ClusterSize::ClusterSize(TTree *tree) : fChain(0) {

  // if parameter tree is not specified (or zero), connect the file
  // used to generate this class and read the Tree.
  if (tree == 0) {
    TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("inputFiles/SourceON_PK-140_Double_A1-A2_Random_220mV_9500V_run20150604173142.root");
    if (!f || !f->IsOpen()) {
      f = new TFile("inputFiles/SourceON_PK-140_Double_A1-A2_Random_220mV_9500V_run20150604173142.root");
    }
    f->GetObject("CollectionTree",tree);
  }
  Init(tree);

}


ClusterSize::~ClusterSize() {

  if (!fChain) return;
  delete fChain->GetCurrentFile();

}


Int_t ClusterSize::GetEntry(Long64_t entry) {

  // Read contents of entry.
  if (!fChain) return 0;
  return fChain->GetEntry(entry);

}


Long64_t ClusterSize::LoadTree(Long64_t entry) {

  // Set the environment to read one entry
  if (!fChain) return -5;
  Long64_t centry = fChain->LoadTree(entry);
  if (centry < 0) return centry;
  if (fChain->GetTreeNumber() != fCurrent) {
    fCurrent = fChain->GetTreeNumber();
    Notify();
  }
  return centry;

}


void ClusterSize::Init(TTree *tree) {

  // The Init() function is called when the selector needs to initialize
  // a new tree or chain. Typically here the branch addresses and branch
  // pointers of the tree will be set.
  // It is normally not necessary to make changes to the generated
  // code, but the routine can be extended by the user if needed.
  // Init() will be called many times when running on PROOF
  // (once per file to be processed).

  // Set object pointer
  TDC_channel = 0;
  TDC_TimeStamp = 0;
  // Set branch addresses and branch pointers
  if (!tree) return;
  fChain = tree;
  fCurrent = -1;
  fChain->SetMakeClass(1);

  fChain->SetBranchAddress("EventNumber", &EventNumber, &b_EventNumber);
  fChain->SetBranchAddress("number_of_hits", &number_of_hits, &b_number_of_hits);
  fChain->SetBranchAddress("TDC_channel", &TDC_channel, &b_TDC_channel);
  fChain->SetBranchAddress("TDC_TimeStamp", &TDC_TimeStamp, &b_TDC_TimeStamp);
  Notify();

}


Bool_t ClusterSize::Notify() {

  // The Notify() function is called when a new file is opened. This
  // can be either for a new TTree in a TChain or when when a new TTree
  // is started when using PROOF. It is normally not necessary to make changes
  // to the generated code, but the routine can be extended by the
  // user if needed. The return value is currently not used.
  return kTRUE;

}


void ClusterSize::Show(Long64_t entry) {

  // Print contents of entry.
  // If entry is not specified, print current entry
  if (!fChain) return;
  fChain->Show(entry);

}


Int_t ClusterSize::Cut(Long64_t entry) {

  // This function may be called from Loop.
  // returns  1 if entry is accepted.
  // returns -1 otherwise.
  return 1;

}

#endif // #ifdef ClusterSize_cxx
