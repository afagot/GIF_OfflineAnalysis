#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#define TDCWINDOW   5000.
#define TDCOFFSET  -5025.

#define NRPCTROLLEY 4
#define NPARTITIONS 3

#define NSTRIPSRPC  96
#define NSTRIPSPART 32
#define NSTRIPSCONN 16

using namespace std;

void MakeHeader(string filename);

//Data in the root file
struct RAWData {
    int             iEvent;     //Event i
    int             TDCNHits;   //Number of hits in event i
    vector<int>    *TDCCh;      //List of channels giving hits per event
    vector<float>  *TDCTS;      //List of the corresponding time stamps
};

void SetIDName(unsigned int station,unsigned int partition, char* partLabel, char* ID, char* Name, char* IDroot, char* Nameroot);

//Hit in the RPC
struct RPCHit {
    int             Channel;    //RPC Channel (X00 - X95)
    int             Strip;      //Strip (0 to 95)
    int             Station;    //Place in the trolley
    int             Partition;  //Partition (0 to 2)
    int             Connector;  //Connector (0 to 5)
    float           TimeStamp;  //TDC time stamp
};

void SetRPCHit(RPCHit& Hit, int Channel, float TimeStamp);
bool SortStrips ( RPCHit A, RPCHit B );
int GetPartition( int strip );

#endif // UTILS_H
