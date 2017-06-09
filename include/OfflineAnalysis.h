#ifndef OFFLINE_H
#define OFFLINE_H

//***************************************************************
// *    GIF OFFLINE TOOL v5
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    OfflineAnalysis.h
// *
// *    Extraction of the data from Scan_00XXXX_HVX_DAQ.root
// *    files :
// *    + Noise or gamma rate estimation
// *    + Channel activity monitoring
// *    + First rude estimation of efficiency (L0)
// *    + Noise or gamma cluster size estimation
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    07/06/2017
//***************************************************************

#include <string>
#include <map>

using namespace std;

void OfflineAnalysis(string baseName);

#endif // OFFLINE_H
