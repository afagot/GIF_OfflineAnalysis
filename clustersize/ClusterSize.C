#define ClusterSize_cxx
#include "ClusterSize.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

using namespace std;


int main(int argc, char *argv[]) {

  // Check if correct number of arguments have been given in the command line.
  if (argc != 3) {
    cout << "*******************************************************" << endl;
    cout << "STOPPING..." << endl;
    cout << "To run this code do: " << endl;
    cout << argv[0] << " inputFile.root outputFile.root" << endl;
    cout << "*******************************************************" << endl;
    return 1;
  } else {

    // Store input and output file names from command line into variables.
    string inputFile = argv[1];
    string outputFile = argv[2];

    // Open input file.
    ifstream file(inputFile.c_str());
    if (file.is_open()) {
      cout << "Reading file: " << inputFile << endl;
    } else {
      cout << "Could not open input file!" << endl;
    }
    file.close();

    // Load data from input file.
    //TChain* c = new TChain("CollectionTree");  // For Isabel's ROOT files 
    TChain* c = new TChain("RAWData");  // For DAQ's ROOT files
    c->Add(argv[1]); 
 
    // Run analysis.
    ClusterSize t(c);
    t.Initialize();
    t.Loop();
    t.Finalize(outputFile);

    return 0;
 
  }

}


// Function to sort vectors of pairs by the first variable.
bool sort_pair(const pair<float,int>& a, const pair<float,int>& b) {
  return (a.first < b.first);
}


void ClusterSize::Initialize() {

  // Initialize histograms.
  h_nHits = new TH1F("h_nHits", "Number of hits per event", 51, -0.5, 50.5);
  h_TDCChannel = new TH1F("h_TDCChannel", "TDC channels", 18, -1.5, 16.5);
  h_TDCTimeStamp = new TH1F("h_TDCTimeStamp", "TDC time stamp", 300, 0.0, 6000.0);
  h_nClusters = new TH1F("h_nClusters", "Number of clusters per event", 51, -0.5, 50.5);
  h_ClusterSize = new TH1F("h_ClusterSize", "Cluster size distribution per event", 5, 0.5, 5.5);

}


void ClusterSize::Finalize(string outputFile) {

  // Write output file with histograms.
  TFile f(outputFile.c_str(), "RECREATE");
  h_nHits->Write("nHits");
  h_TDCChannel->Write("TDCChannel");
  h_TDCTimeStamp->Write("TDCTimeStamp");
  h_nClusters->Write("nClusters");
  h_ClusterSize->Write("ClusterSize");
  f.Close();

}


