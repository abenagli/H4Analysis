#ifndef __DLED_Analyzer__
#define __DLED_Analyzer__

#include <iostream>
#include <math.h>

#include "interface/utils.h"
#include "interface/PluginBase.h"
#include "interface/DigiTree.h"
#include "interface/WFTree.h"
#include "interface/WFClass.h"

class DLEDAnalyzer: public PluginBase
{
public:
    //---ctors---
    DLEDAnalyzer();

    //---dtor---
    ~DLEDAnalyzer(){};

    //---utils---
    bool Begin(map<string, PluginBase*>& plugins, CfgManager& opts, uint64* index);
    bool ProcessEvent(H4Tree& event, map<string, PluginBase*>& plugins, CfgManager& opts);
    
private:    
    //---internal data
    uint64*                   index_;
    int                       delay_;
    string                    srcInstance_;
    vector<string>            channelsNames_;
    vector<string>            templatesNames_;
    map<string, int>          channelsMap_;
    map<string, WFClass*>     WFs_;
};

DEFINE_PLUGIN(DLEDAnalyzer);

#endif
