#include "../include/utils.h"

using namespace std;

void MakeHeader(string filename){
    char part[4] = "ABC";               //Names of the partitions
    string position[4] = {"S4","S3","S2","S1"};

    ofstream outputCSV(filename.c_str(), ios::out | ios::app);

    outputCSV	<< "File Name" << '\t';

    for(unsigned int rpc=0; rpc<NRPCTROLLEY; rpc++){
        for(unsigned int p=0; p<NPARTITIONS; p++){
            char columnLabel[100];
            char errorLabel[100];
            sprintf(columnLabel,"Mean Rate%s%c (Hz/cm2)",position[rpc].c_str(),part[p]);
            sprintf(errorLabel,"Error %s%c (Hz/cm2)",position[rpc].c_str(),part[p]);
            outputCSV << columnLabel << '\t' << errorLabel << '\t';
        }
    }
    outputCSV << '\n';
    outputCSV.close();
}

//Name of histograms
void SetIDName(unsigned int trolley, unsigned int station,unsigned int partition, char* ID, char* Name, const char* IDroot, const char* Nameroot){
    string P[4] = {"A","B","C","D"};
    sprintf(ID,"%s_T%u_S%u_%s",IDroot,trolley,station+1,P[partition].c_str());
    sprintf(Name,"%s T%u_S%u_%s",Nameroot,trolley,station+1,P[partition].c_str());
}

//Set the RPCHit variables
void SetRPCHit(RPCHit& Hit, int Channel, float TimeStamp){
    Hit.Channel     = Channel;                      //RPC channel according to mapping (5 digits)
    Hit.Trolley     = Channel/10000;                //0, 1 or 3 (1st digit of the RPC channel)
    Hit.Station     = (Channel%10000)/1000;         //From 1 to 4 (2nd digit)
    Hit.Strip       = Channel%1000;                 //From 1 to 128 (3 last digits)
    Hit.Partition   = (Hit.Strip-1)/NSTRIPSPART+1;  //From 1 to 4
    Hit.Connector   = (Hit.Strip-1)/NSTRIPSCONN+1;  //From 1 to 8
    Hit.TimeStamp   = TimeStamp;
}

//Function use to sort hits by increasing strip number
bool SortStrips ( RPCHit A, RPCHit B ) {
    return ( A.Strip < B.Strip );
}

//Return the partition corresponding to the strip
int GetPartition( int strip ) {
    return strip/NSTRIPSPART;
}
