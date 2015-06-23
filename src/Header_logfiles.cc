#include <iostream>
#include <iomanip>
#include <fstream>

#include "../include/utils.h"

using namespace std;

void Header_logfiles(string filename){
    char part[4] = "ABC";               //Names of the partitions

    ofstream outputCSV((filename+".csv").c_str(), ios::out | ios::app);

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
