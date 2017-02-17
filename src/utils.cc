//***************************************************************
// *    GIF OFFLINE TOOL v4
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    utils.cc
// *
// *    All usefull functions (type cast, time stamps,...)
// *    and structures (used for the GIF layout definition).
// *
// *    Developped by : Alexis Fagot
// *    07/03/2017
//***************************************************************

#include <cstdlib>
#include <sstream>
#include <ctime>
#include <fstream>
#include <cstdio>
#include <map>

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TF1.h"
#include "TStyle.h"
#include "THistPainter.h"

#include "../include/utils.h"

using namespace std;


// ****************************************************************************************************
// *    bool existFiles(string baseName)
//
//  Function that test if the 3 root files created during the data taking exist in the scan directory
//  or not. This is the needed condition for the offline tool to start.
// ****************************************************************************************************

bool existFile(string ROOTName){
    TFile ROOTFile(ROOTName.c_str());

    return (ROOTFile.IsOpen());
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//  Function that casts a char into an int
// ****************************************************************************************************

unsigned int CharToInt(char &C){
    stringstream ss;
    ss << C;

    unsigned int I;
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
// *    void SetRPC(RPC &rpc, string ID, IniFile *geofile)
//
//  Set up the RPC structure needed which is the innermost part of the GIF++ infrasctructure. The RPCs
//  Are contained inside the Trolleys. For details about RPC members, see file utils.h.
// ****************************************************************************************************

void SetRPC(RPC &rpc, string ID, IniFile *geofile){
    rpc.name        = geofile->stringType(ID,"Name","");
    rpc.nPartitions = geofile->intType(ID,"Partitions",NPARTITIONS);
    rpc.nGaps       = geofile->intType(ID,"Gaps",0);

    for(unsigned int g = 0 ; g < rpc.nGaps; g++){
        string gapID = "Gap" + intToString(g+1);
        rpc.gaps.push_back(geofile->stringType(ID,gapID,""));

        string areaID = "Area" + gapID;
        rpc.gapGeo.push_back(geofile->floatType(ID,areaID,1.));
    }

    rpc.strips      = geofile->intType(ID,"Strips",NSLOTS);
    string partID = "ABCD";

    for(unsigned int p = 0; p < rpc.nPartitions; p++){
        string areaID  = "ActiveArea-"  + CharToString(partID[p]);
        float area = geofile->floatType(ID,areaID,1.);
        rpc.stripGeo.push_back(area);
    }
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//
// ****************************************************************************************************


void SetTrolley(GIFTrolley &trolley, string ID, IniFile *geofile){
    trolley.nSlots = geofile->intType(ID,"nSlots",NSLOTS);
    trolley.SlotsID = geofile->stringType(ID,"SlotsID","");

    for(unsigned int s = 0; s < trolley.nSlots; s++){
        string rpcID = ID + "S" + CharToString(trolley.SlotsID[s]);

        RPC temprpc;
        SetRPC(temprpc,rpcID,geofile);
        trolley.RPCs.push_back(temprpc);
    }
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//
// ****************************************************************************************************

void SetInfrastructure(Infrastructure &infra, IniFile *geofile){
    infra.nTrolleys = geofile->intType("General","nTrolleys",NTROLLEYS);
    infra.TrolleysID = geofile->stringType("General","TrolleysID","");
    infra.Trolleys.clear();

    for(unsigned int t = 0; t < infra.nTrolleys; t++){
        string trolleyID = "T" + CharToString(infra.TrolleysID[t]);

        GIFTrolley tempTrolley;
        SetTrolley(tempTrolley, trolleyID, geofile);
        infra.Trolleys.push_back(tempTrolley);
    }
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//
// ****************************************************************************************************

//Name of histograms
void SetTitleName(string rpcID, unsigned int partition, char* Name, char* Title, string Namebase, string Titlebase){
    string P[4] = {"A","B","C","D"};
    sprintf(Name,"%s_%s_%s",Namebase.c_str(),rpcID.c_str(),P[partition].c_str());
    sprintf(Title,"%s %s_%s",Titlebase.c_str(),rpcID.c_str(),P[partition].c_str());
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//
// ****************************************************************************************************

//Set the RPCHit variables
void SetRPCHit(RPCHit& Hit, int Channel, float TimeStamp, Infrastructure Infra){
    Hit.Channel     = Channel;                      //RPC channel according to mapping (5 digits)
    Hit.Trolley     = Channel/10000;                //0, 1 or 3 (1st digit of the RPC channel)
    Hit.Station     = (Channel%10000)/1000;         //From 1 to 4 (2nd digit)
    Hit.Strip       = Channel%1000;                 //From 1 to 128 (3 last digits)

    int nStripsPart = 0;
    for(unsigned int i = 0; i < Infra.nTrolleys; i++){
        if(CharToInt(Infra.TrolleysID[i]) == Hit.Trolley){
            for(unsigned int j = 0; j < Infra.Trolleys[i].nSlots; j++){
                if(CharToInt(Infra.Trolleys[i].SlotsID[j]) == Hit.Station)
                    nStripsPart = Infra.Trolleys[i].RPCs[j].strips;
            }
        }
    }

    Hit.Partition   = (Hit.Strip-1)/nStripsPart+1;  //From 1 to 4
    Hit.Connector   = (Hit.Strip-1)/NSTRIPSCONN+1;  //From 1 to 8
    Hit.TimeStamp   = TimeStamp;
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//
// ****************************************************************************************************

//Set the beam time window
void SetBeamWindow (float (&PeakTime)[NTROLLEYS][NSLOTS][NPARTITIONS],
                    float (&PeakWidth)[NTROLLEYS][NSLOTS][NPARTITIONS],
                    TTree* mytree, map<int,int> RPCChMap, Infrastructure GIFInfra){
    RAWData mydata;

    mydata.TDCCh = new vector<unsigned int>;
    mydata.TDCTS = new vector<float>;
    mydata.TDCCh->clear();
    mydata.TDCTS->clear();

    mytree->SetBranchAddress("EventNumber",    &mydata.iEvent);
    mytree->SetBranchAddress("number_of_hits", &mydata.TDCNHits);
    mytree->SetBranchAddress("TDC_channel",    &mydata.TDCCh);
    mytree->SetBranchAddress("TDC_TimeStamp",  &mydata.TDCTS);

    TH1F *tmpTimeProfile[NTROLLEYS][NSLOTS][NPARTITIONS];

    for(unsigned int tr = 0; tr < NTROLLEYS; tr++)
        for(unsigned int sl = 0; sl < NSLOTS; sl++)
            for(unsigned int p = 0; p < NPARTITIONS; p++){
                string name = "tmpTProf" + intToString(tr) + intToString(sl) +  intToString(p);
                tmpTimeProfile[tr][sl][p] = new TH1F(name.c_str(),name.c_str(),BMTDCWINDOW/20.,0.,BMTDCWINDOW);
    }

    for(unsigned int i = 0; i < mytree->GetEntries(); i++){
        mytree->GetEntry(i);

        for(int h = 0; h < mydata.TDCNHits; h++){
            RPCHit tmpHit;

            //Get rid of the noise hits outside of the connected channels
            if(mydata.TDCCh->at(h) > 5127) continue;
            if(RPCChMap[mydata.TDCCh->at(h)] == 0) continue;

            SetRPCHit(tmpHit, RPCChMap[mydata.TDCCh->at(h)], mydata.TDCTS->at(h), GIFInfra);
            tmpTimeProfile[tmpHit.Trolley][tmpHit.Station-1][tmpHit.Partition-1]->Fill(tmpHit.TimeStamp);
        }
    }

    //Fit with a gaussian the "Good TDC Time"
    TF1 *slicefit = new TF1("slicefit","gaus(0)+[3]",250.,350.);//Fit function (gaussian)

    //Loop over RPCs
    for(unsigned int tr = 0; tr < NTROLLEYS; tr++){
        for(unsigned int sl = 0; sl < NSLOTS; sl++ ) {
            for(unsigned int p = 0; p < NPARTITIONS; p++ ) {
                slicefit->SetParameter(0,50);      //Amplitude
                slicefit->SetParLimits(0,1,100000);

                slicefit->SetParameter(1,300);     //Mean value
                slicefit->SetParLimits(1,260,340);

                slicefit->SetParameter(2,20);      //RMS
                slicefit->SetParLimits(2,1,40);

                slicefit->SetParameter(3,10);      //Background offset
                slicefit->SetParLimits(3,0,100000);

                if(tmpTimeProfile[tr][sl][p]->GetEntries() > 0.)
                    tmpTimeProfile[tr][sl][p]->Fit(slicefit,"QR");

                PeakTime[tr][sl][p] = slicefit->GetParameter(1);
                PeakWidth[tr][sl][p] = 2.*slicefit->GetParameter(2);

                delete tmpTimeProfile[tr][sl][p];
            }
        }
    }
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//
// ****************************************************************************************************

//Function use to sort hits by increasing strip number
bool SortStrips ( RPCHit A, RPCHit B ) {
    return ( A.Strip < B.Strip );
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//
// ****************************************************************************************************

//Return the partition corresponding to the strip
int GetPartition( int strip ) {
    return strip/NSTRIPSPART;
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//
// ****************************************************************************************************

//Get mean of 1D histograms
float GetTH1Mean(TH1* H){
    int nBins = H->GetNbinsX();
    float mean = 0.;

    for(int b = 1; b <= nBins; b++) mean += H->GetBinContent(b);

    mean /= (float)nBins;

    return mean;
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//
// ****************************************************************************************************

//Get standard deviation of 1D histograms
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
// *    int CharToInt(char &C)
//
//
// ****************************************************************************************************

//Draw 1D histograms
void DrawTH1(TCanvas* C, TH1* H, string xtitle, string ytitle, string option){
    C->cd(0);
    H->SetXTitle(xtitle.c_str());
    H->SetYTitle(ytitle.c_str());
    H->SetFillColor(kBlue);
    H->Draw(option.c_str());
    C->Update();
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//
// ****************************************************************************************************

//Draw 2D histograms
void DrawTH2(TCanvas* C, TH2* H, string xtitle, string ytitle, string ztitle, string option){
    C->cd(0);
    H->SetXTitle(xtitle.c_str());
    H->SetYTitle(ytitle.c_str());
    H->SetXTitle(ztitle.c_str());
    gStyle->SetPalette(55);
    H->Draw(option.c_str());
    C->SetLogz(1);
    C->Update();
}
