#define ClusterSize_cxx
#include "ClusterSize.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <map>

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

    // Load Channel=>Strip mapping file and build map.
    map<int,int> ChannelMap;
    ifstream mappingFile("ChannelsMapping.csv",ios::in);
    if (mappingFile.is_open()) {
      cout << "Channel-to-strip mapping file successfully loaded!" << endl;
      while (mappingFile.good()) {
        int stripNumber;
        int channelNumber;
        mappingFile >> stripNumber >> channelNumber;
        ChannelMap[channelNumber] = stripNumber;
      }
    } else {
      cout << "STOPPING..." << endl;
      cout << "Could not open channel-to-strip mapping file!" << endl;
      return 1;
    }
    mappingFile.close();

    // Run analysis.
    ClusterSize t(c);
    t.Initialize();
    t.Loop(ChannelMap);
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
  h_nClusters[0] = new TH1F("h_A_nClusters", "Number of clusters per event in partition A", 51, -0.5, 50.5);
  h_ClusterSize[0] = new TH1F("h_A_ClusterSize", "Cluster size distribution per event in partition A", 5, 0.5, 5.5);
  h_nClusters[1] = new TH1F("h_B_nClusters", "Number of clusters per event in partition B", 51, -0.5, 50.5);
  h_ClusterSize[1] = new TH1F("h_B_ClusterSize", "Cluster size distribution per event in partition B", 5, 0.5, 5.5);
  h_nClusters[2] = new TH1F("h_C_nClusters", "Number of clusters per event in partition C", 51, -0.5, 50.5);
  h_ClusterSize[2] = new TH1F("h_C_ClusterSize", "Cluster size distribution per event in partition C", 5, 0.5, 5.5);
  h_nClustersChamber = new TH1F("h_nClustersChamber", "Number of clusters per event in chamber", 51, -0.5, 50.5);
  h_ClusterSizeChamber = new TH1F("h_ClusterSizeChamber", "Cluster size distribution per event in chamber", 5, 0.5, 5.5);

}


void ClusterSize::Finalize(string outputFile) {

  // Write output file with histograms.
  TFile f(outputFile.c_str(), "RECREATE");
  h_nHits->Write("nHits");
  h_TDCChannel->Write("TDCChannel");
  h_TDCTimeStamp->Write("TDCTimeStamp");
  h_nClusters[0]->Write("nClusters_partA");
  h_ClusterSize[0]->Write("ClusterSize_partA");
  h_nClusters[1]->Write("nClusters_partB");
  h_ClusterSize[1]->Write("ClusterSize_partB");
  h_nClusters[2]->Write("nClusters_partC");
  h_ClusterSize[2]->Write("ClusterSize_partC");
  h_nClustersChamber->Write("nClusters_chamber");
  h_ClusterSizeChamber->Write("ClusterSize_chamber"); 
  f.Close();

}


