#include "../include/NoiseRate.h"
#include "../include/MsgSvc.h"

#include "TFile.h"
#include "TBranch.h"
#include "TH1F.h"

#include <fstream>
#include <cmath>

string GetPath(string fileName){
    string path;
    path = fileName.substr(0,fileName.find_last_of("/")+1);
    return path;
}

//*******************************************************************************

string GetBaseName(string fileName){
    if(fileName.substr(fileName.find_last_of(".")) == ".root"){
        MSG_INFO("[NoiseRate]: Using data file %s\n",fileName.c_str());
        string base;
        base = fileName.erase(fileName.find_last_of("."));
        return base;
    } else {
        string extension = fileName.substr(fileName.find_last_of("."));
        MSG_ERROR("[NoiseRate]: Wrong file format %s used\n",extension.c_str());
        return "";
    }
}

//*******************************************************************************

float GetStripSurface(string chamberType,char partLabel,IniFile* GeoFile){
    string groupname = chamberType+"-"+partLabel;
    float stripMinor = GeoFile->GetValue(groupname,"Minor",1.);
    float stripMajor = GeoFile->GetValue(groupname,"Major",1.);
    float stripHeight = GeoFile->GetValue(groupname,"Height",1.);

    float stripSurface = (stripMinor+stripMajor)*stripHeight/2.;
    return stripSurface;
}

//*******************************************************************************

map<int,int> TDCMapping(string mappingfName){
    int RPCCh;              //# RPC Channel (X00 to X95 : X stations [X = 0,1,...], 96 strips)
    int TDCCh;              //# TDC Channel (X000 to X127 : X modules [X = 0,1,...], 128 channels)
    map<int,int> Map;       //2D Map of the TDC Channels and their corresponding RPC strips

    ifstream mappingfile(mappingfName.c_str(), ios::in);	//Mapping file

    while (mappingfile.good()) { //Fill the map with RPC and TDC channels
        mappingfile >> RPCCh >> TDCCh;
        if ( TDCCh != -1 ) Map[TDCCh] = RPCCh;
    }
    mappingfile.close();

    return Map;
}

//*******************************************************************************

