#ifndef DIP_H
#define DIP_H

//***************************************************************
// *    GIF OFFLINE TOOL v3
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    DIP.h
// *
// *    DIP extraction from Scan_00XXXX_HVX_DIP.root files
// *
// *    Developped by : Alexis Fagot
// *    22/04/2016
//***************************************************************

#include <string>

using namespace std;

void GetDIP(string baseName);

#endif // DIP_H
