#include "../include/NoiseRate.h"
#include "../include/MsgSvc.h"

#include "TFile.h"
#include "TBranch.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "THistPainter.h"
#include "TColor.h"
#include "TStyle.h"
#include "TString.h"

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
        string base = fileName.erase(fileName.find_last_of("."));
        return base;
    } else {
        string extension = fileName.substr(fileName.find_last_of("."));
        MSG_ERROR("[NoiseRate]: Wrong file format %s used\n",extension.c_str());
        return "";
    }
}

//*******************************************************************************

float GetStripSurface(int GeoID, IniFile* GeoFile){
    float stripMinor = 1.;
    float stripMajor = 1.;
    float stripHeight = 1.;

    //The 3 digit number "chamber" will then be used in a "switch" to select the
    //dimensions of the corresponding chamber type
    switch(GeoID){
        case 111: {
            stripMinor = GeoFile->GetValue("Type2-A","Minor",1.);
            stripMajor = GeoFile->GetValue("Type2-A","Major",1.);
            stripHeight = GeoFile->GetValue("Type2-A","Height",1.);
            break;
        }

        case 112: {
            stripMinor = GeoFile->GetValue("Type2-B","Minor",1.);
            stripMajor = GeoFile->GetValue("Type2-B","Major",1.);
            stripHeight = GeoFile->GetValue("Type2-B","Height",1.);
            break;
        }

        case 113: {
            stripMinor = GeoFile->GetValue("Type2-C","Minor",1.);
            stripMajor = GeoFile->GetValue("Type2-C","Major",1.);
            stripHeight = GeoFile->GetValue("Type2-C","Height",1.);
            break;
        }

        case 121: {
            stripMinor = GeoFile->GetValue("Type2-A","Minor",1.);
            stripMajor = GeoFile->GetValue("Type2-A","Major",1.);
            stripHeight = GeoFile->GetValue("Type2-A","Height",1.);
            break;
        }

        case 122: {
            stripMinor = GeoFile->GetValue("Type2-B","Minor",1.);
            stripMajor = GeoFile->GetValue("Type2-B","Major",1.);
            stripHeight = GeoFile->GetValue("Type2-B","Height",1.);
            break;
        }

        case 123: {
            stripMinor = GeoFile->GetValue("Type2-C","Minor",1.);
            stripMajor = GeoFile->GetValue("Type2-C","Major",1.);
            stripHeight = GeoFile->GetValue("Type2-C","Height",1.);
            break;
        }

        case 131: {
            stripMinor = GeoFile->GetValue("Type2-A","Minor",1.);
            stripMajor = GeoFile->GetValue("Type2-A","Major",1.);
            stripHeight = GeoFile->GetValue("Type2-A","Height",1.);
            break;
        }

        case 132: {
            stripMinor = GeoFile->GetValue("Type2-B","Minor",1.);
            stripMajor = GeoFile->GetValue("Type2-B","Major",1.);
            stripHeight = GeoFile->GetValue("Type2-B","Height",1.);
            break;
        }

        case 133: {
            stripMinor = GeoFile->GetValue("Type2-C","Minor",1.);
            stripMajor = GeoFile->GetValue("Type2-C","Major",1.);
            stripHeight = GeoFile->GetValue("Type2-C","Height",1.);
            break;
        }

        case 141: {
            stripMinor = GeoFile->GetValue("Type2-A","Minor",1.);
            stripMajor = GeoFile->GetValue("Type2-A","Major",1.);
            stripHeight = GeoFile->GetValue("Type2-A","Height",1.);
            break;
        }

        case 142: {
            stripMinor = GeoFile->GetValue("Type2-B","Minor",1.);
            stripMajor = GeoFile->GetValue("Type2-B","Major",1.);
            stripHeight = GeoFile->GetValue("Type2-B","Height",1.);
            break;
        }

        case 143: {
            stripMinor = GeoFile->GetValue("Type2-C","Minor",1.);
            stripMajor = GeoFile->GetValue("Type2-C","Major",1.);
            stripHeight = GeoFile->GetValue("Type2-C","Height",1.);
            break;
        }

        case 311: {
            stripMinor = GeoFile->GetValue("Type1-1-A","Minor",1.);
            stripMajor = GeoFile->GetValue("Type1-1-A","Major",1.);
            stripHeight = GeoFile->GetValue("Type1-1-A","Height",1.);
            break;
        }

        case 312: {
            stripMinor = GeoFile->GetValue("Type1-1-B","Minor",1.);
            stripMajor = GeoFile->GetValue("Type1-1-B","Major",1.);
            stripHeight = GeoFile->GetValue("Type1-1-B","Height",1.);
            break;
        }

        case 313: {
            stripMinor = GeoFile->GetValue("Type1-1-C","Minor",1.);
            stripMajor = GeoFile->GetValue("Type1-1-C","Major",1.);
            stripHeight = GeoFile->GetValue("Type1-1-C","Height",1.);
            break;
        }

        case 314: {
            stripMinor = GeoFile->GetValue("Type1-1-D","Minor",1.);
            stripMajor = GeoFile->GetValue("Type1-1-D","Major",1.);
            stripHeight = GeoFile->GetValue("Type1-1-D","Height",1.);
            break;
        }

        case 321: {
            stripMinor = GeoFile->GetValue("Type1-1-A","Minor",1.);
            stripMajor = GeoFile->GetValue("Type1-1-A","Major",1.);
            stripHeight = GeoFile->GetValue("Type1-1-A","Height",1.);
            break;
        }

        case 322: {
            stripMinor = GeoFile->GetValue("Type1-1-B","Minor",1.);
            stripMajor = GeoFile->GetValue("Type1-1-B","Major",1.);
            stripHeight = GeoFile->GetValue("Type1-1-B","Height",1.);
            break;
        }

        case 323: {
            stripMinor = GeoFile->GetValue("Type1-1-C","Minor",1.);
            stripMajor = GeoFile->GetValue("Type1-1-C","Major",1.);
            stripHeight = GeoFile->GetValue("Type1-1-C","Height",1.);
            break;
        }

        case 324: {
            stripMinor = GeoFile->GetValue("Type1-1-D","Minor",1.);
            stripMajor = GeoFile->GetValue("Type1-1-D","Major",1.);
            stripHeight = GeoFile->GetValue("Type1-1-D","Height",1.);
            break;
        }

        case 331: {
            stripMinor = GeoFile->GetValue("China","Minor",1.);
            stripMajor = GeoFile->GetValue("China","Major",1.);
            stripHeight = GeoFile->GetValue("China","Height",1.);
            break;
        }

        case 341: {
            stripMinor = GeoFile->GetValue("Type2-A","Minor",1.);
            stripMajor = GeoFile->GetValue("Type2-A","Major",1.);
            stripHeight = GeoFile->GetValue("Type2-A","Height",1.);
            break;
        }

        case 342: {
            stripMinor = GeoFile->GetValue("Type2-B","Minor",1.);
            stripMajor = GeoFile->GetValue("Type2-B","Major",1.);
            stripHeight = GeoFile->GetValue("Type2-B","Height",1.);
            break;
        }

        case 343: {
            stripMinor = GeoFile->GetValue("Type2-C","Minor",1.);
            stripMajor = GeoFile->GetValue("Type2-C","Major",1.);
            stripHeight = GeoFile->GetValue("Type2-C","Height",1.);
            break;
        }
        default:
            break;
    }

    float stripSurface = (stripMinor+stripMajor)*stripHeight/2.;
    return stripSurface;
}

