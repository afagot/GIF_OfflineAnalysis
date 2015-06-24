ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --libs) -lCintex -lMinuit -lTreePlayer

CXX = g++
CXXFLAGS = -I$(ROOTSYS)/include
CXXFLAGS += $(ROOTCFLAGS)
CXXFLAGS += -Wno-sign-compare -Wno-unused-variable

LDFLAGS = $(ROOTLIBS)

ROOTCINT = rootcint

DEPS = ClusterSize.h
OBJS = ClusterSize.o

%.o: %.cxx $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

ClusterSize: $(OBJS) ClusterSize.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f *.o ClusterSize
