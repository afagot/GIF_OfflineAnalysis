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

#include "utils.h"
#include "IniFile.h"
#include "RPCDetector.h"

using namespace std;

class GIFTrolley{
    private:
        Uint        nSlots;  //Number of active RPCs in the considered trolley
        string      SlotsID; //Active RPC IDs written into a string
        vector<RPC> RPCs;    //List of active RPCs (struct)

    public:
        GIFTrolley();
        GIFTrolley(string ID, IniFile* geofile);
        virtual ~GIFTrolley();

        Uint GetNSlots();
        string GetSlotsID();
        string GetSlotID(Uint i);

        void SetNSlots(string ID, IniFile* geofile);
        void SetSlotsID(string ID, IniFile* geofile);
        void SetRPCs(string ID, IniFile* geofile);
};

#endif
