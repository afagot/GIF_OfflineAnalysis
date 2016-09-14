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

string GetBaseName(string fName){
    if(fName.substr(fName.find_last_of("_")) == "_DAQ.root"){
        string base = fName.erase(fName.find_last_of("_"));
        MSG_INFO("[Offline] Analysis of " + fName);
        return base;
    } else {
        string extension = fName.substr(fName.find_last_of("_"));
        MSG_ERROR("[Offline] Wrong file format " + extension + " used");
        return "";
    }
}

//*******************************************************************************

string GetPath(string baseName, string stepID){
    string path;
    path = baseName.substr(0,baseName.find_last_of("/")) + "/HV" + stepID + "/DAQ/";
//    path = baseName.substr(0,baseName.find_last_of("/")+1) + baseName.substr(baseName.find_last_of("_")+1) + "/DAQ/";
//    path = baseName.substr(0,baseName.find_last_of("/")+1) + baseName.substr(baseName.find_last_of("/")+1);
    MSG_INFO("[Offline] DQM files in " + path);
    return path;
}

//*******************************************************************************

map<int,int> TDCMapping(){
    int RPCCh; //# RPC Channel (TS000 to TS127 : T trolleys (1 or 3), S slots (1 to 4), up to 127 strips)
    int TDCCh; //# TDC Channel (M000 to M127 : M modules (from 0), 128 channels)
    map<int,int> Map;   //2D Map of the TDC Channels and their corresponding RPC strips

    ifstream mappingfile(__mapping.c_str(), ios::in);	//Mapping file

    while (mappingfile.good()) { //Fill the map with RPC and TDC channels
        mappingfile >> RPCCh >> TDCCh;
        if ( TDCCh != -1 ) Map[TDCCh] = RPCCh;
    }
    mappingfile.close();

    return Map;
}

//*******************************************************************************

