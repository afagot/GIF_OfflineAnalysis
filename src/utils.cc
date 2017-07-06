//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    utils.cc
// *
// *    All usefull functions (type cast, time stamps,...)
// *    and structures (used for the GIF layout definition).
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    07/06/2017
//***************************************************************

#include <cstdlib>
#include <sstream>
#include <ctime>
#include <fstream>
#include <cstdio>
#include <map>
#include <algorithm>

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TF1.h"
#include "TStyle.h"
#include "THistPainter.h"

#include "../include/types.h"
#include "../include/utils.h"
#include "../include/MsgSvc.h"
#include "../include/Mapping.h"
#include "../include/Infrastructure.h"
#include "../include/RPCHit.h"

using namespace std;

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//  Function that casts a char into an int
// ****************************************************************************************************

Uint CharToInt(char &C){
    stringstream ss;
    ss << C;

    Uint I;
    ss >> I;
    return I;
}

// ****************************************************************************************************
// *    string CharToString(char& C)
//
//  Function that casts a char into a string
// ****************************************************************************************************

string CharToString(char& C){
    stringstream ss;
    ss << C;

    string S;
    ss >> S;
    return S;
}

// ****************************************************************************************************
// *    string intTostring(int value)
//
//  Function that casts an int into a string
// ****************************************************************************************************

string intToString(int value){
    string word;
    stringstream ss;
    ss << value;
    ss>> word;

    return word;
}

// ****************************************************************************************************
// *    string longTostring(long value)
//
//  Function that casts a long into a string
// ****************************************************************************************************

string longTostring(long value){
    string word;
    stringstream ss;
    ss << value;
    ss>> word;

    return word;
}

// ****************************************************************************************************
// *    string floatTostring(float value)
//
//  Function that casts a float into a string
// ****************************************************************************************************

string floatTostring(float value){
    string word;
    stringstream ss;
    ss << value;
    ss>> word;

    return word;
}

// ****************************************************************************************************
// *    bool existFiles(string baseName)
//
//  Function that test if the root files created during the data taking exist in the scan directory
//  or not. This is the needed condition for the offline tool to start.
// ****************************************************************************************************

bool existFile(string ROOTName){
    TFile ROOTFile(ROOTName.c_str());

    return (ROOTFile.IsOpen());
}

// ****************************************************************************************************
// *    string GetLogTimeStamp()
//
//  Function that gets the system time. The output format of this function has been optimised to be
//  used in the log file. For each log message, the line starts with this time stamp.
// ****************************************************************************************************

string GetLogTimeStamp(){
    stringstream stream;

    //Get time information
    time_t t = time(0);
    struct tm *Time = localtime(&t);
    int Y = Time->tm_year + 1900;
    int M = Time->tm_mon + 1;
    int D = Time->tm_mday;
    int h = Time->tm_hour;
    int m = Time->tm_min;
    int s = Time->tm_sec;

    //Set the Date
    //Format is YYYY-MM-DD.hh:mm:ss.
    string Date;

    stream << setfill('0') << setw(4) << Y << "-"
           << setfill('0') << setw(2) << M << "-"
           << setfill('0') << setw(2) << D << "."
           << setfill('0') << setw(2) << h << ":"
           << setfill('0') << setw(2) << m << ":"
           << setfill('0') << setw(2) << s << ".";

    stream >> Date;
    stream.clear();

    return Date;
}

// ****************************************************************************************************
// *    void WritePath(string baseName)
//
//  Function that uses the argument "baseName" passed to the main function to write the location of
//  scan's log file (typically into the scan's directory you are analising the files from) in to a
//  system file placed into WEB DCS system directory. This location is then read out by the function
//  that writes the logs into this log file in the scan directory.
//  The choice has been made to use this solution instead of passing the path as an argument of the
//  messaging function in order to make the code a bit lighter (it saves an extra argument at several
//  places of the code).
// ****************************************************************************************************

void WritePath(string baseName){
    //First let's write the path to the log file
    string logpath = baseName.substr(0, baseName.find_last_of("/")+1) + "log.txt";

    ofstream logpathfile(__logpath.c_str(), ios::out);
    logpathfile << logpath;
    logpathfile.close();
}

// ****************************************************************************************************
// *    void SetBeamWindow (muonPeak &PeakTime, muonPeak &PeakWidth,
// *                        TTree* mytree, map<int,int> RPCChMap, Infrastructure GIFInfra)
//
//  Loops over all the data contained inside of the ROOT file and determines for each RPC the center
//  of the muon peak and its spread. Then saves the result in 2 3D tables (#D bescause it follows the
//  dimensions of the GIF++ infrastructure : Trolley, RPC slots and Partitions).
// ****************************************************************************************************

