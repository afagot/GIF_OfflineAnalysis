#ifndef EFFCLUST_H
#define EFFCLUST_H

//***************************************************************
// *    GIF++ OFFLINE TOOL v4
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    EffLevelZero.h
// *
// *    Level 0 efficiency extraction from Scan_00XXXX_HVX_DAQ.root
// *    files. This algorithm does not use any sort of tracking
// *    and only provide a basic understanding of the chamber
// *    performance.
// *
// *    Developped by : Alexis Fagot
// *    30/03/2017
//***************************************************************

#include <string>

using namespace std;

void GetEffLevelZero(string baseName);

#endif // EFFCLUST_H