void GetNoiseRate(string fName, string caenName){ //raw root file name
    // strip off .root from filename - usefull to contruct the
    //output file name
    string baseName = GetBaseName(fName);

    //****************** DAQ ROOT FILE *******************************

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

    //First open the RunParameters TTree from the dataFile
    //Then link a string to the branch corresponding to the beam
    //status and get the entry
    //Convention : ON = beam trigger , OFF = Random trigger
    TTree* RunParameters = (TTree*)dataFile.Get("RunParameters");
    TString* Trigger = new TString();
    RunParameters->SetBranchAddress("Trigger",&Trigger);
    RunParameters->GetEntry(0);

    //Then get the HVstep number from the ID histogram
    TH1D* ID = (TH1D*)dataFile.Get("ID");
    string HVstep = floatTostring(ID->GetBinContent(1));

    //****************** CAEN ROOT FILE ******************************

    //input CAEN ROOT data file containing the values of the HV eff for
    //every HV step
    TFile caenFile(caenName.c_str());
    TH1F *HVeff[NSLOTS];

    //****************** GEOMETRY ************************************

    //Get the chamber geometry
    IniFile* Dimensions = new IniFile(__dimensions.c_str());
    Dimensions->Read();

    //****************** MAPPING *************************************

    map<int,int> RPCChMap = TDCMapping();

    //****************** HISTOGRAMS & CANVAS *************************

    TH2F     *RPCInstantNoiseRate[NSLOTS][NPARTITIONS];
    TProfile *RPCMeanNoiseProfile[NSLOTS][NPARTITIONS];
    TH1I     *RPCHitProfile[NSLOTS][NPARTITIONS];
    TH1I     *RPCBeamProfile[NSLOTS][NPARTITIONS];
    TH1F     *RPCTimeProfile[NSLOTS][NPARTITIONS];
    TH1I     *RPCHitMultiplicity[NSLOTS][NPARTITIONS];

    TCanvas *InstantNoise[NSLOTS][NPARTITIONS];
    TCanvas *MeanNoise[NSLOTS][NPARTITIONS];
    TCanvas *HitProfile[NSLOTS][NPARTITIONS];
    TCanvas *BeamProfile[NSLOTS][NPARTITIONS];
    TCanvas *TimeProfile[NSLOTS][NPARTITIONS];
    TCanvas *HitMultiplicity[NSLOTS][NPARTITIONS];

    char hisid[50];                     //ID of the histogram
    char hisname[50];                   //Name of the histogram

    Infrastructure GIFInfra;
    SetInfrastructure(GIFInfra,Dimensions);

    for (unsigned int s = 0; s < GIFInfra.nSlots; s++){
        unsigned int nPartRPC = GIFInfra.RPCs[s].nPartitions;
        unsigned int slot = CharToInt(GIFInfra.SlotsID[s]) - 1;

        //Initialise
        string HVeffHisto = "HVeff_" + GIFInfra.RPCs[s].name;

        if(caenFile.GetListOfKeys()->Contains(HVeffHisto.c_str()))
            HVeff[slot] = (TH1F*)caenFile.Get(HVeffHisto.c_str());

        else {
            HVeffHisto = "HVeff_" + GIFInfra.RPCs[s].name + "-BOT";
            if(caenFile.GetListOfKeys()->Contains(HVeffHisto.c_str())){
                HVeff[slot] = (TH1F*)caenFile.Get(HVeffHisto.c_str());

                //Control that we are not in single TOP gap mode
                //If this is the case, the voltage will be at 6500V for the BOT gap
                if(HVeff[slot]->GetMean() == 6500.){
                    HVeffHisto = "HVeff_" + GIFInfra.RPCs[s].name + "-TW";
                    if(caenFile.GetListOfKeys()->Contains(HVeffHisto.c_str())){
                        HVeff[slot] = (TH1F*)caenFile.Get(HVeffHisto.c_str());

                        //If the TW was also in standby, set using TN
                        if(HVeff[slot]->GetMean() == 6500.){
                            HVeffHisto = "HVeff_" + GIFInfra.RPCs[s].name + "-TN";
                            if(caenFile.GetListOfKeys()->Contains(HVeffHisto.c_str()))
                                HVeff[slot] = (TH1F*)caenFile.Get(HVeffHisto.c_str());
                        }
                    }

                    //If there are only 2 gaps (BOT and TOP) and BOT was standby, set using TOP
                    else {
                        HVeffHisto = "HVeff_" + GIFInfra.RPCs[s].name + "-TOP";
                        if(caenFile.GetListOfKeys()->Contains(HVeffHisto.c_str()))
                            HVeff[slot] = (TH1F*)caenFile.Get(HVeffHisto.c_str());
                    }
                }
            } else
                HVeff[slot] = new TH1F();
        }

        //Get the chamber ID in terms of trolley + slot position TXSX
        string rpcID = "S" + CharToString(GIFInfra.SlotsID[s]);

        for (unsigned int p = 0; p < nPartRPC; p++){
            //Set bining
            unsigned int nStrips = GIFInfra.RPCs[s].strips;
            float low = nStrips*p + 0.5;
            float high = nStrips*(p+1) + 0.5;

            unsigned int nBinsMult = 101;
            float lowBin = -0.5;
            float highBin = (float)nBinsMult + lowBin;

            //Noise rate bin size depending on the strip surface
            float stripArea = GIFInfra.RPCs[s].stripGeo[p];
            float binWidth = 1.;
            float timeWidth = 1.;

            if(Trigger->CompareTo("Cosmics") == 0){
                binWidth = 1./(BMNOISEWDW*1e-9*stripArea);
                timeWidth = BMTDCWINDOW;
            } else if(Trigger->CompareTo("Random") == 0){
                binWidth = 1./(RDMTDCWINDOW*1e-9*stripArea);
                timeWidth = RDMTDCWINDOW;
            }

            //Instantaneous noise rate 2D map
            SetIDName(rpcID,p,hisid,hisname,"RPC_Instant_Noise","RPC instantaneous noise rate map");
            RPCInstantNoiseRate[slot][p] = new TH2F( hisid, hisname, nStrips, low, high, nBinsMult, lowBin*binWidth, highBin*binWidth);
            InstantNoise[slot][p] = new TCanvas(hisid,hisname);

            //Mean noise rate profile
            SetIDName(rpcID,p,hisid,hisname,"RPC_Mean_Noise","RPC mean noise rate");
            RPCMeanNoiseProfile[slot][p] = new TProfile( hisid, hisname, nStrips, low, high);
            MeanNoise[slot][p] = new TCanvas(hisid,hisname);

            //Hit profile
            SetIDName(rpcID,p,hisid,hisname,"RPC_Hit_Profile","RPC hit profile");
            RPCHitProfile[slot][p] = new TH1I( hisid, hisname, nStrips, low, high);
            HitProfile[slot][p] = new TCanvas(hisid,hisname);

            //Beam profile
            SetIDName(rpcID,p,hisid,hisname,"RPC_Beam_Profile","RPC beam profile");
            RPCBeamProfile[slot][p] = new TH1I( hisid, hisname, nStrips, low, high);
            BeamProfile[slot][p] = new TCanvas(hisid,hisname);

            //Time profile
            SetIDName(rpcID,p,hisid,hisname,"RPC_Time_Profile","RPC time profile");
            RPCTimeProfile[slot][p] = new TH1F( hisid, hisname, (int)timeWidth/10, 0., timeWidth);
            TimeProfile[slot][p] = new TCanvas(hisid,hisname);

            //Hit multiplicity
            SetIDName(rpcID,p,hisid,hisname,"RPC_Hit_Multiplicity","RPC hit multiplicity");
            RPCHitMultiplicity[slot][p] = new TH1I( hisid, hisname, nBinsMult, lowBin, highBin);
            HitMultiplicity[slot][p] = new TCanvas(hisid,hisname);
        }
    }

    //****************** MACRO ***************************************

    //Tabel to count the hits in every chamber partitions - used to
    //compute the noise rate
    int NHitsPerStrip[NSLOTS][NSTRIPSRPC] = { {0} };
    int Multiplicity[NSLOTS][NPARTITIONS] = { {0} };

    unsigned int nEntries = dataTree->GetEntries();

    for(unsigned int i = 0; i < nEntries; i++){
        dataTree->GetEntry(i);

        //Loop over the TDC hits
        for ( int h = 0; h < data.TDCNHits; h++ ) {

            RPCHit hit;

            //Get rid of the noise hits outside of the connected channels
            if(data.TDCCh->at(h) > 127) continue;

            //Get rid of the noise hits in the ground channels of KODEL chambers

            //Set RPC hits
            SetRPCHit(hit, RPCChMap[data.TDCCh->at(h)], data.TDCTS->at(h), GIFInfra);

            //Count the number of hits outside the peak
            bool earlyhit = (hit.TimeStamp >= 50. && hit.TimeStamp < 200.);
            bool intimehit = (hit.TimeStamp >= 255. && hit.TimeStamp < 310.);
            bool latehit = (hit.TimeStamp >= 400. && hit.TimeStamp < 550.);

            if(Trigger->CompareTo("Cosmics") == 0){
                if(earlyhit || latehit)
                    NHitsPerStrip[hit.Station-1][hit.Strip-1]++;
                else if(intimehit)
                    RPCBeamProfile[hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
            } else if(Trigger->CompareTo("Random") == 0)
                NHitsPerStrip[hit.Station-1][hit.Strip-1]++;

            //Fill the RPC profiles
            RPCHitProfile[hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
            RPCTimeProfile[hit.Station-1][hit.Partition-1]->Fill(hit.TimeStamp);
            Multiplicity[hit.Station-1][hit.Partition-1]++;
        }

        //** INSTANTANEOUS NOISE RATE ********************************

        for(unsigned int sl=0; sl<GIFInfra.nSlots; sl++){
            unsigned int nStripsPart = GIFInfra.RPCs[sl].strips;
            unsigned int nStripsSlot = nStripsPart * GIFInfra.RPCs[sl].nPartitions;
            unsigned int slot = CharToInt(GIFInfra.SlotsID[sl]) - 1;

            for(unsigned int st=0; st<nStripsSlot; st++){
                //Partition
                int p = st/nStripsPart;

                //Get the strip geometry
                float stripArea = GIFInfra.RPCs[sl].stripGeo[p];

                //Get the instaneous noise by normalise the hit count to the
                //time window length in seconds and to the strip surface
                float InstantNoise = 0.;

                if(Trigger->CompareTo("Random") == 0)
                    InstantNoise = (float)NHitsPerStrip[slot][st]/(RDMTDCWINDOW*1e-9*stripArea);
                else if (Trigger->CompareTo("Cosmics") == 0)
                    InstantNoise = (float)NHitsPerStrip[slot][st]/(BMNOISEWDW*1e-9*stripArea);

                RPCInstantNoiseRate[slot][p]->Fill(st+1,InstantNoise);

                //Reinitialise the hit count for strip s
                NHitsPerStrip[slot][st]=0;

                //Fill the multiplicity for this event
                if(st%nStripsPart == 0){
                    RPCHitMultiplicity[slot][p]->Fill(Multiplicity[slot][p]);
                    Multiplicity[slot][p] = 0;
                }
            }
        }
    }

    //************** MEAN NOISE RATE *********************************
    //create a ROOT output file to save the histograms
    string fNameROOT = baseName + "_DAQ-Rate.root";
    TFile outputfile(fNameROOT.c_str(), "recreate");

    //output csv file
    string csvName = baseName.substr(0,baseName.find_last_of("/")) + "/Offline-Rate.csv";
    ofstream outputCSV(csvName.c_str(),ios::app);
    //Print the HV step as first column
    outputCSV << HVstep << '\t';

    //output csv file to save the list of parameters saved into the
    //Offline-Rate.csv file - it represents the header of that file
    string listName = baseName.substr(0,baseName.find_last_of("/")) + "/Offline-Rate-Header.csv";
    ofstream listCSV(listName.c_str(),ios::out);
    listCSV << "HVstep\t";

    //Create the output folder for the DQM plots
    string DQMFolder = GetPath(baseName,HVstep);
    string mkdirDQMFolder = "mkdir -p " + DQMFolder;
    system(mkdirDQMFolder.c_str());

    //Variables for the DQM file names
    string PDF;
    PDF.clear();

    string PNG;
    PNG.clear();

    unsigned int nSlots = GIFInfra.nSlots;

    for (unsigned int s = 0; s < nSlots; s++){
        unsigned int nPartRPC = GIFInfra.RPCs[s].nPartitions;
        unsigned int slot = CharToInt(GIFInfra.SlotsID[s]) - 1;

        float HighVoltage = HVeff[slot]->GetMean();
        outputCSV << HighVoltage << '\t';

        //Write the header file
        listCSV << "HVeff-" << GIFInfra.RPCs[s].name << '\t';

        for (unsigned int p = 0; p < nPartRPC; p++){
            string partID = "ABCD";
            //Write the header file
            listCSV << "Rate-"
                    << GIFInfra.RPCs[s].name
                    << "-" << partID[p]
                    << "\tRate-"
                    << GIFInfra.RPCs[s].name
                    << "-" << partID[p]
                    << "_err\t";

            //Project the histograms along the X-axis to get the
            //mean noise profile on the strips
            RPCMeanNoiseProfile[slot][p] = RPCInstantNoiseRate[slot][p]->ProfileX();

            //Write in the output file the mean noise rate per
            //partition and its error defined as twice the RMS
            //over the sqrt of the number of events
            float MeanNoiseRate = RPCInstantNoiseRate[slot][p]->ProjectionY()->GetMean();
            float ErrorMean = 2*RPCInstantNoiseRate[slot][p]->ProjectionY()->GetRMS()/sqrt(nEntries);
            outputCSV << MeanNoiseRate << '\t' << ErrorMean << '\t';

            //Draw the histograms and write the canvas

            InstantNoise[slot][p]->cd(0);
            RPCInstantNoiseRate[slot][p]->SetXTitle("Strip");
            RPCInstantNoiseRate[slot][p]->SetYTitle("Noise rate (Hz/cm^{2})");
            RPCInstantNoiseRate[slot][p]->SetZTitle("# events");
            gStyle->SetPalette(55);
            RPCInstantNoiseRate[slot][p]->Draw("COLZ");
            InstantNoise[slot][p]->SetLogz(1);
            InstantNoise[slot][p]->Update();
            PDF = DQMFolder + InstantNoise[slot][p]->GetName() + ".pdf";
            PNG = DQMFolder + InstantNoise[slot][p]->GetName() + ".png";
            InstantNoise[slot][p]->SaveAs(PDF.c_str());
            InstantNoise[slot][p]->SaveAs(PNG.c_str());
            InstantNoise[slot][p]->Write();

            MeanNoise[slot][p]->cd(0);
            RPCMeanNoiseProfile[slot][p]->SetXTitle("Strip");
            RPCMeanNoiseProfile[slot][p]->SetYTitle("Mean Noise rate (Hz/cm^{2})");
            RPCMeanNoiseProfile[slot][p]->SetFillColor(kBlue);
            RPCMeanNoiseProfile[slot][p]->Draw("HIST");
            RPCMeanNoiseProfile[slot][p]->Draw("E1 SAME");
            MeanNoise[slot][p]->Update();
            PDF = DQMFolder + MeanNoise[slot][p]->GetName() + ".pdf";
            PNG = DQMFolder + MeanNoise[slot][p]->GetName() + ".png";
            MeanNoise[slot][p]->SaveAs(PDF.c_str());
            MeanNoise[slot][p]->SaveAs(PNG.c_str());
            MeanNoise[slot][p]->Write();

            HitProfile[slot][p]->cd(0);
            RPCHitProfile[slot][p]->SetXTitle("Strip");
            RPCHitProfile[slot][p]->SetYTitle("# events");
            RPCHitProfile[slot][p]->SetFillColor(kBlue);
            RPCHitProfile[slot][p]->Draw();
            HitProfile[slot][p]->Update();
            PDF = DQMFolder + HitProfile[slot][p]->GetName() + ".pdf";
            PNG = DQMFolder + HitProfile[slot][p]->GetName() + ".png";
            HitProfile[slot][p]->SaveAs(PDF.c_str());
            HitProfile[slot][p]->SaveAs(PNG.c_str());
            HitProfile[slot][p]->Write();

            BeamProfile[slot][p]->cd(0);
            RPCBeamProfile[slot][p]->SetXTitle("Strip");
            RPCBeamProfile[slot][p]->SetYTitle("# events");
            RPCBeamProfile[slot][p]->SetFillColor(kBlue);
            RPCBeamProfile[slot][p]->Draw();
            BeamProfile[slot][p]->Update();
            PDF = DQMFolder + BeamProfile[slot][p]->GetName() + ".pdf";
            PNG = DQMFolder + BeamProfile[slot][p]->GetName() + ".png";
            BeamProfile[slot][p]->SaveAs(PDF.c_str());
            BeamProfile[slot][p]->SaveAs(PNG.c_str());
            BeamProfile[slot][p]->Write();

            TimeProfile[slot][p]->cd(0);
            RPCTimeProfile[slot][p]->SetXTitle("Time stamp (ns)");
            RPCTimeProfile[slot][p]->SetYTitle("# events");
            RPCTimeProfile[slot][p]->SetFillColor(kBlue);
            RPCTimeProfile[slot][p]->Draw();
            TimeProfile[slot][p]->Update();
            PDF = DQMFolder + TimeProfile[slot][p]->GetName() + ".pdf";
            PNG = DQMFolder + TimeProfile[slot][p]->GetName() + ".png";
            TimeProfile[slot][p]->SaveAs(PDF.c_str());
            TimeProfile[slot][p]->SaveAs(PNG.c_str());
            TimeProfile[slot][p]->Write();

            HitMultiplicity[slot][p]->cd(0);
            RPCHitMultiplicity[slot][p]->SetXTitle("Multiplicity");
            RPCHitMultiplicity[slot][p]->SetYTitle("# events");
            RPCHitMultiplicity[slot][p]->SetFillColor(kBlue);
            RPCHitMultiplicity[slot][p]->Draw();
            HitMultiplicity[slot][p]->Update();
            PDF = DQMFolder + HitMultiplicity[slot][p]->GetName() + ".pdf";
            PNG = DQMFolder + HitMultiplicity[slot][p]->GetName() + ".png";
            HitMultiplicity[slot][p]->SaveAs(PDF.c_str());
            HitMultiplicity[slot][p]->SaveAs(PNG.c_str());
            HitMultiplicity[slot][p]->Write();
        }
    }

    listCSV.close();

    outputCSV << '\n';
    outputCSV.close();

    outputfile.Close();
    caenFile.Close();
    dataFile.Close();

    //Finally give the permission to the DCS to delete the file if necessary
    string GivePermission = "chmod 775 " + fNameROOT;
    system(GivePermission.c_str());
    GivePermission = "chmod -R 775 " + DQMFolder + "*";
    system(GivePermission.c_str());
    GivePermission = "chmod 775 " + csvName;
    system(GivePermission.c_str());
}
