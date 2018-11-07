#ifndef __TYPES_H_
#define __TYPES_H_

//***************************************************************
// *    GIF OFFLINE TOOL v7
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    types.h
// *
// *    Global variables, structs, and typedefs
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    07/06/2017
//***************************************************************

#include "TH1.h"
#include "TH2.h"

using namespace std;

//****************************************************************************

//List of global variables used as default values
//in case of problems to read Dimensions.ini
const float TIMEREJECT   = 100.;
const float TIMEBIN      = 10.;
const float BMTDCWINDOW  = 24.*25.;
const float RDMTDCWINDOW = 400.*25.;
const float RDMNOISEWDW  = RDMTDCWINDOW - TIMEREJECT;

typedef unsigned int Uint;
const Uint NTROLLEYS   = 5;
const Uint NSLOTS      = 9;
const Uint NPARTITIONS = 4;

const Uint NSTRIPSRPC  = 128;
const Uint NSTRIPSPART = 48;
const Uint NSTRIPSCONN = 16;
const Uint NSTRIPSCHIP = 8;

//****************************************************************************

typedef struct GIFnBinsMult { Uint rpc[NTROLLEYS][NSLOTS][NPARTITIONS]; } GIFnBinsMult;
typedef struct GIFH1Array { TH1* rpc[NTROLLEYS][NSLOTS][NPARTITIONS]; } GIFH1Array;
typedef struct GIFH2Array { TH2* rpc[NTROLLEYS][NSLOTS][NPARTITIONS]; } GIFH2Array;
typedef struct GIFintArray { int rpc[NTROLLEYS][NSLOTS][NPARTITIONS]; } GIFintArray;
typedef struct GIFfloatArray { float rpc[NTROLLEYS][NSLOTS][NPARTITIONS]; } GIFfloatArray;
typedef GIFfloatArray muonPeak;

typedef enum _QualityFlag {
    GOOD      = 1,
    CORRUPTED = 2
} QualityFlag;

typedef enum _Efficiency {
    DETECTED = 1,
    MISSED = 0
} Efficiency;

typedef enum _Mask {
    ACTIVE = 1,
    MASKED = 0
} Mask;

typedef enum _ChannelStatus {
    NOCHANNELLINK = 0
} ChannelStatus;

//****************************************************************************

//Path to file where is saved the current data
//file path. Saving this path into the file allows
//to write all the logs in the log.txt file that
//is inside the scan directory.
const string __rundir = "/var/operation/RUN/";
const string __logpath = __rundir + "log-offline";
const string __dimension = "/Dimensions.ini";
const string __mapping = "/ChannelsMapping.csv";

//****************************************************************************

//Structures to interpret the data inside of the root file
//->RAWData represents the data structure inside the root
//  file itself
//->RPCHit is used to translate the TDC data into physical
//  data, assigning each hit to a RPC strip

struct RAWData {
    int            iEvent;   //Event i
    int            TDCNHits; //Number of hits in event i
    int            QFlag;    //Quality flag list (1 flag digit per TDC)
    vector<Uint>  *TDCCh;    //List of channels giving hits per event
    vector<float> *TDCTS;    //List of the corresponding time stamps
};

#endif