void ClusterSize::Loop(map<int,int> ChannelMap) {

  if (fChain == 0) return;

  // Run in debug mode (i.e. print output statements).
  bool debug = false;

  if (debug) {
    cout << "**********************************************" << endl;
    cout << "*** Strip Number => TDC Channel Number Map ***" << endl;
    for (map<int,int>::const_iterator it = ChannelMap.begin(); it != ChannelMap.end(); ++it) {
      cout << it->first << " " << it->second << endl;
    }
    cout << "**********************************************" << endl;
  }

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

    // Separate hits into partitions.
    int nhits[3] = {0, 0, 0};
    vector<int> channel[3];
    channel[0].clear();
    channel[1].clear();
    channel[2].clear();
    vector<int> timestamp[3];
    timestamp[0].clear();
    timestamp[1].clear();
    timestamp[2].clear();
    for (int i=0; i<number_of_hits; i++) {
      if (TDC_channel->at(i)>=0 && TDC_channel->at(i)<32) {
        // Partition A.
        nhits[0]++;
        channel[0].push_back(TDC_channel->at(i));
        timestamp[0].push_back(TDC_TimeStamp->at(i));
      } else if (TDC_channel->at(i)>=32 && TDC_channel->at(i)<64) {
        // Partition B.
        nhits[1]++;
        channel[1].push_back(TDC_channel->at(i));
        timestamp[1].push_back(TDC_TimeStamp->at(i));
      } else if (TDC_channel->at(i)>=64 && TDC_channel->at(i)<96) {
        // Partition C.
        nhits[2]++;
        channel[2].push_back(TDC_channel->at(i));
        timestamp[2].push_back(TDC_TimeStamp->at(i));
      }
    }

    if (debug) {
      cout << "=== New Event ===" << endl;
      cout << "-----> Number of Hits: " << number_of_hits << endl;
      cout << "---> NUmber of Hits in Partition A: " << nhits[0] << endl;
      cout << "---> NUmber of Hits in Partition B: " << nhits[1] << endl;
      cout << "---> NUmber of Hits in Partition C: " << nhits[2] << endl;
    }    

    // Loop over partitions.
    for (int p=0; p<3; p++) {

      // Create a vector of pairs with TDC time stamp and strip number and sort it by time stamp.
      vector<pair<float,int>> HitInfo;
      HitInfo.clear();
      HitInfo.reserve(timestamp[p].size());
      for (int i=0; i<nhits[p]; i++) {
        HitInfo.push_back(make_pair(timestamp[p].at(i),ChannelMap[channel[p].at(i)]));
      }
      sort(HitInfo.begin(), HitInfo.end(), sort_pair);

      if (debug) {
        cout << "*** Hits in event (before sorting) ***" << endl;
        for (int j=0; j<nhits[p]; j++) {
          cout << "Hit: " << j << " TimeStamp: " << timestamp[p].at(j) << " Channel: " << channel[p].at(j) 
               << " Strip: " << ChannelMap[channel[p].at(j)] << endl;
        }
        cout << "*** Hits sorted by time stamp ***" << endl;
        for (int j=0; j<nhits[p]; j++) {
          cout << "HitInfo Vector => Hit: " << j << " TimeStamp: " << HitInfo[j].first << " Strip: " << HitInfo[j].second << endl;
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
      for (int i=0; i<(nhits[p]-1); i++) {
        float timeWindow = 30.0;
        if (newCandidate) {
          if (i!=(lastHitInWindow+1)) continue;
          timeWindow = HitInfo[lastHitInWindow+1].first + 30.0;
          lastHitInWindow = lastHitInWindow + 1; 
        } else {
          lastHitInWindow = i;
          timeWindow = HitInfo[lastHitInWindow].first + 30.0;
        }
        for (int j=lastHitInWindow+1; j<nhits[p]; j++) {
          if (HitInfo[j].first < timeWindow) {
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

      // Fill vectors with cluster candidate number, strip number and TDC time stamp.
      vector<int> candidateNumber;
      vector<int> stripsInCandidate;
      vector<float> timeStampsInCandidate;
      candidateNumber.clear();
      stripsInCandidate.clear();
      timeStampsInCandidate.clear();
      for (int n=0; n<nCandidates; n++) {
        for (int i=0; i<nhits[p]; i++) {
          if ( i>=firstHitsInCandidates.at(n) && i<=lastHitsInCandidates.at(n) ) {
            candidateNumber.push_back(n);
            stripsInCandidate.push_back(HitInfo[i].second);
            timeStampsInCandidate.push_back(HitInfo[i].first);
          }
        }
      } 
    
      // Loop over cluster candidates. For each candidate create a vector pair with strip number and time stamp and sort it by strip number.
      // Count the number of clusters with size >=2.
      // Count the number of strips inside clusters of size >=2.
      // If size of candidate is 2: 
      // => If strips are not consecutive, discard the cluster candidate.
      // => If strips are consecutive, fill cluster size histogram and increase counter for number of strips inside clusters of size >=2.
      // If size of candidate is >2:
      // => Store in a vector the strip numbers of consecutive strips (making sure not to double-count). The size of this vector is the cluster size.
      // => If there are at least two consecutive strips, we have a cluster (single hits are taken into account later). 
      //    Fill cluster size histogram with size of vector and increase counter.
      // => If there are no consecutive strips, we discard the cluster candidate.
      int tmp = 0;
      int nClusters = nCandidates;
      int stripsInClusters = 0;
      for (int n=0; n<nCandidates; n++) {
        int isize = count(candidateNumber.begin(), candidateNumber.end(), n);
        vector<pair<int,float>> candidateByStrip;
        candidateByStrip.reserve(candidateNumber.size());
        candidateByStrip.clear();
        for (int i=0; i<isize; i++) {
          candidateByStrip.push_back(make_pair(stripsInCandidate.at(tmp+i),timeStampsInCandidate.at(tmp+i)));
        }
        sort(candidateByStrip.begin(), candidateByStrip.end(), sort_pair);
        if (debug) {
          cout << "*** Candidate cluster " << n << " sorted by strip number ***" << endl;
          cout << "Size of candidate: " << candidateByStrip.size() << endl;
          for (int i=0; i<candidateByStrip.size(); i++) {
            cout << "Strip Number: " << candidateByStrip[i].first << " Time Stamp: " << candidateByStrip[i].second << endl;
          }
        } 
        if (isize==2) {
          if ( (candidateByStrip[1].first - candidateByStrip[0].first) != 1 ) {
            if (debug) {
              cout << "Bad cluster. Only two strips and none consecutive." << endl;
            }
            nClusters = nClusters - 1;
          } else {
            h_ClusterSize[p]->Fill(2);
            h_ClusterSizeChamber->Fill(2);
            stripsInClusters = stripsInClusters + 2;
          } 
        } else {
          bool goodCluster = false;
          int nConsecutiveStrips = 0;
          vector<int> consecutiveStrips;
          consecutiveStrips.clear();
          for (int j=0; j<(isize-1); j++) {
            if ( (candidateByStrip[j+1].first - candidateByStrip[j].first) != 1 ) {
              // Do nothing.
              if (debug) {
                cout << "Strips " << candidateByStrip[j+1].first << " and " << candidateByStrip[j].first << " are not consecutive!" << endl;
              }
            } else {
              if (consecutiveStrips.empty()) {
                consecutiveStrips.push_back(candidateByStrip[j].first);
                consecutiveStrips.push_back(candidateByStrip[j+1].first);
              } else {
                if ( find(consecutiveStrips.begin(), consecutiveStrips.end(), candidateByStrip[j].first) != consecutiveStrips.end() ) {
                  // Do nothing.
                } else {
                  consecutiveStrips.push_back(candidateByStrip[j].first);
                }
                if ( find(consecutiveStrips.begin(), consecutiveStrips.end(), candidateByStrip[j+1].first) != consecutiveStrips.end() ) {
                  // Do nothing.
                } else {
                  consecutiveStrips.push_back(candidateByStrip[j+1].first);
                }
              }
              nConsecutiveStrips++;
            }
          }
          if (nConsecutiveStrips >= 1) {
            goodCluster = true;
          }
          if (goodCluster) {
            if (debug) {
              cout << "Good cluster with at least 2 consecutive strips!" << endl;
              cout << "Cluster size: " << consecutiveStrips.size() << endl;
            }
            h_ClusterSize[p]->Fill(consecutiveStrips.size());
            h_ClusterSizeChamber->Fill(consecutiveStrips.size());
            stripsInClusters = stripsInClusters + consecutiveStrips.size();
          } else {
            nClusters = nClusters - 1;
            if (debug) {
              cout << "Bad cluster. No consecutive strips found." << endl;
            }
          }
        } 
        tmp += isize;
      }
      if (debug) {
        cout << "*** Event statistics ***" << endl;
        cout << "Number of clusters with size >=2: " << nClusters << endl;
        cout << "Number of strips in clusters: " << stripsInClusters << endl;
      }

      // Count the number of single-strip clusters. Fill the cluster size histogram.
      int singleStrips = nhits[p] - stripsInClusters;
      for (int j=1; j<=singleStrips; j++) {
        h_ClusterSize[p]->Fill(1);
        h_ClusterSizeChamber->Fill(1);
      }
      if (debug) {
        cout << "Number of single strips: " << singleStrips << endl;
      }      

      // Count the total number of clusters in the event. Fill the number of clusters histogram.
      int totalClusters = singleStrips + nClusters;
      if(p == 0) {  // Temporary hack while we read data from more than one partition.
        h_nClusters[p]->Fill(totalClusters);
        h_nClustersChamber->Fill(totalClusters);
      }
      if (debug) {
        cout << "=> Total number of clusters: " << totalClusters << endl;
      }

    }

  }

  cout << "Number of events: " << nevents << endl;

}
