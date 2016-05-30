#ifndef NOISERATE_H
#define NOISERATE_H

#include "utils.h"
#include "TTree.h"
#include "IniFile.h"

string          GetBaseName(string fName);
string          GetPath(string baseName, string stepID);
map<int,int>    TDCMapping();
void            GetNoiseRate(string fName);

#endif // NOISERATE_H
