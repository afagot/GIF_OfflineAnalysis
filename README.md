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

###Usage

The ClusterSize code is currently only available in the clustersize branch.

To run the ClusterSize tool: 
```
cd clustersize
make
./ClusterSize inputFile.root
```

##Noise Rate

####Author: Alexis Fagot
####email : alexis.fagot@ugent.be
####Tel.: +32 9 264 65 69
####Mobile: +32 4 77 91 61 31
####Mobile: +33 6 66 89 02 90

###Usage

In order to use the NoiseRate tool, it is *mandatory* to know the chamber type (RE4-3 or RE4-2) used during the data tiaking. Indeed, the size of the strips and thus their surface varies depending on the type of CMS endcap RPC.
Then, once the chamber type is known, simply type :
```
   bin/offlineanalysis "/path/to/the/data/file/to/analyse.root" "chamberType"
```
Be careful, `"chamberType'` needs to be replaced by either `"RE4-2"` or `"RE4-3"`.
