//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    Cluster.cc
// *
// *    Class that defines Cluster objects. Clusters are not
// *    defined as a list of RPCHits but it could one of the
// *    next updates of the code. For now only saving info
// *    extracted from the RPCHit list is enough (first and
// *    lest strips, center, cluster size, time stamps of the
// *    hits, etc...).
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    22/06/2017
//***************************************************************

#include <string>
#include <vector>

#include "../include/types.h"
#include "../include/Cluster.h"
#include "../include/RPCHit.h"

using namespace std;

// ****************************************************************************************************
// *    RPCCluster()
//
//  Default constructor
// ****************************************************************************************************

RPCCluster::RPCCluster(){

}

// ****************************************************************************************************
// *    RPCCluster(HitList List, Uint cID, Uint cSize, Uint first, Uint firstID)
//
//  Constructor
// ****************************************************************************************************

RPCCluster::RPCCluster(HitList List, Uint cID, Uint cSize, Uint first, Uint firstID){
    ClusterID   = cID;
    ClusterSize = cSize;
    FirstStrip  = first;
    LastStrip   = first+cSize-1;
    Center      = (FirstStrip+LastStrip)/2.;

    //Cluster are reconstructed in the non rejected time window
    float max = 0.;
    float min = 0.;

    for(Uint i = firstID; i <= (firstID + cSize-1); i++){
        if(i == firstID && min == 0. && max == 0.){
            min = List[i].GetTime();
            max = List[i].GetTime();
        } else {
            if(List[i].GetTime() < min ) min = List[i].GetTime();
            if(List[i].GetTime() > max ) max = List[i].GetTime();
        }
    }

    StartStamp  = min;
    StopStamp   = max;
    TimeSpread  = max-min;
}

// ****************************************************************************************************
// *    RPCCluster(const RPCCluster &other)
//
//  Copy constructor
// ****************************************************************************************************

RPCCluster::RPCCluster(const RPCCluster &other){
    ClusterID   = other.ClusterID;
    ClusterSize = other.ClusterSize;
    FirstStrip  = other.FirstStrip;
    LastStrip   = other.LastStrip;
    Center      = other.Center;
    StartStamp  = other.StartStamp;
    StopStamp   = other.StopStamp;
    TimeSpread  = other.TimeSpread;
}

// ****************************************************************************************************
// *    ~RPCCluster()
//
//  Destructor
// ****************************************************************************************************

RPCCluster::~RPCCluster(){

}

// ****************************************************************************************************
// *    RPCCluster& operator=(const RPCCluster& other)
//
//  Copy operator
// ****************************************************************************************************

RPCCluster& RPCCluster::operator=(const RPCCluster& other){
    if(this != &other){
        ClusterID   = other.ClusterID;
        ClusterSize = other.ClusterSize;
        FirstStrip  = other.FirstStrip;
        LastStrip   = other.LastStrip;
        Center      = other.Center;
        StartStamp  = other.StartStamp;
        StopStamp   = other.StopStamp;
        TimeSpread  = other.TimeSpread;
    }

    return *this;
}

// ****************************************************************************************************
// *    Uint GetID()
//
//  Get the private member ClusterID
// ****************************************************************************************************

Uint RPCCluster::GetID(){
    return ClusterID;
}

// ****************************************************************************************************
// *    Uint GetSize()
//
//  Get the private member ClusterSize
// ****************************************************************************************************

Uint RPCCluster::GetSize(){
    return ClusterSize;
}

// ****************************************************************************************************
// *    Uint GetFirstStrip()
//
//  Get the private member FirstStrip
// ****************************************************************************************************

Uint RPCCluster::GetFirstStrip(){
    return FirstStrip;
}

// ****************************************************************************************************
// *    Uint GetLastStrip()
//
//  Get the private member LastStrip
// ****************************************************************************************************

Uint RPCCluster::GetLastStrip(){
    return LastStrip;
}

// ****************************************************************************************************
// *    float GetCenter()
//
//  Get the private member Center
// ****************************************************************************************************

float RPCCluster::GetCenter(){
    return Center;
}

// ****************************************************************************************************
// *    float GetStart()
//
//  Get the private member StartStamp
// ****************************************************************************************************

float RPCCluster::GetStart(){
    return StartStamp;
}

// ****************************************************************************************************
// *    float GetStop()
//
//  Get the private member StopStamp
// ****************************************************************************************************

