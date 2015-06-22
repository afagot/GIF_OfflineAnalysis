#ifndef NOISERATE_H
#define NOISERATE_H

#include "utils.h"
#include "TTree.h"
#include "IniFile.h"

string          GetBaseName(string fileName);
float           GetStripSurface(string chamberType,char partLabel,IniFile* GeoFile);
map<int,int>    TDCMapping(string mappingfName);
void            GetNoiseRate(string fName, string chamberType);

#endif // NOISERATE_H
