//***************************************************************
// *    GIF OFFLINE TOOL v5
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

#include "../include/utils.h"
#include "../include/MsgSvc.h"

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
// *    Mapping TDCMapping(string baseName)
//
//  Returns the map to translate TDC channels into RPC strips and get the mask.
// ****************************************************************************************************

Mapping TDCMapping(string baseName){
    //# RPC Channel (TS000 to TS127 : T = trolleys, S = slots, up to 127 strips)
    Uint RPCCh = 9999;

    //# TDC Channel (M000 to M127 : M modules (from 0), 128 channels)
    Uint TDCCh = 9999;

    //Mask (1 is strip is active and 0 if strip is masked)
    Uint Mask = 1;

    //Maps of:
    //TDC Channels and their corresponding RPC strips
    //RPC Channels and their corresponding mask value
    Mapping Map;

    //File that contains the path to the mapping file located
    //in the scan directory
    string mappingpath = baseName.substr(0,baseName.find_last_of("/"))+"/ChannelsMapping.csv";

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
// *    void SetTrolley(GIFTrolley &trolley, string ID, IniFile *geofile)
//
//  Set up the Trolley structure needed that contains RPCs. Informations are readout from
//  Dimensions.ini . For details about Trolley members, see file utils.h.
// ****************************************************************************************************


void SetTrolley(GIFTrolley &trolley, string ID, IniFile *geofile){
    trolley.nSlots = geofile->intType(ID,"nSlots",NSLOTS);
    trolley.SlotsID = geofile->stringType(ID,"SlotsID","");

    for(Uint s = 0; s < trolley.nSlots; s++){
        string rpcID = ID + "S" + CharToString(trolley.SlotsID[s]);

        RPC temprpc;
        SetRPC(temprpc,rpcID,geofile);
        trolley.RPCs.push_back(temprpc);
    }
}

// ****************************************************************************************************
// *    void SetInfrastructure(Infrastructure &infra, IniFile *geofile)
//
//  Set up the Infrastructure structure needed that represents the GIF++ setup at the moment of data
//  taking, i.e. the active trolleys and the RPC chambers they contain in their slots. Informations
//  are readout from Dimensions.ini . For details about Trolley members, see file utils.h.
// ****************************************************************************************************

void SetInfrastructure(Infrastructure &infra, IniFile *geofile){
    infra.nTrolleys = geofile->intType("General","nTrolleys",NTROLLEYS);
    infra.TrolleysID = geofile->stringType("General","TrolleysID","");
    infra.Trolleys.clear();

    for(Uint t = 0; t < infra.nTrolleys; t++){
        string trolleyID = "T" + CharToString(infra.TrolleysID[t]);

        GIFTrolley tempTrolley;
        SetTrolley(tempTrolley, trolleyID, geofile);
        infra.Trolleys.push_back(tempTrolley);
    }
}

// ****************************************************************************************************
// *    void SetRPCHit(RPCHit& Hit, int Channel, float TimeStamp, Infrastructure Infra)
//
//  Uses the mapping to set up every hit and assign it to the right strip in each active RPC.
// ****************************************************************************************************

void SetRPCHit(RPCHit& Hit, int Channel, float TimeStamp, Infrastructure Infra){
    Hit.Channel     = Channel;              //RPC channel according to mapping (5 digits)
    Hit.Trolley     = Channel/10000;        //0, 1 or 3 (1st digit of the RPC channel)
    Hit.Station     = (Channel%10000)/1000; //From 1 to 4 (2nd digit)
    Hit.Strip       = Channel%1000;         //From 1 to 128 (3 last digits)

    int nStripsPart = 0;
    for(Uint i = 0; i < Infra.nTrolleys; i++){
        if(CharToInt(Infra.TrolleysID[i]) == Hit.Trolley){
            for(Uint j = 0; j < Infra.Trolleys[i].nSlots; j++){
                if(CharToInt(Infra.Trolleys[i].SlotsID[j]) == Hit.Station)
                    nStripsPart = Infra.Trolleys[i].RPCs[j].strips;
            }
        }
    }

    Hit.Partition   = (Hit.Strip-1)/nStripsPart+1; //From 1 to 4
    Hit.TimeStamp   = TimeStamp;
}

// ****************************************************************************************************
// *    void SetCluster(RPCCluster& Cluster, HitList List, Uint cID,
// *                    Uint cSize, Uint first, Uint firstID)
//
//  Set up clusters.
// ****************************************************************************************************

void SetCluster(RPCCluster& Cluster, HitList List, Uint cID, Uint cSize, Uint first, Uint firstID){
    Cluster.ClusterID   = cID;
    Cluster.ClusterSize = cSize;
    Cluster.FirstStrip  = first;
    Cluster.LastStrip   = first+cSize-1;
    Cluster.Center      = (Cluster.FirstStrip+Cluster.LastStrip)/2.;
    Cluster.StartStamp  = GetClusterStartStamp(List,cSize,firstID);
    Cluster.StopStamp   = GetClusterStopStamp(List,cSize,firstID);
    Cluster.TimeSpread  = GetClusterSpreadTime(List,cSize,firstID);
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

    TH1F *tmpTimeProfile[NTROLLEYS][NSLOTS][NPARTITIONS];
    float noiseHits[NTROLLEYS][NSLOTS][NPARTITIONS] = {{{0.}}};
    int binWidth = TIMEBIN;

    for(Uint tr = 0; tr < NTROLLEYS; tr++)
        for(Uint sl = 0; sl < NSLOTS; sl++)
            for(Uint p = 0; p < NPARTITIONS; p++){
                string name = "tmpTProf" + intToString(tr) + intToString(sl) +  intToString(p);
                tmpTimeProfile[tr][sl][p] = new TH1F(name.c_str(),name.c_str(),BMTDCWINDOW/TIMEBIN,0.,BMTDCWINDOW);
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
            if(channel > 5127) continue;
            if(RPCChMap.link[channel] == 0) continue;

            SetRPCHit(tmpHit, RPCChMap.link[channel], timing, GIFInfra);
            Uint T = tmpHit.Trolley;
            Uint S = tmpHit.Station-1;
            Uint P = tmpHit.Partition-1;

            tmpTimeProfile[T][S][P]->Fill(tmpHit.TimeStamp);
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
                if(tmpTimeProfile[tr][sl][p]->GetEntries() > 0.){
                    center.rpc[tr][sl][p] = (float)tmpTimeProfile[tr][sl][p]->GetMaximumBin()*TIMEBIN;
                    lowlimit.rpc[tr][sl][p] = center.rpc[tr][sl][p] - 40.;
                    highlimit.rpc[tr][sl][p] = center.rpc[tr][sl][p] + 40.;

                    float timeWdw = BMTDCWINDOW-TIMEREJECT-(highlimit.rpc[tr][sl][p]-lowlimit.rpc[tr][sl][p]);

                    int nNoiseHitsLow = tmpTimeProfile[tr][sl][p]->Integral(TIMEREJECT/TIMEBIN,lowlimit.rpc[tr][sl][p]/TIMEBIN);
                    int nNoiseHitsHigh = tmpTimeProfile[tr][sl][p]->Integral(highlimit.rpc[tr][sl][p]/TIMEBIN,BMTDCWINDOW/TIMEBIN);

                    noiseHits[tr][sl][p] = (float)binWidth*(nNoiseHitsLow+nNoiseHitsHigh)/timeWdw;

                    for(Uint b = 1; b <= BMTDCWINDOW/binWidth; b++){
                        float binContent = (float)tmpTimeProfile[tr][sl][p]->GetBinContent(b);
                        float correctedContent = (binContent < noiseHits[tr][sl][p]) ? 0. : binContent-noiseHits[tr][sl][p];
                        tmpTimeProfile[tr][sl][p]->SetBinContent(b,correctedContent);
                    }
                }
            }
        }
    }

    //Loop over RPCs
    for(Uint tr = 0; tr < NTROLLEYS; tr++){
        for(Uint sl = 0; sl < NSLOTS; sl++ ){
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

                if(tmpTimeProfile[tr][sl][p]->GetEntries() > 0.)
                    tmpTimeProfile[tr][sl][p]->Fit(slicefit,"QR");

                PeakTime.rpc[tr][sl][p] = slicefit->GetParameter(1);
                PeakWidth.rpc[tr][sl][p] = 3.*slicefit->GetParameter(2);

                delete tmpTimeProfile[tr][sl][p];
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

// ****************************************************************************************************
// *    bool sortbyhit(pair<int, float> p1, pair<int, float> p2)
//
//  Sort RPC hits using strip position information
// ****************************************************************************************************

bool SortHitbyStrip(RPCHit h1, RPCHit h2){
        return (h1.Strip < h2.Strip);
}

// ****************************************************************************************************
// *    bool sortbytime(pair<int, float> p1, pair<int, float> p2)
//
//  Sort RPC hits using time stamp information
// ****************************************************************************************************

//Sort hits by time
bool SortHitbyTime(RPCHit h1, RPCHit h2){
        return (h1.TimeStamp < h2.TimeStamp);
}

// ****************************************************************************************************
// *   float GetClusterSpreadTime(HitList &hits, int cSize, int hitID)
//
//  Return the time difference in between the first and the last hit of the cluster
// ****************************************************************************************************

float GetClusterSpreadTime(HitList &cluster, int cSize, int hitID){
    //Cluster are reconstructed in the non rejected time window
    float max = 0.;
    float min = 0.;

    for(int i = hitID; i <= (hitID + cSize-1); i++){
        if(i == hitID && min == 0. && max == 0.){
            min = cluster[i].TimeStamp;
            max = cluster[i].TimeStamp;
        } else {
            if(cluster[i].TimeStamp < min ) min = cluster[i].TimeStamp;
            if(cluster[i].TimeStamp > max ) max = cluster[i].TimeStamp;
        }
    }

    return (max-min);
}

// ****************************************************************************************************
// *   float GetClusterStartStamp(HitList &cluster,int cSize, int hitID)
//
//  Return the starting time stamp of the cluster
// ****************************************************************************************************

float GetClusterStartStamp(HitList &cluster,int cSize, int hitID){
    //Cluster are reconstructed in the non rejected time window
    float min = 0.;

    for(int i = hitID; i <= (hitID + cSize-1); i++){
        if(i == hitID && min == 0.) min = cluster[i].TimeStamp;
        else if(cluster[i].TimeStamp < min ) min = cluster[i].TimeStamp;
    }

    return min;
}

// ****************************************************************************************************
// *   float GetClusterStopStamp(HitList &cluster,int cSize, int hitID)
//
//  Return the stopping time stamp of the cluster
// ****************************************************************************************************

float GetClusterStopStamp(HitList &cluster,int cSize, int hitID){
    //Cluster are reconstructed in the non rejected time window
    float max = 0.;

    for(int i = hitID; i <= (hitID + cSize-1); i++){
        if(i == hitID && max == 0.) max = cluster[i].TimeStamp;
        else if(cluster[i].TimeStamp > max ) max = cluster[i].TimeStamp;
    }

    return max;
}

// ****************************************************************************************************
// *    void do_cluster(vector < pair<int, float> > &v,
// *                    vector < Cluster > &cluster, bool noflip,int thePartition)
//
//  Build cluster using the sorted RPC hit arrays
//  Wee need the following variables:
//  - cluster size
//  - isolation (nominal, "other side")
//  - cluster:
//      * center,  (left, right sides)
//      * delta time
// ****************************************************************************************************

void BuildClusters(HitList &cluster, ClusterList &clusterList){
    if(cluster.size() > 0){
        // Sort by strip order to make cluster by using adjacent strips
        sort(cluster.begin(), cluster.end(), SortHitbyStrip);

        Uint firstHit = cluster.front().Strip; //first cluster hit
        Uint previous = firstHit;              //previous strip checked
        Uint hitID = 0;      //ID of the first cluster hit to
                             //look for start and stop stamps
        Uint clusterID = 0;  //ID of the cluster starting from 0
        Uint cSize = 0;      //cluster size counter

        //Loop over the hit list and group adjacent strips
        for(Uint i = 0; i < cluster.size(); i++){
            if (cluster[i].Strip-previous > 1){
                clusterList.push_back(RPCCluster());
                SetCluster(clusterList.back(),cluster,clusterID,cSize,firstHit,hitID);

                cSize = 1;
                hitID = i;
                firstHit = cluster[i].Strip;
                clusterID++;
            }
            previous = cluster[i].Strip;
            cSize++;
        }
        clusterID++;

        //Add the last cluster to the list
        clusterList.push_back(RPCCluster());
        SetCluster(clusterList.back(),cluster,clusterID,cSize,firstHit,hitID);
    }
}

// ****************************************************************************************************
// *   void Clusterization(HitList &hits, TH1 *hcSize, TH1 *hcMult)
//
//  Used to loop over the hit list, create clusters and fill histograms
// ****************************************************************************************************

void Clusterization(HitList &hits, TH1 *hcSize, TH1 *hcMult){
    HitList cluster;
    cluster.clear();

    ClusterList clusterList;
    clusterList.clear();

    float timediff = 0.;
    float lastime = 0.;
    Uint multiplicity = 0;

    for(Uint h = 0; h < hits.size(); h ++){
        timediff = hits[h].TimeStamp-lastime;

        //If there is 25 time difference with the previous hit
        //consider that the hit is too far in time and make
        //cluster with what has been saved into the cluster
        //vector
        if(abs(timediff) > 25. && lastime > 0.){
            clusterList.clear();
            BuildClusters(cluster,clusterList);

            for(Uint i = 0; i < clusterList.size(); i++)
                if(clusterList[i].ClusterSize > 0)
                    hcSize->Fill(clusterList[i].ClusterSize);

            multiplicity += clusterList.size();
            cluster.clear();
        }

        lastime = hits[h].TimeStamp;
        cluster.push_back(hits[h]);
    }

    //Make cluster with the very last part of data saved in the list
    if(cluster.size() > 0){
        BuildClusters(cluster,clusterList);

        for(Uint i = 0; i < clusterList.size(); i++)
            if(clusterList[i].ClusterSize > 0)
                hcSize->Fill(clusterList[i].ClusterSize);

        multiplicity += clusterList.size();
        cluster.clear();
    }

    hcMult->Fill(multiplicity);
}
