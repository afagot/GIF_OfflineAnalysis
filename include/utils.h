#ifndef UTILS_H
#define UTILS_H

//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    utils.h
// *
// *    All usefull functions (type cast, time stamps,...)
// *    and structures (used for the GIF layout definition).
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    07/06/2017
//***************************************************************

#include <string>
#include <vector>

#include "TTree.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"

#include "types.h"
#include "Mapping.h"
#include "Infrastructure.h"

using namespace std;

//****************************************************************************

//Structures to interpret the data inside of the root file
//->RAWData represents the data structure inside the root
//  file itself
//->RPCHit is used to translate the TDC data into physical
//  data, assigning each hit to a RPC strip

struct RAWData {
    int            iEvent;   //Event i
    int            TDCNHits; //Number of hits in event i
    vector<Uint>  *TDCCh;    //List of channels giving hits per event
    vector<float> *TDCTS;    //List of the corresponding time stamps
};

//Hit in the RPC
struct RPCHit {
    Uint  Channel;   //RPC Channel (5 digit numbers like XX000 - XX128)
    Uint  Trolley;   //1 or 3 (1st digit of the RPC channel)
    Uint  Station;   //Place in the trolley (S1 to S4 - 2nd digit)
    Uint  Strip;     //Strip (1 to 128 depending on the chamber - 3 last digits)
    Uint  Partition; //Partition (1 to 4)
    float TimeStamp; //TDC time stamp
};
typedef vector<RPCHit> HitList;
typedef struct GIFHitList { HitList rpc[NTROLLEYS][NSLOTS][NPARTITIONS]; } GIFHitList;

//Cluster reconstructed in the RPC
struct RPCCluster {
   Uint  ClusterID;
   Uint  ClusterSize;
   Uint  FirstStrip;
   Uint  LastStrip;
   float Center;
   float StartStamp;
   float StopStamp;
   float TimeSpread;
};
typedef vector<RPCCluster> ClusterList;

//****************************************************************************

//Functions (more details in utils.cc)

Uint    CharToInt(char& C);
string  CharToString(char& C);
string  intToString(int value);
string  longTostring(long value);
string  floatTostring(float value);

bool    existFile(string ROOTName);

string  GetLogTimeStamp();
void    WritePath(string basename);

void    SetRPCHit(RPCHit& Hit, int Channel, float TimeStamp, Infrastructure* Infra);
void    SetCluster(RPCCluster& Cluster, HitList List, Uint cID, Uint cSize, Uint first, Uint firstID);
void    SetBeamWindow (muonPeak &PeakTime, muonPeak &PeakWidth,
                       TTree* mytree, Mapping* RPCChMap, Infrastructure* Infra);
void    SetTitleName(string rpcID, Uint partition, char* Name,
                     char* Title,string Namebase, string Titlebase);
bool    IsEfficiencyRun(TString* runtype);
float   GetTH1Mean(TH1* H);
float   GetTH1StdDev(TH1* H);
float   GetChipBin(TH1* H, Uint chip);
void    SetTH1(TH1* H, string xtitle, string ytitle);
void    SetTH2(TH2* H, string xtitle, string ytitle, string ztitle);

bool    SortHitbyStrip(RPCHit h1, RPCHit h2);
bool    SortHitbyTime(RPCHit h1, RPCHit h2);
void    BuildClusters(HitList &cluster, ClusterList &clusterList);
float   GetClusterStartStamp(HitList &cluster, int cSize, int hitID);
float   GetClusterStopStamp(HitList &cluster,int cSize, int hitID);
float   GetClusterSpreadTime(HitList &cluster, int cSize, int hitID);
void    Clusterization(HitList &hits, TH1 *hcSize, TH1 *hcMult);

#endif // UTILS_H
