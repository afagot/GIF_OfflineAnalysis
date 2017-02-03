#GIF++ Offline Analysis

##Efficiency

####Author:
####email :
####Tel.:
####Mobile:

###Usage

Null.

##Cluster Size

####Author: Cristina Oropeza Barrera
####email : cbarrera@cern.ch
####Tel.: +52 55 5950 4000 ext. 4675
####Mobile: +52 55 5412 0122

###Usage

*Note:* The current version of the code is suitable to analyse data taken during the July test beam. For the time being, the `ClusterSize tool` is run in a separate directory inside the main `RPC_GIF_Offline` branch:

    cd ClusterSize/

Once there, to run the tool you simply need to type:

    ./ClusterSize /path/to/the/data/file/to/analyse.root

The code automatically decides whether the data was taken using a beam or a random trigger, based on the file name. It produces two output files that are stored in `AnalysedData`, located inside the `ClusterSize` directory. One of them is a root file containing histograms for the number of clusters and cluster size for each partition in each chamber. The second output file is a text file (.csv) listing the mean number of clusters and the mean cluster size (with their associated uncertainties) for each partition in each chamber, plus info on the HV and attenuators used. Both output files have the same name as the input file, followed by `-Offline_Cluster_Size.root(csv)`.

If you want to make any changes to the code, remember to compile again before running by doing `make`.

In the `ClusterSize` directory you will find three mapping files:

* `ChannelsMapping.csv`: channel-to-strip mapping for Trolley-0 (deprecated).
* `ChannelsMapping_Trolley1_4TDCs.csv`: channel-to-strip mapping for Trolley-1, for run numbers between 20150718000329 and 20150719180236.
* `ChannelsMapping_Trolley1.csv`: channel-to-strip mapping for Trolley-1, for run numbers starting from 20150719184203.

The code will look at the run number from the input file name and automatically decide which mapping file to use.

####Author: Alexis Fagot
####email : alexis.fagot@ugent.be
####Tel.: +32 9 264 65 69

##Compilation

To compile the project, simply use

   make

###Usage

In order to use the Offline Analysis tool, it is mandatory to know the data files name that you want to analyse.
There can be up to 3 data file per run taken with the GIF++ WebDCS :

* `Scan00XXXX_HVY_DAQ.root` : contains the TDC data from the RPCs
* `Scan00XXXX_HVY_CAEN.root` : contains the HV and LV data from the RPCs (voltages and currents)
* `Scan00XXXX_HVY_DIP.root` : contains the environmental parameters + source and beam information

In all these 3 files, *XXXX* is the scan ID and *Y* is the high voltage step (in case of a high voltage scan, data will be taken for several HV steps).
To start the analysis, simply type :

    bin/offlineanalysis /path/to/Scan00XXXX_HVY

and it will take care by itself of finding the existing files among the 3 previously cited.
Note that this tool is normally meant to be called by the GIF++ WebDCS but it also works by hand.

The analysed output ROOT datafiles for the rate calculation are saved into the data file folder and called `Scan00XXXX_HVY_DAQ-Rate.root` in case
`Scan00XXXX_HVY_DAQ.root` was provided.

Inside those, you will find a more or less long, depending on the number of chambers that were used inside the setup during the data taking, list of *TCanvas*
showing histograms.
For each partition of each chamber, you will find :

* `RPC_Instant_Noise_Tt_Scp` that gives a 2D map of the instantaneous noise per event. It mean that for each event, counting the number of hits and knowing the time window of the TDC, a noise rate is calculated and then filled in the map,
* `RPC_Mean_Noise_Tt_Scp` that gives the projection along the x-axis of the previous 2D map. This way, the result obtained is the mean noise rate during the entire data taking for each strip,
* `RPC_Hit_Profile_Tt_Scp` that gives the hit profile of the partition,
* `RPC_Time_Profile_Tt_Scp` that gives the time profile of the partition, and
* `RPC_Hit_Multiplicity_Tt_Scp` that gives the hit multiplicity of the partition during the last run.

**Note :** in the histogram labels, **t** is for the trolley number (1 or 3), **c** is for chamber label (from 1 to 4 for each trolley) and **p** is for the partition label (A, B, C or D depending on the chamber layout).

Moreover, up to 3 CSV files can created depending on which ones of the 3 input files were in the data folder :

* `Offline-Rate.csv` : contains the summary of the rate calculation for the entire scan, HV step per HV step
* `Offline-Current.csv` : contains the summary of the currents and voltages applied on the RPCs
* `Offline-DIP` : contains the summary of the environmental, beam and source parameters
