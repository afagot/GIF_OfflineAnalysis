#include "../include/NoiseRate.h"
#include "../include/MsgSvc.h"

#include "TFile.h"
#include "TBranch.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"

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
    // strip off .root from filename - usefull to contruct the
    //output file name
    string baseName = GetBaseName(fName);

    //****************** GEOMETRY ************************************

    //Get the chamber geometry
    IniFile* DimensionsRE = new IniFile("DimensionsRE.ini");
    DimensionsRE->Read();

    //Save it into a table
    float stripSurface[NPARTITIONS] = {0.};
    char part[4] = "ABC";               //Names of the partitions

    for(int p=0; p<NPARTITIONS; p++)
        stripSurface[p] = GetStripSurface(chamberType,part[p],DimensionsRE);

    //****************** HISTOGRAMS **********************************

    TH2F *RPCInstantNoiseRate[NRPCTROLLEY][NPARTITIONS];
    TProfile *RPCMeanNoiseProfile[NRPCTROLLEY][NPARTITIONS];

    char hisid[50];                     //ID of the histogram
    char hisname[50];                   //Name of the histogram

    for (unsigned int rpc = 0; rpc < NRPCTROLLEY; rpc++){
        for (unsigned int p = 0; p < NPARTITIONS; p++){
	    //Noise rate bin size depending on the strip surface
	    float binWidth = 1./(TDCWINDOW*1e-9*stripSurface[p]);

            //Instantaneous noise rate 2D map
            SetIDName(rpc,p,part,hisid,hisname,"RPC_Instant_Noise","RPC instantaneous noise rate map");
            RPCInstantNoiseRate[rpc][p] = new TH2F( hisid, hisname, 32, 32*p-0.5, 32*(p+1)-0.5, 11, -0.5*binWidth, 10.5*binWidth);
            RPCInstantNoiseRate[rpc][p]->SetXTitle("Strip");
            RPCInstantNoiseRate[rpc][p]->SetYTitle("Noise rate (Hz/cm^{2})");
            RPCInstantNoiseRate[rpc][p]->SetZTitle("# events");

            //Mean noise rate profile
            SetIDName(rpc,p,part,hisid,hisname,"RPC_Mean_Noise","RPC mean noise rate");
            RPCMeanNoiseProfile[rpc][p] = new TProfile( hisid, hisname, 32, 32*p-0.5, 32*(p+1)-0.5);
            RPCMeanNoiseProfile[rpc][p]->SetXTitle("Strip");
            RPCMeanNoiseProfile[rpc][p]->SetYTitle("Mean Noise rate (Hz/cm^{2})");
        }
    }

    //****************** MAPPING *************************************

    map<int,int> RPCChMap = TDCMapping("ChannelsMapping.csv");

    //****************** ROOT FILE ***********************************

    //input ROOT data file containing the RAWData TTree that we'll
    //link to our RAWData structure
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

    //****************** MACRO ***************************************

    //Tabel to count the hits in every chamber partitions - used to
    //compute the noise rate
    int NHitsPerStrip[NRPCTROLLEY][NSTRIPSRPC] = { {0} };

    unsigned int nEntries = dataTree->GetEntries();

    for(unsigned int i = 0; i < nEntries; i++){
        dataTree->GetEntry(i);

        //Loop over the TDC hits
        for ( int h = 0; h < data.TDCNHits; h++ ) {
            RPCHit temprpchit;
            SetRPCHit(temprpchit, RPCChMap[data.TDCCh->at(h)], data.TDCTS->at(h));

            NHitsPerStrip[temprpchit.Station][temprpchit.Strip]++;
        }

        //** INSTANTANEOUS NOISE RATE ********************************

        for(unsigned int rpc=0; rpc<NRPCTROLLEY; rpc++){
            for(unsigned int s=0; s<NSTRIPSRPC; s++){
                //Partition from 0 to 2
                int p = s/NSTRIPSPART;

                //Get the instaneous noise by normalise the hit count to the
                //time window length in seconds and to the strip surface
		float InstantNoise = (float)NHitsPerStrip[rpc][s]/(TDCWINDOW*1e-9*stripSurface[p]);
                RPCInstantNoiseRate[rpc][p]->Fill(s,InstantNoise);

                //Reinitialise the hit count for strip s
                NHitsPerStrip[rpc][s]=0;
            }
        }
    }
    dataFile.Close();

    //************** MEAN NOISE RATE *********************************
    //create a ROOT output file to save the histograms
    string fNameROOT = "AnalysedData/" + baseName.substr(baseName.find_last_of("/")+1) + "Offline_Noise_Rate.root";
    TFile outputfile(fNameROOT.c_str(), "recreate");

    //output csv file
    string fNameCSV = "AnalysedData/Summary_runs.csv";
    ofstream outputCSV(fNameCSV.c_str(),ios::app);

    //Print the file name as first column
    outputCSV << fName.substr(fName.find_last_of("/")+1) << '\t';

    //Loop over stations
    for ( unsigned int rpc = 0; rpc < NRPCTROLLEY; rpc++ ) {
        //Loop over strips
        for ( unsigned int p = 0; p < NPARTITIONS; p++ ) {
            //Project the histograms along the X-axis to get the
            //mean noise profile on the strips
            RPCMeanNoiseProfile[rpc][p] = RPCInstantNoiseRate[rpc][p]->ProfileX();

            //Write in the output file the mean noise rate per
            //partition and its error defined as twice the RMS
            //over the sqrt of the number of events
            float MeanNoiseRate = RPCInstantNoiseRate[rpc][p]->ProjectionY()->GetMean();
            float ErrorMean = 2*RPCInstantNoiseRate[rpc][p]->ProjectionY()->GetRMS()/sqrt(nEntries);
            outputCSV << MeanNoiseRate << '\t' << ErrorMean << '\t';

	    //Write the histograms and profiles into the ROOTfile
            RPCInstantNoiseRate[rpc][p]->Write();
	    RPCMeanNoiseProfile[rpc][p]->Write();
        }
    }
    outputCSV << '\n';

    outputfile.Close();
}

