#GIF++ Offline Analysis

##Compilation

To compile the project, simply use

   make

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

###Usage

Null.

##Noise Rate

####Author: Alexis Fagot
####email : alexis.fagot@ugent.be
####Tel.: +32 9 264 65 69
####Mobile: +32 4 77 91 61 31
####Mobile: +33 6 66 89 02 90

###Usage

In order to use the NoiseRate tool, it is mandatory to know the data file name that you want to analyse and the type of trigger (from the beam or from a random pulse generator). Then, simply type :

    bin/offlineanalysis /path/to/the/data/file/to/analyse.root triggertype

where `triggertype` should be replaced by either `beam` or `random`. This option allows to select whether it should use the total trigger window (random trigger) or only a little part of it (beam trigger) to evaluate the noise/background rates. For example, you got the a random trigger datafile called `Test_Random_Type2.root` stored into `~/GIF_DAQ/datarun/`. To run the noise rate tool, you will need to do :

    bin/offlineanalysis ../GIF_DAQ/datarun/Tets_Random_Type2.root random

The analysed output ROOT datafiles are saved into the folder `~/GIF_OfflineAnalysis/AnalysedData/` and have the same name than the datafiles followed by `-Offline_Noise_Rate.root`. Inside those, you will find a list of *TCanvas* showing histograms more or less long depending on the number of chambers that were used inside the setup during the data taking. For each partition of each chamber, you will find :

* `RPC_Instant_Noise_Tt_Scp` that gives a 2D map of the instantaneous noise per event. It mean that for each event, counting the number of hits and knowing the time window of the TDC, a noise rate is calculated and then filled in the map,
* `RPC_Mean_Noise_Tt_Scp` that gives the projection along the x-axis of the previous 2D map. This way, the result obtained is the mean noise rate during the entire data taking for each strip,
* `RPC_Hit_Profile_Tt_Scp` that gives the hit profile of the partition,
* `RPC_Time_Profile_Tt_Scp` that gives the time profile of the partition, and
* `RPC_Hit_Multiplicity_Tt_Scp` that gives the hit multiplicity of the partition during the last run.

**Note :** in the histogram labels, **t** is for the trolley number (1 or 3), **c** is for chamber label (from 1 to 4 for each trolley) and **p** is for the partition label (A, B, C or D depending on the chamber layout).
**Advice :** when you want to analyse a complete scan containing several runs, create a file (.txt, .dat or .csv) containing all the run filenames and then loop over these names like follows :

    while read dFile
    do
    bin/offlineanalysis $dFile
    done <AnalysedData/FileList.csv

Here, you tell the terminal to loop over the lines of `AnalysedData/FileList.csv` that contains the runs you want to analyse via `while read dFile`. For each line it then runs the analysis via `bin/offlineanalysis $dFile`.
