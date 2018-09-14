//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    Infrastructure.cc
// *
// *    Class that defines Infrastructure objects. They are
// *    objects that contain Trolleys, that contains RPCs
// *    (itreproduces GIF++ infrastructure).
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    22/06/2017
//***************************************************************

#include <string>
#include <vector>

#include "../include/MsgSvc.h"
#include "../include/types.h"
#include "../include/utils.h"
#include "../include/RPCDetector.h"
#include "../include/Infrastructure.h"

using namespace std;

// ****************************************************************************************************
// *    Infrastructure()
//
//  Default constructor
// ****************************************************************************************************

Infrastructure::Infrastructure(){

}

// ****************************************************************************************************
// *    Infrastructure(IniFile *geofile)
//
//  Constructor. It needs to access the information stored in the Dimensions.ini file.
// ****************************************************************************************************

Infrastructure::Infrastructure(IniFile *geofile){
    nSlots = geofile->intType("General","nSlots",NSLOTS);
    SlotsID = geofile->stringType("General","SlotsID","");

    RPCs.clear();

    for(Uint s = 0; s < GetNSlots(); s++){
        string rpcID = "S" + intToString(GetSlotID(s));

        RPC* temprpc = new RPC(rpcID, geofile);
        RPCs.push_back(temprpc);

        MSG_INFO("[Infrastructure] Found RPC " + GetName(s) + " in slot " + rpcID)
    }
}

// ****************************************************************************************************
// *    Infrastructure(const Infrastructure &other)
//
//  Copy constructor
// ****************************************************************************************************

Infrastructure::Infrastructure(const Infrastructure &other){
    nSlots = other.nSlots;
    SlotsID = other.SlotsID;

    RPCs.clear();
    RPCs = other.RPCs;
}

// ****************************************************************************************************
// *    ~Infrastructure()
//
//  Destructor
// ****************************************************************************************************

Infrastructure::~Infrastructure(){
    vector<RPC*>::iterator it = RPCs.begin();

    while(it != RPCs.end())
        delete *(it++);
}

// ****************************************************************************************************
// *    Infrastructure& operator=(const Infrastructure& other)
//
//  Copy operator
// ****************************************************************************************************

Infrastructure& Infrastructure::operator=(const Infrastructure& other){
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

Uint Infrastructure::GetNSlots(){
    return nSlots;
}

// ****************************************************************************************************
// *    string GetSlotsID()
//
//  Get the private member SlotsID
// ****************************************************************************************************

string Infrastructure::GetSlotsID(){
    return SlotsID;
}

// ****************************************************************************************************
// *    Uint GetSlotID(Uint s)
//
//  Get the private member SlotsID and extract the ID of slot s
// ****************************************************************************************************

Uint Infrastructure::GetSlotID(Uint s){
    return CharToInt(SlotsID[s]);
}

// ****************************************************************************************************
// *    RPC* GetRPC(Uint r)
//
//  Get the private member RPCs and get the RPC r
// ****************************************************************************************************

RPC* Infrastructure::GetRPC(Uint r){
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

void Infrastructure::DeleteRPC(Uint r){
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

string Infrastructure::GetName(Uint r){
    return GetRPC(r)->GetName();
}

// ****************************************************************************************************
// *    Uint GetNGaps(Uint r)
//
//  Get the private member RPCs and get the number of gaps of RPC r
// ****************************************************************************************************

Uint Infrastructure::GetNGaps(Uint r){
    return GetRPC(r)->GetNGaps();
}

// ****************************************************************************************************
// *    Uint GetNPartitions(Uint r)
//
//  Get the private member RPCs and get the number of partitions of RPC r
// ****************************************************************************************************

Uint Infrastructure::GetNPartitions(Uint r){
    return GetRPC(r)->GetNPartitions();
}

// ****************************************************************************************************
// *    Uint GetNStrips(Uint r)
//
//  Get the private member RPCs and get the number of strips per partition of RPC r
// ****************************************************************************************************

Uint Infrastructure::GetNStrips(Uint r){
    return GetRPC(r)->GetNStrips();
}

// ****************************************************************************************************
// *    string GetGap(Uint r, Uint g)
//
//  Get the private member RPCs and get the name of gap g of RPC r
// ****************************************************************************************************

string Infrastructure::GetGap(Uint r, Uint g){
    return GetRPC(r)->GetGap(g);
}

// ****************************************************************************************************
// *    float GetGapGeo(Uint r, Uint g)
//
//  Get the private member RPCs and get the active area of gap g of RPC r
// ****************************************************************************************************

float Infrastructure::GetGapGeo(Uint r, Uint g){
    return GetRPC(r)->GetGapGeo(g);
}

// ****************************************************************************************************
// *    float GetStripGeo(Uint r, Uint p)
//
//  Get the private member RPCs and get the active area of strips in partition p of RPC r
// ****************************************************************************************************

float Infrastructure::GetStripGeo(Uint r, Uint p){
    return GetRPC(r)->GetStripGeo(p);
}
