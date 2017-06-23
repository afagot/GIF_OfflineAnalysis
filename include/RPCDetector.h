#ifndef __RPCDETECTOR_H_
#define __RPCDETECTOR_H_

//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    RPCDetector.h
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

using namespace std;

class RPC{
    private:
        string          name;        //RPC name as in webDCS database
        Uint            nGaps;       //Number of gaps in the RPC
        Uint            nPartitions; //Number of partitions in the RPC
        Uint            nStrips;      //Number of strips per partition
        vector<string>  gaps;        //List of gap labels (BOT, TOP, etc...)
        vector<float>   gapGeo;      //List of gap active areas
        vector<float>   stripGeo;    //List of strip active areas

    public:
        RPC();
        RPC(string ID, IniFile* geofile);
        virtual ~RPC();

        string GetName();
        Uint GetNGaps();
        Uint GetNPartitions();
        Uint GetNStrips();
        string gapsElem(Uint i);
        float gGeoElem(Uint i);
        float sGeoElem(Uint i);

        void SetName(string ID, IniFile* geofile);
        void SetNGaps(string ID, IniFile* geofile);
        void SetNPartitions(string ID, IniFile* geofile);
        void SetNStrips(string ID, IniFile* geofile);
        void SetGapNameList(string ID, IniFile* geofile);
        void SetGapGeoList(string ID, IniFile* geofile);
        void SetStripGeoList(string ID, IniFile* geofile);
};

#endif
