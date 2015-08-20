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
#include <cstdlib>

#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

using namespace std;


int main(int argc, char *argv[]) {

  // Check if correct number of arguments have been given in the command line.
  if (argc != 2) {
    cout << "*******************************************************" << endl;
    cout << "STOPPING..." << endl;
    cout << "To run this code do: " << endl;
    cout << argv[0] << " inputFile.root" << endl;
    cout << "*******************************************************" << endl;
    return 1;
  } else {

    // Store input file name from command line into variable.
    string inputFile = argv[1];

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
    string fileName = inputFile.substr(inputFile.find_last_of("/")+1);
    size_t pos = fileName.find("run");
    string runNumber;
    if (pos != string::npos) {
      runNumber = fileName.substr(pos+3,fileName.find(".root",pos)-(pos+3));
    }
    map<int,int> ChannelMap;
    string mappingFileName;
    long int run = strtol(runNumber.c_str(),NULL,10);
    //ifstream mappingFile("ChannelsMapping.csv",ios::in);  // Mapping file for Trolley 0.
    if (run >= 20150718000329 && run <= 20150719180236) {  // Old mapping file for Trolley 1.
      mappingFileName = "ChannelsMapping_Trolley1_4TDCs.csv";
    } else if (run >= 20150719184203) {  // New mapping file for Trolley 1.
      mappingFileName = "ChannelsMapping_Trolley1.csv";
    }
    ifstream mappingFile(mappingFileName.c_str(),ios::in);
    if (mappingFile.is_open()) {
      cout << "Channel-to-strip mapping file " << mappingFileName << " successfully loaded!" << endl;
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
    t.Initialize(inputFile);
    t.Loop(ChannelMap);
    t.Finalize(inputFile);

    return 0;
 
  }

}


// Function to sort vectors of pairs by the first variable.
bool sort_pair(const pair<float,int>& a, const pair<float,int>& b) {
  return (a.first < b.first);
}


void ClusterSize::Initialize(string inputFile) {

  // Use input file name to determine whether the data was collected using a muon or a random trigger.
  string fileName = inputFile.substr(inputFile.find_last_of("/")+1);
  size_t pos = fileName.find("Beam");
  if (pos != string::npos) {
    muonTrigger = true;
    cout << "Trigger: MUON" << endl;
    size_t pos_trig = fileName.find("Near");
    if (pos_trig != string::npos) {
      trigger_cut_min = 240.0;
      trigger_cut_max = 340.0;
    }
    cout << "Window: [" << trigger_cut_min << " ns , " << trigger_cut_max << " ns]" << endl; 
  } else {
    muonTrigger = false;
    cout << "Trigger: RANDOM" << endl;
  }

  // Initialize histograms.
  h_nHits = new TH1F("h_nHits", "Number of hits per event", 101, -0.5, 100.5);
  h_TDCChannel = new TH1F("h_TDCChannel", "TDC channels", 2128, -0.5, 2127.5);
  h_TDCTimeStamp = new TH1F("h_TDCTimeStamp", "TDC time stamp", 300, 0.0, 6000.0);
  for (int c=0; c<nc; c++) {
    string pname;
    for (int p=0; p<np; p++) {
      if (p==0) {
        pname = "A";
      } else if (p==1) {
        pname = "B";
      } else if (p==2) {
        pname = "C";
      }
      string nClusters_w0bin_part_hname = "h_nClusters_w0bin_"+to_string(c)+pname;
      string nClusters_part_hname = "h_nClusters_"+to_string(c)+pname;
      string ClusterSize_part_hname = "h_ClusterSize_"+to_string(c)+pname;
      string nClusters_w0bin_part_htitle = "Number of clusters (inc. 0bin) per event in partition "+to_string(c)+pname;
      string nClusters_part_htitle = "Number of clusters per event in partition "+to_string(c)+pname;
      string ClusterSize_part_htitle = "Cluster size distribution per event in partition "+to_string(c)+pname; 
      h_nClusters_w0bin[c][p] = new TH1F(nClusters_w0bin_part_hname.c_str(), nClusters_w0bin_part_htitle.c_str(), 101, -0.5, 100.5);
      h_nClusters[c][p] = new TH1F(nClusters_part_hname.c_str(), nClusters_part_htitle.c_str(), 101, -0.5, 100.5);
      h_ClusterSize[c][p] = new TH1F(ClusterSize_part_hname.c_str(), ClusterSize_part_htitle.c_str(), 50, 0.5, 50.5);
    }
    string nClusters_w0bin_hname = "h_nClustersChamber_w0bin_"+to_string(c);
    string nClusters_hname = "h_nClustersChamber_"+to_string(c);
    string ClusterSize_hname = "h_ClusterSize_"+to_string(c);
    string nClusters_w0bin_htitle = "Number of clusters (inc. 0bin) per event in chamber "+to_string(c);
    string nClusters_htitle = "Number of clusters per event in chamber "+to_string(c);
    string ClusterSize_htitle = "Cluster size distribution per event in chamber "+to_string(c);
    h_nClustersChamber_w0bin[c] = new TH1F(nClusters_w0bin_hname.c_str(), nClusters_w0bin_htitle.c_str(), 101, -0.5, 100.5);
    h_nClustersChamber[c] = new TH1F(nClusters_hname.c_str(), nClusters_htitle.c_str(), 101, -0.5, 100.5);
    h_ClusterSizeChamber[c] = new TH1F(ClusterSize_hname.c_str(), ClusterSize_htitle.c_str(), 50, 0.5, 50.5);
  }

}


void ClusterSize::Finalize(string fileName) {

  // Write output file with histograms.
  string baseName;
  baseName = fileName.erase(fileName.find_last_of("."));
  string outputFile = "AnalysedData/"+baseName.substr(baseName.find_last_of("/")+1)+"-Offline_Cluster_Size.root";
  TFile f(outputFile.c_str(), "RECREATE");
  h_nHits->Write("nHits");
  h_TDCChannel->Write("TDCChannel");
  h_TDCTimeStamp->Write("TDCTimeStamp");
  for (int c=0; c<nc; c++) {
    string pname;
    for (int p=0; p<np; p++) {
      if (p==0) {
        pname = "A";
      } else if (p==1) {
        pname = "B";
      } else if (p==2) {
        pname = "C";
      }
      string nClusters_w0bin_part_hname = "nClusters_w0bin_"+to_string(c)+pname;
      string nClusters_part_hname = "nClusters_"+to_string(c)+pname;
      string ClusterSize_part_hname = "ClusterSize_"+to_string(c)+pname;
      h_nClusters_w0bin[c][p]->Write(nClusters_w0bin_part_hname.c_str());
      h_nClusters[c][p]->Write(nClusters_part_hname.c_str());
      h_ClusterSize[c][p]->Write(ClusterSize_part_hname.c_str());
    }
    string nClusters_w0bin_hname = "nClusters_w0bin_"+to_string(c);
    string nClusters_hname = "nClusters_"+to_string(c);
    string ClusterSize_hname = "ClusterSize_"+to_string(c);
    h_nClustersChamber_w0bin[c]->Write(nClusters_w0bin_hname.c_str());
    h_nClustersChamber[c]->Write(nClusters_hname.c_str());
    h_ClusterSizeChamber[c]->Write(ClusterSize_hname.c_str()); 
  }
  f.Close();

  // Write output csv file with mean values.
  // For the nClusters histogram, print only the mean values without the 0bin.
  string fName = baseName.substr(baseName.find_last_of("/")+1);
  string voltageS1;
  string voltageS2;
  string voltageS3;
  string voltageS4;
  string attUP;
  string attDOWN;
  size_t pos_S1S2 = fName.find("S1S2-");
  size_t pos_S3S4 = fName.find("S3S4-");
  size_t pos_S1 = fName.find("S1-");
  size_t pos_S2 = fName.find("S2-");
  size_t pos_S3 = fName.find("S3-");
  size_t pos_S4 = fName.find("S4-");
  size_t pos_UP = fName.find("UP");
  size_t pos_DOWN = fName.find("DOWN");
  if (pos_S1S2 != string::npos) {
    voltageS1 = fName.substr(pos_S1S2+5,fName.find("_",pos_S1S2)-(pos_S1S2+5));
    voltageS2 = fName.substr(pos_S1S2+5,fName.find("_",pos_S1S2)-(pos_S1S2+5));
  }
  if (pos_S3S4 != string::npos) {
    voltageS3 = fName.substr(pos_S3S4+5,fName.find("V_",pos_S3S4)-(pos_S3S4+5));
    voltageS4 = fName.substr(pos_S3S4+5,fName.find("V_",pos_S3S4)-(pos_S3S4+5));
  } 
  if (pos_S1 != string::npos) {
    voltageS1 = fName.substr(pos_S1+3,fName.find("_",pos_S1)-(pos_S1+3));
  }
  if (pos_S2 != string::npos) {
    voltageS2 = fName.substr(pos_S2+3,fName.find("_",pos_S2)-(pos_S2+3));
  }
  if (pos_S3 != string::npos) {
    voltageS3 = fName.substr(pos_S3+3,fName.find("_",pos_S3)-(pos_S3+3));
  }
  if (pos_S4 != string::npos) {
    voltageS4 = fName.substr(pos_S4+3,fName.find("V_",pos_S4)-(pos_S4+3));
  }
  if (pos_UP != string::npos) {
    attUP = fName.substr(pos_UP+2,fName.find("-",pos_UP)-(pos_UP+2));
    attDOWN = fName.substr(pos_DOWN+4,fName.find("_",pos_DOWN)-(pos_DOWN+4));
  } else {
    attUP = "None";
    attDOWN = "None";
  }
  string outputCsvFile = "AnalysedData/"+baseName.substr(baseName.find_last_of("/")+1)+"-Offline_Cluster_Size.csv";
  ofstream outputCSV(outputCsvFile.c_str(), ofstream::out);
  outputCSV << "INFO " << baseName.substr(baseName.find_last_of("/")+1) << '\n';
  outputCSV << "INFO Voltage  AttUP  AttDOWN  Chamber  Partition  <nClusters>  <nClusters>_err  <ClusterSize>  <ClusterSize>_err" << '\n';
  string cname;
  string voltage;
  for (int c=0; c<nc; c++) {
    string pname;
    if (c==0) {
      cname = "S4";
      voltage = voltageS4;
    } else if (c==1) {
      cname = "S3";
      voltage = voltageS3;
    } else if (c==2) {
      cname = "S2";
      voltage = voltageS2;
    } else if (c==3) {
      cname = "S1";
      voltage = voltageS1;
    }
    for (int p=0; p<np; p++) {
      if (p==0) {
        pname = "A";
      } else if (p==1) {
        pname = "B";
      } else if (p==2) {
        pname = "C";
      }
      outputCSV << voltage << " " << attUP << " " << attDOWN << " " << cname << " " << pname << " " << h_nClusters[c][p]->GetMean() 
                << " " << h_nClusters[c][p]->GetMeanError() << " " << h_ClusterSize[c][p]->GetMean() << " " << h_ClusterSize[c][p]->GetMeanError() << '\n';
    }
    outputCSV << voltage << " " << attUP << " " << attDOWN << " " << cname << " " << "all" << " " << h_nClustersChamber[c]->GetMean() 
              << " " << h_nClustersChamber[c]->GetMeanError() << " " << h_ClusterSizeChamber[c]->GetMean() << " " 
              << h_ClusterSizeChamber[c]->GetMeanError() << '\n';
  }
  outputCSV << '\n';
  outputCSV.close();

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
    int nhits_all = 0;
    for (int i=0; i<number_of_hits; i++) {
      // Applying trigger requirement.
      if (muonTrigger==true && (TDC_TimeStamp->at(i) < trigger_cut_min || TDC_TimeStamp->at(i) > trigger_cut_max)) continue;
      nhits_all++;
      h_TDCChannel->Fill(TDC_channel->at(i));
      h_TDCTimeStamp->Fill(TDC_TimeStamp->at(i));
    }
    h_nHits->Fill(nhits_all);

    // Loop over chambers.
    for (int c=0; c<nc; c++) {

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
        // Applying trigger requirement.
        if (muonTrigger==true && (TDC_TimeStamp->at(i) < trigger_cut_min || TDC_TimeStamp->at(i) > trigger_cut_max)) continue;
        if (c==0) {
          // Chamber S4: Strips 000 to 095.
          if (ChannelMap[TDC_channel->at(i)]<0 || ChannelMap[TDC_channel->at(i)]>95) continue;
          if (ChannelMap[TDC_channel->at(i)]>=0 && ChannelMap[TDC_channel->at(i)]<32) {
            // Partition A.
            nhits[0]++;
            channel[0].push_back(TDC_channel->at(i));
            timestamp[0].push_back(TDC_TimeStamp->at(i));
          } else if (ChannelMap[TDC_channel->at(i)]>=32 && ChannelMap[TDC_channel->at(i)]<64) {
            // Partition B.
            nhits[1]++;
            channel[1].push_back(TDC_channel->at(i));
            timestamp[1].push_back(TDC_TimeStamp->at(i));
          } else if (ChannelMap[TDC_channel->at(i)]>=64 && ChannelMap[TDC_channel->at(i)]<96) {
            // Partition C.
            nhits[2]++;
            channel[2].push_back(TDC_channel->at(i));
            timestamp[2].push_back(TDC_TimeStamp->at(i));
          }
        } else if (c==1) {
          // Chamber S3: Strips 100 to 195.
          if (ChannelMap[TDC_channel->at(i)]<100 || ChannelMap[TDC_channel->at(i)]>195) continue;
          if (ChannelMap[TDC_channel->at(i)]>=100 && ChannelMap[TDC_channel->at(i)]<132) {
            // Partition A.
            nhits[0]++;
            channel[0].push_back(TDC_channel->at(i));
            timestamp[0].push_back(TDC_TimeStamp->at(i));
          } else if (ChannelMap[TDC_channel->at(i)]>=132 && ChannelMap[TDC_channel->at(i)]<164) {
            // Partition B.
            nhits[1]++;
            channel[1].push_back(TDC_channel->at(i));
            timestamp[1].push_back(TDC_TimeStamp->at(i));
          } else if (ChannelMap[TDC_channel->at(i)]>=164 && ChannelMap[TDC_channel->at(i)]<196) {
            // Partition C.
            nhits[2]++;
            channel[2].push_back(TDC_channel->at(i));
            timestamp[2].push_back(TDC_TimeStamp->at(i));
          }
        } else if (c==2) {
          // Chamber S2: Strips 200 to 295.
          if (ChannelMap[TDC_channel->at(i)]<200 || ChannelMap[TDC_channel->at(i)]>295) continue;
          if (ChannelMap[TDC_channel->at(i)]>=200 && ChannelMap[TDC_channel->at(i)]<232) {
            // Partition A.
            nhits[0]++;
            channel[0].push_back(TDC_channel->at(i));
            timestamp[0].push_back(TDC_TimeStamp->at(i));
          } else if (ChannelMap[TDC_channel->at(i)]>=232 && ChannelMap[TDC_channel->at(i)]<264) {
            // Partition B.
            nhits[1]++;
            channel[1].push_back(TDC_channel->at(i));
            timestamp[1].push_back(TDC_TimeStamp->at(i));
          } else if (ChannelMap[TDC_channel->at(i)]>=264 && ChannelMap[TDC_channel->at(i)]<296) {
            // Partition C.
            nhits[2]++;
            channel[2].push_back(TDC_channel->at(i));
            timestamp[2].push_back(TDC_TimeStamp->at(i));
          }
        } else if (c==3) {
          // Chamber S1: Strips 300 to 395.
          if (ChannelMap[TDC_channel->at(i)]<300 || ChannelMap[TDC_channel->at(i)]>395) continue;
          if (ChannelMap[TDC_channel->at(i)]>=300 && ChannelMap[TDC_channel->at(i)]<332) {
            // Partition A.
            nhits[0]++;
            channel[0].push_back(TDC_channel->at(i));
            timestamp[0].push_back(TDC_TimeStamp->at(i));
          } else if (ChannelMap[TDC_channel->at(i)]>=332 && ChannelMap[TDC_channel->at(i)]<364) {
            // Partition B.
            nhits[1]++;
            channel[1].push_back(TDC_channel->at(i));
            timestamp[1].push_back(TDC_TimeStamp->at(i));
          } else if (ChannelMap[TDC_channel->at(i)]>=364 && ChannelMap[TDC_channel->at(i)]<396) {
            // Partition C.
            nhits[2]++;
            channel[2].push_back(TDC_channel->at(i));
            timestamp[2].push_back(TDC_TimeStamp->at(i));
          }
        }
      }

      if (debug) {
        cout << "=== New Event ===" << endl;
        cout << "-----> Number of Hits: " << number_of_hits << endl;
        cout << "---> Number of Hits in Partition A: " << nhits[0] << endl;
        cout << "---> Number of Hits in Partition B: " << nhits[1] << endl;
        cout << "---> Number of Hits in Partition C: " << nhits[2] << endl;
      }    

      // Loop over partitions.
      for (int p=0; p<np; p++) {

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
          float timeWindow = clustering_timeWindow;
          if (newCandidate) {
            if (i!=(lastHitInWindow+1)) continue;
            timeWindow = HitInfo[lastHitInWindow+1].first + clustering_timeWindow;
            lastHitInWindow = lastHitInWindow + 1; 
          } else {
            lastHitInWindow = i;
            timeWindow = HitInfo[lastHitInWindow].first + clustering_timeWindow;
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
              h_ClusterSize[c][p]->Fill(2);
              h_ClusterSizeChamber[c]->Fill(2);
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
              h_ClusterSize[c][p]->Fill(consecutiveStrips.size());
              h_ClusterSizeChamber[c]->Fill(consecutiveStrips.size());
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
          h_ClusterSize[c][p]->Fill(1);
          h_ClusterSizeChamber[c]->Fill(1);
        }
        if (debug) {
          cout << "Number of single strips: " << singleStrips << endl;
        }      

        // Count the total number of clusters in the event. Fill the number of clusters histogram.
        int totalClusters = singleStrips + nClusters;
        h_nClusters_w0bin[c][p]->Fill(totalClusters);
        h_nClustersChamber_w0bin[c]->Fill(totalClusters);
        if (totalClusters > 0) {  // Removing 0-bin.
          h_nClusters[c][p]->Fill(totalClusters);
          h_nClustersChamber[c]->Fill(totalClusters);
          if (debug) {
            cout << "=> Total number of clusters: " << totalClusters << endl;
          }
        }

      }

    }

  }

  cout << "Number of events: " << nevents << endl;

}