void SetBeamWindow (muonPeak &PeakTime, muonPeak &PeakWidth,
                    TTree* mytree, Mapping* RPCChMap, Infrastructure* Infra){
    RAWData mydata;

    mydata.TDCCh = new vector<Uint>;
    mydata.TDCTS = new vector<float>;
    mydata.TDCCh->clear();
    mydata.TDCTS->clear();

    mytree->SetBranchAddress("EventNumber",    &mydata.iEvent);
    mytree->SetBranchAddress("number_of_hits", &mydata.TDCNHits);
    mytree->SetBranchAddress("TDC_channel",    &mydata.TDCCh);
    mytree->SetBranchAddress("TDC_TimeStamp",  &mydata.TDCTS);

    GIFH1Array tmpTimeProfile;
    GIFfloatArray noiseHits = {{{0.}}};
    int binWidth = TIMEBIN;

    for(Uint tr = 0; tr < NTROLLEYS; tr++)
        for(Uint sl = 0; sl < NSLOTS; sl++)
            for(Uint p = 0; p < NPARTITIONS; p++){
                string name = "tmpTProf" + intToString(tr) + intToString(sl) +  intToString(p);
                tmpTimeProfile.rpc[tr][sl][p] = new TH1F(name.c_str(),name.c_str(),BMTDCWINDOW/TIMEBIN,0.,BMTDCWINDOW);
            }

    //Loop over the entries to get the hits and fill the time distribution + count the
    //noise hits in the window around the peak

    for(Uint i = 0; i < mytree->GetEntries(); i++){
        mytree->GetEntry(i);

        for(int h = 0; h < mydata.TDCNHits; h++){
            Uint channel = mydata.TDCCh->at(h);
            Uint timing = mydata.TDCTS->at(h);

            //Get rid of the noise hits outside of the connected channels
            if(channel > 5127) continue;
            if(RPCChMap->GetLink(channel) == 0) continue;

            RPCHit tmpHit(RPCChMap->GetLink(channel), timing, Infra);
            Uint T = tmpHit.GetTrolley();
            Uint S = tmpHit.GetStation()-1;
            Uint P = tmpHit.GetPartition()-1;

            tmpTimeProfile.rpc[T][S][P]->Fill(tmpHit.GetTime());
        }
    }

    //Compute the average number of noise hits per 10ns bin and subtract it to the time
    //distribution

    muonPeak center;
    muonPeak lowlimit;
    muonPeak highlimit;

    for(Uint tr = 0; tr < NTROLLEYS; tr++){
        for(Uint sl = 0; sl < NSLOTS; sl++){
            for(Uint p = 0; p < NPARTITIONS; p++){
                //Fit with a gaussian the "Good TDC Time"
                TF1 *slicefit = new TF1("slicefit","gaus(0)",lowlimit.rpc[tr][sl][p],highlimit.rpc[tr][sl][p]);

                //Amplitude
                slicefit->SetParameter(0,50);
                slicefit->SetParLimits(0,1,100000);
                //Mean value
                slicefit->SetParameter(1,center.rpc[tr][sl][p]);
                slicefit->SetParLimits(1,lowlimit.rpc[tr][sl][p],highlimit.rpc[tr][sl][p]);
                //RMS
                slicefit->SetParameter(2,20);
                slicefit->SetParLimits(2,1,40);

                if(tmpTimeProfile.rpc[tr][sl][p]->GetEntries() > 0.){
                    center.rpc[tr][sl][p] = (float)tmpTimeProfile.rpc[tr][sl][p]->GetMaximumBin()*TIMEBIN;
                    lowlimit.rpc[tr][sl][p] = center.rpc[tr][sl][p] - 40.;
                    highlimit.rpc[tr][sl][p] = center.rpc[tr][sl][p] + 40.;

                    float timeWdw = BMTDCWINDOW-TIMEREJECT-(highlimit.rpc[tr][sl][p]-lowlimit.rpc[tr][sl][p]);

                    int nNoiseHitsLow =
                            tmpTimeProfile.rpc[tr][sl][p]->Integral(TIMEREJECT/TIMEBIN,lowlimit.rpc[tr][sl][p]/TIMEBIN);
                    int nNoiseHitsHigh =
                            tmpTimeProfile.rpc[tr][sl][p]->Integral(highlimit.rpc[tr][sl][p]/TIMEBIN,BMTDCWINDOW/TIMEBIN);

                    noiseHits.rpc[tr][sl][p] = (float)binWidth*(nNoiseHitsLow+nNoiseHitsHigh)/timeWdw;

                    for(Uint b = 1; b <= BMTDCWINDOW/binWidth; b++){
                        float binContent = (float)tmpTimeProfile.rpc[tr][sl][p]->GetBinContent(b);
                        float correctedContent =
                                (binContent < noiseHits.rpc[tr][sl][p]) ? 0. : binContent-noiseHits.rpc[tr][sl][p];
                        tmpTimeProfile.rpc[tr][sl][p]->SetBinContent(b,correctedContent);
                    }

                    tmpTimeProfile.rpc[tr][sl][p]->Fit(slicefit,"QR");
                }

                PeakTime.rpc[tr][sl][p] = slicefit->GetParameter(1);
                PeakWidth.rpc[tr][sl][p] = 3.*slicefit->GetParameter(2);

                delete tmpTimeProfile.rpc[tr][sl][p];
                delete slicefit;
            }
        }
    }
}

