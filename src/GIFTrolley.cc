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
    SetNSlots(ID,geofile);
    SetSlotsID(ID,geofile);
    SetRPCs(ID,geofile);
}

// *************************************************************************************************************

Trolley::~Trolley(){
    for(Uint s = 0; s < GetNSlots(); s++){
        delete RPCs[r];
    }
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
    return RPCs[r];
}

// *************************************************************************************************************

string Trolley::GetName(Uint r){
    return RPCs[r]->GetName();
}

// *************************************************************************************************************

Uint Trolley::GetNGaps(Uint r){
    return RPCs[r]->GetNGaps();
}

// *************************************************************************************************************

Uint Trolley::GetNPartitions(Uint r){
    return RPCs[r]->GetNPartitions();
}

// *************************************************************************************************************

Uint Trolley::GetNStrips(Uint r){
    return RPCs[r]->GetNStrips();
}

// *************************************************************************************************************

string Trolley::GetGap(Uint r, Uint g){
    return RPCs[r]->GetGap(g);
}

// *************************************************************************************************************

float Trolley::GetGapGeo(Uint r, Uint g){
    return RPCs[r]->GetGapGeo(g);
}

// *************************************************************************************************************

float Trolley::GetStripGeo(Uint r, Uint p){
    return RPCs[r]->GetStripGeo(p);
}

// *************************************************************************************************************

void Trolley::SetNSlots(string ID, IniFile *geofile){
    nSlots = geofile->intType(ID,"nSlots",NSLOTS);
}

// *************************************************************************************************************

void Trolley::SetSlotsID(string ID, IniFile *geofile){
    SlotsID = geofile->stringType(ID,"SlotsID","");
}

// *************************************************************************************************************

void Trolley::SetRPCs(string ID, IniFile *geofile){
    RPCs.clear();

    for(Uint s = 0; s < GetNSlots(); s++){
        string rpcID = ID + "S" + GetSlotID(s);

        RPC* temprpc = new RPC(rpcID,geofile);
        RPCs.push_back(temprpc);
    }
}
