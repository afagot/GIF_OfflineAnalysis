//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    GIFTrolley.cc
// *
// *    Class that defines Trolley objects. Trolleys are objects
// *    that contain RPCs (it reproduces GIF++ infra
// *    -structure).
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

// ****************************************************************************************************
// *    Trolley()
//
//  Default constructor
// ****************************************************************************************************

Trolley::Trolley(){

}

// ****************************************************************************************************
// *    Trolley(string ID, IniFile *geofile)
//
//  Constructor. It needs to access the information stored in the buffer of the Dimensions.ini file.
// ****************************************************************************************************

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

// ****************************************************************************************************
// *    Trolley(const Trolley &other)
//
//  Copy constructor
// ****************************************************************************************************

Trolley::Trolley(const Trolley &other){
    nSlots = other.nSlots;
    SlotsID = other.SlotsID;

    RPCs.clear();
    RPCs = other.RPCs;
}

// ****************************************************************************************************
// *    ~Trolley()
//
//  Destructor
// ****************************************************************************************************

Trolley::~Trolley(){
    vector<RPC*>::iterator it = RPCs.begin();

    while(it != RPCs.end())
        delete *(it++);
}

// ****************************************************************************************************
// *    Trolley& operator=(const Trolley& other)
//
//  Copy operator
// ****************************************************************************************************

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

// ****************************************************************************************************
// *    Uint GetNSlots()
//
//  Get the private member nSlots
// ****************************************************************************************************

Uint Trolley::GetNSlots(){
    return nSlots;
}

// ****************************************************************************************************
// *    string GetSlotsID()
//
//  Get the private member SlotsID
// ****************************************************************************************************

string Trolley::GetSlotsID(){
    return SlotsID;
}

// ****************************************************************************************************
// *    Uint GetSlotID(Uint s)
//
//  Get the private member SlotsID and extract the ID of slot s
// ****************************************************************************************************

Uint Trolley::GetSlotID(Uint s){
    return CharToInt(SlotsID[s]);
}

// ****************************************************************************************************
// *    RPC* GetRPC(Uint r)
//
//  Get the private member RPCs and get the RPC r
// ****************************************************************************************************

RPC* Trolley::GetRPC(Uint r){
    if(r < RPCs.size())
        return RPCs[r];
    else
        return NULL;
}

// ****************************************************************************************************
// *    void DeleteRPC(Uint r)
//
//  Get the private member RPCs and delete RPC r
// ****************************************************************************************************

void Trolley::DeleteRPC(Uint r){
    if(r < RPCs.size()){
        delete RPCs[r];
        RPCs.erase(RPCs.begin()+r);
        nSlots--;
        SlotsID.erase(SlotsID.begin()+r);
    }
}

// ****************************************************************************************************
// *    string GetName(Uint r)
//
//  Get the private member RPCs and get the name of RPC r
// ****************************************************************************************************

string Trolley::GetName(Uint r){
    return GetRPC(r)->GetName();
}

// ****************************************************************************************************
// *    Uint GetNGaps(Uint r)
//
//  Get the private member RPCs and get the number of gaps of RPC r
// ****************************************************************************************************

Uint Trolley::GetNGaps(Uint r){
    return GetRPC(r)->GetNGaps();
}

// ****************************************************************************************************
// *    Uint GetNPartitions(Uint r)
//
//  Get the private member RPCs and get the number of partitions of RPC r
// ****************************************************************************************************

Uint Trolley::GetNPartitions(Uint r){
    return GetRPC(r)->GetNPartitions();
}

// ****************************************************************************************************
// *    Uint GetNStrips(Uint r)
//
//  Get the private member RPCs and get the number of strips per partition of RPC r
// ****************************************************************************************************

Uint Trolley::GetNStrips(Uint r){
    return GetRPC(r)->GetNStrips();
}

// ****************************************************************************************************
// *    string GetGap(Uint r, Uint g)
//
//  Get the private member RPCs and get the name of gap g of RPC r
// ****************************************************************************************************

string Trolley::GetGap(Uint r, Uint g){
    return GetRPC(r)->GetGap(g);
}

// ****************************************************************************************************
// *    float GetGapGeo(Uint r, Uint g)
//
//  Get the private member RPCs and get the active area of gap g of RPC r
// ****************************************************************************************************

float Trolley::GetGapGeo(Uint r, Uint g){
    return GetRPC(r)->GetGapGeo(g);
}

// ****************************************************************************************************
// *    float GetStripGeo(Uint r, Uint p)
//
//  Get the private member RPCs and get the active area of strips in partition p of RPC r
// ****************************************************************************************************

float Trolley::GetStripGeo(Uint r, Uint p){
    return GetRPC(r)->GetStripGeo(p);
}
