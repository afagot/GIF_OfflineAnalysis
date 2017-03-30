#ifndef EFFCLUST_H
#define EFFCLUST_H

//***************************************************************
// *    904 OFFLINE TOOL v4
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    EffClustZero.h
// *
// *    Level 0 of efficiency, cluster size and cluster
// *    multiplicity extraction from Scan_00XXXX_HVX_DAQ.root
// *    files. This algorithm does not use any sort of tracking
// *    and only provide a basic understanding of the chamber
// *    performance.
// *
// *    Developped by : Alexis Fagot
// *    30/03/2017
//***************************************************************

#include <string>

using namespace std;

void GetEffClustZero(string baseName);

#endif // EFFCLUST_H

