//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    GIFTrolley.cc
// *
// *    To be updated
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    22/06/2017
//***************************************************************

#include <string>
#include <vector>

#include "../include/types.h"
#include "../include/utils.h"
#include "../include/GIFTrolley.h"
#include "../include/RPCDetector.h"

using namespace std;

// *************************************************************************************************************

Trolley::Trolley(){

}

// *************************************************************************************************************
// *    void SetTrolley(GIFTrolley &trolley, string ID, IniFile *geofile)
//
//  Set up the Trolley structure needed that contains RPCs. Informations are readout from
//  Dimensions.ini . For details about Trolley members, see file utils.h.
// *************************************************************************************************************

Trolley::Trolley(string ID, IniFile *geofile){
    nSlots = geofile->intType(ID,"nSlots",NSLOTS);
    SlotsID = geofile->stringType(ID,"SlotsID","");

    RPCs.clear();

    for(Uint s = 0; s < GetNSlots(); s++){
        string rpcID = ID + "S" + intToString(GetSlotID(s));

        RPC* temprpc = new RPC(rpcID,geofile);
        RPCs.push_back(temprpc);
    }
}

Trolley::Trolley(const Trolley &other){
    nSlots = other.nSlots;
    SlotsID = other.SlotsID;

    RPCs.clear();
    RPCs = other.RPCs;
}

// *************************************************************************************************************

Trolley::~Trolley(){
    vector<RPC*>::iterator it = RPCs.begin();

    while(it != RPCs.end())
        delete *(it++);
}

Trolley& Trolley::operator=(const Trolley& other){
    if(this != &other){
        nSlots = other.nSlots;
        SlotsID = other.SlotsID;

        vector<RPC*>::iterator it = RPCs.begin();
        while(it != RPCs.end())
            delete *(it++);

        RPCs.clear();
        RPCs = other.RPCs;
    }

    return *this;
}

// *************************************************************************************************************

Uint Trolley::GetNSlots(){
    return nSlots;
}

// *************************************************************************************************************

string Trolley::GetSlotsID(){
    return SlotsID;
}

// *************************************************************************************************************

Uint Trolley::GetSlotID(Uint s){
    return CharToInt(SlotsID[s]);
}

// *************************************************************************************************************

RPC* Trolley::GetRPC(Uint r){
    if(r < RPCs.size())
        return RPCs[r];
    else
        return NULL;
}

void Trolley::DeleteRPC(Uint r){
    if(r < RPCs.size()){
        delete RPCs[r];
        RPCs.erase(RPCs.begin()+r);
        nSlots--;
        SlotsID.erase(SlotsID.begin()+r);
    }
}

// *************************************************************************************************************

string Trolley::GetName(Uint r){
    return GetRPC(r)->GetName();
}

// *************************************************************************************************************

Uint Trolley::GetNGaps(Uint r){
    return GetRPC(r)->GetNGaps();
}

// *************************************************************************************************************

Uint Trolley::GetNPartitions(Uint r){
    return GetRPC(r)->GetNPartitions();
}

// *************************************************************************************************************

Uint Trolley::GetNStrips(Uint r){
    return GetRPC(r)->GetNStrips();
}

// *************************************************************************************************************

string Trolley::GetGap(Uint r, Uint g){
    return GetRPC(r)->GetGap(g);
}

// *************************************************************************************************************

float Trolley::GetGapGeo(Uint r, Uint g){
    return GetRPC(r)->GetGapGeo(g);
}

// *************************************************************************************************************

float Trolley::GetStripGeo(Uint r, Uint p){
    return GetRPC(r)->GetStripGeo(p);
}
