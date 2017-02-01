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

    //Open mapping file
    ifstream mappingfile(mapping.c_str(), ios::in);
    if(mappingfile){
        while (mappingfile.good()) { //Fill the map with RPC and TDC channels
            mappingfile >> RPCCh >> TDCCh;
            if ( TDCCh != -1 ) Map[TDCCh] = RPCCh;
        }
        mappingfile.close();
    } else {
        MSG_ERROR("[Offline] Couldn't open file " + mapping);
        exit(EXIT_FAILURE);
    }

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
    //Then link a string to the branch corresponding to the
    //run type
    //Convention : efficiency = cosmic trigger , noise_reference = random trigger
    TTree* RunParameters = (TTree*)dataFile.Get("RunParameters");
    TString* RunType = new TString();
    RunParameters->SetBranchAddress("RunType",&RunType);
    RunParameters->GetEntry(0);

    //Then get the HVstep number from the ID histogram
    string HVstep = baseName.substr(baseName.find_last_of("_HV")+1);

    //****************** CAEN ROOT FILE ******************************

    //input CAEN ROOT data file containing the values of the HV eff for
    //every HV step
    TFile caenFile(caenName.c_str());
    TH1F *HVeff[NSLOTS];

    //****************** GEOMETRY ************************************

    //Get the chamber geometry
    string dimpath = daqName.substr(0,daqName.find_last_of("/")) + "/Dimensions.ini";
    IniFile* Dimensions = new IniFile(dimpath.c_str());
    Dimensions->Read();

    //****************** MAPPING *************************************

    map<int,int> RPCChMap = TDCMapping(daqName);

    //****************** HISTOGRAMS & CANVAS *************************

    TH1I     *StripHitProf_H[NSLOTS][NPARTITIONS];
    TH2F     *StripInstNoiseMap_H[NSLOTS][NPARTITIONS];
    TProfile *StripMeanNoiseProf_H[NSLOTS][NPARTITIONS];
    TH1F     *StripActivity_H[NSLOTS][NPARTITIONS];
    TH1F     *StripHomogeneity_H[NSLOTS][NPARTITIONS];
    TH1I     *BeamProf_H[NSLOTS][NPARTITIONS];
    TH1F     *TimeProfile_H[NSLOTS][NPARTITIONS];
    TH1I     *HitMultiplicity_H[NSLOTS][NPARTITIONS];
    TH1I     *ChipHitProf_H[NSLOTS][NPARTITIONS];
    TH2F     *ChipInstNoiseMap_H[NSLOTS][NPARTITIONS];
    TProfile *ChipMeanNoiseProf_H[NSLOTS][NPARTITIONS];
    TH1F     *ChipActivity_H[NSLOTS][NPARTITIONS];
    TH1F     *ChipHomogeneity_H[NSLOTS][NPARTITIONS];

    TCanvas *StripHitProf_C[NSLOTS][NPARTITIONS];
    TCanvas *StripInstNoiseMap_C[NSLOTS][NPARTITIONS];
    TCanvas *StripMeanNoiseProf_C[NSLOTS][NPARTITIONS];
    TCanvas *StripActivity_C[NSLOTS][NPARTITIONS];
    TCanvas *StripHomogeneity_C[NSLOTS][NPARTITIONS];
    TCanvas *BeamProf_C[NSLOTS][NPARTITIONS];
    TCanvas *TimeProfile_C[NSLOTS][NPARTITIONS];
    TCanvas *HitMultiplicity_C[NSLOTS][NPARTITIONS];
    TCanvas *ChipHitProf_C[NSLOTS][NPARTITIONS];
    TCanvas *ChipInstNoiseMap_C[NSLOTS][NPARTITIONS];
    TCanvas *ChipMeanNoiseProf_C[NSLOTS][NPARTITIONS];
    TCanvas *ChipActivity_C[NSLOTS][NPARTITIONS];
    TCanvas *ChipHomogeneity_C[NSLOTS][NPARTITIONS];

    char hisid[50];                     //ID of the histogram
    char hisname[50];                   //Name of the histogram

    Infrastructure GIFInfra;
    SetInfrastructure(GIFInfra,Dimensions);

    unsigned int nSlots = GIFInfra.nSlots;

    for (unsigned int s = 0; s < nSlots; s++){
        unsigned int nPartRPC = GIFInfra.RPCs[s].nPartitions;
        unsigned int slot = CharToInt(GIFInfra.SlotsID[s]) - 1;
        unsigned int nGaps = GIFInfra.RPCs[s].nGaps;

        //Get the HVeff histogram by having the highest gap HVeff
        string HVeffHisto = "";
        float HVmax = 0.;

        for(unsigned int g=0; g<nGaps; g++){
            string rpc = GIFInfra.RPCs[s].name;
            string tmpHisto;

            if(GIFInfra.RPCs[s].gaps[g] == "empty")
                tmpHisto = "HVeff_" + rpc;
            else
                tmpHisto = "HVeff_" + rpc + "-" + GIFInfra.RPCs[s].gaps[g];

            if(caenFile.GetListOfKeys()->Contains(tmpHisto.c_str())){
                float tmpHVeff = ((TH1F*)caenFile.Get(tmpHisto.c_str()))->GetMean();
                if(tmpHVeff > HVmax){
                    HVmax = tmpHVeff;
                    HVeffHisto = tmpHisto;
                }
            }
        }

        if(HVeffHisto != "")
            HVeff[slot] = (TH1F*)caenFile.Get(HVeffHisto.c_str());

        //Get the chamber ID in terms of slot position SX
        string rpcID = "S" + CharToString(GIFInfra.SlotsID[s]);

        for (unsigned int p = 0; p < nPartRPC; p++){
            //Set bining
            unsigned int nStrips = GIFInfra.RPCs[s].strips;
            float low_s = nStrips*p + 0.5;
            float high_s = nStrips*(p+1) + 0.5;

            unsigned int nBinsMult = 101;
            float lowBin = -0.5;
            float highBin = (float)nBinsMult + lowBin;

            //Noise rate bin size depending on the strip surface
            float stripArea = GIFInfra.RPCs[s].stripGeo[p];
            float binWidth = 1.;
            float timeWidth = 1.;

            if(RunType->CompareTo("efficiency") == 0){
                binWidth = 1./(BMNOISEWDW*1e-9*stripArea);
                timeWidth = BMTDCWINDOW;
            } else if(RunType->CompareTo("noise_reference") == 0 || RunType->CompareTo("test") == 0){
                binWidth = 1./(RDMTDCWINDOW*1e-9*stripArea);
                timeWidth = RDMTDCWINDOW;
            }

            //Initialisation of the histograms

            //***************************************** General histograms

            //Beam profile
            SetIDName(rpcID,p,hisid,hisname,"Beam_Profile","Beam profile");
            BeamProf_H[slot][p] = new TH1I( hisid, hisname, nStrips, low_s, high_s);
            BeamProf_C[slot][p] = new TCanvas(hisid,hisname);

            //Time profile
            SetIDName(rpcID,p,hisid,hisname,"Time_Profile","Time profile");
            TimeProfile_H[slot][p] = new TH1F( hisid, hisname, (int)timeWidth/10, 0., timeWidth);
            TimeProfile_C[slot][p] = new TCanvas(hisid,hisname);

            //Hit multiplicity
            SetIDName(rpcID,p,hisid,hisname,"Hit_Multiplicity","Hit multiplicity");
            HitMultiplicity_H[slot][p] = new TH1I( hisid, hisname, nBinsMult, lowBin, highBin);
            HitMultiplicity_C[slot][p] = new TCanvas(hisid,hisname);

            //****************************************** Strip granularuty level histograms

            //Hit profile
            SetIDName(rpcID,p,hisid,hisname,"Strip_Hit_Profile","Strip hit profile");
            StripHitProf_H[slot][p] = new TH1I( hisid, hisname, nStrips, low_s, high_s);
            StripHitProf_C[slot][p] = new TCanvas(hisid,hisname);

            //Instantaneous noise rate 2D map
            SetIDName(rpcID,p,hisid,hisname,"Strip_Instant_Noise_Map","Strip instantaneous noise rate map");
            StripInstNoiseMap_H[slot][p] = new TH2F( hisid, hisname, nStrips, low_s, high_s, nBinsMult, lowBin*binWidth, highBin*binWidth);
            StripInstNoiseMap_C[slot][p] = new TCanvas(hisid,hisname);

            //Mean noise rate profile
            SetIDName(rpcID,p,hisid,hisname,"Strip_Mean_Noise","Strip mean noise rate");
            StripMeanNoiseProf_H[slot][p] = new TProfile( hisid, hisname, nStrips, low_s, high_s);
            StripMeanNoiseProf_C[slot][p] = new TCanvas(hisid,hisname);

            //Strip activity
            SetIDName(rpcID,p,hisid,hisname,"Strip_Activity","Strip activity");
            StripActivity_H[slot][p] = new TH1F( hisid, hisname, nStrips, low_s, high_s);
            StripActivity_C[slot][p] = new TCanvas(hisid,hisname);

            //Noise homogeneity
            SetIDName(rpcID,p,hisid,hisname,"Strip_Homogeneity","Strip homogeneity");
            StripHomogeneity_H[slot][p] = new TH1F( hisid, hisname, 1, 0, 1);
            StripHomogeneity_C[slot][p] = new TCanvas(hisid,hisname);

            //****************************************** Chip granularuty level histograms

            //Hit profile
            SetIDName(rpcID,p,hisid,hisname,"Chip_Hit_Profile","Chip hit profile");
            ChipHitProf_H[slot][p] = new TH1I( hisid, hisname, nStrips/8, low_s, high_s);
            ChipHitProf_C[slot][p] = new TCanvas(hisid,hisname);

            //Instantaneous noise rate 2D map
            SetIDName(rpcID,p,hisid,hisname,"Chip_Instant_Noise_Map","Chip instantaneous noise rate map");
            ChipInstNoiseMap_H[slot][p] = new TH2F( hisid, hisname, nStrips/8, low_s, high_s, nBinsMult, lowBin*binWidth, highBin*binWidth);
            ChipInstNoiseMap_C[slot][p] = new TCanvas(hisid,hisname);

            //Mean noise rate profile
            SetIDName(rpcID,p,hisid,hisname,"Chip_Mean_Noise","Chip mean noise rate");
            ChipMeanNoiseProf_H[slot][p] = new TProfile( hisid, hisname, nStrips/8, low_s, high_s);
            ChipMeanNoiseProf_C[slot][p] = new TCanvas(hisid,hisname);

            //Strip activity
            SetIDName(rpcID,p,hisid,hisname,"Chip_Activity","Chip activity");
            ChipActivity_H[slot][p] = new TH1F( hisid, hisname, nStrips/8, low_s, high_s);
            ChipActivity_C[slot][p] = new TCanvas(hisid,hisname);

            //Noise homogeneity
            SetIDName(rpcID,p,hisid,hisname,"Chip_Homogeneity","Chip homogeneity");
            ChipHomogeneity_H[slot][p] = new TH1F( hisid, hisname, 1, 0, 1);
            ChipHomogeneity_C[slot][p] = new TCanvas(hisid,hisname);
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
            if(RPCChMap[data.TDCCh->at(h)] == 0) continue;

            SetRPCHit(hit, RPCChMap[data.TDCCh->at(h)], data.TDCTS->at(h), GIFInfra);

            //Count the number of hits outside the peak
            bool earlyhit = (hit.TimeStamp >= 100. && hit.TimeStamp < 200.);
            bool intimehit = (hit.TimeStamp >= 255. && hit.TimeStamp < 315.);
            bool latehit = (hit.TimeStamp >= 350. && hit.TimeStamp < 550.);

            if(RunType->CompareTo("efficiency") == 0){
                if(earlyhit || latehit)
                    NHitsPerStrip[hit.Station-1][hit.Strip-1]++;
                else if(intimehit)
                    BeamProf_H[hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
            } else if(RunType->CompareTo("noise_reference") == 0 || RunType->CompareTo("test") == 0)
                NHitsPerStrip[hit.Station-1][hit.Strip-1]++;

            //Fill the profiles
            StripHitProf_H[hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
            ChipHitProf_H[hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
            TimeProfile_H[hit.Station-1][hit.Partition-1]->Fill(hit.TimeStamp);
            Multiplicity[hit.Station-1][hit.Partition-1]++;
        }

        //** INSTANTANEOUS NOISE RATE ********************************

        unsigned int nSlots = GIFInfra.nSlots;

        for(unsigned int sl=0; sl<nSlots; sl++){
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

                if(RunType->CompareTo("noise_reference") == 0 || RunType->CompareTo("test") == 0)
                    InstantNoise = (float)NHitsPerStrip[slot][st]/(RDMTDCWINDOW*1e-9*stripArea);
                else if (RunType->CompareTo("efficiency") == 0)
                    InstantNoise = (float)NHitsPerStrip[slot][st]/(BMNOISEWDW*1e-9*stripArea);

                StripInstNoiseMap_H[slot][p]->Fill(st+1,InstantNoise);
                ChipInstNoiseMap_H[slot][p]->Fill(st+1,InstantNoise);

                //Reinitialise the hit count for strip s
                NHitsPerStrip[slot][st] = 0;

                //Fill the multiplicity for this event
                if(st%nStripsPart == 0){
                    HitMultiplicity_H[slot][p]->Fill(Multiplicity[slot][p]);
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
    string DQMFolder = GetSavePath(baseName,HVstep);
    string mkdirDQMFolder = "mkdir -p " + DQMFolder;
    system(mkdirDQMFolder.c_str());

    for (unsigned int sl = 0; sl < nSlots; sl++){
        unsigned int nPartRPC = GIFInfra.RPCs[sl].nPartitions;
        unsigned int slot = CharToInt(GIFInfra.SlotsID[sl]) - 1;

        float HighVoltage = HVeff[slot]->GetMean();
        outputCSV << HighVoltage << '\t';

        //Write the header file
        listCSV << "HVeff-" << GIFInfra.RPCs[sl].name << '\t';

        for (unsigned int p = 0; p < nPartRPC; p++){
            string partID = "ABCD";
            //Write the header file
            listCSV << "Rate-"
                    << GIFInfra.RPCs[sl].name
                    << "-" << partID[p]
                    << "\tRate-"
                    << GIFInfra.RPCs[sl].name
                    << "-" << partID[p]
                    << "_err\t";

            //Project the histograms along the X-axis to get the
            //mean noise profile on the strips and chips
            StripMeanNoiseProf_H[slot][p] = StripInstNoiseMap_H[slot][p]->ProfileX();
            ChipMeanNoiseProf_H[slot][p] = ChipInstNoiseMap_H[slot][p]->ProfileX();

            //Write in the output file the mean noise rate per
            //partition and its error defined as twice the RMS
            //over the sqrt of the number of events
            float MeanNoiseRate = StripInstNoiseMap_H[slot][p]->ProjectionY()->GetMean();
            float StripRMSMean = StripInstNoiseMap_H[slot][p]->ProjectionY()->GetRMS();
            float ErrorMean = 2*StripRMSMean/sqrt(nEntries);
            outputCSV << MeanNoiseRate << '\t' << ErrorMean << '\t';

            //Get the activity of each strip defined as the mean noise rate
            //the strip normalised to the mean rate of the partition it
            //belongs too. This way, it is possible to keep track of the
            //apparition of noisy strips and/or dead strips.
            unsigned int nStripsPart = GIFInfra.RPCs[sl].strips;

            for(unsigned int st = 0; st < nStripsPart; st++){
                //Extract the noise for each strip
                float StripNoiseRate = StripMeanNoiseProf_H[slot][p]->GetBinContent(st+1);
                float ErrorStripRate = StripMeanNoiseProf_H[slot][p]->GetBinError(st+1);

                //Get the strip activity
                float StripActivity = StripNoiseRate / MeanNoiseRate;
                float ErrorStripAct = StripActivity*(ErrorStripRate/StripNoiseRate + ErrorMean/MeanNoiseRate);

                //Fill the histogram using SetBin methods (to set the error as well)
                StripActivity_H[slot][p]->SetBinContent(st+1,StripActivity);
                StripActivity_H[slot][p]->SetBinError(st+1,ErrorStripAct);

                //Extract the noise for each Chip
                float ChipNoiseRate = ChipMeanNoiseProf_H[slot][p]->GetBinContent(st+1);
                float ErrorChipRate = ChipMeanNoiseProf_H[slot][p]->GetBinError(st+1);

                //Get the chip activity
                float ChipActivity = ChipNoiseRate / MeanNoiseRate;
                float ErrorChipAct = ChipActivity*(ErrorChipRate/ChipNoiseRate + ErrorMean/MeanNoiseRate);

                //Fill the histogram using SetBin methods (to set the error as well)
                ChipActivity_H[slot][p]->SetBinContent(st+1,ChipActivity);
                ChipActivity_H[slot][p]->SetBinError(st+1,ErrorChipAct);
            }

            //Get the partition homogeneity defined as exp(RMS(noise)/MEAN(noise))
            //The closer the homogeneity is to 1 the more homogeneus, the closer
            //the homogeneity is to 0 the less homogeneous.
            //This gives idea about noisy strips and dead strips.
            float strip_homog = exp(-StripRMSMean/MeanNoiseRate);
            StripHomogeneity_H[slot][p]->Fill(0.,strip_homog);

            //Same thing for the chip level - need to get the RMS at the chip level, the mean stays the same
            float ChipRMSMean = ChipInstNoiseMap_H[slot][p]->ProjectionY()->GetRMS();

            float chip_homog = exp(-ChipRMSMean/MeanNoiseRate);
            ChipHomogeneity_H[slot][p]->Fill(0.,chip_homog);

            //Draw and write the histograms into the output ROOT file

            //********************************* General histograms

            DrawTH1(BeamProf_C[slot][p],BeamProf_H[slot][p],"Strip","# events","",DQMFolder);
            BeamProf_H[slot][p]->Write();

            DrawTH1(TimeProfile_C[slot][p],TimeProfile_H[slot][p],"Time stamp (ns)","# events","",DQMFolder);
            TimeProfile_H[slot][p]->Write();

            DrawTH1(HitMultiplicity_C[slot][p],HitMultiplicity_H[slot][p],"Multiplicity","# events","",DQMFolder);
            HitMultiplicity_H[slot][p]->Write();

            //******************************* Strip granularity histograms

            DrawTH1(StripHitProf_C[slot][p],StripHitProf_H[slot][p],"Strip","# events","",DQMFolder);
            StripHitProf_H[slot][p]->Write();

            DrawTH2(StripInstNoiseMap_C[slot][p],StripInstNoiseMap_H[slot][p],"Strip","Noise rate (Hz/cm^{2})","# events","COLZ",DQMFolder);
            StripInstNoiseMap_H[slot][p]->Write();

            DrawTH1(StripMeanNoiseProf_C[slot][p],StripMeanNoiseProf_H[slot][p],"Strip","Mean Noise rate (Hz/cm^{2})","HIST E1",DQMFolder);
            StripMeanNoiseProf_H[slot][p]->Write();

            DrawTH1(StripActivity_C[slot][p],StripActivity_H[slot][p],"Strip","Relative strip activity","HIST E1",DQMFolder);
            StripActivity_H[slot][p]->Write();

            StripHomogeneity_H[slot][p]->GetYaxis()->SetRangeUser(0.,1.);
            DrawTH1(StripHomogeneity_C[slot][p],StripHomogeneity_H[slot][p],"Partition","Homogeneity","HIST TEXT0",DQMFolder);
            StripHomogeneity_H[slot][p]->Write();

            //******************************* Chip granularity histograms

            DrawTH1(ChipHitProf_C[slot][p],ChipHitProf_H[slot][p],"Strip","# events","",DQMFolder);
            ChipHitProf_H[slot][p]->Write();

            DrawTH2(ChipInstNoiseMap_C[slot][p],ChipInstNoiseMap_H[slot][p],"Strip","Noise rate (Hz/cm^{2})","# events","COLZ",DQMFolder);
            ChipInstNoiseMap_H[slot][p]->Write();

            DrawTH1(ChipMeanNoiseProf_C[slot][p],ChipMeanNoiseProf_H[slot][p],"Strip","Mean Noise rate (Hz/cm^{2})","HIST E1",DQMFolder);
            ChipMeanNoiseProf_H[slot][p]->Write();

            DrawTH1(ChipActivity_C[slot][p],ChipActivity_H[slot][p],"Strip","Relative strip activity","HIST E1",DQMFolder);
            ChipActivity_H[slot][p]->Write();

            ChipHomogeneity_H[slot][p]->GetYaxis()->SetRangeUser(0.,1.);
            DrawTH1(ChipHomogeneity_C[slot][p],ChipHomogeneity_H[slot][p],"Partition","Homogeneity","HIST TEXT0",DQMFolder);
            ChipHomogeneity_H[slot][p]->Write();
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