// ****************************************************************************************************
// *    void SetTitleName(string rpcID, Uint partition, char* Name, char* Title,
// *                      string Namebase, string Titlebase)
//
//  Builds the name and title of ROOT objects.
// ****************************************************************************************************

void SetTitleName(string rpcID, Uint partition, char* Name, char* Title,
                  string Namebase, string Titlebase){

    string P[4] = {"A","B","C","D"};
    sprintf(Name,"%s_%s_%s",Namebase.c_str(),rpcID.c_str(),P[partition].c_str());
    sprintf(Title,"%s %s_%s",Titlebase.c_str(),rpcID.c_str(),P[partition].c_str());
}

// ****************************************************************************************************
// *    bool IsEfficiencyRun(TString* runtype)
//
//  Returns true if the comparison of the runtype to the word "efficiency" gives 0 (no difference).
// ****************************************************************************************************

bool IsEfficiencyRun(TString* runtype){
    return (runtype->CompareTo("efficiency") == 0);
}

// ****************************************************************************************************
// *    float GetTH1Mean(TH1* H)
//
//  Returns the mean along the Y axis of a TH1 (value not given by the ROOT statbox).
// ****************************************************************************************************

float GetTH1Mean(TH1* H){
    int nBins = H->GetNbinsX();
    int nActive = nBins;
    float mean = 0.;

    for(int b = 1; b <= nBins; b++){
        float value = H->GetBinContent(b);
        mean += value;
        if(value == 0.) nActive--;
    }

    if(nActive != 0) mean /= (float)nActive;
    else mean = 0.;

    return mean;
}

// ****************************************************************************************************
// *    float GetTH1StdDev(TH1* H)
//
//  Returns the spread of the distribution of values along the Y axis of a TH1 (value not given by
//  the ROOT statbox).
// ****************************************************************************************************

float GetTH1StdDev(TH1* H){
    int nBins = H->GetNbinsX();
    float mean = GetTH1Mean(H);
    float stddev = 0.;
    float variance = 0.;

    for(int b = 1; b <= nBins; b++)
        variance += (H->GetBinContent(b)-mean)*(H->GetBinContent(b)-mean);

    stddev = sqrt(variance/nBins);

    return stddev;
}

// ****************************************************************************************************
// *    float GetChipBin(TH1* H)
//
//  Returns the chip average value from strip histograms by grouping active strips.
// ****************************************************************************************************

float GetChipBin(TH1* H, Uint chip){
    Uint start = 1 + chip*NSTRIPSCHIP;
    int nActive = NSTRIPSCHIP;
    float mean = 0.;

    for(Uint b = start; b <= (chip+1)*NSTRIPSCHIP; b++){
        float value = H->GetBinContent(b);
        mean += value;
        if(value == 0.) nActive--;
    }

    if(nActive != 0) mean /= (float)nActive;
    else mean = 0.;

    return mean;
}

// ****************************************************************************************************
// *    void SetTH1(TH1* H, string xtitle, string ytitle)
//
//  Sets the title of X and Y axis of a TH1.
// ****************************************************************************************************

void SetTH1(TH1* H, string xtitle, string ytitle){
    H->SetXTitle(xtitle.c_str());
    H->SetYTitle(ytitle.c_str());
}

// ****************************************************************************************************
// *    void SetTH2(TH2* H, string xtitle, string ytitle, string ztitle)
//
//  Sets the title of X, Y and Z axis of a TH1.
// ****************************************************************************************************

//Draw 2D histograms
void SetTH2(TH2* H, string xtitle, string ytitle, string ztitle){
    H->SetXTitle(xtitle.c_str());
    H->SetYTitle(ytitle.c_str());
    H->SetXTitle(ztitle.c_str());
}