void ClusterSize::Loop() {

  if (fChain == 0) return;

  // Run in debug mode (i.e. print output statements).
  bool debug = false;

  Long64_t nentries = fChain->GetEntries();
  if(debug) {
    nentries = 100;
  }
  cout << "Number of entries in TTree: " << nentries << endl;

  int nevents = 0;

  for (Long64_t jentry=0; jentry<nentries; jentry++) {

    GetEntry(jentry);

    // Sanity checks on the quality of the data.
    if (EventNumber<=0) continue;  // #1: Veto events in which EventNumber is set to -99 as data was probably not read properly by the buffer.
    if (TDC_channel->size() != TDC_TimeStamp->size()) continue;  // #2: Veto events in which TDC_* vectors have different sizes.
    if (number_of_hits != TDC_channel->size()) continue;  // #3: Veto events in which the size of TDC_* vectors differs from the value of number_of_hits.

    // Good-event counter.
    nevents++;

    // Fill histograms for tree variables.
    h_nHits->Fill(number_of_hits);
    for (int i=0; i<number_of_hits; i++) {
      h_TDCChannel->Fill(TDC_channel->at(i));
      h_TDCTimeStamp->Fill(TDC_TimeStamp->at(i));
    }

    if (debug) {
      cout << "=== New Event ===" << endl;
      cout << "-----> Number of Hits: " << number_of_hits << endl;
    }    

    // Create a vector of pairs with TDC information (time stamp and channel) and sort it by time stamp.
    vector<pair<float,int>> TDCInfo;
    TDCInfo.reserve(TDC_TimeStamp->size());
    for (int i=0; i<number_of_hits; i++) {
      TDCInfo.push_back(make_pair(TDC_TimeStamp->at(i),TDC_channel->at(i)));
    }
    sort(TDCInfo.begin(), TDCInfo.end(), sort_pair);

    if (debug) {
      cout << "*** Hits sorted by time stamp ***" << endl;
      for (int j=0; j<number_of_hits; j++) {
        cout << "TDCInfo Vector => Hit: " << j << " TimeStamp: " << TDCInfo[j].first << " Channel: " << TDCInfo[j].second << endl;
      }
    }

    // Loop over hits sorted by time stamp. Build cluster candidates with all hits found within a 30ns time window.
    // Store the first and last hits inside this time window. 
    int nCandidates = 0;
    int lastHitInWindow = 0;
    bool newCandidate = false;
    bool singleHit = false;
    vector<int> firstHitsInCandidates;
    vector<int> lastHitsInCandidates;
    firstHitsInCandidates.clear();
    lastHitsInCandidates.clear();
    for (int i=0; i<(number_of_hits-1); i++) {
      float timeWindow = 30.0;
      if (newCandidate) {
        if (i!=(lastHitInWindow+1)) continue;
        timeWindow = TDCInfo[lastHitInWindow+1].first + 30.0;
        lastHitInWindow = lastHitInWindow + 1; 
      } else {
        lastHitInWindow = i;
        timeWindow = TDCInfo[lastHitInWindow].first + 30.0;
      }
      for (int j=lastHitInWindow+1; j<number_of_hits; j++) {
        if (TDCInfo[j].first < timeWindow) {
          lastHitInWindow = j;
        } else {
          break;
        }
      }
      if (lastHitInWindow != i) {
        newCandidate = true;
        nCandidates++;
      } else {
        newCandidate = false;
        singleHit = true;
      }
      if (newCandidate) {
        firstHitsInCandidates.push_back(i);
        lastHitsInCandidates.push_back(lastHitInWindow);  
      }
    }

    if (debug) {
      cout << "*** Candidate Clusters ***" << endl;
      for (int i=0; i<firstHitsInCandidates.size(); i++) {
        cout << "Candidate " << i << " => First Hit: " << firstHitsInCandidates.at(i) << ". Last Hit: " << lastHitsInCandidates.at(i) << endl;      
      }
    }

    // Fill vectors with cluster candidate number, TDC channel and TDC time stamp.
    vector<int> candidateNumber;
    vector<int> channelsInCandidate;
    vector<float> timeStampsInCandidate;
    candidateNumber.clear();
    channelsInCandidate.clear();
    timeStampsInCandidate.clear();
    for (int n=0; n<nCandidates; n++) {
      for (int i=0; i<number_of_hits; i++) {
        if ( i>=firstHitsInCandidates.at(n) && i<=lastHitsInCandidates.at(n) ) {
          candidateNumber.push_back(n);
          channelsInCandidate.push_back(TDCInfo[i].second);
          timeStampsInCandidate.push_back(TDCInfo[i].first);
        }
      }
    } 
    
    // Loop over cluster candidates. For each candidate create a vector pair with TDC information (channel and time stamp) and sort it by channel number.
    // Count the number of clusters with size >=2.
    // Count the number of channels inside clusters of size >=2.
    // If size of candidate is 2: 
    // => If channels are not consecutive, discard the cluster candidate.
    // => If channels are consecutive, fill cluster size histogram and increase counter for number of channels inside clusters of size >=2.
    // If size of candidate is >2:
    // => Store in a vector the channel numbers of consecutive channels (making sure not to double-count). The size of this vector is the cluster size.
    // => If there are at least two consecutive channels, we have a cluster. Fill cluster size histogram with size of vector and increase counter.
    // => If there are no consecutive channels, we discard the cluster candidate.
    int tmp = 0;
    int nClusters = nCandidates;
    int channelsInClusters = 0;
    for (int n=0; n<nCandidates; n++) {
      int isize = count(candidateNumber.begin(), candidateNumber.end(), n);
      vector<pair<int,float>> candidateByChannel;
      candidateByChannel.reserve(candidateNumber.size());
      candidateByChannel.clear();
      for (int i=0; i<isize; i++) {
        candidateByChannel.push_back(make_pair(channelsInCandidate.at(tmp+i),timeStampsInCandidate.at(tmp+i)));
      }
      sort(candidateByChannel.begin(), candidateByChannel.end(), sort_pair);
      if (debug) {
        cout << "*** Candidate cluster " << n << " sorted by channel number ***" << endl;
        cout << "Size of candidate: " << candidateByChannel.size() << endl;
        for (int i=0; i<candidateByChannel.size(); i++) {
          cout << "Channel: " << candidateByChannel[i].first << " Time Stamp: " << candidateByChannel[i].second << endl;
        }
      } 
      if (isize==2) {
        if ( (candidateByChannel[1].first - candidateByChannel[0].first) != 1 ) {
          if (debug) {
            cout << "Bad cluster. Only two channels and none consecutive." << endl;
          }
          nClusters = nClusters - 1;
        } else {
          h_ClusterSize->Fill(2);
          channelsInClusters = channelsInClusters + 2;
        } 
      } else {
        bool goodCluster = false;
        int nConsecutiveChannels = 0;
        vector<int> consecutiveChannels;
        consecutiveChannels.clear();
        for (int j=0; j<(isize-1); j++) {
          if ( (candidateByChannel[j+1].first - candidateByChannel[j].first) != 1 ) {
            // Do nothing.
            if (debug) {
              cout << "Channels " << candidateByChannel[j+1].first << " and " << candidateByChannel[j].first << " are not consecutive!" << endl;
            }
          } else {
            if (consecutiveChannels.empty()) {
              consecutiveChannels.push_back(candidateByChannel[j].first);
              consecutiveChannels.push_back(candidateByChannel[j+1].first);
            } else {
              if ( find(consecutiveChannels.begin(), consecutiveChannels.end(), candidateByChannel[j].first) != consecutiveChannels.end() ) {
                // Do nothing.
              } else {
                consecutiveChannels.push_back(candidateByChannel[j].first);
              }
              if ( find(consecutiveChannels.begin(), consecutiveChannels.end(), candidateByChannel[j+1].first) != consecutiveChannels.end() ) {
                // Do nothing.
              } else {
                consecutiveChannels.push_back(candidateByChannel[j+1].first);
              }
            }
            nConsecutiveChannels++;
          }
        }
        if (nConsecutiveChannels >= 1) {
          goodCluster = true;
        }
        if (goodCluster) {
          if (debug) {
            cout << "Good cluster with at least 2 consecutive channels!" << endl;
            cout << "Cluster size: " << consecutiveChannels.size() << endl;
          }
          h_ClusterSize->Fill(consecutiveChannels.size());
          channelsInClusters = channelsInClusters + consecutiveChannels.size();
        } else {
          nClusters = nClusters - 1;
          if (debug) {
            cout << "Bad cluster. No consecutive channels found." << endl;
          }
        }
      } 
      tmp += isize;
    }
    if (debug) {
      cout << "*** Event statistics ***" << endl;
      cout << "Number of clusters with size >=2: " << nClusters << endl;
      cout << "Number of channels in clusters: " << channelsInClusters << endl;
    }

    // Count the number of single-channel clusters. Fill the cluster size histogram.
    int singleChannels = number_of_hits - channelsInClusters;
    for (int j=1; j<=singleChannels; j++) {
      h_ClusterSize->Fill(1);
    }
    if (debug) {
      cout << "Number of single channels: " << singleChannels << endl;
    }    

    // Count the total number of clusters in the event. Fill the number of clusters histogram.
    int totalClusters = singleChannels + nClusters;
    h_nClusters->Fill(totalClusters);
    if (debug) {
      cout << "Total number of clusters: " << totalClusters << endl;
    }

  }

  cout << "Number of events: " << nevents << endl;

}
