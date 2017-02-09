#ifndef UTILS_H
#define UTILS_H

//***************************************************************
// *    GIF OFFLINE TOOL v4
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
// *    07/03/2017
//***************************************************************

#include <string>
#include <vector>

#include "TTree.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"

#include "IniFile.h"

using namespace std;

const float RDMTDCWINDOW    = 400.*25.;
const float RDMNOISEWDW     = RDMTDCWINDOW - 200.;
const float BMTDCWINDOW     = 24.*25.;
const float BMNOISEWDW      = 300.;

const unsigned int NTROLLEYS         = 5;
const unsigned int NSLOTS            = 9;
const unsigned int NPARTITIONS       = 4;

const unsigned int NSTRIPSRPC        = 128;
const unsigned int NSTRIPSPART       = 48;
const unsigned int NSTRIPSCONN       = 16;
const unsigned int NSTRIPSCHIP       = 8;

const string __rundir = "/var/operation/RUN/";
const string __logpath = __rundir + "log-offline";

bool            existFile(string ROOTName);
unsigned int    CharToInt(char& C);
string          CharToString(char& C);
string          intToString(int value);
string          longTostring(long value);
string          floatTostring(float value);
string          GetLogTimeStamp();
void            WritePath(string basename);

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
    int            iEvent;     //Event i
    int            TDCNHits;   //Number of hits in event i
    vector<unsigned int>   *TDCCh;      //List of channels giving hits per event
    vector<float>          *TDCTS;      //List of the corresponding time stamps
};

void SetTitleName(string rpcID, unsigned int partition, char* Name, char* Title, string Namebase, string Titlebase);

//Hit in the RPC
struct RPCHit {
    unsigned int    Channel;    //RPC Channel (5 digit numbers like XX000 - XX128)
    unsigned int    Trolley;    //1 or 3 (1st digit of the RPC channel)
    unsigned int    Station;    //Place in the trolley (S1 to S4 - 2nd digit)
    unsigned int    Strip;      //Strip (1 to 128 depending on the chamber - 3 last digits)
    unsigned int    Partition;  //Partition (1 to 4)
    unsigned int    Connector;  //Connector (1 to 8)
    float           TimeStamp;  //TDC time stamp
};

void  SetRPCHit(RPCHit& Hit, int Channel, float TimeStamp, Infrastructure Infra);
void  SetBeamWindow (float (&PeakTime)[NTROLLEYS][NSLOTS][NPARTITIONS], float (&PeakWidth)[NTROLLEYS][NSLOTS][NPARTITIONS], TTree* mytree, map<int, int> RPCChMap, Infrastructure GIFInfra);
bool  SortStrips ( RPCHit A, RPCHit B );
int   GetPartition( int strip );
float GetTH1Mean(TH1* H);
float GetTH1StdDev(TH1* H);
void  DrawTH1(TCanvas* C, TH1* H, string xtitle, string ytitle, string option);
void  DrawTH2(TCanvas* C, TH2* H, string xtitle, string ytitle, string ztitle, string option);

#endif // UTILS_H
