#ifndef UTILS_H
#define UTILS_H

//***************************************************************
// *    GIF OFFLINE TOOL v3
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    utils.h
// *
// *    All usefull functions (type cast, time stamps,...)
// *    and structures (used for the GIF layout definition).
// *
// *    Developped by : Alexis Fagot
// *    22/04/2016
//***************************************************************

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <unistd.h>
#include <iomanip>

#include "TFile.h"
#include "TBranch.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "THistPainter.h"
#include "TColor.h"
#include "TStyle.h"
#include "TString.h"

#include "IniFile.h"

#define RDMTDCWINDOW   400.*25.
#define BMTDCWINDOW    24.*25.
#define BMNOISEWDW     300.

#define NTROLLEYS      5
#define NSLOTS         9
#define NPARTITIONS    4

#define NSTRIPSRPC     128
#define NSTRIPSPART    48
#define NSTRIPSCONN    16

using namespace std;

const string __rundir = "/var/operation/RUN/";
const string __logpath = __rundir + "log-offline";

bool    existFile(string ROOTName);
int     CharToInt(char& C);
string  CharToString(char& C);
string  intTostring(int value);
string  longTostring(long value);
string  floatTostring(float value);
string  GetLogTimeStamp();
void    WritePath(string basename);

//Infrastructure inside GIF++
struct RPC{
    string          name;
    unsigned int    nPartitions;
    unsigned int    nGaps;
    vector<string>  gaps;
    vector<float>   gapGeo;
    unsigned int    strips;
    vector<float>   stripGeo;
};

void SetRPC(RPC& rpc, string ID, IniFile* geofile);

struct GIFTrolley {
    unsigned int nSlots;
    string       SlotsID;
    vector<RPC>  RPCs;
};

void SetTrolley(GIFTrolley& trolley, string ID, IniFile* geofile);

struct Infrastructure {
    unsigned int       nTrolleys;
    string             TrolleysID;
    vector<GIFTrolley> Trolleys;
};

void SetInfrastructure(Infrastructure& infra, IniFile* geofile);

//Data in the root file
struct RAWData {
    int             iEvent;     //Event i
    int             TDCNHits;   //Number of hits in event i
    vector<int>    *TDCCh;      //List of channels giving hits per event
    vector<float>  *TDCTS;      //List of the corresponding time stamps
};

void SetIDName(string rpcID, unsigned int partition, char* ID, char* Name, string IDroot, string Nameroot);

//Hit in the RPC
struct RPCHit {
    int             Channel;    //RPC Channel (5 digit numbers like XX000 - XX128)
    int             Trolley;    //1 or 3 (1st digit of the RPC channel)
    int             Station;    //Place in the trolley (S1 to S4 - 2nd digit)
    int             Strip;      //Strip (1 to 128 depending on the chamber - 3 last digits)
    int             Partition;  //Partition (1 to 4)
    int             Connector;  //Connector (1 to 8)
    float           TimeStamp;  //TDC time stamp
};

void SetRPCHit(RPCHit& Hit, int Channel, float TimeStamp, Infrastructure Infra);
bool SortStrips ( RPCHit A, RPCHit B );
int GetPartition( int strip );
void DrawTH1(TCanvas* C, TH1* H, string xtitle, string ytitle, string option, string DQMFolder);
void DrawTH2(TCanvas* C, TH2* H, string xtitle, string ytitle, string ztitle, string option, string DQMFolder);

#endif // UTILS_H
