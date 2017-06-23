#ifndef __GIFTROLLEY_H_
#define __GIFTROLLEY_H_

//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    GIFTrolley.h
// *
// *    To be updated
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    22/06/2017
//***************************************************************

#include <string>
#include <vector>

#include "types.h"
#include "utils.h"
#include "IniFile.h"
#include "RPCDetector.h"

using namespace std;

class Trolley{
    private:
        Uint         nSlots;  //Number of active RPCs in the considered trolley
        string       SlotsID; //Active RPC IDs written into a string
        vector<RPC*> RPCs;    //List of active RPCs

    public:
        //Constructors & destructor
        Trolley();
        Trolley(string ID, IniFile* geofile);
        ~Trolley();

        //Get GIFTrolley members
        Uint   GetNSlots();
        string GetSlotsID();
        Uint   GetSlotID(Uint s);
        RPC*   GetRPC(Uint r);

        //Methods to get members of RPC objects stored in RPCs
        string GetName(Uint r);
        Uint   GetNGaps(Uint r);
        Uint   GetNPartitions(Uint r);
        Uint   GetNStrips(Uint r);
        string GetGap(Uint r, Uint g);
        float  GetGapGeo(Uint r, Uint g);
        float  GetStripGeo(Uint r, Uint p);

        //Set GIFTrolley members
        void SetNSlots(string ID, IniFile* geofile);
        void SetSlotsID(string ID, IniFile* geofile);
        void SetRPCs(string ID, IniFile* geofile);
};

#endif
