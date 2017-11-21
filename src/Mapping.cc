//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    Mapping.cc
// *
// *    Class that defines Mapping objects. Mapping objects are
// *    used to contain and easily jump from TDC channels to RPC
// *    RPC channels (i.e. strips). This is useful to plot the
// *    results in a human comprehensible way.
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    07/06/2017
//***************************************************************

#include <iostream>
#include <map>
#include <string>
#include <fstream>

#include "../include/Mapping.h"
#include "../include/MsgSvc.h"
#include "../include/types.h"
#include "../include/utils.h"

using namespace std;

// ****************************************************************************************************
// *    Mapping()
//
//  Default constructor
// ****************************************************************************************************

Mapping::Mapping(){

}

// ****************************************************************************************************
// *    Mapping(string baseName)
//
//  Constructor. It needs the information stored in the ChannelsMapping.csv file
// ****************************************************************************************************

Mapping::Mapping(string baseName){
    SetFileName(baseName);
}

// ****************************************************************************************************
// *    ~Mapping()
//
//  Destructor
// ****************************************************************************************************

Mapping::~Mapping(){

};

// ****************************************************************************************************
// *    bool CheckIfNewLine(char next)
//
//  Private method used to parse the mapping file. It looks for '\n' characters.
// ****************************************************************************************************

bool Mapping::CheckIfNewLine(char next){
    return ( next == '\n' );
}

// ****************************************************************************************************
// *    bool CheckIfTDCCh(Uint channel)
//
//  Private method used to parse the mapping file. It checks that the channel is in the TDCs.
// ****************************************************************************************************

bool Mapping::CheckIfTDCCh(Uint channel){
    return ( channel > 0 || channel <= 5127);
}

// ****************************************************************************************************
// *    void SetFileName(const string filename)
//
//  Set the name of private membre FileName. This is the name of the mapping file.
// ****************************************************************************************************

void Mapping::SetFileName(const string filename){
    FileName = filename;
}

// ****************************************************************************************************
// *    int Read()
//
//  Open, read and save into the mapping buffer (private members Link and Mask) the content of the
//  mapping file.
// ****************************************************************************************************

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
            //In case of 2 columns, the enfline will have a format
            //where the \n is separated from the TDCCh column by an
            //empty character. Thus it is needed to read 2 char.
            //In the case of 3 columns that means that we will be
            //reding both the space or tab and the mask. The mask
            //is then taken from the last char readout.
            char next;
            map.get(next);
            map.get(next);
            if(CheckIfNewLine(next))
                mask = 1;
            else
                mask = CharToInt(next);

            if(CheckIfTDCCh(TDCCh)){
                Link[TDCCh] = RPCCh;
                ReverseLink[RPCCh] = TDCCh;
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

// ****************************************************************************************************
// *    Uint GetLink(Uint tdcchannel)
//
//  Get link in betweeen TDC channel tdcchannel and the corresponding RPC channel.
// ****************************************************************************************************

Uint Mapping::GetLink(Uint tdcchannel){
    return Link[tdcchannel];
}

// ****************************************************************************************************
// *    Uint GetReverse(Uint rpcchannel)
//
//  Get link in betweeen RPC channel tdcchannel and the corresponding TDC channel.
// ****************************************************************************************************

Uint Mapping::GetReverse(Uint rpcchannel){
    return ReverseLink[rpcchannel];
}

// ****************************************************************************************************
// *    Uint GetMask(Uint rpcchannel)
//
//  Get mask of RPC channel rpcchannel to know if it is masked or active.
// ****************************************************************************************************

Uint Mapping::GetMask(Uint rpcchannel){
    return Mask[rpcchannel];
}
