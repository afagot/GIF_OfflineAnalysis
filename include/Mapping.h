#ifndef MAPPING_H
#define MAPPING_H

//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    Mapping.h
// *
// *    To update
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    07/06/2017
//***************************************************************

#include <map>
#include <string>

#include "types.h"

using namespace std;

// *************************************************************************************************************

const int MAP_OK                            = 0;

// File Errors
const int MAP_ERROR_CANNOT_OPEN_READ_FILE   = 10;

// *************************************************************************************************************

typedef map<Uint,Uint> MappingData;

// *************************************************************************************************************

class Mapping {
    private:
        bool        CheckIfNewLine(char next);
        bool        CheckIfTDCCh(Uint channel);
        string      FileName;
        MappingData Link;
        MappingData Mask;
        int         Error;

    public:
        Mapping();
        Mapping(string baseName);
        ~Mapping();

        void SetFileName(const string filename);
        int Read();
        Uint GetLink(Uint tdcchannel);
        Uint GetMask(Uint rpcchannel);
};

#endif
