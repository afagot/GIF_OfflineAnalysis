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


// ****************************************************************************************************
// *    bool existFiles(string baseName)
//
//  Function that test if the 3 root files created during the data taking exist in the scan directory
//  or not. This is the needed condition for the offline tool to start.
// ****************************************************************************************************

bool existFiles(string baseName){
    string DAQname = baseName + "_DAQ.root";
    string CAENname = baseName + "_CAEN.root";

    TFile DAQfile(DAQname.c_str());
    TFile CAENfile(CAENname.c_str());

    return (DAQfile.IsOpen() || CAENfile.IsOpen());
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//  Function that casts a char into an int
// ****************************************************************************************************

int CharToInt(char &C){
    stringstream ss;
    ss << C;

    int I;
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

string intTostring(int value){
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
// *    void SetRPC(RPC &rpc, string ID, IniFile *geofile)
//
//  Set up the RPC structure needed which is the innermost part of the GIF++ infrasctructure. The RPCs
//  Are contained inside the Trolleys. For details about RPC members, see file utils.h.
// ****************************************************************************************************

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
// *    int CharToInt(char &C)
//
//
// ****************************************************************************************************

void SetInfrastructure(Infrastructure &infra, IniFile *geofile){
    infra.nSlots = geofile->intType("General","nSlots",NSLOTS);
    infra.SlotsID = geofile->stringType("General","SlotsID","");

    for(unsigned int s = 0; s < infra.nSlots; s++){
        string rpcID = "S" + CharToString(infra.SlotsID[s]);

        RPC temprpc;
        SetRPC(temprpc,rpcID,geofile);
        infra.RPCs.push_back(temprpc);
    }
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//
// ****************************************************************************************************

//Name of histograms
void SetIDName(string rpcID, unsigned int partition, char* ID, char* Name, string IDroot, string Nameroot){
    string P[4] = {"A","B","C","D"};
    sprintf(ID,"%s_%s_%s",IDroot.c_str(),rpcID.c_str(),P[partition].c_str());
    sprintf(Name,"%s %s_%s",Nameroot.c_str(),rpcID.c_str(),P[partition].c_str());
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//
// ****************************************************************************************************

//Set the RPCHit variables
void SetRPCHit(RPCHit& Hit, int Channel, float TimeStamp, Infrastructure Infra){
    Hit.Channel     = Channel;                      //RPC channel according to mapping (4 digits)
    Hit.Station     = (Channel%10000)/1000;         //From 1 to 9 (1st digit)
    Hit.Strip       = Channel%1000;                 //From 1 to 128 (3 last digits)

    int nStripsPart = 0;
    for(unsigned int i = 0; i < Infra.nSlots; i++){
            if(CharToInt(Infra.SlotsID[i]) == Hit.Station)
                nStripsPart = Infra.RPCs[i].strips;
    }

    Hit.Partition   = (Hit.Strip-1)/nStripsPart+1;  //From 1 to 4
    Hit.Connector   = (Hit.Strip-1)/NSTRIPSCONN+1;  //From 1 to 8
    Hit.TimeStamp   = TimeStamp;
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//
// ****************************************************************************************************

//Function use to sort hits by increasing strip number
bool SortStrips ( RPCHit A, RPCHit B ) {
    return ( A.Strip < B.Strip );
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//
// ****************************************************************************************************

//Return the partition corresponding to the strip
int GetPartition( int strip ) {
    return strip/NSTRIPSPART;
}

// ****************************************************************************************************
// *    int CharToInt(char &C)
//
//
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
// *    int CharToInt(char &C)
//
//
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
