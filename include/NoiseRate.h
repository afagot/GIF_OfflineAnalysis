#ifndef NOISERATE_H
#define NOISERATE_H

#include "utils.h"
#include "TTree.h"

string          GetBaseName(string fileName);
void            SetBranchAddresses(TTree* mytree, RAWData mydata);
map<int,int>    TDCMapping(string mappingfName);

void            GetNoiseRate(string fName, string chamberType);

#endif // NOISERATE_H
