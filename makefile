#file: Makefile

DAQ_BIN_DIR = ./bin
DAQ_INC_DIR = ./include
DAQ_SRC_DIR = ./src
DAQ_OBJ_DIR = ./obj

CC = g++ -std=c++11

ROOT_INC = $(ROOTSYS)/include
ROOTCFLAGS := $(shell root-config --cflags)
ROOTLIBS := $(shell root-config --libs)

LFLAGS = -L$(DAQ_HOME_DIR)/lib -L/usr/lib \
		-Wl,--no-as-needed $(ROOTLIBS) 

CFLAGS = -ggdb -fPIC -DLINUX -Wall -funsigned-char \
		-I$(DAQ_INC_DIR) -I$(ROOT_INC) -I$(ROOTCFLAGS)

all: $(DAQ_BIN_DIR) $(DAQ_OBJ_DIR) offlineanalysis

offlineanalysis: main.o OfflineAnalysis.o Current.o IniFile.o MsgSvc.o Infrastructure.o GIFTrolley.o RPCDetector.o Cluster.o Mapping.o utils.o
		g++ $(CFLAGS) $(DAQ_OBJ_DIR)/main.o \
		$(DAQ_OBJ_DIR)/OfflineAnalysis.o \
		$(DAQ_OBJ_DIR)/Current.o \
		$(DAQ_OBJ_DIR)/IniFile.o \
		$(DAQ_OBJ_DIR)/MsgSvc.o \
		$(DAQ_OBJ_DIR)/Infrastructure.o \
		$(DAQ_OBJ_DIR)/GIFTrolley.o \
		$(DAQ_OBJ_DIR)/RPCDetector.o \
		$(DAQ_OBJ_DIR)/Cluster.o \
		$(DAQ_OBJ_DIR)/Mapping.o \
		$(DAQ_OBJ_DIR)/utils.o \
		-o $(DAQ_BIN_DIR)/offlineanalysis \
		$(LFLAGS) \
		-l curses

main.o:
	$(CC) $(CFLAGS) -c $(DAQ_SRC_DIR)/main.cc -o $(DAQ_OBJ_DIR)/main.o
OfflineAnalysis.o:
	$(CC) $(CFLAGS) -c $(DAQ_SRC_DIR)/OfflineAnalysis.cc -o $(DAQ_OBJ_DIR)/OfflineAnalysis.o
Current.o:
	$(CC) $(CFLAGS) -c $(DAQ_SRC_DIR)/Current.cc -o $(DAQ_OBJ_DIR)/Current.o
IniFile.o:
	$(CC) $(CFLAGS) -c $(DAQ_SRC_DIR)/IniFile.cc -o $(DAQ_OBJ_DIR)/IniFile.o
MsgSvc.o:
	$(CC) $(CFLAGS) -c $(DAQ_SRC_DIR)/MsgSvc.cc -o $(DAQ_OBJ_DIR)/MsgSvc.o
Infrastructure.o:
	$(CC) $(CFLAGS) -c $(DAQ_SRC_DIR)/Infrastructure.cc -o $(DAQ_OBJ_DIR)/Infrastructure.o
GIFTrolley.o:
	$(CC) $(CFLAGS) -c $(DAQ_SRC_DIR)/GIFTrolley.cc -o $(DAQ_OBJ_DIR)/GIFTrolley.o
RPCDetector.o:
	$(CC) $(CFLAGS) -c $(DAQ_SRC_DIR)/RPCDetector.cc -o $(DAQ_OBJ_DIR)/RPCDetector.o
Cluster.o:
	$(CC) $(CFLAGS) -c $(DAQ_SRC_DIR)/Cluster.cc -o $(DAQ_OBJ_DIR)/Cluster.o
Mapping.o:
	$(CC) $(CFLAGS) -c $(DAQ_SRC_DIR)/Mapping.cc -o $(DAQ_OBJ_DIR)/Mapping.o
utils.o:
	$(CC) $(CFLAGS) -c $(DAQ_SRC_DIR)/utils.cc -o $(DAQ_OBJ_DIR)/utils.o

$(DAQ_BIN_DIR):
	mkdir -p $(DAQ_BIN_DIR)/

$(DAQ_OBJ_DIR):
	mkdir -p $(DAQ_OBJ_DIR)/
	
clean:
	rm -rf $(DAQ_BIN_DIR)/
	rm -rf $(DAQ_OBJ_DIR)/

remove:
	rm -rf $(DAQ_BIN_DIR)/
	rm -rf $(DAQ_OBJ_DIR)/
	
