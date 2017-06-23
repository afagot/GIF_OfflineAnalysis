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

#include "../include/GIFTrolley.h"
#include "../include/RPCDetector.h"

using namespace std;

// *************************************************************************************************************

GIFTrolley::GIFTrolley(){

}

// *************************************************************************************************************

GIFTrolley::GIFTrolley(string ID, IniFile *geofile){
    SetNSlots(ID,geofile);
    SetSlotsID(ID,geofile);
    SetRPCs(ID,geofile);
}

// *************************************************************************************************************

GIFTrolley::~GIFTrolley(){

}

// *************************************************************************************************************

Uint GIFTrolley::GetNSlots(){
    return nSlots;
}

// *************************************************************************************************************

string GIFTrolley::GetSlotsID(){
    return SlotsID;
}

// *************************************************************************************************************

string GIFTrolley::GetSlotID(Uint i){
    return CharToString(SlotsID[i]);
}

// *************************************************************************************************************

void GIFTrolley::SetNSlots(string ID, IniFile *geofile){
    nSlots = geofile->intType(ID,"nSlots",NSLOTS);
}

// *************************************************************************************************************

void GIFTrolley::SetSlotsID(string ID, IniFile *geofile){
    SlotsID = geofile->stringType(ID,"SlotsID","");
}

// *************************************************************************************************************

void GIFTrolley::SetRPCs(string ID, IniFile *geofile){
    RPCs.clear();

    for(Uint s = 0; s < GetNSlots(); s++){
        string rpcID = ID + "S" + GetSlotID(s);

        RPC temprpc(rpcID,geofile);
        RPCs.push_back(temprpc);
    }
}