//*******************************************************************************

map<int,int> TDCMapping(){
    int RPCCh;              //# RPC Channel (X00 to X95 : X stations [X = 0,1,...], 96 strips)
    int TDCCh;              //# TDC Channel (X000 to X127 : X modules [X = 0,1,...], 128 channels)
    map<int,int> Map;       //2D Map of the TDC Channels and their corresponding RPC strips

    ifstream mappingfile(__mapping.c_str(), ios::in);	//Mapping file

    while (mappingfile.good()) { //Fill the map with RPC and TDC channels
        mappingfile >> RPCCh >> TDCCh;
        if ( TDCCh != -1 ) Map[TDCCh] = RPCCh;
    }
    mappingfile.close();

    return Map;
}

//*******************************************************************************

void GetNoiseRate(string fName){ //raw root file name
    // strip off .root from filename - usefull to contruct the
    //output file name
    string baseName = GetBaseName(fName);

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

    //****************** TRIGGER TYPE ********************************

    //Get the chamber geometry
    //First open the RunParameters TTree from the dataFile
    //Then link a string to the branch corresponding to the beam
    //status and get the entry
    //Convention : ON = beam trigger , OFF = Random trigger
    TTree* RunParameters = (TTree*)dataFile.Get("RunParameters");
    TString* Beam = new TString();
    RunParameters->SetBranchAddress("Beam",&Beam);
    RunParameters->GetEntry(0);

    //****************** GEOMETRY ************************************

    //Get the chamber geometry
    IniFile* DimensionsRE = new IniFile("DimensionsRE.ini");
    DimensionsRE->Read();
    map<int,float> StripSurface;

    //****************** MAPPING *************************************

    map<int,int> RPCChMap = TDCMapping();

    //****************** HISTOGRAMS & CANVAS *************************

    TH2F     *RPCInstantNoiseRate[NTROLLEYS][NRPCTROLLEY][NPARTITIONS];
    TProfile *RPCMeanNoiseProfile[NTROLLEYS][NRPCTROLLEY][NPARTITIONS];
    TH1I     *RPCHitProfile[NTROLLEYS][NRPCTROLLEY][NPARTITIONS];
    TH1F     *RPCTimeProfile[NTROLLEYS][NRPCTROLLEY][NPARTITIONS];
    TH1I     *RPCHitMultiplicity[NTROLLEYS][NRPCTROLLEY][NPARTITIONS];

    TCanvas *InstantNoise[NTROLLEYS][NRPCTROLLEY][NPARTITIONS];
    TCanvas *MeanNoise[NTROLLEYS][NRPCTROLLEY][NPARTITIONS];
    TCanvas *HitProfile[NTROLLEYS][NRPCTROLLEY][NPARTITIONS];
    TCanvas *TimeProfile[NTROLLEYS][NRPCTROLLEY][NPARTITIONS];
    TCanvas *HitMultiplicity[NTROLLEYS][NRPCTROLLEY][NPARTITIONS];

    char hisid[50];                     //ID of the histogram
    char hisname[50];                   //Name of the histogram

    for (unsigned int t = 0; t < NTROLLEYS; t++){
        if(t != 1) continue;
        for (unsigned int rpc = 0; rpc < NRPCTROLLEY; rpc++){
            for (unsigned int p = 0; p < NPARTITIONS; p++){
                //Get profit of these loops to fill the strip surface map
                int GeoID = t*100 + (rpc+1)*10 + p+1;
                StripSurface[GeoID] = GetStripSurface(GeoID,DimensionsRE);

            //Noise rate bin size depending on the strip surface
            float binWidth = 1.;
            float timeWidth = 1.;

            if(Beam->CompareTo("ON") == 0){
                    binWidth = 1./(BMNOISEWDW*1e-9*StripSurface[GeoID]);
		    timeWidth = BMTDCWINDOW;
            } else if(Beam->CompareTo("OFF") == 0){
                    binWidth = 1./(RDMTDCWINDOW*1e-9*StripSurface[GeoID]);
		    timeWidth = RDMTDCWINDOW;
                }

                //Instantaneous noise rate 2D map
                SetIDName(t,rpc,p,hisid,hisname,"RPC_Instant_Noise","RPC instantaneous noise rate map");
                RPCInstantNoiseRate[t][rpc][p] = new TH2F( hisid, hisname, 32, 32*p+0.5, 32*(p+1)+0.5, 21, -0.5*binWidth, 20.5*binWidth);
                InstantNoise[t][rpc][p] = new TCanvas(hisid,hisname);

                //Mean noise rate profile
                SetIDName(t,rpc,p,hisid,hisname,"RPC_Mean_Noise","RPC mean noise rate");
                RPCMeanNoiseProfile[t][rpc][p] = new TProfile( hisid, hisname, 32, 32*p+0.5, 32*(p+1)+0.5);
                MeanNoise[t][rpc][p] = new TCanvas(hisid,hisname);

                //Hit profile
                SetIDName(t,rpc,p,hisid,hisname,"RPC_Hit_Profile","RPC hit profile");
                RPCHitProfile[t][rpc][p] = new TH1I( hisid, hisname, 32, 32*p+0.5, 32*(p+1)+0.5);
                HitProfile[t][rpc][p] = new TCanvas(hisid,hisname);

                //Time profile
                SetIDName(t,rpc,p,hisid,hisname,"RPC_Time_Profile","RPC time profile");
                RPCTimeProfile[t][rpc][p] = new TH1F( hisid, hisname, (int)timeWidth, 0., timeWidth);
                TimeProfile[t][rpc][p] = new TCanvas(hisid,hisname);

                //Hit multiplicity
                SetIDName(t,rpc,p,hisid,hisname,"RPC_Hit_Multiplicity","RPC hit multiplicity");
                RPCHitMultiplicity[t][rpc][p] = new TH1I( hisid, hisname, 33, -0.5, 32.5);
                HitMultiplicity[t][rpc][p] = new TCanvas(hisid,hisname);
            }
        }
    }

    //****************** MACRO ***************************************

    //Tabel to count the hits in every chamber partitions - used to
    //compute the noise rate
    int NHitsPerStrip[NTROLLEYS][NRPCTROLLEY][NSTRIPSRPC] = { {0} };
    int Multiplicity[NTROLLEYS][NRPCTROLLEY][NPARTITIONS] = { {0} };

    unsigned int nEntries = dataTree->GetEntries();

    for(unsigned int i = 0; i < nEntries; i++){
        dataTree->GetEntry(i);

        //Loop over the TDC hits
        for ( int h = 0; h < data.TDCNHits; h++ ) {
            RPCHit hit;

            //Get rid of the noise hits outside of the connected channels
            if(data.TDCCh->at(h) > 5095) continue;

            SetRPCHit(hit, RPCChMap[data.TDCCh->at(h)], data.TDCTS->at(h));

            //Count the number of hits outside the peak
            bool earlyhit = (hit.TimeStamp >= 150. && hit.TimeStamp < 250.);
            bool latehit = (hit.TimeStamp >= 350. && hit.TimeStamp < 550.);

            if(Beam->CompareTo("ON") == 0 && (earlyhit || latehit))
                NHitsPerStrip[hit.Trolley][hit.Station-1][hit.Strip-1]++;
            else if(Beam->CompareTo("OFF") == 0)
                NHitsPerStrip[hit.Trolley][hit.Station-1][hit.Strip-1]++;

            //Fill the RPC profiles
            RPCHitProfile[hit.Trolley][hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
            RPCTimeProfile[hit.Trolley][hit.Station-1][hit.Partition-1]->Fill(hit.TimeStamp);
            Multiplicity[hit.Trolley][hit.Station-1][hit.Partition-1]++;
        }

        //** INSTANTANEOUS NOISE RATE ********************************

        for(unsigned int t=0; t<NTROLLEYS; t++){
            if(t != 1) continue; //since we only use T1

            for(unsigned int rpc=0; rpc<NRPCTROLLEY; rpc++){
                for(unsigned int s=0; s<NSTRIPSRPC; s++){
                    //Partition from 0 to 2
                    int p = s/NSTRIPSPART;

                    //Get the geometry ID
                    int GeoID = t*100 + (rpc+1)*10 + p+1;

                    //Get the instaneous noise by normalise the hit count to the
                    //time window length in seconds and to the strip surface
                    float InstantNoise = 0.;

                    if(Beam->CompareTo("OFF") == 0)
                        InstantNoise = (float)NHitsPerStrip[t][rpc][s]/(RDMTDCWINDOW*1e-9*StripSurface[GeoID]);
                    else if (Beam->CompareTo("ON") == 0)
                        InstantNoise = (float)NHitsPerStrip[t][rpc][s]/(BMNOISEWDW*1e-9*StripSurface[GeoID]);

                    RPCInstantNoiseRate[t][rpc][p]->Fill(s+1,InstantNoise);

                    //Reinitialise the hit count for strip s
                    NHitsPerStrip[t][rpc][s]=0;

                    //Fill the multiplicity for this event
                    if(s == 0 || s == 32 || s == 64 || s == 96){
                        RPCHitMultiplicity[t][rpc][p]->Fill(Multiplicity[t][rpc][p]);
                        Multiplicity[t][rpc][p] = 0;
                    }
                }
            }
        }
    }

    //************** MEAN NOISE RATE *********************************
    //create a ROOT output file to save the histograms
    string fNameROOT = baseName + "-Rate.root";
    TFile outputfile(fNameROOT.c_str(), "recreate");

    //output csv file
    string csvName = fName.substr(0,fName.find_last_of("/")+1) + "Offline-Rate.csv";
    ofstream outputCSV(csvName.c_str(),ios::app);
    //Print the file name as first column
    outputCSV << fName.substr(fName.find_last_of("/")+1) << '\t';

    //Loop over trolleys
    for (unsigned int t = 0; t < NTROLLEYS; t++){
        if(t != 1) continue; //since we only use T1
        //Loop over stations
        for (unsigned int rpc = 0; rpc < NRPCTROLLEY; rpc++){
            //Loop over partitions
            for ( unsigned int p = 0; p < NPARTITIONS; p++ ) {
                if(p == 3) continue; //there are only 3 partitions in type 2 chambers
                //Project the histograms along the X-axis to get the
                //mean noise profile on the strips
                RPCMeanNoiseProfile[t][rpc][p] = RPCInstantNoiseRate[t][rpc][p]->ProfileX();

                //Write in the output file the mean noise rate per
                //partition and its error defined as twice the RMS
                //over the sqrt of the number of events
                float MeanNoiseRate = RPCInstantNoiseRate[t][rpc][p]->ProjectionY()->GetMean();
                float ErrorMean = 2*RPCInstantNoiseRate[t][rpc][p]->ProjectionY()->GetRMS()/sqrt(nEntries);
                outputCSV << MeanNoiseRate << '\t' << ErrorMean << '\t';

                //Draw the histograms and write the canvas

                InstantNoise[t][rpc][p]->cd(0);
                RPCInstantNoiseRate[t][rpc][p]->SetXTitle("Strip");
                RPCInstantNoiseRate[t][rpc][p]->SetYTitle("Noise rate (Hz/cm^{2})");
                RPCInstantNoiseRate[t][rpc][p]->SetZTitle("# events");
                gStyle->SetPalette(55);
                RPCInstantNoiseRate[t][rpc][p]->Draw("COLZ");
                InstantNoise[t][rpc][p]->SetLogz(1);
                InstantNoise[t][rpc][p]->Write();

                MeanNoise[t][rpc][p]->cd(0);
                RPCMeanNoiseProfile[t][rpc][p]->SetXTitle("Strip");
                RPCMeanNoiseProfile[t][rpc][p]->SetYTitle("Mean Noise rate (Hz/cm^{2})");
                RPCMeanNoiseProfile[t][rpc][p]->Draw();
                MeanNoise[t][rpc][p]->Write();

                HitProfile[t][rpc][p]->cd(0);
                RPCHitProfile[t][rpc][p]->SetXTitle("Strip");
                RPCHitProfile[t][rpc][p]->SetYTitle("# events");
                RPCHitProfile[t][rpc][p]->Draw();
                HitProfile[t][rpc][p]->Write();

                TimeProfile[t][rpc][p]->cd(0);
                RPCTimeProfile[t][rpc][p]->SetXTitle("Time stamp (ns)");
                RPCTimeProfile[t][rpc][p]->SetYTitle("# events");
                RPCTimeProfile[t][rpc][p]->Draw();
                TimeProfile[t][rpc][p]->Write();

                HitMultiplicity[t][rpc][p]->cd(0);
                RPCHitMultiplicity[t][rpc][p]->SetXTitle("Multiplicity");
                RPCHitMultiplicity[t][rpc][p]->SetYTitle("# events");
                RPCHitMultiplicity[t][rpc][p]->Draw();
                HitMultiplicity[t][rpc][p]->Write();
           }
        }
    }
	
    outputCSV << '\n';
    outputCSV.close();

    outputfile.Close();
    dataFile.Close();
}

