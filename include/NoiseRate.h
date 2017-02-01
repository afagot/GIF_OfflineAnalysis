#ifndef NOISERATE_H
#define NOISERATE_H

//***************************************************************
// *    GIF OFFLINE TOOL v3
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    NoiseRate.h
// *
// *    Extraction of the rates from Scan_00XXXX_HVX_DAQ.root
// *    files + monitoring of strip activities and noise
// *    homogeneity.
// *
// *    Developped by : Alexis Fagot
// *    22/04/2016
//***************************************************************

#include "utils.h"
#include "TTree.h"
#include "IniFile.h"

string          GetBaseName(string fName);
string          GetSavePath(string baseName, string stepID);
map<int,int>    TDCMapping(string baseName);
void            GetNoiseRate(string baseName);

#endif // NOISERATE_H
