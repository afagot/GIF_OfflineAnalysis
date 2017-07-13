//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    RPCDetector.cc
// *
// *    Class the defines RPC objects. RPCs are objects
// *    containing all the useful information about the
// *    detectors being tested inside of GIF++ needed to
// *    make the rate and current calculations.
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

// ****************************************************************************************************
// *    RPC()
//
//  Default constructor
// ****************************************************************************************************

RPC::RPC(){

}

// ****************************************************************************************************
// *    RPC(string ID, IniFile *geofile)
//
//  Constructor. It needs to access the information stored in the buffer of the Dimensions.ini file.
// ****************************************************************************************************

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

// ****************************************************************************************************
// *    RPC(const RPC &other)
//
//  Copy constructor
// ****************************************************************************************************

RPC::RPC(const RPC &other){
    name = other.name;
    nGaps = other.nGaps;
    nPartitions = other.nPartitions;
    nStrips = other.nStrips;
    gaps = other.gaps;
    gapGeo = other.gapGeo;
    stripGeo = other.stripGeo;
}

// ****************************************************************************************************
// *    RPC& operator =(const RPC& other)
//
//  Copy operator
// ****************************************************************************************************

RPC& RPC::operator =(const RPC& other){
    if(this != &other){
        name = other.name;
        nGaps = other.nGaps;
        nPartitions = other.nPartitions;
        nStrips = other.nStrips;
        gaps = other.gaps;
        gapGeo = other.gapGeo;
        stripGeo = other.stripGeo;
    }

    return *this;
}

// ****************************************************************************************************
// *    ~RPC()
//
//  Destructor
// ****************************************************************************************************

RPC::~RPC(){

}

// ****************************************************************************************************
// *    string GetName()
//
//  Get the private member name
// ****************************************************************************************************

string RPC::GetName(){
    return name;
}

// ****************************************************************************************************
// *    Uint GetNGaps()
//
//  Get the private member nGaps
// ****************************************************************************************************

Uint RPC::GetNGaps(){
    return nGaps;
}

// ****************************************************************************************************
// *    Uint GetNPartitions()
//
//  Get the private member nPartitions
// ****************************************************************************************************

Uint RPC::GetNPartitions(){
    return nPartitions;
}

// ****************************************************************************************************
// *    Uint GetNStrips()
//
//  Get the private member nStrips
// ****************************************************************************************************

Uint RPC::GetNStrips(){
    return nStrips;
}

// ****************************************************************************************************
// *    string GetGap(Uint g)
//
//  Get the private member gaps and get the name of gap g
// ****************************************************************************************************

string RPC::GetGap(Uint g){
    return gaps[g];
}

// ****************************************************************************************************
// *    float GetGapGeo(Uint g)
//
//  Get the private member gapGeo and get the active area of gap g
// ****************************************************************************************************

float RPC::GetGapGeo(Uint g){
    return gapGeo[g];
}

// ****************************************************************************************************
// *    float GetStripGeo(Uint p)
//
//  Get the private member stripGeo and get the active area of strips in partition p
// ****************************************************************************************************

float RPC::GetStripGeo(Uint p){
    return stripGeo[p];
}
