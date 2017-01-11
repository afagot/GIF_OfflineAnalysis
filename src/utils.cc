//***************************************************************
// *    GIF OFFLINE TOOL v3
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
// *    22/04/2016
//***************************************************************

#include <cmath>
#include "../include/utils.h"

using namespace std;


bool existFiles(string basename){
    string DAQname = basename + "_DAQ.root";
    string CAENname = basename + "_CAEN.root";
    string DIPname = basename + "_DIP.root";

    ifstream DAQfile(DAQname.c_str());
    ifstream CAENfile(CAENname.c_str());
    ifstream DIPfile(DIPname.c_str());

    return (DAQfile.good() && CAENfile.good() && DIPfile.good());
}

// ****************************************************************************************************

int CharToInt(char &C){
    stringstream ss;
    ss << C;

    int I;
    ss >> I;
    return I;
}

// ****************************************************************************************************


string CharToString(char& C){
    stringstream ss;
    ss << C;

    string S;
    ss >> S;
    return S;
}

// ****************************************************************************************************

string intTostring(int value){
    string word;
    stringstream ss;
    ss << value;
    ss>> word;

    return word;
}

// ****************************************************************************************************

string longTostring(long value){
    string word;
    stringstream ss;
    ss << value;
    ss>> word;

    return word;
}

// ****************************************************************************************************

string floatTostring(float value){
    string word;
    stringstream ss;
    ss << value;
    ss>> word;

    return word;
}

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

void WritePath(string baseName){
    //First let's write the path to the log file
    string logpath = baseName.substr(0, baseName.find_last_of("/")+1) + "log.txt";

    ofstream logpathfile(__logpath.c_str(), ios::out);
    logpathfile << logpath;
    logpathfile.close();
}

// ****************************************************************************************************

//Functions to set up the structures needed to define the GIF++ infrastructure
void SetRPC(RPC &rpc, string ID, IniFile *geofile){
    rpc.name        = geofile->stringType(ID,"Name","");
    rpc.nPartitions = geofile->intType(ID,"Partitions",NPARTITIONS);
    rpc.nGaps       = geofile->intType(ID,"Gaps",0);

    for(unsigned int g = 0 ; g < rpc.nGaps; g++){
        string gapID = "Gap" + intTostring(g+1);
        rpc.gaps.push_back(geofile->stringType(ID,gapID,""));

        string areaID = "Area" + gapID;
        rpc.gapGeo.push_back(geofile->floatType(ID,areaID,1.));
    }

    rpc.strips      = geofile->intType(ID,"Strips",NSLOTS);
    string partID = "ABCD";

    for(unsigned int p = 0; p < rpc.nPartitions; p++){
        string minorID  = "Minor-"  + CharToString(partID[p]);
        string majorID  = "Major-"  + CharToString(partID[p]);
        string heightID = "Height-" + CharToString(partID[p]);

        float minor  = geofile->floatType(ID,minorID,1.);
        float major  = geofile->floatType(ID,majorID,1.);
        float height = geofile->floatType(ID,heightID,1.);

        float area = ((minor + major) * height)/2.;
        rpc.stripGeo.push_back(area);
    }
}

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


//Name of histograms
void SetIDName(string rpcID, unsigned int partition, char* ID, char* Name, string IDroot, string Nameroot){
    string P[4] = {"A","B","C","D"};
    sprintf(ID,"%s_%s_%s",IDroot.c_str(),rpcID.c_str(),P[partition].c_str());
    sprintf(Name,"%s %s_%s",Nameroot.c_str(),rpcID.c_str(),P[partition].c_str());
}

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


//Function use to sort hits by increasing strip number
bool SortStrips ( RPCHit A, RPCHit B ) {
    return ( A.Strip < B.Strip );
}

// ****************************************************************************************************


//Return the partition corresponding to the strip
int GetPartition( int strip ) {
    return strip/NSTRIPSPART;
}

// ****************************************************************************************************


//Draw 1D histograms
void DrawTH1(TCanvas* C, TH1* H, string xtitle, string ytitle, string option, string DQMFolder){
    C->cd(0);
    H->SetXTitle(xtitle.c_str());
    H->SetYTitle(ytitle.c_str());
    H->SetFillColor(kBlue);
    H->Draw(option.c_str());
    C->Update();
    string PDF = DQMFolder + C->GetName() + ".pdf";
    string PNG = DQMFolder + C->GetName() + ".png";
    C->SaveAs(PDF.c_str());
    C->SaveAs(PNG.c_str());
}

// ****************************************************************************************************


//Draw 2D histograms
void DrawTH2(TCanvas* C, TH2* H, string xtitle, string ytitle, string ztitle, string option, string DQMFolder){
    C->cd(0);
    H->SetXTitle(xtitle.c_str());
    H->SetYTitle(ytitle.c_str());
    H->SetXTitle(ztitle.c_str());
    gStyle->SetPalette(55);
    H->Draw(option.c_str());
    C->SetLogz(1);
    C->Update();
    string PDF = DQMFolder + C->GetName() + ".pdf";
    string PNG = DQMFolder + C->GetName() + ".png";
    C->SaveAs(PDF.c_str());
    C->SaveAs(PNG.c_str());
}