void GetNoiseRate(string fName,string chamberType){ //raw root file name
    // strip off .root from filename
    string baseName = GetBaseName(fName);

//****************** HISTOGRAMS **************************************

    TH1F *RPCNoiseRates[NRPCTROLLEY][NPARTITIONS];
    TH1F *RPCMeanNoiseRate[NRPCTROLLEY][NPARTITIONS];

    char part[4] = "ABC";               //Names of the partitions
    char hisid[50];                     //ID of the histogram
    char hisname[50];                   //Name of the histogram

    for (unsigned int rpc = 0; rpc < NRPCTROLLEY; rpc++){
        for (unsigned int p = 0; p < NPARTITIONS; p++){
            //Noise rate for each strip
            SetIDName(rpc,p,part,hisid,hisname,"RPC_Noise","RPC noise rates");
            RPCNoiseRates[rpc][p] = new TH1F( hisid, hisname, 32, 32*p+0.5, 32*(p+1)+0.5);
            RPCNoiseRates[rpc][p]->SetXTitle("# Strip");
            RPCNoiseRates[rpc][p]->SetYTitle("Noise rate (Hz/cm^{2})");

            //Mean noise rate
            SetIDName(rpc,p,part,hisid,hisname,"RPC_Mean_Noise","RPC mean noise rate");
            RPCMeanNoiseRate[rpc][p] = new TH1F( hisid, hisname, 100, 0., 5e3);
            RPCMeanNoiseRate[rpc][p]->SetXTitle("Noise rate (Hz/cm^{2})");
            RPCMeanNoiseRate[rpc][p]->SetYTitle("# events");
        }
    }

//****************** MAPPING *****************************************

    map<int,int> RPCChMap = TDCMapping("ChannelsMapping.csv");

//****************** ROOT FILE ***************************************

    TFile   dataFile(fName.c_str());
    TTree*  dataTree = (TTree*)dataFile.Get("RAWData");
    RAWData data;

    data.TDCCh = new vector<int>;
    data.TDCTS = new vector<float>;
    data.TDCCh->clear();
    data.TDCTS->clear();

    dataTree->SetBranchAddress("EventNumber",    &data.iEvent);
    dataTree->SetBranchAddress("number_of_hits", &data.TDCNHits);
    dataTree->SetBranchAddress("TDC_channel",    &data.TDCCh);
    dataTree->SetBranchAddress("TDC_TimeStamp",  &data.TDCTS);

//****************** GEOMETRY ****************************************

    //Get the chamber geometry
    IniFile* DimensionsRE = new IniFile("DimensionsRE.ini");
    DimensionsRE->Read();
    float stripSurface[NPARTITIONS] = {0.};

    for(int p=0; p<NPARTITIONS; p++)
        stripSurface[p] = GetStripSurface(chamberType,part[p],DimensionsRE);

//****************** MACRO *******************************************

    float NoiseRates[NRPCTROLLEY][NSTRIPSRPC] = { {0.} };   //Noise rates
    int NHitsPerPart[NRPCTROLLEY][NPARTITIONS] = { {0} };

    unsigned int nEntries = dataTree->GetEntries();

    for(unsigned int i = 0; i < nEntries; i++){
        dataTree->GetEntry(i);

        //Loop over the TDC hits
        for ( unsigned int h = 0; h < data.TDCNHits; h++ ) {
            RPCHit temprpchit;
            SetRPCHit(temprpchit, RPCChMap[data.TDCCh->at(h)], data.TDCTS->at(h));

            NoiseRates[temprpchit.Station][temprpchit.Strip]++;
            NHitsPerPart[temprpchit.Station][temprpchit.Partition]++;
        }

        //** INSTANTANEOUS NOISE RATE ********************************

        //Get the noise rate for each event - this is related to the number
        //of hits in each partition
        for(unsigned int rpc=0; rpc<NRPCTROLLEY; rpc++){
            for(unsigned int p=0; p<NPARTITIONS; p++){
                //Normalise to the time window length in seconds
                //and to the 32 strip surface (now only 16 are
        //connected...)
                float eventNoise = NHitsPerPart[rpc][p]/(TDCWINDOW*1e-9*16*stripSurface[p]);
                RPCMeanNoiseRate[rpc][p]->Fill(eventNoise);
                NHitsPerPart[rpc][p]=0;
            }
        }
    }

    dataFile.Close();

    //************** RESULTS *****************************************

    //output csv file
    string fNameCSV = GetPath(fName) + "Summary_runs.csv";
    ofstream outputCSV(fNameCSV.c_str(),ios::app);

    outputCSV << baseName << '\t';

    //Loop over stations
    for ( unsigned int rpc = 0; rpc < NRPCTROLLEY; rpc++ ) {
        //Loop over strips
        for ( unsigned int s = 0; s < NSTRIPSRPC; s++ ) {
            int p = s/NSTRIPSPART;  //Partition from 0 to 2

            //Write in the output file the mean noise rate per partition
            //only once
            if(s%32 == 0){
                float MeanNoiseRate = RPCMeanNoiseRate[rpc][p]->GetMean();
                float ErrorMean = 2*RPCMeanNoiseRate[rpc][p]->GetMean()/sqrt(nEntries);
                outputCSV << MeanNoiseRate << '\t' << ErrorMean << '\t';
            }

            //Normalise to the time window length in seconds,
            //to the number of trigger and to the strip surface
            NoiseRates[rpc][s] /= TDCWINDOW * 1e-9 * nEntries * stripSurface[p];

            RPCNoiseRates[rpc][p]->Fill(s, NoiseRates[rpc][s]);
            outputCSV << NoiseRates[rpc][s];
            if(s == NSTRIPSRPC-1) outputCSV << '\n';
            else outputCSV << '\t';
        }
    }

    //****** Root output file ****************************************
    TFile outputfile((baseName + "_Offline_Noise_Rate.root").c_str(), "recreate");

    for ( unsigned int rpc = 0; rpc < NRPCTROLLEY; rpc++ ) {
        for (unsigned int p = 0; p < NPARTITIONS; p++ ) {
            RPCMeanNoiseRate[rpc][p]->Write();
            RPCNoiseRates[rpc][p]->Write();
        }
    }
    outputfile.Close();
}

