//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    RPCDetector.cc
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
#include "../include/RPCDetector.h"
#include "../include/IniFile.h"

using namespace std;

// *************************************************************************************************************

RPC::RPC(){

}

// *************************************************************************************************************
// *    RPC(string ID, IniFile *geofile)
//
//  Constructor of the RPC class which is the innermost part of the GIF++ infrasctructure. The RPCs
//  are contained inside the Trolleys. Informations are readout from Dimensions.ini . For details about
//  RPC members, see file RPCDetector.h.
// *************************************************************************************************************

RPC::RPC(string ID, IniFile* geofile){
    name = geofile->stringType(ID,"Name","");
    nGaps = geofile->intType(ID,"Gaps",1);
    nStrips = geofile->intType(ID,"Strips",1);
    nPartitions = geofile->intType(ID,"Partitions",1);

    gaps.clear();
    gapGeo.clear();

    for(Uint g = 0 ; g < nGaps; g++){
        string gapID = "Gap" + intToString(g+1);
        gaps.push_back(geofile->stringType(ID,gapID,""));

        string areaID = "AreaGap" + intToString(g+1);
        gapGeo.push_back(geofile->floatType(ID,areaID,1.));
    }

    stripGeo.clear();
    string partID = "ABCD";

    for(Uint p = 0; p < GetNPartitions(); p++){
        string areaID  = "ActiveArea-"  + CharToString(partID[p]);
        stripGeo.push_back(geofile->floatType(ID,areaID,1.));
    }
}

// *************************************************************************************************************

RPC::~RPC(){

}

// *************************************************************************************************************

string RPC::GetName(){
    return name;
}

// *************************************************************************************************************

Uint RPC::GetNGaps(){
    return nGaps;
}

// *************************************************************************************************************

Uint RPC::GetNPartitions(){
    return nPartitions;
}

// *************************************************************************************************************

Uint RPC::GetNStrips(){
    return nStrips;
}

// *************************************************************************************************************

string RPC::GetGap(Uint g){
    return gaps[g];
}

// *************************************************************************************************************

float RPC::GetGapGeo(Uint g){
    return gapGeo[g];
}

// *************************************************************************************************************

float RPC::GetStripGeo(Uint p){
    return stripGeo[p];
}
