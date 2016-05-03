#file: Makefile


DAQ_HOME_DIR = /var/www/software/GIF_OfflineAnalysis
#DAQ_HOME_DIR = /home/alex/Desktop/RPCs/GIF_OfflineAnalysis
DAQ_BIN_DIR = $(DAQ_HOME_DIR)/bin
DAQ_INC_DIR = $(DAQ_HOME_DIR)/include
DAQ_SRC_DIR = $(DAQ_HOME_DIR)/src
DAQ_OBJ_DIR = $(DAQ_HOME_DIR)/obj

ROOT_INC 	= $(ROOTSYS)/include
ROOTCFLAGS   	:= $(shell root-config --cflags)
ROOTLIBS     	:= $(shell root-config --libs)

LFLAGS     = -L$(DAQ_HOME_DIR)/lib -L/usr/lib \
             -Wl,--no-as-needed $(ROOTLIBS) 

CFLAGS     = -ggdb -fPIC -DLINUX -Wall -funsigned-char \
             -I$(DAQ_INC_DIR) -I$(ROOT_INC) -I$(ROOTCFLAGS)

all:    offlineanalysis

offlineanalysis: 	main.o NoiseRate.o utils.o IniFile.o
			g++ $(CFLAGS) $(DAQ_OBJ_DIR)/main.o \
			$(DAQ_OBJ_DIR)/utils.o \
			$(DAQ_OBJ_DIR)/NoiseRate.o \
			$(DAQ_OBJ_DIR)/IniFile.o \
        		-o $(DAQ_BIN_DIR)/offlineanalysis \
        		$(LFLAGS)  \
        		 -l curses

clean:
	-rm $(DAQ_BIN_DIR)/offlineanalysis
	-rm $(DAQ_OBJ_DIR)/*.o

remove:
	-rm $(DAQ_BIN_DIR)/offlineanalysis
	-rm $(DAQ_OBJ_DIR)/*.o

main.o:
	g++ -std=c++11 -c $(CFLAGS) $(DAQ_SRC_DIR)/main.cc -o $(DAQ_OBJ_DIR)/main.o
utils.o:
	g++ -std=c++11 -c $(CFLAGS) $(DAQ_SRC_DIR)/utils.cc -o $(DAQ_OBJ_DIR)/utils.o
NoiseRate.o:
	g++ -std=c++11 -c $(CFLAGS) $(DAQ_SRC_DIR)/NoiseRate.cc -o $(DAQ_OBJ_DIR)/NoiseRate.o
IniFile.o:
	g++ -std=c++11 -c $(CFLAGS) $(DAQ_SRC_DIR)/IniFile.cc -o $(DAQ_OBJ_DIR)/IniFile.o
