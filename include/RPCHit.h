#ifndef __HIT_H_
#define __HIT_H_

//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    RPCHit.h
// *
// *    Class that defines RPCHit objects. RPCHits are simply
// *    storing the time stamp converting the rpc channel into
// *    strip, station, trolley information. This inofrmation
// *    is extracted from the format of the rpc channel :
// *    it is a 5 digit unsigned int TSCCC
// *    T : trolley ID
// *    S : slot ID (or station)
// *    CCC : rpc strip (depending on the chamber it can go up
// *    to 128 strips so it was decided to use 3 digit for the
// *    strip ID).
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    22/06/2017
//***************************************************************

#include <vector>

#include "types.h"
#include "Mapping.h"
#include "Infrastructure.h"

#include "TTree.h"

using namespace std;

//Hit in the RPC
class RPCHit {
    private:
        Uint  Channel;
        Uint  Trolley;
        Uint  Station;
        Uint  Strip;
        Uint  Partition;
        float TimeStamp;

    public:
        RPCHit();
        RPCHit(int channel, float time, Infrastructure* Infra);
        RPCHit(const RPCHit& other);
        ~RPCHit();
        RPCHit& operator=(const RPCHit& other);

        Uint  GetChannel();
        Uint  GetTrolley();
        Uint  GetStation();
        Uint  GetStrip();
        Uint  GetPartition();
        float GetTime();
};

typedef vector<RPCHit> HitList;
typedef struct GIFHitList { HitList rpc[NTROLLEYS][NSLOTS][NPARTITIONS]; } GIFHitList;

bool SortHitbyStrip(RPCHit h1, RPCHit h2);
bool SortHitbyTime(RPCHit h1, RPCHit h2);

void SetBeamWindow (muonPeak &PeakTime, muonPeak &PeakWidth,
                       TTree* mytree, Mapping* RPCChMap, Infrastructure* Infra);

#endif
