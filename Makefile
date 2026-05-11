DIR := ${CURDIR}

HDR = ./interface/
SRC = ./src/
PLG = ./plugins/
PRG = ./main/
OBJ = ./lib/
LIB = ./lib/
BIN = ./bin/

HDRSuf = .h
SRCSuf = .cc
PRGSuf = .cpp
OBJSuf = .o
LIBSuf = .so
BINSuf = .exe

HDRS     = $(wildcard $(HDR)*$(HDRSuf))
SRCS     = $(wildcard $(SRC)*$(SRCSuf))
PLGS     = $(wildcard $(PLG)*$(SRCSuf))

_OBJS    = $(patsubst %$(SRCSuf),%$(OBJSuf),$(SRCS))
OBJS     = $(patsubst $(SRC)%,$(OBJ)%,$(_OBJS))

_LIBS    = $(patsubst %$(SRCSuf),%$(LIBSuf),$(PLGS))
LIBS     = $(patsubst $(PLG)%,$(LIB)lib%,$(_LIBS))

PRGS     = $(wildcard $(PRG)*$(PRGSuf))
_BINS    = $(wildcard $(PRG)*$(PRGSuf))
__BINS   = $(_BINS:$(PRGSuf)=$(BINSuf))
___BINS  = $(notdir $(__BINS))
BINS     = $(addprefix $(BIN),$(___BINS))

LINKDEF  = $(wildcard $(HDR)*LinkDef.h)
DICTHDRS = $(patsubst $(LINKDEF),,$(HDRS)) $(LINKDEF)

ARCH = $(shell root-config --arch)

ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS   = $(shell root-config --libs) -lMinuit -lTreePlayer -lMathMore -lGenVector

CXX = $(shell root-config --cxx)
CPP = $(CXX)
LD  = $(CXX)

COMMONFLAGS = -Wall -Wno-sign-compare -Wno-overloaded-virtual -O2 -fPIC \
              -I$(DIR) \
              -I$(DIR)/interface \
              $(ROOTCFLAGS)

CXXFLAGS = $(COMMONFLAGS)
CPPFLAGS = $(COMMONFLAGS)

ifeq ($(parallel),1)
CXXFLAGS += -DPARALLEL -fopenmp
CPPFLAGS += -DPARALLEL -fopenmp
endif

LIBSuf = .so
SONAME = libH4Analysis.so

LDFLAGS = -shared -O2
SOFLAGS = -Wl,-soname,
EXTRAFS = -lstdc++fs

RPATHFLAGS = -Wl,-rpath,$(DIR)/lib \
             -Wl,-rpath,$(DIR)/DynamicTTree/lib \
             -Wl,-rpath,$(DIR)/CfgManager/lib

ifneq (,$(findstring macosx,$(ARCH)))
LIBSuf = .dylib
SONAME = libH4Analysis.dylib

LDFLAGS = -dynamiclib -single_module -undefined dynamic_lookup
SOFLAGS = -Wl,-install_name,
EXTRAFS =
endif

GLIBS = -lm \
        -L./DynamicTTree/lib \
        -L./CfgManager/lib \
        $(RPATHFLAGS) \
        -lDTT -lCFGMan \
        $(ROOTLIBS)

.PHONY: all clean test dynTTree cfgMan

all: dynTTree cfgMan $(LIB)$(SONAME) $(LIBS) $(BINS)

test:
	@echo "ARCH = $(ARCH)"
	@echo "CXX = $(CXX)"
	@echo "SONAME = $(SONAME)"
	@echo "LIBSuf = $(LIBSuf)"
	@echo "SRCS = $(SRCS)"
	@echo "PLGS = $(PLGS)"
	@echo "OBJS = $(OBJS)"
	@echo "LIBS = $(LIBS)"
	@echo "BINS = $(BINS)"

$(BIN)%$(BINSuf): $(PRG)%$(PRGSuf) $(HDRS) $(LIB)$(SONAME) Makefile
	@echo "Building main executable $@"
	@echo " CXX $<"
	@$(CPP) $(CPPFLAGS) -o $@ $< -L$(LIB) -lH4Analysis -lWFAnalyzer $(GLIBS) $(EXTRAFS)

$(OBJ)%$(OBJSuf): $(SRC)%$(SRCSuf) Makefile
	@echo " CXX $<"
	@$(CXX) -c $(CXXFLAGS) -o $@ $<

$(LIB)libH4Analysis.cc: $(DICTHDRS) src/classes_def.xml
	@echo "Generating dictionary..."
	@genreflex $(DICTHDRS) -o $@ -s src/classes_def.xml -I$(DIR)/interface/ -I$(DIR)

$(LIB)libH4Analysis.o: $(LIB)libH4Analysis.cc Makefile
	@echo " CXX $<"
	@$(CXX) -c $(CXXFLAGS) -o $@ $<

$(LIB)$(SONAME): $(OBJS) $(LIB)libH4Analysis.o
	@echo "Linking $(SONAME):"
	@$(LD) $(LDFLAGS) -o $(LIB)$(SONAME) $(OBJS) $(LIB)libH4Analysis.o $(SOFLAGS)$(SONAME) $(GLIBS)

$(LIB)lib%$(LIBSuf): $(PLG)%$(SRCSuf) $(PLG)%$(HDRSuf) $(OBJS) $(LIB)$(SONAME)
	@echo "Creating plugin library $@"
	@echo " CXX $<"
	@$(LD) $(CXXFLAGS) $(LDFLAGS) -o $@ $< -L$(LIB) -lH4Analysis $(GLIBS)

dynTTree:
	cd DynamicTTree && $(MAKE)

cfgMan:
	cd CfgManager && $(MAKE)

clean:
	@echo "cleaning..."
	rm -f $(OBJ)*$(OBJSuf) $(LIB)*$(LIBSuf) $(LIB)libH4Analysis* $(BIN)*$(BINSuf)
	cd DynamicTTree && $(MAKE) clean
	cd CfgManager && $(MAKE) clean
