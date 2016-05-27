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
#include <cstdlib>

string GetPath(string fName){
    string path;
    path = fName.substr(0,fName.find_last_of("/")+1);
    return path;
}

//*******************************************************************************

string GetBaseName(string fName){
    if(fName.substr(fName.find_last_of(".")) == ".root"){
        MSG_INFO("[NoiseRate]: Using data file " + fName + " \n");
        string base = fName.erase(fName.find_last_of("."));
        return base;
    } else {
        string extension = fName.substr(fName.find_last_of("."));
        MSG_ERROR("[NoiseRate]: Wrong file format " + extension + " used\n");
        return "";
    }
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
    IniFile* Dimensions = new IniFile(__dimensions.c_str());
    Dimensions->Read();

    //****************** MAPPING *************************************

    map<int,int> RPCChMap = TDCMapping();

    //****************** HISTOGRAMS & CANVAS *************************

    TH2F     *RPCInstantNoiseRate[NTROLLEYS][NSLOTS][NPARTITIONS];
    TProfile *RPCMeanNoiseProfile[NTROLLEYS][NSLOTS][NPARTITIONS];
    TH1I     *RPCHitProfile[NTROLLEYS][NSLOTS][NPARTITIONS];
    TH1F     *RPCTimeProfile[NTROLLEYS][NSLOTS][NPARTITIONS];
    TH1I     *RPCHitMultiplicity[NTROLLEYS][NSLOTS][NPARTITIONS];

    TCanvas *InstantNoise[NTROLLEYS][NSLOTS][NPARTITIONS];
    TCanvas *MeanNoise[NTROLLEYS][NSLOTS][NPARTITIONS];
    TCanvas *HitProfile[NTROLLEYS][NSLOTS][NPARTITIONS];
    TCanvas *TimeProfile[NTROLLEYS][NSLOTS][NPARTITIONS];
    TCanvas *HitMultiplicity[NTROLLEYS][NSLOTS][NPARTITIONS];

    char hisid[50];                     //ID of the histogram
    char hisname[50];                   //Name of the histogram

    Infrastructure GIFInfra;
    SetInfrastructure(GIFInfra,Dimensions);

    for (unsigned int t = 0; t < GIFInfra.nTrolleys; t++){
        unsigned int nSlotsTrolley = GIFInfra.Trolleys[t].nSlots;
        unsigned int trolley = CharToInt(GIFInfra.TrolleysID[t]);

        for (unsigned int s = 0; s < nSlotsTrolley; s++){
            unsigned int nPartRPC = GIFInfra.Trolleys[t].RPCs[s].nPartitions;
            unsigned int slot = CharToInt(GIFInfra.Trolleys[t].SlotsID[s]) - 1;

            string rpcID = "T"+ CharToString(GIFInfra.TrolleysID[t]) +
                    "S" + CharToString(GIFInfra.Trolleys[t].SlotsID[s]);

            for (unsigned int p = 0; p < nPartRPC; p++){
                //Set bining
                unsigned int nStrips = GIFInfra.Trolleys[t].RPCs[s].strips;
                float low = nStrips*p + 0.5;
                float high = nStrips*(p+1) + 0.5;

                unsigned int nBinsMult = 101;
                float lowBin = -0.5;
                float highBin = (float)nBinsMult + lowBin;

                //Noise rate bin size depending on the strip surface
                float stripArea = GIFInfra.Trolleys[t].RPCs[s].stripGeo[p];
                float binWidth = 1.;
                float timeWidth = 1.;

                if(Beam->CompareTo("ON") == 0){
                    binWidth = 1./(BMNOISEWDW*1e-9*stripArea);
                    timeWidth = BMTDCWINDOW;
                } else if(Beam->CompareTo("OFF") == 0){
                    binWidth = 1./(RDMTDCWINDOW*1e-9*stripArea);
                    timeWidth = RDMTDCWINDOW;
                }

                //Instantaneous noise rate 2D map
                SetIDName(rpcID,p,hisid,hisname,"RPC_Instant_Noise","RPC instantaneous noise rate map");
                RPCInstantNoiseRate[trolley][slot][p] = new TH2F( hisid, hisname, nStrips, low, high, nBinsMult, lowBin*binWidth, highBin*binWidth);
                InstantNoise[trolley][slot][p] = new TCanvas(hisid,hisname);

                //Mean noise rate profile
                SetIDName(rpcID,p,hisid,hisname,"RPC_Mean_Noise","RPC mean noise rate");
                RPCMeanNoiseProfile[trolley][slot][p] = new TProfile( hisid, hisname, nStrips, low, high);
                MeanNoise[trolley][slot][p] = new TCanvas(hisid,hisname);

                //Hit profile
                SetIDName(rpcID,p,hisid,hisname,"RPC_Hit_Profile","RPC hit profile");
                RPCHitProfile[trolley][slot][p] = new TH1I( hisid, hisname, nStrips, low, high);
                HitProfile[trolley][slot][p] = new TCanvas(hisid,hisname);

                //Time profile
                SetIDName(rpcID,p,hisid,hisname,"RPC_Time_Profile","RPC time profile");
                RPCTimeProfile[trolley][slot][p] = new TH1F( hisid, hisname, (int)timeWidth, 0., timeWidth);
                TimeProfile[trolley][slot][p] = new TCanvas(hisid,hisname);

                //Hit multiplicity
                SetIDName(rpcID,p,hisid,hisname,"RPC_Hit_Multiplicity","RPC hit multiplicity");
                RPCHitMultiplicity[trolley][slot][p] = new TH1I( hisid, hisname, nBinsMult, lowBin, highBin);
                HitMultiplicity[trolley][slot][p] = new TCanvas(hisid,hisname);
            }
        }
    }

    //****************** MACRO ***************************************

    //Tabel to count the hits in every chamber partitions - used to
    //compute the noise rate
    int NHitsPerStrip[NTROLLEYS][NSLOTS][NSTRIPSRPC] = { {0} };
    int Multiplicity[NTROLLEYS][NSLOTS][NPARTITIONS] = { {0} };

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

        for(unsigned int t=0; t<GIFInfra.nTrolleys; t++){
            unsigned int nSlotsTrolley = GIFInfra.Trolleys[t].nSlots;
            unsigned int trolley = CharToInt(GIFInfra.TrolleysID[t]);

            for(unsigned int sl=0; sl<nSlotsTrolley; sl++){
                unsigned int nStripsPart = GIFInfra.Trolleys[t].RPCs[sl].strips;
                unsigned int nStripsSlot = nStripsPart * GIFInfra.Trolleys[t].RPCs[sl].nPartitions;
                unsigned int slot = CharToInt(GIFInfra.Trolleys[t].SlotsID[sl]) - 1;

                for(unsigned int st=0; st<nStripsSlot; st++){
                    //Partition
                    int p = st/nStripsPart;

                    //Get the strip geometry
                    float stripArea = GIFInfra.Trolleys[t].RPCs[sl].stripGeo[p];

                    //Get the instaneous noise by normalise the hit count to the
                    //time window length in seconds and to the strip surface
                    float InstantNoise = 0.;

                    if(Beam->CompareTo("OFF") == 0)
                        InstantNoise = (float)NHitsPerStrip[trolley][slot][st]/(RDMTDCWINDOW*1e-9*stripArea);
                    else if (Beam->CompareTo("ON") == 0)
                        InstantNoise = (float)NHitsPerStrip[trolley][slot][st]/(BMNOISEWDW*1e-9*stripArea);

                    RPCInstantNoiseRate[trolley][slot][p]->Fill(st+1,InstantNoise);

                    //Reinitialise the hit count for strip s
                    NHitsPerStrip[trolley][slot][st]=0;

                    //Fill the multiplicity for this event
                    if(st%nStripsPart == 0){
                        RPCHitMultiplicity[trolley][slot][p]->Fill(Multiplicity[trolley][slot][p]);
                        Multiplicity[trolley][slot][p] = 0;
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

    //Create the output folder for the DQM plots
    string mkdirDQMFolder = "mkdir -p " + baseName;
    system(mkdirDQMFolder.c_str());

    //Variables for the DQM file names
    string PDF;
    PDF.clear();

    string PNG;
    PNG.clear();

    //Loop over trolleys
    for (unsigned int t = 0; t < GIFInfra.nTrolleys; t++){
        unsigned int nSlotsTrolley = GIFInfra.Trolleys[t].nSlots;
        unsigned int trolley = CharToInt(GIFInfra.TrolleysID[t]);

        for (unsigned int s = 0; s < nSlotsTrolley; s++){
            unsigned int nPartRPC = GIFInfra.Trolleys[t].RPCs[s].nPartitions;
            unsigned int slot = CharToInt(GIFInfra.Trolleys[t].SlotsID[s]) - 1;

            for (unsigned int p = 0; p < nPartRPC; p++){
                //Project the histograms along the X-axis to get the
                //mean noise profile on the strips
                RPCMeanNoiseProfile[trolley][slot][p] = RPCInstantNoiseRate[trolley][slot][p]->ProfileX();

                //Write in the output file the mean noise rate per
                //partition and its error defined as twice the RMS
                //over the sqrt of the number of events
                float MeanNoiseRate = RPCInstantNoiseRate[trolley][slot][p]->ProjectionY()->GetMean();
                float ErrorMean = 2*RPCInstantNoiseRate[trolley][slot][p]->ProjectionY()->GetRMS()/sqrt(nEntries);
                outputCSV << MeanNoiseRate << '\t' << ErrorMean << '\t';

                //Draw the histograms and write the canvas

                InstantNoise[trolley][slot][p]->cd(0);
                RPCInstantNoiseRate[trolley][slot][p]->SetXTitle("Strip");
                RPCInstantNoiseRate[trolley][slot][p]->SetYTitle("Noise rate (Hz/cm^{2})");
                RPCInstantNoiseRate[trolley][slot][p]->SetZTitle("# events");
                gStyle->SetPalette(55);
                RPCInstantNoiseRate[trolley][slot][p]->Draw("COLZ");
                InstantNoise[trolley][slot][p]->SetLogz(1);
                InstantNoise[trolley][slot][p]->Update();
                PDF = baseName  + "/" + InstantNoise[trolley][slot][p]->GetName() + ".pdf";
                PNG = baseName  + "/" + InstantNoise[trolley][slot][p]->GetName() + ".png";
                InstantNoise[trolley][slot][p]->SaveAs(PDF.c_str());
                InstantNoise[trolley][slot][p]->SaveAs(PNG.c_str());
                InstantNoise[trolley][slot][p]->Write();

                MeanNoise[trolley][slot][p]->cd(0);
                RPCMeanNoiseProfile[trolley][slot][p]->SetXTitle("Strip");
                RPCMeanNoiseProfile[trolley][slot][p]->SetYTitle("Mean Noise rate (Hz/cm^{2})");
                RPCMeanNoiseProfile[trolley][slot][p]->SetFillColor(kBlue);
                RPCMeanNoiseProfile[trolley][slot][p]->Draw("HIST");
                RPCMeanNoiseProfile[trolley][slot][p]->Draw("E1 SAME");
                MeanNoise[trolley][slot][p]->Update();
                PDF = baseName  + "/" + MeanNoise[trolley][slot][p]->GetName() + ".pdf";
                PNG = baseName  + "/" + MeanNoise[trolley][slot][p]->GetName() + ".png";
                MeanNoise[trolley][slot][p]->SaveAs(PDF.c_str());
                MeanNoise[trolley][slot][p]->SaveAs(PNG.c_str());
                MeanNoise[trolley][slot][p]->Write();

                HitProfile[trolley][slot][p]->cd(0);
                RPCHitProfile[trolley][slot][p]->SetXTitle("Strip");
                RPCHitProfile[trolley][slot][p]->SetYTitle("# events");
                RPCHitProfile[trolley][slot][p]->SetFillColor(kBlue);
                RPCHitProfile[trolley][slot][p]->Draw();
                HitProfile[trolley][slot][p]->Update();
                PDF = baseName  + "/" + HitProfile[trolley][slot][p]->GetName() + ".pdf";
                PNG = baseName  + "/" + HitProfile[trolley][slot][p]->GetName() + ".png";
                HitProfile[trolley][slot][p]->SaveAs(PDF.c_str());
                HitProfile[trolley][slot][p]->SaveAs(PNG.c_str());
                HitProfile[trolley][slot][p]->Write();

                TimeProfile[trolley][slot][p]->cd(0);
                RPCTimeProfile[trolley][slot][p]->SetXTitle("Time stamp (ns)");
                RPCTimeProfile[trolley][slot][p]->SetYTitle("# events");
                RPCTimeProfile[trolley][slot][p]->SetFillColor(kBlue);
                RPCTimeProfile[trolley][slot][p]->Draw();
                TimeProfile[trolley][slot][p]->Update();
                PDF = baseName  + "/" + TimeProfile[trolley][slot][p]->GetName() + ".pdf";
                PNG = baseName  + "/" + TimeProfile[trolley][slot][p]->GetName() + ".png";
                TimeProfile[trolley][slot][p]->SaveAs(PDF.c_str());
                TimeProfile[trolley][slot][p]->SaveAs(PNG.c_str());
                TimeProfile[trolley][slot][p]->Write();

                HitMultiplicity[trolley][slot][p]->cd(0);
                RPCHitMultiplicity[trolley][slot][p]->SetXTitle("Multiplicity");
                RPCHitMultiplicity[trolley][slot][p]->SetYTitle("# events");
                RPCHitMultiplicity[trolley][slot][p]->SetFillColor(kBlue);
                RPCHitMultiplicity[trolley][slot][p]->Draw();
                HitMultiplicity[trolley][slot][p]->Update();
                PDF = baseName  + "/" + HitMultiplicity[trolley][slot][p]->GetName() + ".pdf";
                PNG = baseName  + "/" + HitMultiplicity[trolley][slot][p]->GetName() + ".png";
                HitMultiplicity[trolley][slot][p]->SaveAs(PDF.c_str());
                HitMultiplicity[trolley][slot][p]->SaveAs(PNG.c_str());
                HitMultiplicity[trolley][slot][p]->Write();
           }
        }
    }

    outputCSV << '\n';
    outputCSV.close();

    outputfile.Close();
    dataFile.Close();
}

