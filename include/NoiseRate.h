#ifndef NOISERATE_H
#define NOISERATE_H

#include "utils.h"
#include "TTree.h"
#include "IniFile.h"

string          GetPath(string fName);
string          GetBaseName(string fName);
map<int,int>    TDCMapping();
void            GetNoiseRate(string fName);

#endif // NOISERATE_H
