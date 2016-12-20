//***************************************************************
// *    GIF OFFLINE TOOL v3
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    NoiseRate.cc
// *
// *    Extraction of the rates from Scan_00XXXX_HVX_DAQ.root
// *    files + monitoring of strip activities and noise
// *    homogeneity.
// *
// *    Developped by : Alexis Fagot
// *    22/04/2016
//***************************************************************

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

//*******************************************************************************

string GetSavePath(string baseName, string stepID){
    string path;
    path = baseName.substr(0,baseName.find_last_of("/")) + "/HV" + stepID + "/DAQ/";
    MSG_INFO("[Offline] DQM files in " + path);
    return path;
}

//*******************************************************************************

map<int,int> TDCMapping(string baseName){
    //# RPC Channel (TS000 to TS127 : T = trolleys, S = slots, up to 127 strips)
    int RPCCh;

    //# TDC Channel (M000 to M127 : M modules (from 0), 128 channels)
    int TDCCh;

    //2D Map of the TDC Channels and their corresponding RPC strips
    map<int,int> Map;

    //File that contains the path to the mapping file located
    //in the scan directory
    string mapping = baseName.substr(0,baseName.find_last_of("/")) + "/ChannelsMapping.csv";
    ifstream mappathfile(mapping.c_str(), ios::in);
    string mappath;
    mappathfile >> mappath;
    mappathfile.close();

    //Open mapping file
    ifstream mappingfile(mappath.c_str(), ios::in);

    while (mappingfile.good()) { //Fill the map with RPC and TDC channels
        mappingfile >> RPCCh >> TDCCh;
        if ( TDCCh != -1 ) Map[TDCCh] = RPCCh;
    }
    mappingfile.close();

    return Map;
}

//*******************************************************************************

