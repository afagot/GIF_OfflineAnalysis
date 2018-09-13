#ifndef __INFRASTRUCTURE_H_
#define __INFRASTRUCTURE_H_

//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    Infrastructure.h
// *
// *    Class that defines Infrastructure objects. They are
// *    objects that contain Trolleys, that contains RPCDetectors
// *    (itreproduces GIF++ infrastructure).
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    22/06/2017
//***************************************************************

#include <string>
#include <vector>

#include "types.h"
#include "RPCDetector.h"

using namespace std;

class Infrastructure {
    private:
        Uint         nSlots;  //Number of active RPCs in the considered trolley
        string       SlotsID; //Active RPC IDs written into a string
        vector<RPC*> RPCs;    //List of active RPCs

    public:
        //Constructors, destructor and operator =
        Infrastructure();
        Infrastructure(IniFile* geofile);
        Infrastructure(const Infrastructure& other);
        ~Infrastructure();
        Infrastructure& operator=(const Infrastructure& other);

        //Get Infrastructure members members
        Uint   GetNSlots();
        string GetSlotsID();
        Uint   GetSlotID(Uint s);

        //Manage RPC list
        RPC*   GetRPC(Uint r);
        void   DeleteRPC(Uint r);

        //Methods to get members of RPC objects stored in RPCs
        string GetName(Uint r);
        Uint   GetNGaps(Uint r);
        Uint   GetNPartitions(Uint r);
        Uint   GetNStrips(Uint r);
        string GetGap(Uint r, Uint g);
        float  GetGapGeo(Uint r, Uint g);
        float  GetStripGeo(Uint r, Uint p);
};

#endif
