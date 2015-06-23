#include "../include/utils.h"

using namespace std;

void MakeHeader(string filename){
    char part[4] = "ABC";               //Names of the partitions

    ofstream outputCSV(filename.c_str(), ios::out | ios::app);

    outputCSV	<< "File Name" << '\t';

    for(unsigned int rpc=0; rpc<NRPCTROLLEY; rpc++){
        for(unsigned int p=0; p<NPARTITIONS; p++){
            char columnLabel[100];
            char errorLabel[100];
            sprintf(columnLabel,"Mean Rate%u%c (Hz/cm2)",rpc,part[p]);
            sprintf(errorLabel,"Error %u%c (Hz/cm2)",rpc,part[p]);
            outputCSV << columnLabel << '\t' << errorLabel << '\t';
        }
    }
    outputCSV << '\n';
    outputCSV.close();
}

//Name of histograms
void SetIDName(unsigned int station,unsigned int partition, char* partLabel, char* ID, char* Name, char* IDroot, char* Nameroot){
    sprintf(ID,"%s_%i%c",IDroot,station,partLabel[partition]);
    sprintf(Name,"%s %i, Partition %c",Nameroot,station,partLabel[partition]);
}

//Set the RPCHit variables
void SetRPCHit(RPCHit& Hit, int Channel, float TimeStamp){
    Hit.Channel     = Channel;
    Hit.Station     = Channel/100;              //From 0 to NRPCTROLLEY
    Hit.Strip       = Channel%100;              //From 0 to 95
    Hit.Partition   = Hit.Strip/NSTRIPSPART;    //From 0 to 2
    Hit.Connector   = Hit.Strip/NSTRIPSCONN;    //From 0 to 5
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
