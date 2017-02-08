#ifndef CURRENT_H
#define CURRENT_H

//***************************************************************
// *    904 OFFLINE TOOL v4
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    Current.h
// *
// *    Current extraction from Scan_00XXXX_HVX_CAEN.root files
// *
// *    Developped by : Alexis Fagot
// *    07/03/2017
//***************************************************************

#include <string>

using namespace std;

void GetCurrent(string baseName);

#endif // CURRENT_H
