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

#include "../include/types.h"
#include "../include/utils.h"
#include "../include/GIFTrolley.h"
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
    nTrolleys = geofile->intType("General","nTrolleys",NTROLLEYS);
    TrolleysID = geofile->stringType("General","TrolleysID","");

    Trolleys.clear();

    for(Uint t = 0; t < GetNTrolleys(); t++){
        string trolleyID = "T" + intToString(GetTrolleyID(t));

        Trolley* tempTrolley = new Trolley(trolleyID, geofile);
        Trolleys.push_back(tempTrolley);
    }
}

// ****************************************************************************************************
// *    Infrastructure(const Infrastructure &other)
//
//  Copy constructor
// ****************************************************************************************************

Infrastructure::Infrastructure(const Infrastructure &other){
    nTrolleys = other.nTrolleys;
    TrolleysID = other.TrolleysID;

    Trolleys.clear();
    Trolleys = other.Trolleys;
}

// ****************************************************************************************************
// *    ~Infrastructure()
//
//  Destructor
// ****************************************************************************************************

Infrastructure::~Infrastructure(){
    vector<Trolley*>::iterator it = Trolleys.begin();

    while(it != Trolleys.end())
        delete *(it++);
}

// ****************************************************************************************************
// *    Infrastructure& operator=(const Infrastructure& other)
//
//  Copy operator
// ****************************************************************************************************

Infrastructure& Infrastructure::operator=(const Infrastructure& other){
    if(this != &other){
        nTrolleys = other.nTrolleys;
        TrolleysID = other.TrolleysID;

        vector<Trolley*>::iterator it = Trolleys.begin();
        while(it != Trolleys.end())
            delete *(it++);

        Trolleys.clear();
        Trolleys = other.Trolleys;
    }

    return *this;
}

// ****************************************************************************************************
// *    Uint GetNTrolleys()
//
//  Get the private member nTrolleys
// ****************************************************************************************************

Uint Infrastructure::GetNTrolleys(){
    return nTrolleys;
}

// ****************************************************************************************************
// *    string InfrastructureGetTrolleysID()
//
//  Get the private member TrolleysID
// ****************************************************************************************************

string Infrastructure::GetTrolleysID(){
    return TrolleysID;
}

// ****************************************************************************************************
// *    Uint GetTrolleyID(Uint t)
//
//  Get the private member TrolleysID and extract the ID of Trolley t
// ****************************************************************************************************

Uint Infrastructure::GetTrolleyID(Uint t){
    return CharToInt(TrolleysID[t]);
}

// ****************************************************************************************************
// *    Trolley* GetTrolley(Uint t)
//
//  Get the private member Trolleys and get the Trolley t
// ****************************************************************************************************

Trolley* Infrastructure::GetTrolley(Uint t){
    if(t < Trolleys.size())
        return Trolleys[t];
    else
        return NULL;
}

// ****************************************************************************************************
// *    void DeleteTrolley(Uint t)
//
//  Get the private member Trolleys and delete Trolley t
// ****************************************************************************************************

void Infrastructure::DeleteTrolley(Uint t){
    if(t < Trolleys.size()){
        delete Trolleys[t];
        Trolleys.erase(Trolleys.begin()+t);
        nTrolleys--;
        TrolleysID.erase(TrolleysID.begin()+t);
    }
}

// ****************************************************************************************************
// *    Uint GetNSlots(Uint t)
//
//  Get the private member Trolleys and get the number of slots of Trolley t
// ****************************************************************************************************

Uint Infrastructure::GetNSlots(Uint t){
    return GetTrolley(t)->GetNSlots();
}

// ****************************************************************************************************
// *    string GetSlotsID(Uint t)
//
//  Get the private member Trolleys and get the slots IDs of Trolley t
// ****************************************************************************************************

string Infrastructure::GetSlotsID(Uint t){
    return GetTrolley(t)->GetSlotsID();
}

// ****************************************************************************************************
// *    Uint GetSlotID(Uint t, Uint s)
//
//  Get the private member Trolleys and get the ID of slot s of Trolley t
// ****************************************************************************************************

Uint Infrastructure::GetSlotID(Uint t, Uint s){
    return GetTrolley(t)->GetSlotID(s);
}

// ****************************************************************************************************
// *    RPC* GetRPC(Uint t, Uint r)
//
//  Get the private member Trolleys and get RPC r of Trolley t
// ****************************************************************************************************

RPC* Infrastructure::GetRPC(Uint t, Uint r){
    return GetTrolley(t)->GetRPC(r);
}

// ****************************************************************************************************
// *    string GetName(Uint t, Uint r)
//
//  Get the private member Trolleys and get the name of RPC r of Trolley t
// ****************************************************************************************************

string Infrastructure::GetName(Uint t, Uint r){
    return GetTrolley(t)->GetName(r);
}

// ****************************************************************************************************
// *    Uint GetNGaps(Uint t, Uint r)
//
//  Get the private member Trolleys and get the number of gaps of RPC r of Trolley t
// ****************************************************************************************************

Uint Infrastructure::GetNGaps(Uint t, Uint r){
    return GetTrolley(t)->GetNGaps(r);
}

// ****************************************************************************************************
// *    Uint GetNPartitions(Uint t, Uint r)
//
//  Get the private member Trolleys and get the number of partitions of RPC r of Trolley t
// ****************************************************************************************************

Uint Infrastructure::GetNPartitions(Uint t, Uint r){
    return GetTrolley(t)->GetNPartitions(r);
}

// ****************************************************************************************************
// *    Uint GetNStrips(Uint t, Uint r)
//
//  Get the private member Trolleys and get the number of strips/part. of RPC r of Trolley t
// ****************************************************************************************************

Uint Infrastructure::GetNStrips(Uint t, Uint r){
    return GetTrolley(t)->GetNStrips(r);
}

// ****************************************************************************************************
// *    string GetGap(Uint t, Uint r, Uint g)
//
//  Get the private member Trolleys and get the name of gap g of RPC r of Trolley t
// ****************************************************************************************************

string Infrastructure::GetGap(Uint t, Uint r, Uint g){
    return GetTrolley(t)->GetGap(r,g);
}

// ****************************************************************************************************
// *    float GetGapGeo(Uint t, Uint r, Uint g)
//
//  Get the private member Trolleys and get the active area of gap g of RPC r of Trolley t
// ****************************************************************************************************

float Infrastructure::GetGapGeo(Uint t, Uint r, Uint g){
    return GetTrolley(t)->GetGapGeo(r,g);
}

// ****************************************************************************************************
// *    float GetStripGeo(Uint t, Uint r, Uint p)
//
//  Get the private member Trolleys and get active area of part p strips of RPC r of Trolley t
// ****************************************************************************************************

float Infrastructure::GetStripGeo(Uint t, Uint r, Uint p){
    return GetTrolley(t)->GetStripGeo(r,p);
}
