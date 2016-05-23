#file: Makefile
#
# test program for CMS RPC
#
# 14/01/14  A.Fagot
# from makefile created by Y.Benhammou


DAQ_BIN_DIR  = ./bin
DAQ_INC_DIR  = ./include
DAQ_SRC_DIR  = ./src
DAQ_OBJ_DIR  = ./obj
RUN_REGISTRY = ./RunRegistry

CC = g++ -std=c++11

ROOT_INC        := $(ROOTSYS)/include
ROOTCFLAGS	:= $(shell root-config --cflags)
ROOTLIBS        := $(shell root-config --libs)

LFLAGS          := -Llib -L/usr/lib \
                $(ROOTLIBS)

CFLAGS          := -ggdb -fPIC -DLINUX -Wall -funsigned-char \
                -I$(DAQ_INC_DIR) -I$(ROOT_INC) -I$(ROOTCFLAGS)

all:    offlineanalysis

offlineanalysis: 	main.o NoiseRate.o utils.o IniFile.o
			g++ $(CFLAGS) $(DAQ_OBJ_DIR)/main.o \
			$(DAQ_OBJ_DIR)/utils.o \
			$(DAQ_OBJ_DIR)/NoiseRate.o \
			$(DAQ_OBJ_DIR)/IniFile.o \
        		-o $(DAQ_BIN_DIR)/offlineanalysis \
        		$(LFLAGS)  \
        		-l CAENVME -l curses

main.o:
	$(CC) $(CFLAGS) -c $(DAQ_SRC_DIR)/main.cc -o $(DAQ_OBJ_DIR)/main.o
utils.o:
	$(CC) $(CFLAGS) -c $(DAQ_SRC_DIR)/utils.cc -o $(DAQ_OBJ_DIR)/utils.o
NoiseRate.o:
	$(CC) $(CFLAGS) -c $(DAQ_SRC_DIR)/NoiseRate.cc -o $(DAQ_OBJ_DIR)/NoiseRate.o
IniFile.o:
	$(CC) $(CFLAGS) -c $(DAQ_SRC_DIR)/IniFile.cc -o $(DAQ_OBJ_DIR)/IniFile.o

clean:
	rm -rf $(DAQ_BIN_DIR)/offlineanalysis
	rm -rf $(DAQ_OBJ_DIR)/*.o

remove:
	rm -rf $(DAQ_BIN_DIR)/offlineanalysis
	rm -rf $(DAQ_OBJ_DIR)/*.o