float RPCCluster::GetStop(){
    return StopStamp;
}

// ****************************************************************************************************
// *    float GetSpread()
//
//  Get the private member TimeSpread
// ****************************************************************************************************

float RPCCluster::GetSpread(){
    return TimeSpread;
}

// ****************************************************************************************************
// *    void BuildClusters(HitList &cluster, ClusterList &clusterList)
//
//  Build cluster using the sorted RPC hit arrays
//  We need the following variables:
//  - cluster size
//  - isolation (nominal, "other side")
//  - cluster:
//      * center,  (left, right sides)
//      * delta time
// ****************************************************************************************************

void BuildClusters(HitList &cluster, ClusterList &clusterList){
    if(cluster.size() > 0){
        // Sort by strip order to make cluster by using adjacent strips
        sort(cluster.begin(), cluster.end(), SortHitbyStrip);

        Uint firstHit = cluster.front().GetStrip(); //first cluster hit
        Uint previous = firstHit;              //previous strip checked
        Uint hitID = 0;      //ID of the first cluster hit to
                             //look for start and stop stamps
        Uint clusterID = 0;  //ID of the cluster starting from 0
        Uint cSize = 0;      //cluster size counter

        //Loop over the hit list and group adjacent strips
        for(Uint i = 0; i < cluster.size(); i++){
            if (cluster[i].GetStrip()-previous > 1){
                clusterList.push_back(RPCCluster(cluster,clusterID,cSize,firstHit,hitID));

                cSize = 1;
                hitID = i;
                firstHit = cluster[i].GetStrip();
                clusterID++;
            }
            previous = cluster[i].GetStrip();
            cSize++;
        }
        clusterID++;

        //Add the last cluster to the list
        clusterList.push_back(RPCCluster(cluster,clusterID,cSize,firstHit,hitID));
    }
}

// ****************************************************************************************************
// *   clusterInfo Clusterization(HitList &hits, TH1 *hcSize, TH1 *hcMult)
//
//  Used to loop over the hit list, create clusters and fill histograms. Calls BuildClusters.
//  The cluster size and multiplicity can be corrected. This is useful for a good evaluation
//  of the muon cluster size and multiplicity. The contribution of the noise/gamma in the peak
//  region to the cluster size and multiplicity can be inserted from the result obtained
//  outside of the peak region as a correction.
//  Returns the cluster multiplicity and size.
// ****************************************************************************************************

clusterInfo Clusterization(HitList &hits, TH1 *hcSize, TH1 *hcMult, clusterInfo clCorr){
    HitList cluster;
    cluster.clear();

    ClusterList clusterList;
    clusterList.clear();

    float timediff = 0.;
    float lastime = 0.;

    for(Uint h = 0; h < hits.size(); h ++){
        timediff = hits[h].GetTime()-lastime;

        //If there is 25 time difference with the previous hit
        //consider that the hit is too far in time and make
        //cluster with what has been saved into the cluster
        //vector
        if(abs(timediff) > 25. && lastime > 0.){
            BuildClusters(cluster,clusterList);
            cluster.clear();
        }

        lastime = hits[h].GetTime();
        cluster.push_back(hits[h]);
    }

    //Make cluster with the very last part of data saved in the list
    if(cluster.size() > 0){
        BuildClusters(cluster,clusterList);
        cluster.clear();
    }

    //Make the needed corrections using the information from clCorr

    //First correct the multiplicity
    int clMult = clusterList.size()-clCorr.first;

    //Then get the global cluster size of the reconstructed cluster list
    float clSize = 0.;
    for(Uint i = 0; i < clusterList.size(); i++)
        if(clusterList[i].GetSize() > 0)
            clSize += clusterList[i].GetSize();
    clSize = clSize/(float)clusterList.size();

    //Correct it taking into account that the global cluster size Clg is
    // Clg = (Cl*M + Clc*Mc) / Mg <=> Cl = (Clg*Mg - Clc*Mc)/M
    //where Cl and M are the cluster size and multiplicity to extract,
    //Clc and Mc are the cluster size and multiplicity of the correcting
    //component, and Mg is the global multiplicity.
    clSize = (clSize*clusterList.size() - clCorr.first*clCorr.second)/(float)clMult;

    //Finally fill the histograms
    hcMult->Fill(clMult);
    hcSize->Fill(clSize);

    clusterInfo clinfo;
    clinfo.first = clMult;
    clinfo.second = clSize;
    return clinfo;
}
