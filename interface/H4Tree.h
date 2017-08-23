#ifndef __H4_TREE__
#define __H4_TREE__

#include <iostream>
#include <map>
#include <string>

#include "DynamicTTree/interface/DynamicTTreeBase.h"

using namespace std;

#define MAX_TDC_CHANNELS 200

typedef unsigned long int uint32;
typedef unsigned long long int uint64;
 
//****************************************************************************************
//----------Tree reader class-------------------------------------------------------------
#undef DYNAMIC_TREE_NAME
#undef DATA_TABLE
#undef DATA_VECT_TABLE

#define DYNAMIC_TREE_NAME H4TreeBase

#define DATA_TABLE                              \
    DATA(unsigned int,  evtTimeStart)           \
    DATA(unsigned int,  runNumber)              \
    DATA(unsigned int,  spillNumber)            \
    DATA(unsigned int,  evtNumber)              \
    DATA(unsigned int,  nEvtTimes)              \
    DATA(unsigned int,  nAdcChannels)           \
    DATA(unsigned int,  nTdcChannels)           \
    DATA(unsigned int,  nPatterns)              \
    DATA(unsigned int,  nDigiSamples)               

#define DATA_VECT_TABLE                                 \
    DATA(unsigned int, evtTimeBoard, nEvtTimes)         \
    DATA(uint64,       evtTime, nEvtTimes)              \
    DATA(unsigned int, adcBoard, nAdcChannels)          \
    DATA(unsigned int, adcChannel, nAdcChannels)        \
    DATA(unsigned int, adcData, nAdcChannels)           \
    DATA(unsigned int, tdcChannel, MAX_TDC_CHANNELS)    \
    DATA(unsigned int, tdcData, MAX_TDC_CHANNELS)       \
    DATA(unsigned int, pattern, nPatterns)              \
    DATA(unsigned int, patternBoard, nPatterns)         \
    DATA(unsigned int, patternChannel, nPatterns)       \
    DATA(unsigned int, digiGroup, nDigiSamples)         \
    DATA(unsigned int, digiChannel, nDigiSamples)       \
    DATA(float,        digiSampleValue, nDigiSamples)   

#include "DynamicTTree/interface/DynamicTTreeInterface.h"

class H4Tree : public H4TreeBase
{
public:
    //---ctors
    H4Tree(TChain* t):
        H4TreeBase(t)
        {
            Init();
        }
    H4Tree(TTree* t):
        H4TreeBase(t)
        {
            Init();
        }    
    //---dtor
    ~H4Tree();
    
    //---members
    void Init();
    uint64 GetEntries(){ return tree_->GetEntriesFast(); };
    
    map<pair<int, int>, int> digiMap;
};
   
#endif 
