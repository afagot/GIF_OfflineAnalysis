//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    Mapping.cc
// *
// *    To update
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    07/06/2017
//***************************************************************

#include <map>
#include <string>
#include <fstream>

#include "../include/Mapping.h"
#include "../include/MsgSvc.h"
#include "../include/types.h"

using namespace std;

// *************************************************************************************************************

Mapping::Mapping(){

}

// *************************************************************************************************************

Mapping::Mapping(string baseName){
    SetFileName(baseName);
}

Mapping::~Mapping(){

};

bool Mapping::CheckIfNewLine(char next){
    return ( next == '\n' );
}

bool Mapping::CheckIfTDCCh(Uint channel){
    return ( channel <= 0 || channel > 5127);
}

void Mapping::SetFileName(const string filename){
    FileName = filename;
}

int Mapping::Read(){
    ifstream map(FileName.c_str());
    Uint RPCCh, TDCCh, mask;

    // Loading the file into the parser
    if(map){
        Error = MAP_OK;

        while (map.good()) { //Fill the map with RPC and TDC channels
            map >> RPCCh >> TDCCh;

            //Check the TDC mapping file format:
            // 2 columns -> old format
            // 3 columns -> new format including mask
            char next;
            map.get(next);
            if(CheckIfNewLine(next))
                mask = 1;
            else
                map >> mask;

            if(CheckIfTDCCh(TDCCh)){
                Link[TDCCh] = RPCCh;
                Mask[RPCCh] = mask;
            }
        }

        map.close();

        return Error;
    } else {
        Error = MAP_ERROR_CANNOT_OPEN_READ_FILE;
        MSG_ERROR("[Offline-Mapping] Read error " + to_string(Error));

        return Error;
    }
}

Uint Mapping::GetLink(Uint tdcchannel){
    return Link[tdcchannel];
}

Uint Mapping::GetMask(Uint rpcchannel){
    return Mask[rpcchannel];
}
