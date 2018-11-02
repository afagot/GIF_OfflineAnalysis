# GIF++ Offline Analysis

#### Authors: Alexis Fagot < Alexis.Fagot@cern.ch >, Salvador Carillo < Salvador.Carrillo@cern.ch >


## Compilation

The project is compiled with cmake. To compile, first create a build/ directory. and compile from there:

    mkdir build
    cd build
    cmake ..
    make
    make install

To clean the directory and create a new build directory, you can use the bash script cleandir.sh:

    ./cleandir.sh

### Usage

In order to use the Offline Analysis tool, it is mandatory to know the Scan number and the HV Step of the run you want to analyse:

* `Scan00XXXX_HVY`

where *XXXX* is the scan ID and *Y* is the high voltage step (in case of a high voltage scan, data will be taken for several HV steps).
Usually, the offline analysis tool is automatically called by the WebDCS of GIF++ but, nontheless, to locally start the analysis for tests, simply type :

    bin/offlineanalysis /path/to/Scan00XXXX_HVY

and it will take care by itself of finding the data ROOT files:

* `Scan00XXXX_HVY_DAQ.root` containing the TDC data (events, hit and time lists)
* `Scan00XXXX_HVY_CAEN.root` containing the CAEN mainframe data (HVs and currents basically)

The analysed output ROOT datafiles are saved into the data file folder and called `Scan00XXXX_HVY_Offline.root`.

Inside those, you will find a more or less long, depending on the number of chambers that were used inside the setup during the data taking, list of *TH1*.
For each partition of each chamber, you will find :

* `Time_Profile_Tt_Sc_p` shows the time profile of all recorded events,
* `Hit_Profile_Tt_Sc_p` shows the hit profile of all recorded events,
* `Hit_Multiplicity_Tt_Sc_p` shows the hit multiplicity of all recorded events (number of hits per event),
* `Strip_Mean_Noise_Tt_Sc_p` shows noise/gamma rate for each strip in a selected time range,
* `Strip_Activity_Tt_Sc_p` shows noise/gamma activity for each strip (normalised version of previous histogram - strip activity = strip rate / average partition rate),
* `Strip_Homogeneity_Tt_Sc_p` shows the homogeneity of a given partition (homogeneity = exp(-StdDev(strip rates in partition)/(average partition rate))
* `mask_Strip_Mean_Noise_Tt_Sc_p` shows noise/gamma rate for each masked strip in a selected time range,
* `mask_Strip_Activity_Tt_Sc_p` shows noise/gamma activity for each masked strip with repect to the average rate of active strips,
* `NoiseCSize_H_Tt_Sc_p` shows noise/gamma cluster size,
* `NoiseCMult_H_Tt_Sc_p` shows noise/gamma cluster multiplicity (number of reconstructed clusters per event),
* `Chip_Mean_Noise_Tt_Sc_p` shows the same information than Strip_Mean_Noise_Tt_Scp using a different binning (1 chip = 8 strips),
* `Chip_Activity_Tt_Sc_p` shows the same information than Strip_Activity_Tt_Scp using a different binning,
* `Chip_Homogeneity_Tt_Sc_p` shows the homogeneity of a given partition using chip binning,
* `Beam_Profile_Tt_Sc_p` shows the estimated beam profile when taking efficiency scan (constructed with the hits contained in the muon peak where the noise/gamma background has been subtracted),
* `Efficiency_Fake_Tt_Ss_p` shows the efficiency given by fake hits by probing outside the peak in an uncorrelated window as wide as the peak window,
* `Efficiency_Peak_Tt_Ss_p` shows the efficiency given by hits contained in the peak window,
* `PeakCSize_H_Tt_Sc_p` shows the cluster size that was estimated using the hits in the peak window,
* `PeakCMult_H_Tt_Sc_p`shows the cluster multiplicity that was estimated using the hits in the peak window,
* `L0_Efficiency_Tt_Sc_p` shows the level 0 muon efficiency that was estimated WITHOUT muon tracking after correction,
* `MuonCSize_H_Tt_Sc_p` shows the level 0 muon cluster size that was estimated WITHOUT muon tracking after correction, and
* `MuonCMult_H_Tt_Sc_p`shows the level 0 muon cluster multiplicity that was estimated WITHOUT muon tracking after correction.

**Note :** in the histogram labels, **t** stands for the trolley number (1 or 3), **c** for the chamber slot label in trolley **t** and **p** for the partition label (A, B, C or D depending on the chamber layout).

Moreover, up to 4 CSV files can created depending on which ones of the 3 input files were in the data folder :

* `Offline-Rate.csv` : contains the summary of the noise/gamma rates and clusters
* `Offline-Corrupted.csv` : contains, for old data format files, the summary of the amount of corrupted data contained in the RAW data file after performing a fit on the multiplicity profile to discard the artificial content of bin 0
* `Offline-Current.csv` : contains the summary of the currents and voltages applied on the RPCs
* `Offline-L0-EffCl.csv` : contains the summary of the level 0 efficiency and muon cluster information without tracking

Note that these 4 CSV files are created along there "headers" (file containing the names of the data columns) and are automatically merged together when the offline analysis is used via the *RunDQM* button of the WebDCS.
Thus, the resulting files are :

* `Rates.csv`
* `Corrupted.csv`
* `Currents.csv`
* `L0-EffCl.csv`
