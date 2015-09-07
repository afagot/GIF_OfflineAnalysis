#ifndef NOISERATE_H
#define NOISERATE_H

#include "utils.h"
#include "TTree.h"
#include "IniFile.h"

string          GetPath(string fileName);
string          GetBaseName(string fileName);
float           GetStripSurface(int GeoID, IniFile* GeoFile);
map<int,int>    TDCMapping(string mappingfName);
void            GetNoiseRate(string fName, string trigger);

#endif // NOISERATE_H
