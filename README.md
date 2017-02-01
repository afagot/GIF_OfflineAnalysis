#904 Offline Analysis

####Author: Alexis Fagot
####email : alexis.fagot@ugent.be
####Tel.: +32 9 264 65 69

##Compilation

To compile the project, simply use

   make

In the following 2 files, *XXXX* is the scan ID and *Y* is the high voltage step (in case of a high voltage scan, data will be taken for several HV steps).
To start the analysis, simply type :

    bin/offlineanalysis /path/to/Scan00XXXX_HVY

and it will take care by itself of finding the existing files among the 2 files previously mentioned (DAQ and CAEN files).
Note that this tool is normally meant to be called by the 904 WebDCS but it also works by hand.

The analysed output ROOT datafiles for the rate calculation are saved into the data file folder and called `Scan00XXXX_HVY_DAQ-Rate.root` in case
`Scan00XXXX_HVY_DAQ.root` was provided.

Inside those, you will find a more or less long, depending on the number of chambers that were used inside the setup during the data taking, list of *TCanvas*
showing histograms.
For each partition of each chamber, you will find :

* `RPC_Instant_Noise_Scp` that gives a 2D map of the instantaneous noise per event. It mean that for each event, counting the number of hits and knowing the time window of the TDC, a noise rate is calculated and then filled in the map,
* `RPC_Mean_Noise_Scp` that gives the projection along the x-axis of the previous 2D map. This way, the result obtained is the mean noise rate during the entire data taking for each strip,
* `RPC_Hit_Profile_Scp` that gives the hit profile of the partition,
* `RPC_Time_Profile_Scp` that gives the time profile of the partition, and
* `RPC_Hit_Multiplicity_Scp` that gives the hit multiplicity of the partition during the last run.

**Note :** in the histogram labels, **c** is for chamber label (from 1 to 9) and **p** is for the partition label (A, B, C or D depending on the chamber layout).

Moreover, up to 2 CSV files can created depending on which ones of the 2 input files were in the data folder :

* `Offline-Rate.csv` : contains the summary of the rate calculation for the entire scan, HV step per HV step
* `Offline-Current.csv` : contains the summary of the currents and voltages applied on the RPCs
