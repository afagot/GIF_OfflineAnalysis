#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#define RDMTDCWINDOW   400.*25.
#define RDMTDCOFFSET  -401.*25.

//#define RDMTDCWINDOW  24.*25.
//#define RDMTDCOFFSET -29.*25.

#define NTROLLEYS      5
#define NRPCTROLLEY    4
#define NPARTITIONS    4

#define NSTRIPSRPC     128
#define NSTRIPSPART    32
#define NSTRIPSCONN    16

using namespace std;

const string __mapping = "Mappings/ChannelsMapping_T1_T3_20150928-2015XXXX.csv";
const string __analyseddata = "/mnt/nfs/daq_data/DQM/";

void MakeHeader(string filename);

//Data in the root file
struct RAWData {
    int             iEvent;     //Event i
    int             TDCNHits;   //Number of hits in event i
    vector<int>    *TDCCh;      //List of channels giving hits per event
    vector<float>  *TDCTS;      //List of the corresponding time stamps
};

void SetIDName(unsigned int trolley, unsigned int station, unsigned int partition, char* ID, char* Name, const char* IDroot, const char* Nameroot);

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

void SetRPCHit(RPCHit& Hit, int Channel, float TimeStamp);
bool SortStrips ( RPCHit A, RPCHit B );
int GetPartition( int strip );

#endif // UTILS_H
