//***************************************************************
// *    904 OFFLINE TOOL v4
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
#include "../include/MsgSvc.h"

using namespace std;

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
// *    map<int,int> TDCMapping(string baseName)
//
//  Returns the map to translate TDC channels into RPC strips.
// ****************************************************************************************************

Mapping TDCMapping(string baseName){
    //# RPC Channel (TS000 to TS127 : T = trolleys, S = slots, up to 127 strips)
    Uint RPCCh = 9999;

    //# TDC Channel (M000 to M127 : M modules (from 0), 128 channels)
    Uint TDCCh = 9999;

    //Mask (1 is strip is active and 0 if strip is masked)
    Uint Mask = 1;

    //2D Map of the TDC Channels and their corresponding RPC strips
    Mapping Map;

    //File that contains the path to the mapping file located
    //in the scan directory
    string mappingpath = baseName.substr(0,baseName.find_last_of("/")) + "/Mapping.csv";

    //Open mapping file
    ifstream mappingfile(mappingpath.c_str(), ios::in);
    if(mappingfile){
        while (mappingfile.good()) { //Fill the map with RPC and TDC channels
            mappingfile >> RPCCh >> TDCCh;

            //Check the TDC mapping file format (2 columns - old format -
            //or 3 columns - new format including mask - )
            char next;
            mappingfile.get(next);
            if(next == '\n')
                Mask = 1;
            else
                mappingfile >> Mask;

            if ( TDCCh != 9999 ){
                Map.link[TDCCh] = RPCCh;
                Map.mask[RPCCh] = Mask;
            }
        }
        mappingfile.close();
    } else {
        MSG_ERROR("[Offline] Couldn't open file " + mappingpath);
        exit(EXIT_FAILURE);
    }

    return Map;
}

// ****************************************************************************************************
// *    void SetRPC(RPC &rpc, string ID, IniFile *geofile)
//
//  Set up the RPC structure needed which is the innermost part of the GIF++ infrasctructure. The RPCs
//  are contained inside the Trolleys. Informations are readout from Dimensions.ini . For details about
//  RPC members, see file utils.h.
// ****************************************************************************************************

void SetRPC(RPC &rpc, string ID, IniFile *geofile){
    rpc.name        = geofile->stringType(ID,"Name","");
    rpc.nPartitions = geofile->intType(ID,"Partitions",NPARTITIONS);
    rpc.nGaps       = geofile->intType(ID,"Gaps",0);

    for(Uint g = 0 ; g < rpc.nGaps; g++){
        string gapID = "Gap" + intToString(g+1);
        rpc.gaps.push_back(geofile->stringType(ID,gapID,""));

        string areaID = "Area" + gapID;
        rpc.gapGeo.push_back(geofile->floatType(ID,areaID,1.));
    }

    rpc.strips      = geofile->intType(ID,"Strips",NSLOTS);
    string partID = "ABCD";

    for(Uint p = 0; p < rpc.nPartitions; p++){
        string areaID  = "ActiveArea-"  + CharToString(partID[p]);
        float area = geofile->floatType(ID,areaID,1.);
        rpc.stripGeo.push_back(area);
    }
}

// ****************************************************************************************************
// *    void SetInfrastructure(Infrastructure &infra, IniFile *geofile)
//
//  Set up the Infrastructure structure needed that represents 904's setup at the moment of data
//  taking, i.e. the active RPC chambers. Informations are readout from Dimensions.ini . For details
//  about Trolley members, see file utils.h.
// ****************************************************************************************************

void SetInfrastructure(Infrastructure &infra, IniFile *geofile){
    infra.nSlots = geofile->intType("General","nSlots",NSLOTS);
    infra.SlotsID = geofile->stringType("General","SlotsID","");

    for(Uint s = 0; s < infra.nSlots; s++){
        string rpcID = "S" + CharToString(infra.SlotsID[s]);

        RPC temprpc;
        SetRPC(temprpc,rpcID,geofile);
        infra.RPCs.push_back(temprpc);
    }
}

// ****************************************************************************************************
// *    void SetRPCHit(RPCHit& Hit, int Channel, float TimeStamp, Infrastructure Infra)
//
//  Uses the mapping to set up every hit and assign it to the right strip in each active RPC.
// ****************************************************************************************************

