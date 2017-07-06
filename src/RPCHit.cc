//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    RPCHit.h
// *
// *    To be updated
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    22/06/2017
//***************************************************************

#include <vector>

#include "../include/RPCHit.h"
#include "../include/types.h"
#include "../include/Infrastructure.h"

using namespace std;

RPCHit::RPCHit(){

}

// ****************************************************************************************************
// *    RPCHit(int channel, float time, Infrastructure* Infra)
//
//  Uses the mapping to set up every hit and assign it to the right strip in each active RPC.
// ****************************************************************************************************

RPCHit::RPCHit(int channel, float time, Infrastructure* Infra){
    Channel     = channel;              //RPC channel according to mapping (5 digits)
    Trolley     = channel/10000;        //0, 1 or 3 (1st digit of the RPC channel)
    Station     = (channel%10000)/1000; //From 1 to 4 (2nd digit)
    Strip       = channel%1000;         //From 1 to 128 (3 last digits)

    int nStripsPart = 0;
    for(Uint tr = 0; tr < Infra->GetNTrolleys(); tr++){
        if(Infra->GetTrolleyID(tr) == Trolley){
            for(Uint sl = 0; sl < Infra->GetNSlots(tr); sl++){
                if(Infra->GetSlotID(tr,sl) == Station)
                    nStripsPart = Infra->GetNStrips(tr,sl);
            }
        }
    }

    Partition   = (Strip-1)/nStripsPart+1; //From 1 to 4
    TimeStamp   = time;
}

RPCHit::~RPCHit(){

}

// ****************************************************************************************************
// *    bool sortbyhit(pair<int, float> p1, pair<int, float> p2)
//
//  Sort RPC hits using strip position information
// ****************************************************************************************************

bool SortHitbyStrip(RPCHit h1, RPCHit h2){
        return (h1.GetStrip() < h2.GetStrip());
}

// ****************************************************************************************************
// *    bool sortbytime(pair<int, float> p1, pair<int, float> p2)
//
//  Sort RPC hits using time stamp information
// ****************************************************************************************************

//Sort hits by time
bool SortHitbyTime(RPCHit h1, RPCHit h2){
        return (h1.GetTime() < h2.GetTime());
}
