//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    RPCHit.h
// *
// *    Class that defines RPCHit objects. RPCHits are simply
// *    storing the time stamp converting the rpc channel into
// *    strip, station, trolley information. This inofrmation
// *    is extracted from the format of the rpc channel :
// *    it is a 5 digit unsigned int TSCCC
// *    T : trolley ID
// *    S : slot ID (or station)
// *    CCC : rpc strip (depending on the chamber it can go up
// *    to 128 strips so it was decided to use 3 digit for the
// *    strip ID).
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    22/06/2017
//***************************************************************

#include <vector>

#include "../include/RPCHit.h"
#include "../include/Mapping.h"
#include "../include/types.h"
#include "../include/utils.h"
#include "../include/Infrastructure.h"

#include "TTree.h"
#include "TF1.h"

using namespace std;

// ****************************************************************************************************
// *    RPCHit()
//
//  Default constructor
// ****************************************************************************************************

RPCHit::RPCHit(){

}

// ****************************************************************************************************
// *    RPCHit(int channel, float time, Infrastructure* Infra)
//
//  Constructor
// ****************************************************************************************************

RPCHit::RPCHit(Uint channel, float time, Infrastructure* Infra){
    Channel     = channel;              //RPC channel according to mapping (5 digits)
    Trolley     = channel/10000;        //0, 1 or 3 (1st digit of the RPC channel)
    Station     = (channel%10000)/1000; //From 1 to 4 (2nd digit)
    Strip       = channel%1000;         //From 1 to 128 (3 last digits)

    int nStripsPart = 0;
    for(Uint tr = 0; tr < Infra->GetNTrolleys(); tr++){
        if(Infra->GetTrolleyID(tr) == Trolley){
            for(Uint sl = 0; sl < Infra->GetNSlots(tr); sl++){
                if(Infra->GetSlotID(tr,sl) == Station)
                    nStripsPart = Infra->GetNStrips(tr,sl);
            }
        }
    }

    Partition   = (Strip-1)/nStripsPart+1; //From 1 to 4
    TimeStamp   = time;
}

// ****************************************************************************************************
// *    RPCHit(const RPCHit &other)
//
//  Copy constructor
// ****************************************************************************************************

RPCHit::RPCHit(const RPCHit &other){
    Channel = other.Channel;
    Trolley = other.Trolley;
    Station = other.Station;
    Strip = other.Strip;
    Partition = other.Partition;
    TimeStamp = other.TimeStamp;
}

// ****************************************************************************************************
// *    ~RPCHit()
//
//  Destructor
// ****************************************************************************************************

RPCHit::~RPCHit(){

}

// ****************************************************************************************************
// *    RPCHit& operator =(const RPCHit& other)
//
//  Copy operator
// ****************************************************************************************************

RPCHit& RPCHit::operator =(const RPCHit& other){
    if(this != &other){
        Channel = other.Channel;
        Trolley = other.Trolley;
        Station = other.Station;
        Strip = other.Strip;
        Partition = other.Partition;
        TimeStamp = other.TimeStamp;
    }

    return *this;
}

// ****************************************************************************************************
// *    Uint GetChannel()
//
//  Get the private member Channel
// ****************************************************************************************************

Uint RPCHit::GetChannel(){
    return Channel;
}

// ****************************************************************************************************
// *    Uint GetTrolley()
//
//  Get the private member Trolley
// ****************************************************************************************************

Uint RPCHit::GetTrolley(){
    return Trolley;
}

// ****************************************************************************************************
// *    Uint GetStation()
//
//  Get the private member Station
// ****************************************************************************************************

Uint RPCHit::GetStation(){
    return Station;
}

// ****************************************************************************************************
// *    Uint GetStrip()
//
//  Get the private member Strip
// ****************************************************************************************************

Uint RPCHit::GetStrip(){
    return Strip;
}

// ****************************************************************************************************
// *    Uint GetPartition()
//
//  Get the private member Partition
// ****************************************************************************************************

Uint RPCHit::GetPartition(){
    return Partition;
}

// ****************************************************************************************************
// *    Uint GetTime()
//
//  Get the private member TimeStamp
// ****************************************************************************************************

float RPCHit::GetTime(){
    return TimeStamp;
}

// ****************************************************************************************************
// *    bool SortHitbyStrip(RPCHit h1, RPCHit h2)
//
//  Sort RPC hits using strip position information
// ****************************************************************************************************

bool SortHitbyStrip(RPCHit h1, RPCHit h2){
    return (h1.GetStrip() < h2.GetStrip());
}

// ****************************************************************************************************
// *    bool SortHitbyTime(RPCHit h1, RPCHit h2)
//
//  Sort RPC hits using time stamp information
// ****************************************************************************************************

//Sort hits by time
bool SortHitbyTime(RPCHit h1, RPCHit h2){
    return (h1.GetTime() < h2.GetTime());
}

// ****************************************************************************************************
// *    void SetBeamWindow (muonPeak &PeakTime, muonPeak &PeakWidth,
// *                        TTree* mytree, Mapping* RPCChMap, Infrastructure* Infra)
//
//  Loops over all the data contained inside of the ROOT file and determines for each RPC the center
//  of the muon peak and its spread. Then saves the result in 2 3D tables (#D bescause it follows the
//  dimensions of the GIF++ infrastructure : Trolley, RPC slots and Partitions).
// ****************************************************************************************************

