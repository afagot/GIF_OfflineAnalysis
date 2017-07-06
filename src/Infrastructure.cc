//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    Infrastructure.cc
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
#include "../include/Infrastructure.h"
#include "../include/GIFTrolley.h"

using namespace std;

// *************************************************************************************************************

Infrastructure::Infrastructure(){

}

// *************************************************************************************************************
// *    Infrastructure(IniFile *geofile)
//
//  Set up the Infrastructure structure needed that represents the GIF++ setup at the moment of data
//  taking, i.e. the active trolleys and the RPC chambers they contain in their slots. Informations
//  are readout from Dimensions.ini . For details about Trolley members, see file utils.h.
// *************************************************************************************************************

Infrastructure::Infrastructure(IniFile *geofile){
    nTrolleys = geofile->intType("General","nTrolleys",NTROLLEYS);
    TrolleysID = geofile->stringType("General","TrolleysID","");

    Trolleys.clear();

    for(Uint t = 0; t < GetNTrolleys(); t++){
        string trolleyID = "T" + intToString(GetTrolleyID(t));

        Trolley* tempTrolley = new Trolley(trolleyID, geofile);
        Trolleys.push_back(tempTrolley);
        delete tempTrolley;
    }
}

// *************************************************************************************************************

Infrastructure::~Infrastructure(){
    vector<Trolley*>::iterator it = Trolleys.begin();

    while(it != Trolleys.end())
        delete *(it++);
}

// *************************************************************************************************************

Uint Infrastructure::GetNTrolleys(){
    return nTrolleys;
}

// *************************************************************************************************************

string Infrastructure::GetTrolleysID(){
    return TrolleysID;
}

// *************************************************************************************************************

Uint Infrastructure::GetTrolleyID(Uint t){
    return CharToInt(TrolleysID[t]);
}

// *************************************************************************************************************

Trolley* Infrastructure::GetTrolley(Uint t){
    if(t < Trolleys.size())
        return Trolleys[t];
    else
        return NULL;
}

void Infrastructure::DeleteTrolley(Uint t){
    if(t < Trolleys.size()){
        delete Trolleys[t];
        Trolleys.erase(Trolleys.begin()+t);
        nTrolleys--;
        TrolleysID.erase(TrolleysID.begin()+t);
    }
}

// *************************************************************************************************************

Uint Infrastructure::GetNSlots(Uint t){
    return GetTrolley(t)->GetNSlots();
}

// *************************************************************************************************************

string Infrastructure::GetSlotsID(Uint t){
    return GetTrolley(t)->GetSlotsID();
}

// *************************************************************************************************************

Uint Infrastructure::GetSlotID(Uint t, Uint s){
    return GetTrolley(t)->GetSlotID(s);
}

// *************************************************************************************************************

RPC* Infrastructure::GetRPC(Uint t, Uint r){
    return GetTrolley(t)->GetRPC(r);
}

// *************************************************************************************************************

string Infrastructure::GetName(Uint t, Uint r){
    return GetTrolley(t)->GetName(r);
}

// *************************************************************************************************************

Uint Infrastructure::GetNGaps(Uint t, Uint r){
    return GetTrolley(t)->GetNGaps(r);
}

// *************************************************************************************************************

Uint Infrastructure::GetNPartitions(Uint t, Uint r){
    return GetTrolley(t)->GetNPartitions(r);
}

// *************************************************************************************************************

Uint Infrastructure::GetNStrips(Uint t, Uint r){
    return GetTrolley(t)->GetNStrips(r);
}

// *************************************************************************************************************

string Infrastructure::GetGap(Uint t, Uint r, Uint g){
    return GetTrolley(t)->GetGap(r,g);
}

// *************************************************************************************************************

float Infrastructure::GetGapGeo(Uint t, Uint r, Uint g){
    return GetTrolley(t)->GetGapGeo(r,g);
}

// *************************************************************************************************************

float Infrastructure::GetStripGeo(Uint t, Uint r, Uint p){
    return GetTrolley(t)->GetStripGeo(r,p);
}
