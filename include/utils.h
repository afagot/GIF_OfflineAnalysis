#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <unistd.h>
#include <iomanip>

#include "IniFile.h"

#define RDMTDCWINDOW   400.*25.
#define BMTDCWINDOW    24.*25.
#define BMNOISEWDW     300.

#define NTROLLEYS      5
#define NSLOTS         4
#define NPARTITIONS    4

#define NSTRIPSRPC     128
#define NSTRIPSPART    48
#define NSTRIPSCONN    16

using namespace std;

//const string __mapping = "Mappings/ChannelsMapping_T3.csv";
const string __mapping = "Mappings/ChannelsMapping_T1_T3_20160526-2016XXXX.csv";
//const string __mapping = "Mappings/ChannelsMapping_T1_T3_20150928-20160512.csv";
const string __dimensions = "Dimensions/Dimensions_20160526-2016XXXX.ini";
//const string __dimensions = "Dimensions/Dimensions_20150928-20160512.ini";
const string __logpath = "/var/operation/RUN/log";
//const string __logpath = "log";
const string __lastpath = "/var/operation/RUN/last";
//const string __lastpath = "001329/last";

bool    existFile(string filename);
int     CharToInt(char& C);
string  CharToString(char& C);
string  intTostring(int value);
string  longTostring(long value);
string  floatTostring(float value);
string  GetLogTimeStamp();

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

#endif // UTILS_H
