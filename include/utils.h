#ifndef UTILS_H
#define UTILS_H

//***************************************************************
// *    904 OFFLINE TOOL v4
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

//****************************************************************************

typedef unsigned int Uint;
typedef map<Uint,Uint> mapping;

//****************************************************************************

//List of global variables used as default values
//in case of problems to read Dimensions.ini
const float TIMEREJECT   = 100.;
const float TIMEBIN      = 10.;
const float BMTDCWINDOW  = 24.*25.;
const float RDMTDCWINDOW = 400.*25.;
const float RDMNOISEWDW  = RDMTDCWINDOW - TIMEREJECT;

const Uint NSLOTS      = 9;
const Uint NPARTITIONS = 4;

const Uint NSTRIPSRPC  = 128;
const Uint NSTRIPSPART = 48;
const Uint NSTRIPSCONN = 16;
const Uint NSTRIPSCHIP = 8;

//****************************************************************************

typedef struct muonPeak { float rpc[NSLOTS][NPARTITIONS]; } muonPeak;

//****************************************************************************

//Path to file where is saved the current data
//file path. Saving this path into the file allows
//to write all the logs in the log.txt file that
//is inside the scan directory.

const string __rundir = "/var/operation/RUN/";
const string __logpath = __rundir + "log-offline";

//****************************************************************************

//Structures representing the infrastructure inside GIF++
//-> The infrastructure contains trolleys
//-> Trolleys contains RPCs
//-> RPCs contain gaps and have partitions

struct RPC{
    string          name;        //RPC name as in webDCS database
    Uint            nGaps;       //Number of gaps in the RPC
    vector<string>  gaps;        //List of gap labels (BOT, TOP, etc...)
    vector<float>   gapGeo;      //List of gap active areas
    Uint            nPartitions; //Number of partitions in the RPC
    Uint            strips;      //Number of strips per partition
    vector<float>   stripGeo;    //List of strip active areas
};

struct Infrastructure {
    Uint         nSlots;  //Number of active RPCs in the considered trolley
    string       SlotsID; //Active RPC IDs written into a string
    vector<RPC>  RPCs;    //List of active RPCs (struct)
};

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
    vector<float> *TDCtTS;   //List of the corresponding time stamps (trailing)
    vector<float> *TDClTS;   //List of the corresponding time stamps (leading)
};

//Hit in the RPC
struct RPCHit {
    Uint  Channel;    //RPC Channel (4 digit numbers like X000 - X128)
    Uint  Station;    //Place in the cosmic stand (S1 to S9 - 1st digit)
    Uint  Strip;      //Strip (1 to 128 depending on the chamber - 3 last digits)
    Uint  Partition;  //Partition (1 to 4)
    float TimeStamp;  //TDC time stamp
    bool  isLeading;
};

struct Mapping {
    mapping link;
    mapping mask;
};

//Cluster of hits in a partition of the RPC
struct RPCCluster {
    Uint  Station;    //Station
    Uint  Partition;  //Partition
    Uint  FirstStrip; //First strip of the cluster
    Uint  LastStrip;  //Last strip of the cluster
    float Center;     //Central strip ( (First + Last) / 2 )
    Uint  Size;       //Cluster size
    float TimeStamp;  //TDC time stamp
};

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
Mapping TDCMapping(string baseName);
void    SetRPC(RPC& rpc, string ID, IniFile* geofile);
void    SetInfrastructure(Infrastructure& infra, IniFile* geofile);
void    SetRPCHit(RPCHit& Hit, int Channel, float TimeStamp, Infrastructure Infra);
void    SetBeamWindow (muonPeak &PeakTime, muonPeak &PeakWidth,
                       TTree* mytree, Mapping RPCChMap, Infrastructure GIFInfra);
bool    SortStrips (RPCHit A, RPCHit B);
void    SetTitleName(string rpcID, unsigned int partition,
                     char* Name, char* Title, string Namebase, string Titlebase);
float   GetTH1Mean(TH1* H);
float   GetTH1StdDev(TH1* H);
float   GetChipBin(TH1* H, Uint chip);
void    SetTH1(TH1* H, string xtitle, string ytitle);
void    SetTH2(TH2* H, string xtitle, string ytitle, string ztitle);

#endif // UTILS_H
