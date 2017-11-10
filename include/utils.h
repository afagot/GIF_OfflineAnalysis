#ifndef __UTILS_H_
#define __UTILS_H_

//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    utils.h
// *
// *    All usefull functions (type cast, time stamps, histos...)
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    07/06/2017
//***************************************************************

#include <string>
#include <vector>

#include "TTree.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"

#include "types.h"
#include "Mapping.h"
#include "Infrastructure.h"

using namespace std;

//****************************************************************************

//Functions (more details in utils.cc)

Uint    CharToInt(char& C);
string  CharToString(char& C);
string  intToString(int value);
string  longTostring(long value);
string  floatTostring(float value);

bool    existFile(string ROOTName);
void    WritePath(string basename);
void    SetTitleName(string rpcID, Uint partition, char* Name,
                     char* Title,string Namebase, string Titlebase);
bool    IsEfficiencyRun(TString* runtype);
bool    IsCorruptedEvent(int qflag);
float   GetTH1Mean(TH1* H);
float   GetTH1StdDev(TH1* H);
float   GetChipBin(TH1* H, Uint chip);
void    SetTH1(TH1* H, string xtitle, string ytitle);
void    SetTH2(TH2* H, string xtitle, string ytitle, string ztitle);

#endif // UTILS_H
