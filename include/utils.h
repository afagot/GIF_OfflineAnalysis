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

//****************************************************************************

//List of global variables used as default values
//in case of problems to read Dimensions.ini
const float TIMEREJECT   = 100.;
const float TIMEBIN      = 10.;
const float BMTDCWINDOW  = 24.*25.;
const float RDMTDCWINDOW = 400.*25.;
const float RDMNOISEWDW  = RDMTDCWINDOW - TIMEREJECT;

const unsigned int NTROLLEYS   = 5;
const unsigned int NSLOTS      = 9;
const unsigned int NPARTITIONS = 4;

const unsigned int NSTRIPSRPC  = 128;
const unsigned int NSTRIPSPART = 48;
const unsigned int NSTRIPSCONN = 16;
const unsigned int NSTRIPSCHIP = 8;

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
    unsigned int    nGaps;       //Number of gaps in the RPC
    vector<string>  gaps;        //List of gap labels (BOT, TOP, etc...)
    vector<float>   gapGeo;      //List of gap active areas
    unsigned int    nPartitions; //Number of partitions in the RPC
    unsigned int    strips;      //Number of strips per partition
    vector<float>   stripGeo;    //List of strip active areas
};

struct GIFTrolley {
    unsigned int nSlots;  //Number of active RPCs in the considered trolley
    string       SlotsID; //Active RPC IDs written into a string
    vector<RPC>  RPCs;    //List of active RPCs (struct)
};

struct Infrastructure {
    unsigned int       nTrolleys;  //Number of active Trolleys in the run
    string             TrolleysID; //Active trolley IDs written into a string
    vector<GIFTrolley> Trolleys;   //List of active Trolleys (struct)
};

//****************************************************************************

//Structures to interpret the data inside of the root file
//->RAWData represents the data structure inside the root
//  file itself
//->RPCHit is used to translate the TDC data into physical
//  data, assigning each hit to a RPC strip

struct RAWData {
    int                     iEvent;   //Event i
    int                     TDCNHits; //Number of hits in event i
    vector<unsigned int>   *TDCCh;    //List of channels giving hits per event
    vector<float>          *TDCTS;    //List of the corresponding time stamps
};

//Hit in the RPC
struct RPCHit {
    unsigned int    Channel;   //RPC Channel (5 digit numbers like XX000 - XX128)
    unsigned int    Trolley;   //1 or 3 (1st digit of the RPC channel)
    unsigned int    Station;   //Place in the trolley (S1 to S4 - 2nd digit)
    unsigned int    Strip;     //Strip (1 to 128 depending on the chamber - 3 last digits)
    unsigned int    Partition; //Partition (1 to 4)
    float           TimeStamp; //TDC time stamp
};

//****************************************************************************

//Functions (more details in utils.cc)

unsigned int CharToInt(char& C);
string       CharToString(char& C);
string       intToString(int value);
string       longTostring(long value);
string       floatTostring(float value);
bool         existFile(string ROOTName);
string       GetLogTimeStamp();
void         WritePath(string basename);
map<int,int> TDCMapping(string baseName);
void         SetRPC(RPC& rpc, string ID, IniFile* geofile);
void         SetTrolley(GIFTrolley& trolley, string ID, IniFile* geofile);
void         SetInfrastructure(Infrastructure& infra, IniFile* geofile);
void         SetRPCHit(RPCHit& Hit, int Channel, float TimeStamp, Infrastructure Infra);
void         SetBeamWindow (float (&PeakTime)[NTROLLEYS][NSLOTS][NPARTITIONS],
                            float (&PeakWidth)[NTROLLEYS][NSLOTS][NPARTITIONS],
                            TTree* mytree, map<int, int> RPCChMap, Infrastructure GIFInfra);
void         SetTitleName(string rpcID, unsigned int partition,
                          char* Name, char* Title, string Namebase, string Titlebase);
float        GetTH1Mean(TH1* H);
float        GetTH1StdDev(TH1* H);
void         SetTH1(TH1* H, string xtitle, string ytitle);
void         SetTH2(TH2* H, string xtitle, string ytitle, string ztitle);

#endif // UTILS_H