void SetRPCHit(RPCHit& Hit, int Channel, float TimeStamp, Infrastructure Infra){
    Hit.Channel     = Channel;      //RPC channel according to mapping (4 digits)
    Hit.Station     = Channel/1000; //From 1 to 9 (1st digit)
    Hit.Strip       = Channel%1000; //From 1 to 128 (3 last digits)

    int nStripsPart = 0;
    for(unsigned int i = 0; i < Infra.nSlots; i++){
            if(CharToInt(Infra.SlotsID[i]) == Hit.Station)
                nStripsPart = Infra.RPCs[i].strips;
    }

    Hit.Partition   = (Hit.Strip-1)/nStripsPart+1; //From 1 to 4
    Hit.TimeStamp   = TimeStamp;
}

// ****************************************************************************************************
// *    void SetBeamWindow (float (&PeakTime)[NSLOTS][NPARTITIONS],
// *                        float (&PeakWidth)[NSLOTS][NPARTITIONS],
// *                        TTree* mytree, map<int,int> RPCChMap, Infrastructure GIFInfra)
//
//  Loops over all the data contained inside of the ROOT file and determines for each RPC the center
//  of the muon peak and its spread. Then saves the result in 2 3D tables (#D bescause it follows the
//  dimensions of the GIF++ infrastructure : Trolley, RPC slots and Partitions).
// ****************************************************************************************************

//Set the beam time window
void SetBeamWindow (muonPeak &PeakTime, muonPeak &PeakWidth,
                    TTree* mytree, Mapping RPCChMap, Infrastructure GIFInfra){
    RAWData mydata;

    mydata.TDCCh = new vector<Uint>;
    mydata.TDCTS = new vector<float>;
    mydata.TDCCh->clear();
    mydata.TDCTS->clear();

    mytree->SetBranchAddress("EventNumber",    &mydata.iEvent);
    mytree->SetBranchAddress("number_of_hits", &mydata.TDCNHits);
    mytree->SetBranchAddress("TDC_channel",    &mydata.TDCCh);
    mytree->SetBranchAddress("TDC_TimeStamp",  &mydata.TDCTS);

    TH1F *tmpTimeProfile[NSLOTS][NPARTITIONS];
    float noiseHits[NSLOTS][NPARTITIONS] = {{0.}};
    int binWidth = TIMEBIN;

    for(Uint sl = 0; sl < NSLOTS; sl++)
        for(Uint p = 0; p < NPARTITIONS; p++){
            string name = "tmpTProf" + intToString(sl) +  intToString(p);
            tmpTimeProfile[sl][p] = new TH1F(name.c_str(),name.c_str(),BMTDCWINDOW/binWidth,0.,BMTDCWINDOW);
    }

    //Loop over the entries to get the hits and fill the time distribution + count the
    //noise hits in the window around the peak

    for(Uint i = 0; i < mytree->GetEntries(); i++){
        mytree->GetEntry(i);

        for(int h = 0; h < mydata.TDCNHits; h++){
            RPCHit tmpHit;
            Uint channel = mydata.TDCCh->at(h);
            Uint timing = mydata.TDCTS->at(h);

            //Get rid of the noise hits outside of the connected channels
            if(RPCChMap.link[channel] == 0) continue;

            SetRPCHit(tmpHit, RPCChMap.link[channel], timing, GIFInfra);
            Uint S = tmpHit.Station-1;
            Uint P = tmpHit.Partition-1;
            tmpTimeProfile[S][P]->Fill(tmpHit.TimeStamp);
        }
    }

    //Compute the average number of noise hits per 10ns bin and subtract it to the time
    //distribution
    muonPeak center;
    muonPeak lowlimit;
    muonPeak highlimit;

    for(Uint sl = 0; sl < NSLOTS; sl++){
        for(Uint p = 0; p < NPARTITIONS; p++){
            if(tmpTimeProfile[sl][p]->GetEntries() > 0.){
                center.rpc[sl][p] = (float)tmpTimeProfile[sl][p]->GetMaximumBin()*TIMEBIN;
                lowlimit.rpc[sl][p] = center.rpc[sl][p] - 40.;
                highlimit.rpc[sl][p] = center.rpc[sl][p] + 40.;

                float timeWdw = BMTDCWINDOW-TIMEREJECT-(highlimit.rpc[sl][p]-lowlimit.rpc[sl][p]);

                int nNoiseHitsLow = tmpTimeProfile[sl][p]->Integral(TIMEREJECT/TIMEBIN,lowlimit.rpc[sl][p]/TIMEBIN);
                int nNoiseHitsHigh = tmpTimeProfile[sl][p]->Integral(highlimit.rpc[sl][p]/TIMEBIN,BMTDCWINDOW/TIMEBIN);

                noiseHits[sl][p] = (float)binWidth*(nNoiseHitsLow+nNoiseHitsHigh)/timeWdw;

                for(Uint b = 1; b <= BMTDCWINDOW/binWidth; b++){
                    float binContent = (float)tmpTimeProfile[sl][p]->GetBinContent(b);
                    float correctedContent = (binContent < noiseHits[sl][p]) ? 0. : binContent-noiseHits[sl][p];
                    tmpTimeProfile[sl][p]->SetBinContent(b,correctedContent);
                }
            }
        }
    }

    //Fit with a gaussian the muon peak and extract the peak center as well as its width
    //defined as 3 standard deviation (99.7% of the peak area)

    for(Uint sl = 0; sl < NSLOTS; sl++){
        for(Uint p = 0; p < NPARTITIONS; p++){
            TF1 *peakfit = new TF1("peakfit","gaus(0)",lowlimit.rpc[sl][p],highlimit.rpc[sl][p]);
            //Amplitude
            peakfit->SetParameter(0,50);
            peakfit->SetParLimits(0,1,100000);
            //Mean value
            peakfit->SetParameter(1,center.rpc[sl][p]);
            peakfit->SetParLimits(1,lowlimit.rpc[sl][p],highlimit.rpc[sl][p]);
            //RMS
            peakfit->SetParameter(2,20);
            peakfit->SetParLimits(2,1,40);

            if(tmpTimeProfile[sl][p]->GetEntries() > 0.)
                tmpTimeProfile[sl][p]->Fit(peakfit,"QR");

            //Save the peak center and width from gaussian fit
            PeakTime.rpc[sl][p] = peakfit->GetParameter(1);
            PeakWidth.rpc[sl][p] = 3.*peakfit->GetParameter(2);

            delete tmpTimeProfile[sl][p];
        }
    }
}