void GetNoiseRate(string baseName){

    string daqName = baseName + "_DAQ.root";
    string caenName = baseName + "_CAEN.root";

    //****************** DAQ ROOT FILE *******************************

    //input ROOT data file containing the RAWData TTree that we'll
    //link to our RAWData structure
    TFile   dataFile(daqName.c_str());
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
    TString* RunType = new TString();
    RunParameters->SetBranchAddress("RunType",&RunType);
    RunParameters->GetEntry(0);

    //Then get the HVstep number from the ID histogram
    TH1D* ID = (TH1D*)dataFile.Get("ID");
    string HVstep = floatTostring(ID->GetBinContent(1));

    //****************** CAEN ROOT FILE ******************************

    //input CAEN ROOT data file containing the values of the HV eff for
    //every HV step
    TFile caenFile(caenName.c_str());
    TH1F *HVeff[NTROLLEYS][NSLOTS];

    //****************** GEOMETRY ************************************

    //Get the chamber geometry
    string dimpath = daqName.substr(0,daqName.find_last_of("/")) + "/Dimensioins.ini";
    IniFile* Dimensions = new IniFile(dimpath.c_str());
    Dimensions->Read();

    //****************** MAPPING *************************************

    map<int,int> RPCChMap = TDCMapping(daqName);

    //****************** HISTOGRAMS & CANVAS *************************

    TH2F     *RPCInstantNoiseRate[NTROLLEYS][NSLOTS][NPARTITIONS];
    TProfile *RPCMeanNoiseProfile[NTROLLEYS][NSLOTS][NPARTITIONS];
    TH1I     *RPCHitProfile[NTROLLEYS][NSLOTS][NPARTITIONS];
    TH1I     *RPCBeamProfile[NTROLLEYS][NSLOTS][NPARTITIONS];
    TH1F     *RPCTimeProfile[NTROLLEYS][NSLOTS][NPARTITIONS];
    TH1I     *RPCHitMultiplicity[NTROLLEYS][NSLOTS][NPARTITIONS];
    TH1F     *RPCStripActivity[NTROLLEYS][NSLOTS][NPARTITIONS];
    TH1F     *RPCNoiseHomogeneity[NTROLLEYS][NSLOTS][NPARTITIONS];

    TCanvas *InstantNoise[NTROLLEYS][NSLOTS][NPARTITIONS];
    TCanvas *MeanNoise[NTROLLEYS][NSLOTS][NPARTITIONS];
    TCanvas *HitProfile[NTROLLEYS][NSLOTS][NPARTITIONS];
    TCanvas *BeamProfile[NTROLLEYS][NSLOTS][NPARTITIONS];
    TCanvas *TimeProfile[NTROLLEYS][NSLOTS][NPARTITIONS];
    TCanvas *HitMultiplicity[NTROLLEYS][NSLOTS][NPARTITIONS];
    TCanvas *StripActivity[NTROLLEYS][NSLOTS][NPARTITIONS];
    TCanvas *NoiseHomogeneity[NTROLLEYS][NSLOTS][NPARTITIONS];

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
            unsigned int nGaps = GIFInfra.Trolleys[t].RPCs[s].nGaps;

            //Get the HVeff histogram by having the highest gap HVeff
            string HVeffHisto = "";
            float HVmax = 0.;

            for(unsigned int g=0; g<nGaps; g++){
                string rpc = GIFInfra.Trolleys[t].RPCs[s].name;
                string tmpHisto;

                if(GIFInfra.Trolleys[t].RPCs[s].gaps[g] == "empty")
                    tmpHisto = "HVeff_" + rpc;
                else
                    tmpHisto = "HVeff_" + rpc + "-" + GIFInfra.Trolleys[t].RPCs[s].gaps[g];

                if(caenFile.GetListOfKeys()->Contains(tmpHisto.c_str())){
                    float tmpHVeff = ((TH1F*)caenFile.Get(tmpHisto.c_str()))->GetMean();
                    if(tmpHVeff > HVmax){
                        HVmax = tmpHVeff;
                        HVeffHisto = tmpHisto;
                    }
                }
            }

            if(HVeffHisto != "") HVeff[trolley][slot] = (TH1F*)caenFile.Get(HVeffHisto.c_str());

            //Get the chamber ID in terms of trolley + slot position TXSX
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

                if(RunType->CompareTo("efficiency") == 0){
                    binWidth = 1./(BMNOISEWDW*1e-9*stripArea);
                    timeWidth = BMTDCWINDOW;
                } else if(RunType->CompareTo("rate") == 0 || RunType->CompareTo("noise_reference") == 0){
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

                //Beam profile
                SetIDName(rpcID,p,hisid,hisname,"RPC_Beam_Profile","RPC beam profile");
                RPCBeamProfile[trolley][slot][p] = new TH1I( hisid, hisname, nStrips, low, high);
                BeamProfile[trolley][slot][p] = new TCanvas(hisid,hisname);

                //Time profile
                SetIDName(rpcID,p,hisid,hisname,"RPC_Time_Profile","RPC time profile");
                RPCTimeProfile[trolley][slot][p] = new TH1F( hisid, hisname, (int)timeWidth/10, 0., timeWidth);
                TimeProfile[trolley][slot][p] = new TCanvas(hisid,hisname);

                //Hit multiplicity
                SetIDName(rpcID,p,hisid,hisname,"RPC_Hit_Multiplicity","RPC hit multiplicity");
                RPCHitMultiplicity[trolley][slot][p] = new TH1I( hisid, hisname, nBinsMult, lowBin, highBin);
                HitMultiplicity[trolley][slot][p] = new TCanvas(hisid,hisname);

                //Strip activity
                SetIDName(rpcID,p,hisid,hisname,"RPC_Strip_Activity","RPC strip activity");
                RPCStripActivity[trolley][slot][p] = new TH1F( hisid, hisname, nStrips, low, high);
                StripActivity[trolley][slot][p] = new TCanvas(hisid,hisname);

                //Noise homogeneity
                SetIDName(rpcID,p,hisid,hisname,"RPC_Noise_homogeneity","RPC noise homogeneity");
                RPCNoiseHomogeneity[trolley][slot][p] = new TH1F( hisid, hisname, 1, 0, 1);
                NoiseHomogeneity[trolley][slot][p] = new TCanvas(hisid,hisname);
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
            if(data.TDCCh->at(h) > 5127) continue;
            if(RPCChMap[data.TDCCh->at(h)] == 0) continue;

            //Get rid of the noise hits in the ground channels of KODEL chambers

            SetRPCHit(hit, RPCChMap[data.TDCCh->at(h)], data.TDCTS->at(h), GIFInfra);

            //Count the number of hits outside the peak
            bool earlyhit = (hit.TimeStamp >= 100. && hit.TimeStamp < 200.);
            bool intimehit = (hit.TimeStamp >= 255. && hit.TimeStamp < 315.);
            bool latehit = (hit.TimeStamp >= 350. && hit.TimeStamp < 550.);

            if(RunType->CompareTo("efficiency") == 0){
                if(earlyhit || latehit)
                    NHitsPerStrip[hit.Trolley][hit.Station-1][hit.Strip-1]++;
                else if(intimehit)
                    RPCBeamProfile[hit.Trolley][hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
            } else if(RunType->CompareTo("rate") == 0 || RunType->CompareTo("noise_reference") == 0)
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

                    if(RunType->CompareTo("rate") == 0 || RunType->CompareTo("noise_reference") == 0)
                        InstantNoise = (float)NHitsPerStrip[trolley][slot][st]/(RDMTDCWINDOW*1e-9*stripArea);
                    else if (RunType->CompareTo("efficiency") == 0)
                        InstantNoise = (float)NHitsPerStrip[trolley][slot][st]/(BMNOISEWDW*1e-9*stripArea);

                    RPCInstantNoiseRate[trolley][slot][p]->Fill(st+1,InstantNoise);

                    //Reinitialise the hit count for strip s
                    NHitsPerStrip[trolley][slot][st] = 0;

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
    string DQMFolder = GetSavePath(baseName,HVstep);
    string mkdirDQMFolder = "mkdir -p " + DQMFolder;
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

        for (unsigned int sl = 0; sl < nSlotsTrolley; sl++){
            unsigned int nPartRPC = GIFInfra.Trolleys[t].RPCs[sl].nPartitions;
            unsigned int slot = CharToInt(GIFInfra.Trolleys[t].SlotsID[sl]) - 1;

            float HighVoltage = HVeff[trolley][slot]->GetMean();
            outputCSV << HighVoltage << '\t';

            //Write the header file
            listCSV << "HVeff-" << GIFInfra.Trolleys[t].RPCs[sl].name << '\t';

            for (unsigned int p = 0; p < nPartRPC; p++){
                string partID = "ABCD";
                //Write the header file
                listCSV << "Rate-"
                        << GIFInfra.Trolleys[t].RPCs[sl].name
                        << "-" << partID[p]
                        << "\tRate-"
                        << GIFInfra.Trolleys[t].RPCs[sl].name
                        << "-" << partID[p]
                        << "_err\t";

                //Project the histograms along the X-axis to get the
                //mean noise profile on the strips
                RPCMeanNoiseProfile[trolley][slot][p] = RPCInstantNoiseRate[trolley][slot][p]->ProfileX();

                //Write in the output file the mean noise rate per
                //partition and its error defined as twice the RMS
                //over the sqrt of the number of events
                float MeanNoiseRate = RPCInstantNoiseRate[trolley][slot][p]->ProjectionY()->GetMean();
                float RMSMean = RPCInstantNoiseRate[trolley][slot][p]->ProjectionY()->GetRMS();
                float ErrorMean = 2*RMSMean/sqrt(nEntries);
                outputCSV << MeanNoiseRate << '\t' << ErrorMean << '\t';

                //Get the activity of each strip defined as the mean noise rate
                //the strip normalised to the mean rate of the partition it
                //belongs too. This way, it is possible to keep track of the
                //apparition of noisy strips and/or dead strips.
                unsigned int nStripsPart = GIFInfra.Trolleys[t].RPCs[sl].strips;

                for(unsigned int st = 0; st < nStripsPart; st++){
                    //Extract the noise for each strip
                    float StripNoiseRate = RPCMeanNoiseProfile[trolley][slot][p]->GetBinContent(st+1);
                    float ErrorStripRate = RPCMeanNoiseProfile[trolley][slot][p]->GetBinError(st+1);

                    //Get the strip activity
                    float StripActivity = StripNoiseRate / MeanNoiseRate;
                    float ErrorStripAct = StripActivity*(ErrorStripRate/StripNoiseRate + ErrorMean/MeanNoiseRate);

                    //Fill the histogram using SetBin methods (to set the error as well)
                    RPCStripActivity[trolley][slot][p]->SetBinContent(st+1,StripActivity);
                    RPCStripActivity[trolley][slot][p]->SetBinError(st+1,ErrorStripAct);
                }

                //Get the partition homogeneity defined as exp(RMS(noise)/MEAN(noise))
                //The closer the homogeneity is to 1 the more homogeneus, the closer
                //the homogeneity is to 0 the less homogeneous.
                //This gives idea about noisy strips and dead strips.
                float homogeneity = exp(-RMSMean/MeanNoiseRate);

                RPCNoiseHomogeneity[trolley][slot][p]->Fill(0.,homogeneity);

                //Draw the histograms and write the canvas

                InstantNoise[trolley][slot][p]->cd(0);
                RPCInstantNoiseRate[trolley][slot][p]->SetXTitle("Strip");
                RPCInstantNoiseRate[trolley][slot][p]->SetYTitle("Noise rate (Hz/cm^{2})");
                RPCInstantNoiseRate[trolley][slot][p]->SetZTitle("# events");
                gStyle->SetPalette(55);
                RPCInstantNoiseRate[trolley][slot][p]->Draw("COLZ");
                InstantNoise[trolley][slot][p]->SetLogz(1);
                InstantNoise[trolley][slot][p]->Update();
                PDF = DQMFolder + InstantNoise[trolley][slot][p]->GetName() + ".pdf";
                PNG = DQMFolder + InstantNoise[trolley][slot][p]->GetName() + ".png";
                InstantNoise[trolley][slot][p]->SaveAs(PDF.c_str());
                InstantNoise[trolley][slot][p]->SaveAs(PNG.c_str());
                RPCInstantNoiseRate[trolley][slot][p]->Write();

                MeanNoise[trolley][slot][p]->cd(0);
                RPCMeanNoiseProfile[trolley][slot][p]->SetXTitle("Strip");
                RPCMeanNoiseProfile[trolley][slot][p]->SetYTitle("Mean Noise rate (Hz/cm^{2})");
                RPCMeanNoiseProfile[trolley][slot][p]->SetFillColor(kBlue);
                RPCMeanNoiseProfile[trolley][slot][p]->Draw("HIST");
                RPCMeanNoiseProfile[trolley][slot][p]->Draw("E1 SAME");
                MeanNoise[trolley][slot][p]->Update();
                PDF = DQMFolder + MeanNoise[trolley][slot][p]->GetName() + ".pdf";
                PNG = DQMFolder + MeanNoise[trolley][slot][p]->GetName() + ".png";
                MeanNoise[trolley][slot][p]->SaveAs(PDF.c_str());
                MeanNoise[trolley][slot][p]->SaveAs(PNG.c_str());
                RPCMeanNoiseProfile[trolley][slot][p]->Write();

                HitProfile[trolley][slot][p]->cd(0);
                RPCHitProfile[trolley][slot][p]->SetXTitle("Strip");
                RPCHitProfile[trolley][slot][p]->SetYTitle("# events");
                RPCHitProfile[trolley][slot][p]->SetFillColor(kBlue);
                RPCHitProfile[trolley][slot][p]->Draw();
                HitProfile[trolley][slot][p]->Update();
                PDF = DQMFolder + HitProfile[trolley][slot][p]->GetName() + ".pdf";
                PNG = DQMFolder + HitProfile[trolley][slot][p]->GetName() + ".png";
                HitProfile[trolley][slot][p]->SaveAs(PDF.c_str());
                HitProfile[trolley][slot][p]->SaveAs(PNG.c_str());
                RPCHitProfile[trolley][slot][p]->Write();

                BeamProfile[trolley][slot][p]->cd(0);
                RPCBeamProfile[trolley][slot][p]->SetXTitle("Strip");
                RPCBeamProfile[trolley][slot][p]->SetYTitle("# events");
                RPCBeamProfile[trolley][slot][p]->SetFillColor(kBlue);
                RPCBeamProfile[trolley][slot][p]->Draw();
                BeamProfile[trolley][slot][p]->Update();
                PDF = DQMFolder + BeamProfile[trolley][slot][p]->GetName() + ".pdf";
                PNG = DQMFolder + BeamProfile[trolley][slot][p]->GetName() + ".png";
                BeamProfile[trolley][slot][p]->SaveAs(PDF.c_str());
                BeamProfile[trolley][slot][p]->SaveAs(PNG.c_str());
                RPCBeamProfile[trolley][slot][p]->Write();

                TimeProfile[trolley][slot][p]->cd(0);
                RPCTimeProfile[trolley][slot][p]->SetXTitle("Time stamp (ns)");
                RPCTimeProfile[trolley][slot][p]->SetYTitle("# events");
                RPCTimeProfile[trolley][slot][p]->SetFillColor(kBlue);
                RPCTimeProfile[trolley][slot][p]->Draw();
                TimeProfile[trolley][slot][p]->Update();
                PDF = DQMFolder + TimeProfile[trolley][slot][p]->GetName() + ".pdf";
                PNG = DQMFolder + TimeProfile[trolley][slot][p]->GetName() + ".png";
                TimeProfile[trolley][slot][p]->SaveAs(PDF.c_str());
                TimeProfile[trolley][slot][p]->SaveAs(PNG.c_str());
                RPCTimeProfile[trolley][slot][p]->Write();

                HitMultiplicity[trolley][slot][p]->cd(0);
                RPCHitMultiplicity[trolley][slot][p]->SetXTitle("Multiplicity");
                RPCHitMultiplicity[trolley][slot][p]->SetYTitle("# events");
                RPCHitMultiplicity[trolley][slot][p]->SetFillColor(kBlue);
                RPCHitMultiplicity[trolley][slot][p]->Draw();
                HitMultiplicity[trolley][slot][p]->Update();
                PDF = DQMFolder + HitMultiplicity[trolley][slot][p]->GetName() + ".pdf";
                PNG = DQMFolder + HitMultiplicity[trolley][slot][p]->GetName() + ".png";
                HitMultiplicity[trolley][slot][p]->SaveAs(PDF.c_str());
                HitMultiplicity[trolley][slot][p]->SaveAs(PNG.c_str());
                RPCHitMultiplicity[trolley][slot][p]->Write();

                StripActivity[trolley][slot][p]->cd(0);
                RPCStripActivity[trolley][slot][p]->SetXTitle("Strip");
                RPCStripActivity[trolley][slot][p]->SetYTitle("Relative strip activity");
                RPCStripActivity[trolley][slot][p]->SetFillColor(kBlue);
                RPCStripActivity[trolley][slot][p]->Draw("HIST");
                RPCStripActivity[trolley][slot][p]->Draw("E1 SAME");
                StripActivity[trolley][slot][p]->Update();
                PDF = DQMFolder + StripActivity[trolley][slot][p]->GetName() + ".pdf";
                PNG = DQMFolder + StripActivity[trolley][slot][p]->GetName() + ".png";
                StripActivity[trolley][slot][p]->SaveAs(PDF.c_str());
                StripActivity[trolley][slot][p]->SaveAs(PNG.c_str());
                RPCStripActivity[trolley][slot][p]->Write();

                NoiseHomogeneity[trolley][slot][p]->cd(0);
                RPCNoiseHomogeneity[trolley][slot][p]->SetXTitle("Partition");
                RPCNoiseHomogeneity[trolley][slot][p]->SetYTitle("Homogeneity");
                RPCNoiseHomogeneity[trolley][slot][p]->SetFillColor(kBlue);
                RPCNoiseHomogeneity[trolley][slot][p]->Draw("HIST TEXT0");
                RPCNoiseHomogeneity[trolley][slot][p]->GetYaxis()->SetRangeUser(0.,1.);
                NoiseHomogeneity[trolley][slot][p]->Update();
                PDF = DQMFolder + NoiseHomogeneity[trolley][slot][p]->GetName() + ".pdf";
                PNG = DQMFolder + NoiseHomogeneity[trolley][slot][p]->GetName() + ".png";
                NoiseHomogeneity[trolley][slot][p]->SaveAs(PDF.c_str());
                NoiseHomogeneity[trolley][slot][p]->SaveAs(PNG.c_str());
                RPCNoiseHomogeneity[trolley][slot][p]->Write();
           }
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
