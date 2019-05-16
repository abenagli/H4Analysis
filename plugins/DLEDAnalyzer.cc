#include "DLEDAnalyzer.h"

#include "TLinearFitter.h"
#include <algorithm>

//----------Constructor-------------------------------------------------------------------
DLEDAnalyzer::DLEDAnalyzer()
{}

//----------Utils-------------------------------------------------------------------------
bool DLEDAnalyzer::Begin(map<string, PluginBase*>& plugins, CfgManager& opts, uint64* index)
{

    //---register shared DLEDs
    //   nSamples is divided by two if DLED is from time to frequency domain
    srcInstance_ = opts.GetOpt<string>(instanceName_+".srcInstanceName");    
    channelsNames_ = opts.GetOpt<vector<string> >(instanceName_+".channelsNames");    
    delay_ = opts.GetOpt<int>(instanceName_+".delay");
    
    for(auto& channel : channelsNames_)
    {
      if(!opts.OptExist(channel+".tUnit"))
      {
          cout << ">>> DLEDAnalyzer ERROR: configuration for channel < " << channel << " > not found." << endl;
          return false;
      }
      
      WFs_[channel] = new WFClass(1, opts.GetOpt<float>(channel+".tUnit"));
      RegisterSharedData(WFs_[channel], channel, false);
    }
    
    return true;
}

bool DLEDAnalyzer::ProcessEvent(H4Tree& event, map<string, PluginBase*>& plugins, CfgManager& opts)
{
    for(auto& channel : channelsNames_)
    {
      auto wf = (WFClass*)plugins[srcInstance_]->GetSharedData(srcInstance_+"_"+channel, "", false).at(0).obj;
      auto samples = wf->GetSamples();
      auto times = wf->GetTimes();
      auto tUnit = wf->GetTUnit();
      
      //---get DLED from source instance data and reset old WF
      WFs_[channel]->Reset();

      
      for(int jj = 0; jj < delay_+1; ++jj)
        WFs_[channel]->AddSample(0.);
      
      int currentSample = 0;
      for(unsigned int jSample = delay_+1; jSample < samples->size(); ++jSample)
      {
        float time1 = jSample * tUnit;
        int sampleBef1 = -1;
        int sampleAft1 = -1;
        
        float time2 = (jSample-delay_) * tUnit;
        int sampleBef2 = -1;
        int sampleAft2 = -1;
        
        for(unsigned int it = currentSample; it < samples->size(); ++it)
        {
          if( times->at(it) > time2 )
          {
            sampleBef2 = it-1;
            sampleAft2 = it;
            currentSample = it;
            break;
          }
        }
        for(unsigned int it = currentSample; it < samples->size(); ++it)
        {
          if( times->at(it) > time1 )
          {
            sampleBef1 = it-1;
            sampleAft1 = it;
            break;
          }
        }
        
        float val1 = (*samples)[sampleBef1] + ((*samples)[sampleAft1]-(*samples)[sampleBef1])/((*times)[sampleAft1]-(*times)[sampleBef1]) * (time1-(*times)[sampleBef1]);
        float val2 = (*samples)[sampleBef2] + ((*samples)[sampleAft2]-(*samples)[sampleBef2])/((*times)[sampleAft2]-(*times)[sampleBef2]) * (time2-(*times)[sampleBef2]);
        WFs_[channel]->AddSample(val1-val2);
      }
    }
    
    return true;
}