// ****************************************************************************************************
// *    bool SortStrips (RPCHit A, RPCHit B)
//
//  Tells if Strip A < Strip B.
// ****************************************************************************************************

//Function use to sort hits by increasing strip number
bool SortStrips (RPCHit A, RPCHit B){
    return ( A.Strip < B.Strip );
}

// ****************************************************************************************************
// *    void SetTitleName(string rpcID, unsigned int partition,
// *                      char* Name, char* Title, string Namebase, string Titlebase)
//
//  Builds the name and title of ROOT objects.
// ****************************************************************************************************

//Name of histograms
void SetTitleName(string rpcID, unsigned int partition,
                  char* Name, char* Title, string Namebase, string Titlebase){

    string P[4] = {"A","B","C","D"};
    sprintf(Name,"%s_%s_%s",Namebase.c_str(),rpcID.c_str(),P[partition].c_str());
    sprintf(Title,"%s %s_%s",Titlebase.c_str(),rpcID.c_str(),P[partition].c_str());
}

// ****************************************************************************************************
// *    float GetTH1Mean(TH1* H)
//
//  Returns the mean along the Y axis of a TH1 (value not given by the ROOT statbox).
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
// *    float GetTH1StdDev(TH1* H)
//
//  Returns the spread of the distribution of values along the Y axis of a TH1 (value not given by
//  the ROOT statbox).
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
// *    float GetChipBin(TH1* H)
//
//  Returns the chip average value from strip histograms by grouping active strips.
// ****************************************************************************************************

//Get mean of Chip from 1D histograms
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

//Draw 1D histograms
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