void SetBeamWindow (muonPeak &PeakTime, muonPeak &PeakWidth,
                    TTree* mytree, Mapping* RPCChMap, Infrastructure* Infra){
    RAWData mydata;

    mydata.TDCCh = new vector<Uint>;
    mydata.TDCTS = new vector<float>;
    mydata.TDCCh->clear();
    mydata.TDCTS->clear();

    mytree->SetBranchAddress("EventNumber",    &mydata.iEvent);
    mytree->SetBranchAddress("number_of_hits", &mydata.TDCNHits);
    mytree->SetBranchAddress("TDC_channel",    &mydata.TDCCh);
    mytree->SetBranchAddress("TDC_TimeStamp",  &mydata.TDCTS);

    GIFH1Array tmpTimeProfile;
    GIFfloatArray noiseHits = {{{0.}}};
    int binWidth = TIMEBIN;

    for(Uint tr = 0; tr < NTROLLEYS; tr++)
        for(Uint sl = 0; sl < NSLOTS; sl++)
            for(Uint p = 0; p < NPARTITIONS; p++){
                string name = "tmpTProf" + intToString(tr) + intToString(sl) +  intToString(p);
                tmpTimeProfile.rpc[tr][sl][p] = new TH1F(name.c_str(),name.c_str(),BMTDCWINDOW/TIMEBIN,0.,BMTDCWINDOW);
            }

    //Loop over the entries to get the hits and fill the time distribution + count the
    //noise hits in the window around the peak

    for(Uint i = 0; i < mytree->GetEntries(); i++){
        mytree->GetEntry(i);

        for(int h = 0; h < mydata.TDCNHits; h++){
            Uint channel = mydata.TDCCh->at(h);
            float timing = mydata.TDCTS->at(h);

            //Get rid of the noise hits outside of the connected channels
            if(channel > 5127) continue;
            if(RPCChMap->GetLink(channel) == 0) continue;
            RPCHit tmpHit(RPCChMap->GetLink(channel), timing, Infra);
            Uint T = tmpHit.GetTrolley();
            Uint S = tmpHit.GetStation()-1;
            Uint P = tmpHit.GetPartition()-1;

            tmpTimeProfile.rpc[T][S][P]->Fill(tmpHit.GetTime());
        }
    }

    //Compute the average number of noise hits per 10ns bin and subtract it to the time
    //distribution

    muonPeak center;
    muonPeak lowlimit;
    muonPeak highlimit;

    for(Uint tr = 0; tr < NTROLLEYS; tr++){
        for(Uint sl = 0; sl < NSLOTS; sl++){
            for(Uint p = 0; p < NPARTITIONS; p++){
                //Fit with a gaussian the "Good TDC Time"
                TF1 *slicefit = new TF1("slicefit","gaus(0)",TIMEREJECT,BMTDCWINDOW);

                //Initialise with default parameters
                //Amplitude
                slicefit->SetParameter(0,50);
                slicefit->SetParLimits(0,1,100000);
                //Mean value
                slicefit->SetParameter(1,300);
                slicefit->SetParLimits(1,200,400);
                //RMS
                slicefit->SetParameter(2,10);
                slicefit->SetParLimits(2,1,40);

                //Work only with the filled histograms.
                //Find the highest bin. Assume than the beam peak is within
                //a range of 80ns around the max bin. Evaluate the level of
                //noise outside of this range and subtract it from each bin.
                //Then finally fir and extract fit parameters.
                if(tmpTimeProfile.rpc[tr][sl][p]->GetEntries() > 0.){
                    center.rpc[tr][sl][p] = (float)tmpTimeProfile.rpc[tr][sl][p]->GetMaximumBin()*TIMEBIN;
                    lowlimit.rpc[tr][sl][p] = center.rpc[tr][sl][p] - 40.;
                    highlimit.rpc[tr][sl][p] = center.rpc[tr][sl][p] + 40.;

                    float timeWdw = BMTDCWINDOW-TIMEREJECT-(highlimit.rpc[tr][sl][p]-lowlimit.rpc[tr][sl][p]);

                    int nNoiseHitsLow =
                            tmpTimeProfile.rpc[tr][sl][p]->Integral(TIMEREJECT/TIMEBIN,lowlimit.rpc[tr][sl][p]/TIMEBIN);
                    int nNoiseHitsHigh =
                            tmpTimeProfile.rpc[tr][sl][p]->Integral(highlimit.rpc[tr][sl][p]/TIMEBIN,BMTDCWINDOW/TIMEBIN);

                    noiseHits.rpc[tr][sl][p] = (float)binWidth*(nNoiseHitsLow+nNoiseHitsHigh)/timeWdw;

                    for(Uint b = 1; b <= BMTDCWINDOW/binWidth; b++){
                        float binContent = (float)tmpTimeProfile.rpc[tr][sl][p]->GetBinContent(b);
                        float correctedContent =
                                (binContent < noiseHits.rpc[tr][sl][p]) ? 0. : binContent-noiseHits.rpc[tr][sl][p];
                        tmpTimeProfile.rpc[tr][sl][p]->SetBinContent(b,correctedContent);
                    }

                    //Reset the fit function range with position of max bin
                    slicefit->SetRange(lowlimit.rpc[tr][sl][p],highlimit.rpc[tr][sl][p]);

                    //Reset amplitude with amplitude of highest bin
                    slicefit->SetParameter(0,(float)tmpTimeProfile.rpc[tr][sl][p]->GetMaximum());

                    //Fit the peak time
                    tmpTimeProfile.rpc[tr][sl][p]->Fit(slicefit,"QR");
                }

                //Extract the fit parameters
                PeakTime.rpc[tr][sl][p] = slicefit->GetParameter(1);
                PeakWidth.rpc[tr][sl][p] = 3.*slicefit->GetParameter(2);
            }
        }
    }
}
