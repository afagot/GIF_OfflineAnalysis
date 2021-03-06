#ifndef __CURRENT_H_
#define __CURRENT_H_

//***************************************************************
// *    GIF OFFLINE TOOL v7
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    Current.h
// *
// *    Current extraction from Scan_00XXXX_HVX_CAEN.root files.
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    07/06/2017
//***************************************************************

#include <string>

using namespace std;

void GetCurrent(string baseName);

#endif // CURRENT_H
