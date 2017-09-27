 #include "interface/CTRUtils.h"
 #include "interface/FitUtils.h"
 #include "interface/SetTDRStyle.h"

 #include "CfgManager/interface/CfgManager.h"
 #include "CfgManager/interface/CfgManagerT.h"

 #include <iostream>
 #include <vector>
 #include <map>
 #include <algorithm>
 #include <cstdlib>

 #include "TSystem.h"
 #include "TFile.h"
 #include "TTree.h"
 #include "TChain.h"
 #include "TGraph.h"
 #include "TGraphErrors.h"
 #include "TH1F.h"
 #include "TH2F.h"
 #include "TProfile.h"
 #include "TProfile2D.h"
 #include "TF1.h"
 #include "TCanvas.h"
 #include "TLatex.h"
 #include "TLine.h"
 #include "TBox.h"
 #include "TEllipse.h"
 #include "TApplication.h"

 bool popupPlots = false;



 int main(int argc, char** argv)
 {
   if( argc < 2 )
   {
     std::cerr << ">>>>> drawCTRChess.cpp::usage:   " << argv[0] << " configFileName" << std::endl;
     return 1;
   }



   //----------------------
   // parse the config file
   CfgManager opts;
   opts.ParseConfigFile(argv[1]);

   std::vector<std::string> inputFiles = opts.GetOpt<std::vector<std::string> >("Input.inputFiles");

   std::vector<std::string> timeChannels = opts.GetOpt<std::vector<std::string> >("Input.timeChannels");
   std::string timeCh0 = timeChannels.at(0);
   std::string timeCh1 = timeChannels.at(1);
   std::vector<std::string> energyChannels = opts.GetOpt<std::vector<std::string> >("Input.energyChannels");
   std::string enCh0 = energyChannels.at(0);
   std::string enCh1 = energyChannels.at(1);
   std::vector<float> isMCP = opts.GetOpt<std::vector<float> >("Input.isMCP");
   bool isMCP0 = bool(isMCP.at(0));
   bool isMCP1 = bool(isMCP.at(1));
   std::string xLabel1_en("SiPM_{1}"); if( isMCP0 ) xLabel1_en = "MCP";
   std::string xLabel2_en("SiPM_{2}"); if( isMCP1 ) xLabel2_en = "MCP";
   std::string xLabel1_time("NINO_{1}"); if( isMCP0 ) xLabel1_time = "MCP";
   std::string xLabel2_time("NINO_{2}"); if( isMCP1 ) xLabel2_time = "MCP";

   int VbiasIndex1 = opts.GetOpt<int>("Input.VbiasIndex1");
   int VbiasIndex2 = opts.GetOpt<int>("Input.VbiasIndex2");
   int configuration = opts.GetOpt<int>("Input.configuration");
   std::string extra = opts.GetOpt<std::string>("Input.extra");
   float MCPIntrinsic = opts.GetOpt<float>("Input.MCPIntrinsic");

   float cut_NINOthr = opts.GetOpt<float>("Cuts.NINOthr");
   float cut_Vbias = opts.GetOpt<float>("Cuts.Vbias");  

   float cut_ampMin1 = opts.GetOpt<float>("Cuts.minAmplitude1");
   float cut_ampMax1 = opts.GetOpt<float>("Cuts.maxAmplitude1");
   float cut_ampMin2 = opts.GetOpt<float>("Cuts.minAmplitude2");
   float cut_ampMax2 = opts.GetOpt<float>("Cuts.maxAmplitude2");

   float cut_durMin1 = opts.GetOpt<float>("Cuts.minDuration1");
   float cut_durMax1 = opts.GetOpt<float>("Cuts.maxDuration1");
   float cut_durMin2 = opts.GetOpt<float>("Cuts.minDuration2");
   float cut_durMax2 = opts.GetOpt<float>("Cuts.maxDuration2");

   float cut_rangeXMin = opts.GetOpt<float>("Cuts.rangeXMin");
   float cut_rangeXMax = opts.GetOpt<float>("Cuts.rangeXMax");
   float cut_rangeYMin = opts.GetOpt<float>("Cuts.rangeYMin");
   float cut_rangeYMax = opts.GetOpt<float>("Cuts.rangeYMax");

   float cut_crystalXMin = opts.GetOpt<float>("Cuts.crystalXMin");
   float cut_crystalXMax = opts.GetOpt<float>("Cuts.crystalXMax");
   float cut_crystalYMin = opts.GetOpt<float>("Cuts.crystalYMin");
   float cut_crystalYMax = opts.GetOpt<float>("Cuts.crystalYMax");

   float cut_MCPX = opts.GetOpt<float>("Cuts.MCPX");
   float cut_MCPY = opts.GetOpt<float>("Cuts.MCPY");
   float cut_MCPR = opts.GetOpt<float>("Cuts.MCPR");

   int nBinsX = opts.GetOpt<int>("Cuts.nBinsX");
   int nBinsY = opts.GetOpt<int>("Cuts.nBinsY");

   int rebin = opts.GetOpt<int>("Plots.rebin");
   std::string label1 = opts.GetOpt<std::string>("Plots.label1");
   std::string label2 = opts.GetOpt<std::string>("Plots.label2");
   std::string label12;
   if( label1 == label2 ) label12 = label1;
   else label12 = std::string(Form("%s-%s",label1.c_str(),label2.c_str()));


   //------------------------
   // labels and canvas style
   setTDRStyle();
   gStyle->SetPaintTextFormat("4.1f");
   gStyle->SetPadLeftMargin(0.10);
   gStyle->SetPadRightMargin(0.15);

   TApplication* theApp;
   if( popupPlots )
     theApp = new TApplication("App", &argc, argv);

   TLatex* latexLabel1 = new TLatex(0.13,0.97,Form("%s",label1.c_str()));
   TLatex* latexLabel2 = new TLatex(0.13,0.97,Form("%s",label2.c_str()));
   TLatex* latexLabel12 = new TLatex(0.13,0.97,Form("%s",label12.c_str()));
   latexLabel1 -> SetNDC();
   latexLabel1 -> SetTextFont(42);
   latexLabel1 -> SetTextSize(0.03);
   latexLabel2 -> SetNDC();
   latexLabel2 -> SetTextFont(42);
   latexLabel2 -> SetTextSize(0.03);
   latexLabel12 -> SetNDC();
   latexLabel12 -> SetTextFont(42);
   latexLabel12 -> SetTextSize(0.03);

   TLatex* latexLabel_all = new TLatex(0.40,0.90,Form("all events"));
   latexLabel_all -> SetNDC();
   latexLabel_all -> SetTextFont(42);
   latexLabel_all -> SetTextColor(kBlack);
   latexLabel_all -> SetTextSize(0.04);
   TLatex* latexLabel_center = new TLatex(0.40,0.85,Form("center selection"));
   latexLabel_center -> SetNDC();
   latexLabel_center -> SetTextFont(42);
   latexLabel_center -> SetTextColor(kRed);
   latexLabel_center -> SetTextSize(0.04);
   TLatex* latexLabel_border = new TLatex(0.40,0.80,Form("border selection"));
   latexLabel_border -> SetNDC();
   latexLabel_border -> SetTextFont(42);
   latexLabel_border -> SetTextColor(kBlue);
   latexLabel_border -> SetTextSize(0.04);

   TLatex* latexLabel_ysel = new TLatex(0.40,0.85,Form("Y selection"));
   latexLabel_ysel -> SetNDC();
   latexLabel_ysel -> SetTextFont(42);
   latexLabel_ysel -> SetTextColor(kRed);
   latexLabel_ysel -> SetTextSize(0.04);

   TLatex* latexLabel_xsel = new TLatex(0.40,0.85,Form("X selection"));
   latexLabel_xsel -> SetNDC();
   latexLabel_xsel -> SetTextFont(42);
   latexLabel_xsel -> SetTextColor(kBlue);
   latexLabel_xsel -> SetTextSize(0.04);

   std::string baseDir(Form("/afs/cern.ch/user/a/abenagli/www/TIMING/TBatH6August2017/chess_config%02d%s/",configuration,extra.c_str()));
   system(Form("mkdir -p %s",baseDir.c_str()));
   system(Form("cp /afs/cern.ch/user/a/abenagli/public/index.php %s",baseDir.c_str()));
   std::string plotDir(Form("%s/%s/",baseDir.c_str(),label12.c_str()));
   system(Form("mkdir %s",plotDir.c_str()));
   system(Form("cp /afs/cern.ch/user/a/abenagli/public/index.php %s",plotDir.c_str()));


   //---------------------------
   // open input files and trees
   TChain* chain0 = new TChain("h4","h4");
   // TChain* chain1 = new TChain("info","info");
   // TChain* chain2 = new TChain("wire","wire");
   // TChain* chain3 = new TChain("digi","digi");
   for(unsigned int fileIt = 0; fileIt < inputFiles.size(); ++fileIt)
   {
     chain0 -> Add(inputFiles.at(fileIt).c_str());
     // chain1 -> Add(inputFiles.at(fileIt).c_str());
     // chain2 -> Add(inputFiles.at(fileIt).c_str());
     // chain3 -> Add(inputFiles.at(fileIt).c_str());
   }
   // chain2 -> BuildIndex("index");
   // chain1 -> AddFriend("hodo");
   // chain3 -> BuildIndex("index");
   // chain1 -> AddFriend("digi");
   // chain1 -> BuildIndex("index");
   std::cout << " Read " << chain0->GetEntries() << " total events in tree " << chain0->GetName() << std::endl;
   // std::cout << " Read " << chain1->GetEntries() << " total events in tree " << chain1->GetName() << std::endl;
   // std::cout << " Read " << chain2->GetEntries() << " total events in tree " << chain2->GetName() << std::endl;
   // std::cout << " Read " << chain3->GetEntries() << " total events in tree " << chain3->GetName() << std::endl;

   TreeVars treeVars;
   InitTreeVars(chain0,treeVars,opts);


   //------------------
   // Define histograms
   TH2F* h2_beam_Y_vs_X = new TH2F("h2_beam_Y_vs_X","",2.*(cut_rangeXMax-cut_rangeXMin),cut_rangeXMin,cut_rangeXMax,2.*(cut_rangeYMax-cut_rangeYMin),cut_rangeYMin,cut_rangeYMax);
   TH2F* h2_beam_Y_vs_X_cut = new TH2F("h2_beam_Y_vs_X_cut","",2.*(cut_rangeXMax-cut_rangeXMin),cut_rangeXMin,cut_rangeXMax,2.*(cut_rangeYMax-cut_rangeYMin),cut_rangeYMin,cut_rangeYMax);

   TH1F* h_amp1 = new TH1F(Form("h_amp1"),"",250,0.,1000.);
   TH1F* h_amp2 = new TH1F(Form("h_amp2"),"",250,0.,1000.);
   TH1F* h_amp1_center = new TH1F(Form("h_amp1_center"),"",250,0.,1000.);
   TH1F* h_amp2_center = new TH1F(Form("h_amp2_center"),"",250,0.,1000.);
   TH1F* h_amp1_border = new TH1F(Form("h_amp1_border"),"",250,0.,1000.);
   TH1F* h_amp2_border = new TH1F(Form("h_amp2_border"),"",250,0.,1000.);
   TH1F* h_ampRatio = new TH1F(Form("h_ampRatio"),"",1000,0.,3.);
   TH1F* h_amp1_cut = new TH1F(Form("h_amp1_cut"),"",250,0.,1000.);
   TH1F* h_amp2_cut = new TH1F(Form("h_amp2_cut"),"",250,0.,1000.);
   TH1F* h_ampRatio_cut = new TH1F(Form("h_ampRatio_cut"),"",1000,0.,3.);

   TH1F* h_dur1 = new TH1F(Form("h_dur1"),"",200,0.,100.);
   TH1F* h_dur2 = new TH1F(Form("h_dur2"),"",200,0.,100.);
   TH1F* h_dur1_cut = new TH1F(Form("h_dur1_cut"),"",200,0.,100.);
   TH1F* h_dur2_cut = new TH1F(Form("h_dur2_cut"),"",200,0.,100.);

   TProfile* p1_amp1_vs_beam_X = new TProfile("p1_amp1_vs_beam_X","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_amp1_vs_beam_Y = new TProfile("p1_amp1_vs_beam_Y","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile* p1_amp2_vs_beam_X = new TProfile("p1_amp2_vs_beam_X","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_amp2_vs_beam_Y = new TProfile("p1_amp2_vs_beam_Y","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile2D* p2_amp1_vs_beam_Y_vs_X = new TProfile2D("p2_amp1_vs_beam_Y_vs_X","",2.*(cut_rangeXMax-cut_rangeXMin),cut_rangeXMin,cut_rangeXMax,2.*(cut_rangeYMax-cut_rangeYMin),cut_rangeYMin,cut_rangeYMax);
   TProfile2D* p2_amp2_vs_beam_Y_vs_X = new TProfile2D("p2_amp2_vs_beam_Y_vs_X","",2.*(cut_rangeXMax-cut_rangeXMin),cut_rangeXMin,cut_rangeXMax,2.*(cut_rangeYMax-cut_rangeYMin),cut_rangeYMin,cut_rangeYMax);
   TProfile* p1_amp1_vs_beam_X_cut = new TProfile("p1_amp1_vs_beam_X_cut","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_amp1_vs_beam_Y_cut = new TProfile("p1_amp1_vs_beam_Y_cut","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile* p1_amp2_vs_beam_X_cut = new TProfile("p1_amp2_vs_beam_X_cut","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_amp2_vs_beam_Y_cut = new TProfile("p1_amp2_vs_beam_Y_cut","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile2D* p2_amp1_vs_beam_Y_vs_X_cut = new TProfile2D("p2_amp1_vs_beam_Y_vs_X_cut","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax,2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile2D* p2_amp2_vs_beam_Y_vs_X_cut = new TProfile2D("p2_amp2_vs_beam_Y_vs_X_cut","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax,2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);

   TProfile* p1_CTR_vs_beam_X = new TProfile("p1_CTR_vs_beam_X","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_CTR_vs_beam_Y = new TProfile("p1_CTR_vs_beam_Y","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile* p1_CTR_vs_beam_X_cut = new TProfile("p1_CTR_vs_beam_X_cut","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_CTR_vs_beam_Y_cut = new TProfile("p1_CTR_vs_beam_Y_cut","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile2D* p2_CTR_vs_beam_Y_vs_X = new TProfile2D("p2_CTR_vs_beam_Y_vs_X","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax,2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);

   TProfile* p1_CTR_ampCorr_vs_beam_X = new TProfile("p1_CTR_ampCorr_vs_beam_X","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_CTR_ampCorr_vs_beam_Y = new TProfile("p1_CTR_ampCorr_vs_beam_Y","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile2D* p2_CTR_ampCorr_vs_beam_Y_vs_X = new TProfile2D("p2_CTR_ampCorr_vs_beam_Y_vs_X","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax,2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile* p1_CTR_ampCorr_vs_beam_X_cut = new TProfile("p1_CTR_ampCorr_vs_beam_X_cut","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_CTR_ampCorr_vs_beam_Y_cut = new TProfile("p1_CTR_ampCorr_vs_beam_Y_cut","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);

   TProfile* p1_CTR_ampCorrEff_vs_beam_X = new TProfile("p1_CTR_ampCorrEff_vs_beam_X","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_CTR_ampCorrEff_vs_beam_Y = new TProfile("p1_CTR_ampCorrEff_vs_beam_Y","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile2D* p2_CTR_ampCorrEff_vs_beam_Y_vs_X = new TProfile2D("p2_CTR_ampCorrEff_vs_beam_Y_vs_X","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax,2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile* p1_CTR_ampCorrEff_vs_beam_X_cut = new TProfile("p1_CTR_ampCorrEff_vs_beam_X_cut","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_CTR_ampCorrEff_vs_beam_Y_cut = new TProfile("p1_CTR_ampCorrEff_vs_beam_Y_cut","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);

   TProfile* p1_CTR_ampCorrPerBin_vs_beam_X = new TProfile("p1_CTR_ampCorrPerBin_vs_beam_X","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_CTR_ampCorrPerBin_vs_beam_Y = new TProfile("p1_CTR_ampCorrPerBin_vs_beam_Y","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile2D* p2_CTR_ampCorrPerBin_vs_beam_Y_vs_X = new TProfile2D("p2_CTR_ampCorrPerBin_vs_beam_Y_vs_X","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax,2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile* p1_CTR_ampCorrPerBin_vs_beam_X_cut = new TProfile("p1_CTR_ampCorrPerBin_vs_beam_X_cut","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_CTR_ampCorrPerBin_vs_beam_Y_cut = new TProfile("p1_CTR_ampCorrPerBin_vs_beam_Y_cut","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);

   TProfile* p1_CTR_posCorr_vs_beam_X = new TProfile("p1_CTR_posCorr_vs_beam_X","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_CTR_posCorr_vs_beam_Y = new TProfile("p1_CTR_posCorr_vs_beam_Y","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile2D* p2_CTR_posCorr_vs_beam_Y_vs_X = new TProfile2D("p2_CTR_posCorr_vs_beam_Y_vs_X","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax,2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile* p1_CTR_posCorr_vs_beam_X_cut = new TProfile("p1_CTR_posCorr_vs_beam_X_cut","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_CTR_posCorr_vs_beam_Y_cut = new TProfile("p1_CTR_posCorr_vs_beam_Y_cut","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);

   TProfile* p1_CTR_posCorrAmpCorr_vs_beam_X = new TProfile("p1_CTR_posCorrAmpCorr_vs_beam_X","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_CTR_posCorrAmpCorr_vs_beam_Y = new TProfile("p1_CTR_posCorrAmpCorr_vs_beam_Y","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile2D* p2_CTR_posCorrAmpCorr_vs_beam_Y_vs_X = new TProfile2D("p2_CTR_posCorrAmpCorr_vs_beam_Y_vs_X","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax,2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile* p1_CTR_posCorrAmpCorr_vs_beam_X_cut = new TProfile("p1_CTR_posCorrAmpCorr_vs_beam_X_cut","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_CTR_posCorrAmpCorr_vs_beam_Y_cut = new TProfile("p1_CTR_posCorrAmpCorr_vs_beam_Y_cut","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);

   TProfile* p1_CTR_ampCorrEffPosCorr_vs_beam_X = new TProfile("p1_CTR_ampCorrEffPosCorr_vs_beam_X","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_CTR_ampCorrEffPosCorr_vs_beam_Y = new TProfile("p1_CTR_ampCorrEffPosCorr_vs_beam_Y","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile2D* p2_CTR_ampCorrEffPosCorr_vs_beam_Y_vs_X = new TProfile2D("p2_CTR_ampCorrEffPosCorr_vs_beam_Y_vs_X","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax,2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
   TProfile* p1_CTR_ampCorrEffPosCorr_vs_beam_X_cut = new TProfile("p1_CTR_ampCorrEffPosCorr_vs_beam_X_cut","",2*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
   TProfile* p1_CTR_ampCorrEffPosCorr_vs_beam_Y_cut = new TProfile("p1_CTR_ampCorrEffPosCorr_vs_beam_Y_cut","",2*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);


   TH1F* h_CTR_raw = new TH1F("h_CTR_raw","",50000,-100.,100.);
   
   TProfile* p1_CTR_vs_amp1 = new TProfile("p1_CTR_vs_amp1","",100,0.,1000.);
   TProfile* p1_CTR_vs_amp1_center = new TProfile("p1_CTR_vs_amp1_center","",100,0.,1000.);
   TProfile* p1_CTR_vs_amp1_border = new TProfile("p1_CTR_vs_amp1_border","",100,0.,1000.);
   TProfile* p1_CTR_vs_amp2 = new TProfile("p1_CTR_vs_amp2","",100,0.,1000.);
   TProfile* p1_CTR_vs_amp2_center = new TProfile("p1_CTR_vs_amp2_center","",100,0.,1000.);
   TProfile* p1_CTR_vs_amp2_border = new TProfile("p1_CTR_vs_amp2_border","",100,0.,1000.);
   TProfile* p1_CTR_vs_ampRatio = new TProfile("p1_CTR_vs_ampRatio","",100,0.,5.);
   TProfile* p1_CTR_vs_ampRatio_center = new TProfile("p1_CTR_vs_ampRatio_center","",100,0.,5.);
   TProfile* p1_CTR_vs_ampRatio_border = new TProfile("p1_CTR_vs_ampRatio_border","",100,0.,5.);

   TProfile* p1_CTR_ampCorr_vs_amp1 = new TProfile("p1_CTR_ampCorr_vs_amp1","",100,0.,1000.);
   TProfile* p1_CTR_ampCorr_vs_amp2 = new TProfile("p1_CTR_ampCorr_vs_amp2","",100,0.,1000.);
   TProfile* p1_CTR_ampCorr_vs_ampRatio = new TProfile("p1_CTR_ampCorr_vs_ampRatio","",100,0.,5.);

   TProfile* p1_CTR_ampCorrEff_vs_amp1 = new TProfile("p1_CTR_ampCorrEff_vs_amp1","",100,0.,1000.);
   TProfile* p1_CTR_ampCorrEff_vs_amp2 = new TProfile("p1_CTR_ampCorrEff_vs_amp2","",100,0.,1000.);
   TProfile* p1_CTR_ampCorrEff_vs_ampRatio = new TProfile("p1_CTR_ampCorrEff_vs_ampRatio","",100,0.,5.);

   TProfile* p1_CTR_ampCorrPerBin_vs_amp1 = new TProfile("p1_CTR_ampCorrPerBin_vs_amp1","",100,0.,1000.);
   TProfile* p1_CTR_ampCorrPerBin_vs_amp2 = new TProfile("p1_CTR_ampCorrPerBin_vs_amp2","",100,0.,1000.);
   TProfile* p1_CTR_ampCorrPerBin_vs_ampRatio = new TProfile("p1_CTR_ampCorrPerBin_vs_ampRatio","",100,0.,5.);

   TProfile* p1_CTR_posCorr_vs_amp1 = new TProfile("p1_CTR_posCorr_vs_amp1","",100,0.,1000.);
   TProfile* p1_CTR_posCorr_vs_amp2 = new TProfile("p1_CTR_posCorr_vs_amp2","",100,0.,1000.);
   TProfile* p1_CTR_posCorr_vs_ampRatio = new TProfile("p1_CTR_posCorr_vs_ampRatio","",100,0.,5.);

   TProfile* p1_CTR_posCorrAmpCorr_vs_amp1 = new TProfile("p1_CTR_posCorrAmpCorr_vs_amp1","",100,0.,1000.);
   TProfile* p1_CTR_posCorrAmpCorr_vs_amp2 = new TProfile("p1_CTR_posCorrAmpCorr_vs_amp2","",100,0.,1000.);
   TProfile* p1_CTR_posCorrAmpCorr_vs_ampRatio = new TProfile("p1_CTR_posCorrAmpCorr_vs_ampRatio","",100,0.,5.);

   TProfile* p1_CTR_ampCorrEffPosCorr_vs_amp1 = new TProfile("p1_CTR_ampCorrEffPosCorr_vs_amp1","",100,0.,1000.);
   TProfile* p1_CTR_ampCorrEffPosCorr_vs_amp2 = new TProfile("p1_CTR_ampCorrEffPosCorr_vs_amp2","",100,0.,1000.);
   TProfile* p1_CTR_ampCorrEffPosCorr_vs_ampRatio = new TProfile("p1_CTR_ampCorrEffPosCorr_vs_ampRatio","",100,0.,5.);

   std::map<std::string,TH1F*> map_h_amp1;
   std::map<std::string,TH1F*> map_h_amp2;
   std::map<std::string,TH1F*> map_h_ampRatio;
   
   std::map<std::string,TProfile*> map_p1_CTR_vs_amp1;
   std::map<std::string,TProfile*> map_p1_CTR_vs_amp2;
   std::map<std::string,TProfile*> map_p1_CTR_vs_ampRatio;
   std::map<std::string,TProfile*> map_p1_CTR_ampCorr_vs_amp1;
   std::map<std::string,TProfile*> map_p1_CTR_ampCorr_vs_amp2;
   std::map<std::string,TProfile*> map_p1_CTR_ampCorr_vs_ampRatio;
   
   for(int binX = 0; binX < nBinsX; ++binX)
     for(int binY = 0; binY < nBinsY; ++binY)
     {
       std::string label(Form("%d-%d",binX,binY));
       
       map_h_amp1[label] = new TH1F(Form("h_amp1_%s",label.c_str()),"",250,0.,1000.);
       map_h_amp2[label] = new TH1F(Form("h_amp2_%s",label.c_str()),"",250,0.,1000.);
       map_h_ampRatio[label] = new TH1F(Form("h_ampRatio_%s",label.c_str()),"",1000,0.,3.);
       
       map_p1_CTR_vs_amp1[label] = new TProfile(Form("p1_CTR_vs_amp1_%s",label.c_str()),"",50,0.,1000.);
       map_p1_CTR_vs_amp2[label] = new TProfile(Form("p1_CTR_vs_amp2_%s",label.c_str()),"",50,0.,1000.);
       map_p1_CTR_vs_ampRatio[label] = new TProfile(Form("p1_CTR_vs_ampRatio_%s",label.c_str()),"",50,0.,5.);
       map_p1_CTR_ampCorr_vs_amp1[label] = new TProfile(Form("p1_CTR_ampCorr_vs_amp1_%s",label.c_str()),"",50,0.,1000.);
       map_p1_CTR_ampCorr_vs_amp2[label] = new TProfile(Form("p1_CTR_ampCorr_vs_amp2_%s",label.c_str()),"",50,0.,1000.);
       map_p1_CTR_ampCorr_vs_ampRatio[label] = new TProfile(Form("p1_CTR_ampCorr_vs_ampRatio_%s",label.c_str()),"",50,0.,5.);
     }

   TH1F* h1_chessOccupancyX  = new TH1F("h1_chessOccupancyX", "",nBinsX,cut_crystalXMin,cut_crystalXMax);
   TH1F* h1_chessOccupancyY  = new TH1F("h1_chessOccupancyY", "",nBinsY,cut_crystalYMin,cut_crystalYMax);
   TH2F* h2_chessOccupancyXY = new TH2F("h2_chessOccupancyXY","",nBinsX,cut_crystalXMin,cut_crystalXMax,nBinsY,cut_crystalYMin,cut_crystalYMax);

   TH2F* h2_CTR_chess = new TH2F("h2_CTR_chess","",nBinsX,cut_crystalXMin,cut_crystalXMax,nBinsY,cut_crystalYMin,cut_crystalYMax);
   TH2F* h2_CTR_chess_ampCorr = new TH2F("h2_CTR_chess_ampCorr","",nBinsX,cut_crystalXMin,cut_crystalXMax,nBinsY,cut_crystalYMin,cut_crystalYMax);
   TH2F* h2_CTR_chess_ampCorrEff = new TH2F("h2_CTR_chess_ampCorrEff","",nBinsX,cut_crystalXMin,cut_crystalXMax,nBinsY,cut_crystalYMin,cut_crystalYMax);
   TH2F* h2_CTR_chess_ampCorrPerBin = new TH2F("h2_CTR_chess_ampCorrPerBin","",nBinsX,cut_crystalXMin,cut_crystalXMax,nBinsY,cut_crystalYMin,cut_crystalYMax);


   //-----------------------
   // first loop over events
   int nEntries = chain0 -> GetEntries();
   int nEntries_cut = 0;
   for(int entry = 0; entry < nEntries; ++entry)
   {
     if( entry%1000 == 0 ) std::cout << ">>> loop 1/3: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
     chain0 -> GetEntry(entry);
     
     float amp1 = treeVars.t_amp[(*treeVars.t_channelId)[enCh0]] * 0.25;
     float amp2 = treeVars.t_amp[(*treeVars.t_channelId)[enCh1]] * 0.25;
     float dur1 = treeVars.t_dur[(*treeVars.t_channelId)[timeCh0]] * 0.2;
     float dur2 = treeVars.t_dur[(*treeVars.t_channelId)[timeCh1]] * 0.2;
     int extraIt1 = isMCP0 ? treeVars.t_CFD : treeVars.t_LED;
     int extraIt2 = isMCP1 ? treeVars.t_CFD : treeVars.t_LED;
     float time1 = treeVars.t_time[(*treeVars.t_channelId)[timeCh0]+extraIt1];
     float time2 = treeVars.t_time[(*treeVars.t_channelId)[timeCh1]+extraIt2];
     float CTR = time2 - time1;

     bool isCenter = false;
     bool isBorder = false;
     if( treeVars.t_beamX[0] >= cut_crystalXMin &&
         treeVars.t_beamX[0]  < cut_crystalXMax &&
         treeVars.t_beamY[0] >= cut_crystalYMin &&
         treeVars.t_beamY[0]  < cut_crystalYMax )
     {
       isCenter = false;
       isBorder = true;
     }
     if( treeVars.t_beamX[0] >= 0.5*(cut_crystalXMin+cut_crystalXMax)-2. &&
         treeVars.t_beamX[0]  < 0.5*(cut_crystalXMin+cut_crystalXMax)+2. &&
         treeVars.t_beamY[0] >= 0.5*(cut_crystalYMin+cut_crystalYMax)-2. &&
         treeVars.t_beamY[0]  < 0.5*(cut_crystalYMin+cut_crystalYMax)+2. )
     {
       isCenter = true;
       isBorder = false;
     }

     // if( !AcceptEventVbias(VbiasIndex1,VbiasIndex2,treeVars,cut_Vbias,cut_Vbias) ) continue;
     // if( !AcceptEventTh(treeVars,cut_NINOthr,cut_NINOthr) ) continue;
     if( !AcceptEventAmp(enCh0,enCh1,treeVars,1.,1000.,1.,1000.) ) continue;

     h2_beam_Y_vs_X -> Fill(treeVars.t_beamX[0],treeVars.t_beamY[0]);

     p2_amp1_vs_beam_Y_vs_X -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],amp1 );
     p2_amp2_vs_beam_Y_vs_X -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],amp2 );

     if( AcceptEventAmp(enCh0,enCh1,treeVars,cut_ampMin1,cut_ampMax1,1.,1000.) )
     {
       p1_amp1_vs_beam_X -> Fill( treeVars.t_beamX[0],amp1 );
       p1_amp1_vs_beam_Y -> Fill( treeVars.t_beamY[0],amp1 );
       p2_amp1_vs_beam_Y_vs_X_cut -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],amp1 );

       if( fabs( treeVars.t_beamY[0] - 0.5*(cut_crystalYMin+cut_crystalYMax) ) < 2. )
       {
         p1_amp1_vs_beam_X_cut -> Fill( treeVars.t_beamX[0],amp1 );
       }
       if( fabs( treeVars.t_beamX[0] - 0.5*(cut_crystalXMin+cut_crystalXMax) ) < 2. )
       {
         p1_amp1_vs_beam_Y_cut -> Fill( treeVars.t_beamY[0],amp1 );
       }
     }
     if( AcceptEventAmp(enCh0,enCh1,treeVars,1.,1000.,cut_ampMin2,cut_ampMax2) )
     {
       p1_amp2_vs_beam_X -> Fill( treeVars.t_beamX[0],amp2 );
       p1_amp2_vs_beam_Y -> Fill( treeVars.t_beamY[0],amp2 );
       p2_amp2_vs_beam_Y_vs_X_cut -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],amp2 );

       if( fabs( treeVars.t_beamY[0] - 0.5*(cut_crystalYMin+cut_crystalYMax) ) < 2. )
       {
         p1_amp2_vs_beam_X_cut -> Fill( treeVars.t_beamX[0],amp2 );
       }
       if( fabs( treeVars.t_beamX[0] - 0.5*(cut_crystalXMin+cut_crystalXMax) ) < 2. )
       {
         p1_amp2_vs_beam_Y_cut -> Fill( treeVars.t_beamY[0],amp2 );
       }
     }

     if( !AcceptEvent(enCh0,enCh1,timeCh0,timeCh1,treeVars,
                      1,cut_crystalXMin,cut_crystalXMax,cut_crystalYMin,cut_crystalYMax,
                      -1) )
       continue;

     if( isMCP0 || isMCP1 )
       if( !AcceptEventMCP(treeVars,cut_MCPX,cut_MCPY,cut_MCPR) ) continue;

     h_amp1 -> Fill( amp1 );
     h_amp2 -> Fill( amp2 );
     h_ampRatio -> Fill( amp2/amp1 );
     if( isCenter )
     {
       h_amp1_center -> Fill( amp1 );
       h_amp2_center -> Fill( amp2 );    
     }
     if( isBorder )
     {
       h_amp1_border -> Fill( amp1 );
       h_amp2_border -> Fill( amp2 );    
     }

     if( !AcceptEventAmp(enCh0,enCh1,treeVars,cut_ampMin1,cut_ampMax1,cut_ampMin2,cut_ampMax2) ) continue;

     h_dur1 -> Fill( dur1 );
     h_dur2 -> Fill( dur2 );

     if( !AcceptEventDur(timeCh0,timeCh1,treeVars,cut_durMin1,cut_durMax1,cut_durMin2,cut_durMax2) ) continue;

     h2_beam_Y_vs_X_cut -> Fill(treeVars.t_beamX[0],treeVars.t_beamY[0]);
     
     if( !AcceptEventTime(timeCh0,timeCh1,isMCP0,isMCP1,treeVars,0.,200.,0.,200.) ) continue;

    h_amp1_cut -> Fill( amp1 );
    h_amp2_cut -> Fill( amp2 );
    h_ampRatio_cut -> Fill( amp2/amp1 );
    
    h_dur1_cut -> Fill( dur1 );
    h_dur2_cut -> Fill( dur2 );
    
    h_CTR_raw -> Fill( CTR );
    
    p1_CTR_vs_amp1 -> Fill( amp1,CTR );
    p1_CTR_vs_amp2 -> Fill( amp2,CTR );
    p1_CTR_vs_ampRatio -> Fill( amp2/amp1,CTR );
    if( isCenter )
    {
      p1_CTR_vs_amp1_center -> Fill( amp1,CTR );
      p1_CTR_vs_amp2_center -> Fill( amp2,CTR );
      p1_CTR_vs_ampRatio_center -> Fill( amp2/amp1,CTR );    
    }
    if( isBorder )
    {
      p1_CTR_vs_amp1_border -> Fill( amp1,CTR );
      p1_CTR_vs_amp2_border -> Fill( amp2,CTR );
      p1_CTR_vs_ampRatio_border -> Fill( amp2/amp1,CTR );    
    }
    
    p1_CTR_vs_beam_X -> Fill( treeVars.t_beamX[0],CTR );
    p1_CTR_vs_beam_Y -> Fill( treeVars.t_beamY[0],CTR );
    p2_CTR_vs_beam_Y_vs_X -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],CTR );
    
    if( fabs( treeVars.t_beamY[0] - 0.5*(cut_crystalYMin+cut_crystalYMax) ) < 2. )
    {
      p1_CTR_vs_beam_X_cut -> Fill( treeVars.t_beamX[0],CTR );
    }
    if( fabs( treeVars.t_beamX[0] - 0.5*(cut_crystalXMin+cut_crystalXMax) ) < 2. )
    {
      p1_CTR_vs_beam_Y_cut -> Fill( treeVars.t_beamY[0],CTR );
    }
    
    // int binX = int( (treeVars.t_beamX[0]-cut_crystalXMin) / nBinsX );
    // int binY = int( (treeVars.t_beamY[0]-cut_crystalYMin) / nBinsY );
    int binX = h1_chessOccupancyX -> Fill(treeVars.t_beamX[0]);
    int binY = h1_chessOccupancyY -> Fill(treeVars.t_beamY[0]);
    h2_chessOccupancyXY -> Fill(treeVars.t_beamX[0],treeVars.t_beamY[0]);
    
    std::string label(Form("%d-%d",binX-1,binY-1));
    map_h_amp1[label] -> Fill( amp1 );
    map_h_amp2[label] -> Fill( amp2 );
    map_h_ampRatio[label] -> Fill( amp2/amp1 );
    map_p1_CTR_vs_amp1[label] -> Fill( amp1,CTR );
    map_p1_CTR_vs_amp2[label] -> Fill( amp2,CTR );
    map_p1_CTR_vs_ampRatio[label] -> Fill( amp2/amp1,CTR );
    
    ++nEntries_cut;
  }
  std::cout << std::endl;
  std::cout << ">>> Selected " << nEntries_cut << " / " << nEntries << " events" << std::endl;
  
  
  
  //--------------------------
  // amplitude walk correction
  TF1* fitFunc_corrAmp = new TF1(Form("fitFunc_corrAmp"),"[0]*log([1]*x)+[2]",0.,1000.);
  fitFunc_corrAmp -> SetParameters(-0.2,0.0000001,0.);
  if( isMCP0 && !isMCP1)
    p1_CTR_vs_amp2 -> Fit(Form("fitFunc_corrAmp"),"QNS+","",h_amp2_center->GetMean()+0.5*h_amp2_center->GetRMS(),h_amp2_center->GetMean()+10.*h_amp2_center->GetRMS());
  else if( !isMCP0 && isMCP1)
    p1_CTR_vs_amp1 -> Fit(Form("fitFunc_corrAmp"),"QNS+","",h_amp1_center->GetMean()+0.5*h_amp1_center->GetRMS(),h_amp1_center->GetMean()+10.*h_amp1_center->GetRMS());
  else
    p1_CTR_vs_ampRatio -> Fit(Form("fitFunc_corrAmp"),"QNS+","");
  
  
  
  // find CTR ranges
  float CTRMean = -1;
  float CTRSigma = -1;
  {
    TH1F* histo = h_CTR_raw;
    float* vals = new float[4];
    FindSmallestInterval(vals,histo,0.68,true); 
    
    float mean = vals[0];
    float min = vals[2];
    float max = vals[3];
    float delta = max-min;
    float sigma = 0.5*delta;
    float effSigma = sigma;
    CTRMean = mean;
    CTRSigma = effSigma;
  }
  
  TH1F* h_CTR = new TH1F("h_CTR","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_center = new TH1F("h_CTR_center","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_border = new TH1F("h_CTR_border","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_ampCorr = new TH1F("h_CTR_ampCorr","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_ampCorr_center = new TH1F("h_CTR_ampCorr_center","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_ampCorr_border = new TH1F("h_CTR_ampCorr_border","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_ampCorrEff = new TH1F("h_CTR_ampCorrEff","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_ampCorrEff_center = new TH1F("h_CTR_ampCorrEff_center","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_ampCorrEff_border = new TH1F("h_CTR_ampCorrEff_border","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_ampCorrPerBin = new TH1F("h_CTR_ampCorrPerBin","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_ampCorrPerBin_center = new TH1F("h_CTR_ampCorrPerBin_center","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_ampCorrPerBin_border = new TH1F("h_CTR_ampCorrPerBin_border","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_posCorr = new TH1F("h_CTR_posCorr","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_posCorr_center = new TH1F("h_CTR_posCorr_center","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_posCorr_border = new TH1F("h_CTR_posCorr_border","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_posCorrAmpCorr = new TH1F("h_CTR_posCorrAmpCorr","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_posCorrAmpCorr_center = new TH1F("h_CTR_posCorrAmpCorr_center","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_posCorrAmpCorr_border = new TH1F("h_CTR_posCorrAmpCorr_border","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_ampCorrEffPosCorr = new TH1F("h_CTR_ampCorrEffPosCorr","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_ampCorrEffPosCorr_center = new TH1F("h_CTR_ampCorrEffPosCorr_center","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_ampCorrEffPosCorr_border = new TH1F("h_CTR_ampCorrEffPosCorr_border","",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  
  std::map<std::string,TH1F*> map_h_CTR;
  std::map<std::string,TH1F*> map_h_CTR_ampCorr;
  std::map<std::string,TH1F*> map_h_CTR_ampCorrEff;
  std::map<std::string,TH1F*> map_h_CTR_ampCorrPerBin;
  
  for(int binX = 0; binX < nBinsX; ++binX)
    for(int binY = 0; binY < nBinsY; ++binY)
    {
      std::string label(Form("%d-%d",binX,binY));
      
      map_h_CTR[label] = new TH1F(Form("h_CTR_%s",label.c_str()),"",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
      map_h_CTR_ampCorr[label] = new TH1F(Form("h_CTR_ampCorr_%s",label.c_str()),"",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
      map_h_CTR_ampCorrEff[label] = new TH1F(Form("h_CTR_ampCorrEff_%s",label.c_str()),"",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
      map_h_CTR_ampCorrPerBin[label] = new TH1F(Form("h_CTR_ampCorrPerBin_%s",label.c_str()),"",2000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
    }
  
  
  
  //------------------------
  // second loop over events
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> loop 2/3: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain0 -> GetEntry(entry);
    
    if( !AcceptEvent(enCh0,enCh1,timeCh0,timeCh1,treeVars,
                     1,cut_crystalXMin,cut_crystalXMax,cut_crystalYMin,cut_crystalYMax,
                     -1) )
      continue;
    
    // if( !AcceptEventVbias(VbiasIndex1,VbiasIndex2,treeVars,cut_Vbias,cut_Vbias) ) continue;
    // if( !AcceptEventTh(treeVars,cut_NINOthr,cut_NINOthr) ) continue;
    
    float amp1 = treeVars.t_amp[(*treeVars.t_channelId)[enCh0]] * 0.25;
    float amp2 = treeVars.t_amp[(*treeVars.t_channelId)[enCh1]] * 0.25;
    int extraIt1 = isMCP0 ? treeVars.t_CFD : treeVars.t_LED;
    int extraIt2 = isMCP1 ? treeVars.t_CFD : treeVars.t_LED;
    float time1 = treeVars.t_time[(*treeVars.t_channelId)[timeCh0]+extraIt1];
    float time2 = treeVars.t_time[(*treeVars.t_channelId)[timeCh1]+extraIt2];
    float CTR = time2 - time1;
    
    if( !AcceptEventAmp(enCh0,enCh1,treeVars,cut_ampMin1,cut_ampMax1,cut_ampMin2,cut_ampMax2) ) continue;
    if( !AcceptEventDur(timeCh0,timeCh1,treeVars,cut_durMin1,cut_durMax1,cut_durMin2,cut_durMax2) ) continue;
    if( !AcceptEventTime(timeCh0,timeCh1,isMCP0,isMCP1,treeVars,0.,200.,0.,200.) ) continue;
    if( isMCP0 || isMCP1 ) if( !AcceptEventMCP(treeVars,cut_MCPX,cut_MCPY,cut_MCPR) ) continue;
    
    bool isCenter = false;
    bool isBorder = false;
    if( treeVars.t_beamX[0] > cut_crystalXMin &&
        treeVars.t_beamX[0] < cut_crystalXMax &&
        treeVars.t_beamY[0] > cut_crystalYMin &&
        treeVars.t_beamY[0] < cut_crystalYMax )
    {
      isCenter = false;
      isBorder = true;
    }
    if( treeVars.t_beamX[0] > 0.5*(cut_crystalXMin+cut_crystalXMax)-2. &&
        treeVars.t_beamX[0] < 0.5*(cut_crystalXMin+cut_crystalXMax)+2. &&
        treeVars.t_beamY[0] > 0.5*(cut_crystalYMin+cut_crystalYMax)-2. &&
        treeVars.t_beamY[0] < 0.5*(cut_crystalYMin+cut_crystalYMax)+2. )
    {
      isCenter = true;
      isBorder = false;
    }
    
    int binX = h1_chessOccupancyX -> Fill(treeVars.t_beamX[0]);
    int binY = h1_chessOccupancyY -> Fill(treeVars.t_beamY[0]);
    
    std::string label(Form("%d-%d",binX-1,binY-1));
    
    float CTR_ampCorr;
    if( isMCP0 && !isMCP1 )
      CTR_ampCorr = CTR -
        p1_CTR_vs_amp2->GetBinContent( p1_CTR_vs_amp2->FindBin(amp2) ) + 
        p1_CTR_vs_amp2->GetBinContent( p1_CTR_vs_amp2->FindBin(h_amp2_cut->GetMean()) );
    else if( !isMCP0 && isMCP1 )
      CTR_ampCorr = CTR -
        p1_CTR_vs_amp1->GetBinContent( p1_CTR_vs_amp1->FindBin(amp1) ) + 
        p1_CTR_vs_amp1->GetBinContent( p1_CTR_vs_amp1->FindBin(h_amp1_cut->GetMean()) );
    else
      CTR_ampCorr = CTR -
        p1_CTR_vs_ampRatio->GetBinContent( p1_CTR_vs_ampRatio->FindBin(amp2/amp1) ) + 
        p1_CTR_vs_ampRatio->GetBinContent( p1_CTR_vs_ampRatio->FindBin(h_ampRatio_cut->GetMean()) );
    
    float CTR_ampCorrEff;
    if( isMCP0 && !isMCP1 )
      CTR_ampCorrEff = CTR - 
        fitFunc_corrAmp -> Eval( amp2 ) + 
        fitFunc_corrAmp -> Eval( h_amp2_cut->GetMean() );
    else if( !isMCP0 && isMCP1 )
      CTR_ampCorrEff = CTR - 
        fitFunc_corrAmp -> Eval( amp1 ) + 
        fitFunc_corrAmp -> Eval( h_amp1_cut->GetMean() );
    else
      CTR_ampCorrEff = CTR - 
        fitFunc_corrAmp -> Eval( amp2/amp1 ) + 
        fitFunc_corrAmp -> Eval( h_ampRatio_cut->GetMean() );
    
    float CTR_ampCorrPerBin;
    if( isMCP0 && !isMCP1 )
      CTR_ampCorrPerBin = CTR -
        map_p1_CTR_vs_amp2[label]->GetBinContent( map_p1_CTR_vs_amp2[label]->FindBin(amp2) ) + 
        map_p1_CTR_vs_amp2[label]->GetBinContent( map_p1_CTR_vs_amp2[label]->FindBin(map_h_amp2[label]->GetMean()) );
    else if( !isMCP0 && isMCP1 )
      CTR_ampCorrPerBin = CTR -
        map_p1_CTR_vs_amp1[label]->GetBinContent( map_p1_CTR_vs_amp1[label]->FindBin(amp1) ) + 
        map_p1_CTR_vs_amp1[label]->GetBinContent( map_p1_CTR_vs_amp1[label]->FindBin(map_h_amp1[label]->GetMean()) );
    else
      CTR_ampCorrPerBin = CTR -
        map_p1_CTR_vs_ampRatio[label]->GetBinContent( map_p1_CTR_vs_ampRatio[label]->FindBin(amp2/amp1) ) + 
        map_p1_CTR_vs_ampRatio[label]->GetBinContent( map_p1_CTR_vs_ampRatio[label]->FindBin(map_h_ampRatio[label]->GetMean()) );
    
    float CTR_posCorr = CTR -
      p2_CTR_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(treeVars.t_beamX[0],treeVars.t_beamY[0]) ) + 
      p2_CTR_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(0.5*(cut_crystalXMin+cut_crystalXMax),0.5*(cut_crystalYMin+cut_crystalYMax)) );
    
    
    h_CTR -> Fill(CTR);
    if( isCenter ) h_CTR_center -> Fill(CTR);
    if( isBorder ) h_CTR_border -> Fill(CTR);
    
    h_CTR_ampCorr -> Fill( CTR_ampCorr );
    if( isCenter ) h_CTR_ampCorr_center -> Fill( CTR_ampCorr );
    if( isBorder ) h_CTR_ampCorr_border -> Fill( CTR_ampCorr );
    
    h_CTR_ampCorrEff -> Fill( CTR_ampCorrEff );
    if( isCenter ) h_CTR_ampCorrEff_center -> Fill( CTR_ampCorrEff );
    if( isBorder ) h_CTR_ampCorrEff_border -> Fill( CTR_ampCorrEff );
    
    h_CTR_ampCorrPerBin -> Fill( CTR_ampCorrPerBin );
    if( isCenter ) h_CTR_ampCorrPerBin_center -> Fill( CTR_ampCorrPerBin );
    if( isBorder ) h_CTR_ampCorrPerBin_border -> Fill( CTR_ampCorrPerBin );
    
    h_CTR_posCorr -> Fill( CTR_posCorr );
    if( isCenter ) h_CTR_posCorr_center -> Fill( CTR_posCorr );
    if( isBorder ) h_CTR_posCorr_border -> Fill( CTR_posCorr );
    
    map_h_CTR[label] -> Fill( CTR );
    map_h_CTR_ampCorr[label] -> Fill( CTR_ampCorr );
    map_h_CTR_ampCorrEff[label] -> Fill( CTR_ampCorrEff );
    map_h_CTR_ampCorrPerBin[label] -> Fill( CTR_ampCorrPerBin );
    
    p1_CTR_ampCorr_vs_beam_X -> Fill( treeVars.t_beamX[0],CTR_ampCorr );
    p1_CTR_ampCorr_vs_beam_Y -> Fill( treeVars.t_beamY[0],CTR_ampCorr );
    p2_CTR_ampCorr_vs_beam_Y_vs_X -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],CTR_ampCorr );
    
    p1_CTR_ampCorrEff_vs_beam_X -> Fill( treeVars.t_beamX[0],CTR_ampCorrEff );
    p1_CTR_ampCorrEff_vs_beam_Y -> Fill( treeVars.t_beamY[0],CTR_ampCorrEff );
    p2_CTR_ampCorrEff_vs_beam_Y_vs_X -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],CTR_ampCorrEff );
    
    p1_CTR_ampCorrPerBin_vs_beam_X -> Fill( treeVars.t_beamX[0],CTR_ampCorrPerBin );
    p1_CTR_ampCorrPerBin_vs_beam_Y -> Fill( treeVars.t_beamY[0],CTR_ampCorrPerBin );
    p2_CTR_ampCorrPerBin_vs_beam_Y_vs_X -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],CTR_ampCorrPerBin );
    
    p1_CTR_posCorr_vs_beam_X -> Fill( treeVars.t_beamX[0],CTR_posCorr );
    p1_CTR_posCorr_vs_beam_Y -> Fill( treeVars.t_beamY[0],CTR_posCorr );
    p2_CTR_posCorr_vs_beam_Y_vs_X -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],CTR_posCorr );
    
    if( fabs( treeVars.t_beamY[0] - 0.5*(cut_crystalYMin+cut_crystalYMax) ) < 2. )
    {
      p1_CTR_ampCorr_vs_beam_X_cut -> Fill( treeVars.t_beamX[0],CTR_ampCorr );
      p1_CTR_ampCorrEff_vs_beam_X_cut -> Fill( treeVars.t_beamX[0],CTR_ampCorrEff );
      p1_CTR_ampCorrPerBin_vs_beam_X_cut -> Fill( treeVars.t_beamX[0],CTR_ampCorrPerBin );
      p1_CTR_posCorr_vs_beam_X_cut -> Fill( treeVars.t_beamX[0],CTR_posCorr );
    }
    
    if( fabs( treeVars.t_beamX[0] - 0.5*(cut_crystalXMin+cut_crystalXMax) ) < 2. )
    {
      p1_CTR_ampCorr_vs_beam_Y_cut -> Fill( treeVars.t_beamY[0],CTR_ampCorr );
      p1_CTR_ampCorrEff_vs_beam_Y_cut -> Fill( treeVars.t_beamY[0],CTR_ampCorrEff );
      p1_CTR_ampCorrPerBin_vs_beam_Y_cut -> Fill( treeVars.t_beamY[0],CTR_ampCorrPerBin );
      p1_CTR_posCorr_vs_beam_Y_cut -> Fill( treeVars.t_beamY[0],CTR_posCorr );
    }
    
    p1_CTR_ampCorr_vs_amp1 -> Fill( amp1,CTR_ampCorr );
    p1_CTR_ampCorr_vs_amp2 -> Fill( amp2,CTR_ampCorr );
    p1_CTR_ampCorr_vs_ampRatio -> Fill( amp2/amp1,CTR_ampCorr );
    
    p1_CTR_ampCorrEff_vs_amp1 -> Fill( amp1,CTR_ampCorrEff );
    p1_CTR_ampCorrEff_vs_amp2 -> Fill( amp2,CTR_ampCorrEff );
    p1_CTR_ampCorrEff_vs_ampRatio -> Fill( amp2/amp1,CTR_ampCorrEff );
    
    p1_CTR_ampCorrPerBin_vs_amp1 -> Fill( amp1,CTR_ampCorrPerBin );
    p1_CTR_ampCorrPerBin_vs_amp2 -> Fill( amp2,CTR_ampCorrPerBin );
    p1_CTR_ampCorrPerBin_vs_ampRatio -> Fill( amp2/amp1,CTR_ampCorrPerBin );
    
    p1_CTR_posCorr_vs_amp1 -> Fill( amp1,CTR_posCorr );
    p1_CTR_posCorr_vs_amp2 -> Fill( amp2,CTR_posCorr );
    p1_CTR_posCorr_vs_ampRatio -> Fill( amp2/amp1,CTR_posCorr );
  }
  std::cout << std::endl;
  
  
  //-----------------------
  // third loop over events
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> loop 3/3: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain0 -> GetEntry(entry);
    
    if( !AcceptEvent(enCh0,enCh1,timeCh0,timeCh1,treeVars,
                     1,cut_crystalXMin,cut_crystalXMax,cut_crystalYMin,cut_crystalYMax,
                     -1) )
      continue;
    
    // if( !AcceptEventVbias(VbiasIndex1,VbiasIndex2,treeVars,cut_Vbias,cut_Vbias) ) continue;
    // if( !AcceptEventTh(treeVars,cut_NINOthr,cut_NINOthr) ) continue;
    
    float amp1 = treeVars.t_amp[(*treeVars.t_channelId)[enCh0]] * 0.25;
    float amp2 = treeVars.t_amp[(*treeVars.t_channelId)[enCh1]] * 0.25;
    int extraIt1 = isMCP0 ? treeVars.t_CFD : treeVars.t_LED;
    int extraIt2 = isMCP1 ? treeVars.t_CFD : treeVars.t_LED;
    float time1 = treeVars.t_time[(*treeVars.t_channelId)[timeCh0]+extraIt1];
    float time2 = treeVars.t_time[(*treeVars.t_channelId)[timeCh1]+extraIt2];
    float CTR = time2 - time1;
    
    if( !AcceptEventAmp(enCh0,enCh1,treeVars,cut_ampMin1,cut_ampMax1,cut_ampMin2,cut_ampMax2) ) continue;
    if( !AcceptEventDur(timeCh0,timeCh1,treeVars,cut_durMin1,cut_durMax1,cut_durMin2,cut_durMax2) ) continue;
    if( !AcceptEventTime(timeCh0,timeCh1,isMCP0,isMCP1,treeVars,0.,200.,0.,200.) ) continue;
    if( isMCP0 || isMCP1 ) if( !AcceptEventMCP(treeVars,cut_MCPX,cut_MCPY,cut_MCPR) ) continue;
    
    bool isCenter = false;
    bool isBorder = false;
    if( treeVars.t_beamX[0] > cut_crystalXMin &&
        treeVars.t_beamX[0] < cut_crystalXMax &&
        treeVars.t_beamY[0] > cut_crystalYMin &&
        treeVars.t_beamY[0] < cut_crystalYMax )
    {
      isCenter = false;
      isBorder = true;
    }
    if( treeVars.t_beamX[0] > 0.5*(cut_crystalXMin+cut_crystalXMax)-2. &&
        treeVars.t_beamX[0] < 0.5*(cut_crystalXMin+cut_crystalXMax)+2. &&
        treeVars.t_beamY[0] > 0.5*(cut_crystalYMin+cut_crystalYMax)-2. &&
        treeVars.t_beamY[0] < 0.5*(cut_crystalYMin+cut_crystalYMax)+2. )
    {
      isCenter = true;
      isBorder = false;
    }
    
    float ampCorr;
    if( isMCP0 && !isMCP1 )
      ampCorr = 
        -p1_CTR_posCorr_vs_amp2->GetBinContent( p1_CTR_posCorr_vs_amp2->FindBin(amp2) ) + 
        p1_CTR_posCorr_vs_amp2->GetBinContent( p1_CTR_posCorr_vs_amp2->FindBin(h_amp2_cut->GetMean()) );
    else if( !isMCP0 && isMCP1 )
      ampCorr = 
        -p1_CTR_posCorr_vs_amp1->GetBinContent( p1_CTR_posCorr_vs_amp1->FindBin(amp1) ) + 
        p1_CTR_posCorr_vs_amp1->GetBinContent( p1_CTR_posCorr_vs_amp1->FindBin(h_amp1_cut->GetMean()) );
    else
      ampCorr = 
        -p1_CTR_posCorr_vs_ampRatio->GetBinContent( p1_CTR_posCorr_vs_ampRatio->FindBin(amp2/amp1) ) + 
        p1_CTR_posCorr_vs_ampRatio->GetBinContent( p1_CTR_posCorr_vs_ampRatio->FindBin(h_ampRatio_cut->GetMean()) );
    
    float CTR_posCorrAmpCorr = CTR -
      p2_CTR_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(treeVars.t_beamX[0],treeVars.t_beamY[0]) ) + 
      p2_CTR_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(0.5*(cut_crystalXMin+cut_crystalXMax),0.5*(cut_crystalYMin+cut_crystalYMax)) ) +
      ampCorr;
    
    float CTR_ampCorrEffPosCorr;
    if( isMCP0 && !isMCP1 )
      CTR_ampCorrEffPosCorr= CTR -
        fitFunc_corrAmp -> Eval( amp2 ) +
        fitFunc_corrAmp -> Eval( h_amp2_cut->GetMean() ) -
        p2_CTR_ampCorrEff_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(treeVars.t_beamX[0],treeVars.t_beamY[0]) ) +
        p2_CTR_ampCorrEff_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(0.5*(cut_crystalXMin+cut_crystalXMax),0.5*(cut_crystalYMin+cut_crystalYMax)) );
    else if( !isMCP0 && isMCP1 )
      CTR_ampCorrEffPosCorr= CTR -
        fitFunc_corrAmp -> Eval( amp1 ) +
        fitFunc_corrAmp -> Eval( h_amp1_cut->GetMean() ) -
        p2_CTR_ampCorrEff_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(treeVars.t_beamX[0],treeVars.t_beamY[0]) ) +
        p2_CTR_ampCorrEff_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(0.5*(cut_crystalXMin+cut_crystalXMax),0.5*(cut_crystalYMin+cut_crystalYMax)) );
    else
      CTR_ampCorrEffPosCorr= CTR -
        fitFunc_corrAmp -> Eval( amp2/amp1 ) +
        fitFunc_corrAmp -> Eval( h_ampRatio_cut->GetMean() ) -
        p2_CTR_ampCorrEff_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(treeVars.t_beamX[0],treeVars.t_beamY[0]) ) +
        p2_CTR_ampCorrEff_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(0.5*(cut_crystalXMin+cut_crystalXMax),0.5*(cut_crystalYMin+cut_crystalYMax)) );
    
    h_CTR_posCorrAmpCorr -> Fill( CTR_posCorrAmpCorr );
    if( isCenter ) h_CTR_posCorrAmpCorr_center -> Fill( CTR_posCorrAmpCorr );
    if( isBorder ) h_CTR_posCorrAmpCorr_border -> Fill( CTR_posCorrAmpCorr );
    
    h_CTR_ampCorrEffPosCorr -> Fill( CTR_ampCorrEffPosCorr );
    if( isCenter ) h_CTR_ampCorrEffPosCorr_center -> Fill( CTR_ampCorrEffPosCorr );
    if( isBorder ) h_CTR_ampCorrEffPosCorr_border -> Fill( CTR_ampCorrEffPosCorr );
    
    p1_CTR_posCorrAmpCorr_vs_beam_X -> Fill( treeVars.t_beamX[0],CTR_posCorrAmpCorr );
    p1_CTR_posCorrAmpCorr_vs_beam_Y -> Fill( treeVars.t_beamY[0],CTR_posCorrAmpCorr );
    p2_CTR_posCorrAmpCorr_vs_beam_Y_vs_X -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],CTR_posCorrAmpCorr );
    
    p1_CTR_ampCorrEffPosCorr_vs_beam_X -> Fill( treeVars.t_beamX[0],CTR_ampCorrEffPosCorr );
    p1_CTR_ampCorrEffPosCorr_vs_beam_Y -> Fill( treeVars.t_beamY[0],CTR_ampCorrEffPosCorr );
    p2_CTR_ampCorrEffPosCorr_vs_beam_Y_vs_X -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],CTR_ampCorrEffPosCorr );
    
    if( fabs( treeVars.t_beamY[0] - 0.5*(cut_crystalYMin+cut_crystalYMax) ) < 2. )
    {
      p1_CTR_posCorrAmpCorr_vs_beam_X_cut -> Fill( treeVars.t_beamX[0],CTR_posCorrAmpCorr );
      p1_CTR_ampCorrEffPosCorr_vs_beam_X_cut -> Fill( treeVars.t_beamX[0],CTR_ampCorrEffPosCorr );
    }
    
    if( fabs( treeVars.t_beamX[0] - 0.5*(cut_crystalXMin+cut_crystalXMax) ) < 2. )
    {
      p1_CTR_posCorrAmpCorr_vs_beam_Y_cut -> Fill( treeVars.t_beamY[0],CTR_posCorrAmpCorr );
      p1_CTR_ampCorrEffPosCorr_vs_beam_Y_cut -> Fill( treeVars.t_beamY[0],CTR_ampCorrEffPosCorr );
    }
    
    p1_CTR_posCorrAmpCorr_vs_amp1 -> Fill( amp1,CTR_posCorrAmpCorr );
    p1_CTR_posCorrAmpCorr_vs_amp2 -> Fill( amp2,CTR_posCorrAmpCorr );
    p1_CTR_posCorrAmpCorr_vs_ampRatio -> Fill( amp2/amp1,CTR_posCorrAmpCorr );
    
    p1_CTR_ampCorrEffPosCorr_vs_amp1 -> Fill( amp1,CTR_ampCorrEffPosCorr );
    p1_CTR_ampCorrEffPosCorr_vs_amp2 -> Fill( amp2,CTR_ampCorrEffPosCorr );
    p1_CTR_ampCorrEffPosCorr_vs_ampRatio -> Fill( amp2/amp1,CTR_ampCorrEffPosCorr );
  }
  std::cout << std::endl;
  
  
  // float CTRMean_ampCorr = -1;
  // float CTRSigma_ampCorr = -1;
  // {
  //   TH1F* histo = h_CTR_ampCorr;
  //   float* vals = new float[4];
  //   FindSmallestInterval(vals,histo,0.68,true); 
    
  //   float mean = vals[0];      
  //   float min = vals[2];
  //   float max = vals[3];
  //   float delta = max-min;
  //   float sigma = 0.5*delta;
  //   float effSigma = sigma;
  //   CTRMean_ampCorr = mean;
  //   CTRSigma_ampCorr = effSigma;
  // }
  
  // float CTRMean_posCorr = -1;
  // float CTRSigma_posCorr = -1;
  // {
  //   TH1F* histo = h_CTR_posCorr;
  //   float* vals = new float[4];
  //   FindSmallestInterval(vals,histo,0.68,true); 
    
  //   float mean = vals[0];      
  //   float min = vals[2];
  //   float max = vals[3];
  //   float delta = max-min;
  //   float sigma = 0.5*delta;
  //   float effSigma = sigma;
  //   CTRMean_posCorr = mean;
  //   CTRSigma_posCorr = effSigma;
  // }
  
  
  
  //------------------------
  // define rectangles, etc.
  
  TBox* box_crystal = new TBox(cut_crystalXMin,cut_crystalYMin,cut_crystalXMax,cut_crystalYMax);
  box_crystal -> SetFillStyle(0);
  box_crystal -> SetLineColor(kBlack);
  box_crystal -> SetLineStyle(9);
  box_crystal -> SetLineWidth(3);
  
  TEllipse* ell_MCP = new TEllipse(cut_MCPX,cut_MCPY,cut_MCPR); 
  ell_MCP -> SetFillStyle(0);
  ell_MCP -> SetLineColor(kBlack);
  ell_MCP -> SetLineStyle(9);
  ell_MCP -> SetLineWidth(3);
  
  TLine* xBand_low = new TLine(cut_crystalXMin,0.5*(cut_crystalYMin+cut_crystalYMax)-2.,cut_crystalXMax,0.5*(cut_crystalYMin+cut_crystalYMax)-2.);
  TLine* xBand_hig = new TLine(cut_crystalXMin,0.5*(cut_crystalYMin+cut_crystalYMax)+2.,cut_crystalXMax,0.5*(cut_crystalYMin+cut_crystalYMax)+2.);
  xBand_low -> SetLineColor(kRed);
  xBand_hig -> SetLineColor(kRed);
  xBand_low -> SetLineStyle(9);
  xBand_hig -> SetLineStyle(9);
  xBand_low -> SetLineWidth(2);
  xBand_hig -> SetLineWidth(2);
  
  TLine* yBand_lef = new TLine(0.5*(cut_crystalXMin+cut_crystalXMax)-2.,cut_crystalYMin,0.5*(cut_crystalXMin+cut_crystalXMax)-2.,cut_crystalYMax);
  TLine* yBand_rig = new TLine(0.5*(cut_crystalXMin+cut_crystalXMax)+2.,cut_crystalYMin,0.5*(cut_crystalXMin+cut_crystalXMax)+2.,cut_crystalYMax);
  yBand_lef -> SetLineColor(kBlue);
  yBand_rig -> SetLineColor(kBlue);
  yBand_lef -> SetLineStyle(9);
  yBand_rig -> SetLineStyle(9);
  yBand_lef -> SetLineWidth(2);
  yBand_rig -> SetLineWidth(2);
  
  
  
  //-----------
  // draw plots
  TCanvas* c1 = new TCanvas(Form("c1_amplitudes"),Form("amp-dur"),1434,1330);
  c1 -> Divide(2,2);
  c1 -> cd(1);
  gPad -> SetLogy();
  h_amp1 -> GetXaxis() -> SetRangeUser(0.,1000.);
  h_amp1 -> SetTitle(Form(";%s amp_{max} (mV); events",xLabel1_en.c_str()));
  h_amp1 -> SetLineWidth(2);
  h_amp1 -> Draw();
  h_amp1_center -> SetLineColor(kRed);
  h_amp1_center -> Draw("same");
  h_amp1_border -> SetLineColor(kBlue);
  h_amp1_border -> Draw("same");
  h_amp1 -> Draw("same");
  TLine* line_cutAmpMin1 = new TLine(cut_ampMin1,0.,cut_ampMin1,1.5*h_amp1->GetMaximum());
  TLine* line_cutAmpMax1 = new TLine(cut_ampMax1,0.,cut_ampMax1,1.5*h_amp1->GetMaximum());
  line_cutAmpMin1 -> SetLineColor(kBlack);
  line_cutAmpMax1 -> SetLineColor(kBlack);
  line_cutAmpMin1 -> SetLineStyle(9);
  line_cutAmpMax1 -> SetLineStyle(9);
  line_cutAmpMin1 -> SetLineWidth(2);
  line_cutAmpMax1 -> SetLineWidth(2);
  line_cutAmpMin1 -> Draw("same");
  line_cutAmpMax1 -> Draw("same");
  latexLabel1 -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_center -> Draw("same");
  latexLabel_border -> Draw("same");
  c1 -> cd(2);
  gPad -> SetLogy();
  h_amp2 -> GetXaxis() -> SetRangeUser(0.,1000.);
  h_amp2 -> SetTitle(Form(";%s amp_{max} (mV); events",xLabel2_en.c_str()));
  h_amp2 -> SetLineWidth(2);
  h_amp2 -> Draw();
  h_amp2_center -> SetLineColor(kRed);
  h_amp2_center -> Draw("same");
  h_amp2_border -> SetLineColor(kBlue);
  h_amp2_border -> Draw("same");
  h_amp2 -> Draw("same");
  TLine* line_cutAmpMin2 = new TLine(cut_ampMin2,0.,cut_ampMin2,1.5*h_amp2->GetMaximum());
  TLine* line_cutAmpMax2 = new TLine(cut_ampMax2,0.,cut_ampMax2,1.5*h_amp2->GetMaximum());
  line_cutAmpMin2 -> SetLineColor(kBlack);
  line_cutAmpMax2 -> SetLineColor(kBlack);
  line_cutAmpMin2 -> SetLineStyle(9);
  line_cutAmpMax2 -> SetLineStyle(9);
  line_cutAmpMin2 -> SetLineWidth(2);
  line_cutAmpMax2 -> SetLineWidth(2);
  line_cutAmpMin2 -> Draw("same");
  line_cutAmpMax2 -> Draw("same");
  latexLabel2 -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_center -> Draw("same");
  latexLabel_border -> Draw("same");
  gPad -> Update();
  c1 -> cd(3);
  gPad -> SetLogy();
  h_dur1 -> GetXaxis() -> SetRangeUser(0.,200.);
  h_dur1 -> SetTitle(Form(";%s pulse length (ns); events",xLabel1_time.c_str()));
  h_dur1 -> Draw();
  TLine* line_cutDurMin1 = new TLine(cut_durMin1,0.,cut_durMin1,1.05*h_dur1->GetMaximum());
  TLine* line_cutDurMax1 = new TLine(cut_durMax1,0.,cut_durMax1,1.05*h_dur1->GetMaximum());
  line_cutDurMin1 -> SetLineColor(kBlack);
  line_cutDurMax1 -> SetLineColor(kBlack);
  line_cutDurMin1 -> SetLineStyle(9);
  line_cutDurMax1 -> SetLineStyle(9);
  line_cutDurMin1 -> SetLineWidth(2);
  line_cutDurMax1 -> SetLineWidth(2);
  line_cutDurMin1 -> Draw("same");
  line_cutDurMax1 -> Draw("same");
  latexLabel1 -> Draw("same");
  c1 -> cd(4);
  gPad -> SetLogy();
  h_dur2 -> GetXaxis() -> SetRangeUser(0.,200.);
  h_dur2 -> SetTitle(Form(";%s pulse length (ns); events",xLabel2_time.c_str()));
  h_dur2 -> Draw();
  TLine* line_cutDurMin2 = new TLine(cut_durMin2,0.,cut_durMin2,1.05*h_dur2->GetMaximum());
  TLine* line_cutDurMax2 = new TLine(cut_durMax2,0.,cut_durMax2,1.05*h_dur2->GetMaximum());
  line_cutDurMin2 -> SetLineColor(kBlack);
  line_cutDurMax2 -> SetLineColor(kBlack);
  line_cutDurMin2 -> SetLineStyle(9);
  line_cutDurMax2 -> SetLineStyle(9);
  line_cutDurMin2 -> SetLineWidth(2);
  line_cutDurMax2 -> SetLineWidth(2);
  line_cutDurMin2 -> Draw("same");
  line_cutDurMax2 -> Draw("same");
  latexLabel2 -> Draw("same");
  gPad -> Update();
  c1 -> Print(Form("%s/c__amp-dur__config%d.png",plotDir.c_str(),configuration));
  
  
  c1 = new TCanvas("c1_beam_Y_vs_X","beam profile",1430,665);
  c1 -> Divide(2,1);
  c1 -> cd(1);
  gPad -> SetLogz();
  h2_beam_Y_vs_X -> SetTitle(";beam X (mm);beam Y (mm);events");
  h2_beam_Y_vs_X -> Draw("colz");
  c1 -> cd(2);
  gPad -> SetLogz();
  h2_beam_Y_vs_X_cut -> SetTitle(";beam X (mm);beam Y (mm);events");
  h2_beam_Y_vs_X_cut -> Draw("colz");
  gPad -> Update();
  c1 -> Print(Form("%s/c__beamProfile__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_chessOccupancy","chess occupancy");
  c1 -> cd();
  gPad -> SetLogz();
  h2_chessOccupancyXY -> SetMinimum(1.);
  h2_chessOccupancyXY -> SetTitle(";beam X (mm);beam Y (mm);events");
  h2_chessOccupancyXY -> Draw("colz");
  c1 -> Print(Form("%s/c__chessOccupancy__config%d.png",plotDir.c_str(),configuration));
  
  
  c1 = new TCanvas("c1_amp1_beam_X_Y","amplitude profile",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  gPad -> SetGridy();
  p1_amp1_vs_beam_X -> SetTitle(Form(";beam X (mm);#LT %s amp_{max} #GT (mV)",xLabel1_en.c_str()));
  p1_amp1_vs_beam_X -> SetMinimum(cut_ampMin1);
  p1_amp1_vs_beam_X -> SetMaximum(h_amp1_cut->GetMean()+2.*h_amp1_cut->GetRMS());
  p1_amp1_vs_beam_X -> Draw();
  p1_amp1_vs_beam_X_cut -> SetMarkerColor(kRed);
  p1_amp1_vs_beam_X_cut -> SetLineColor(kRed);
  p1_amp1_vs_beam_X_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_ysel -> Draw("same");
  latexLabel1 -> Draw("same");
  c1 -> cd(2);
  gPad -> SetGridy();
  p1_amp1_vs_beam_Y -> SetTitle(Form(";beam Y (mm);#LT %s amp_{max} #GT (mV)",xLabel1_en.c_str()));
  p1_amp1_vs_beam_Y -> SetMinimum(cut_ampMin1);
  p1_amp1_vs_beam_Y -> SetMaximum(h_amp1_cut->GetMean()+2.*h_amp1_cut->GetRMS());
  p1_amp1_vs_beam_Y -> Draw();
  p1_amp1_vs_beam_Y_cut -> SetMarkerColor(kBlue);
  p1_amp1_vs_beam_Y_cut -> SetLineColor(kBlue);
  p1_amp1_vs_beam_Y_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_xsel -> Draw("same");
  latexLabel1 -> Draw("same");
  c1 -> cd(3);
  p2_amp1_vs_beam_Y_vs_X_cut -> SetTitle(Form(";beam X (mm);beam Y (mm);#LT %s amp_{max} #GT (mV)",xLabel1_en.c_str()));
  //gPad -> SetLogz();
  p2_amp1_vs_beam_Y_vs_X_cut -> SetMinimum(cut_ampMin1);
  p2_amp1_vs_beam_Y_vs_X_cut -> SetMaximum(h_amp1_cut->GetMean()+2.*h_amp1_cut->GetRMS());
  p2_amp1_vs_beam_Y_vs_X_cut -> Draw("colz");
  xBand_low -> Draw("same");
  xBand_hig -> Draw("same");
  yBand_lef -> Draw("same");
  yBand_rig -> Draw("same");
  latexLabel1 -> Draw("same");
  gPad -> Update();
  c1 -> Print(Form("%s/c__amplitudeProfile1__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_amp2_beam_X_Y","amplitude profile",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  gPad -> SetGridy();
  p1_amp2_vs_beam_X -> SetTitle(Form(";beam X (mm);#LT %s amp_{max} #GT (mV)",xLabel2_en.c_str()));
  p1_amp2_vs_beam_X -> SetMinimum(cut_ampMin2);
  p1_amp2_vs_beam_X -> SetMaximum(h_amp2_cut->GetMean()+2.*h_amp2_cut->GetRMS());
  p1_amp2_vs_beam_X -> Draw();
  p1_amp2_vs_beam_X_cut -> SetMarkerColor(kRed);
  p1_amp2_vs_beam_X_cut -> SetLineColor(kRed);
  p1_amp2_vs_beam_X_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_ysel -> Draw("same");
  latexLabel2 -> Draw("same");
  c1 -> cd(2);
  gPad -> SetGridy();
  p1_amp2_vs_beam_Y -> SetTitle(Form(";beam Y (mm);#LT %s amp_{max} #GT (mV)",xLabel2_en.c_str()));
  p1_amp2_vs_beam_Y -> SetMinimum(cut_ampMin2);
  p1_amp2_vs_beam_Y -> SetMaximum(h_amp2_cut->GetMean()+2.*h_amp2_cut->GetRMS());
  p1_amp2_vs_beam_Y -> Draw();
  p1_amp2_vs_beam_Y_cut -> SetMarkerColor(kBlue);
  p1_amp2_vs_beam_Y_cut -> SetLineColor(kBlue);
  p1_amp2_vs_beam_Y_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_xsel -> Draw("same");
  latexLabel2 -> Draw("same");
  c1 -> cd(3);
  p2_amp2_vs_beam_Y_vs_X_cut -> SetTitle(Form(";beam X (mm);beam Y (mm);#LT %s amp_{max} #GT (mV)",xLabel2_en.c_str()));
  //gPad -> SetLogz();
  p2_amp2_vs_beam_Y_vs_X_cut -> SetMinimum(cut_ampMin2);
  p2_amp2_vs_beam_Y_vs_X_cut -> SetMaximum(h_amp2_cut->GetMean()+2.*h_amp2_cut->GetRMS());
  p2_amp2_vs_beam_Y_vs_X_cut -> Draw("colz");
  xBand_low -> Draw("same");
  xBand_hig -> Draw("same");
  yBand_lef -> Draw("same");
  yBand_rig -> Draw("same");
  latexLabel2 -> Draw("same");
  gPad -> Update();
  c1 -> Print(Form("%s/c__amplitudeProfile2__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_amp_beam_Y_vs_X","amplitude profile",1430,665);
  c1 -> Divide(2,1);
  c1 -> cd(1);
  p2_amp1_vs_beam_Y_vs_X -> SetTitle(Form(";beam X (mm);beam Y (mm);#LT %s amp_{max} #GT (mV)",xLabel1_en.c_str()));
  //gPad -> SetLogz();
  p2_amp1_vs_beam_Y_vs_X -> SetMinimum(0.);
  p2_amp1_vs_beam_Y_vs_X -> SetMaximum(h_amp1_cut->GetMean()+2.*h_amp1_cut->GetRMS());
  p2_amp1_vs_beam_Y_vs_X -> Draw("colz");
  if( isMCP0 ) ell_MCP -> Draw("same");
  else box_crystal -> Draw("same");
  latexLabel1 -> Draw("same");
  c1 -> cd(2);
  p2_amp2_vs_beam_Y_vs_X -> SetTitle(Form(";beam X (mm);beam Y (mm);#LT %s amp_{max} #GT (mV)",xLabel2_en.c_str()));
  //gPad -> SetLogz();
  p2_amp2_vs_beam_Y_vs_X -> SetMinimum(0.);
  p2_amp2_vs_beam_Y_vs_X -> SetMaximum(h_amp2_cut->GetMean()+2.*h_amp2_cut->GetRMS());
  p2_amp2_vs_beam_Y_vs_X -> Draw("colz");
  if( isMCP1 ) ell_MCP -> Draw("same");
  else box_crystal -> Draw("same");
  latexLabel2 -> Draw("same");
  gPad -> Update();
  c1 -> Print(Form("%s/c__amplitudeProfile__config%d.png",plotDir.c_str(),configuration));
  
  
  c1 = new TCanvas("c1_CTR_beam_X_Y","DeltaT profile",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  gPad -> SetGridy();
  p1_CTR_vs_beam_X -> SetMinimum(CTRMean-3.*CTRSigma);
  p1_CTR_vs_beam_X -> SetMaximum(CTRMean+2.*CTRSigma);
  p1_CTR_vs_beam_X -> SetTitle(";beam X (mm);#LT #Deltat #GT (ns)");
  p1_CTR_vs_beam_X -> Draw();
  p1_CTR_vs_beam_X_cut -> SetMarkerColor(kRed);
  p1_CTR_vs_beam_X_cut -> SetLineColor(kRed);
  p1_CTR_vs_beam_X_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_ysel -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(2);
  gPad -> SetGridy();
  p1_CTR_vs_beam_Y -> SetMinimum(CTRMean-3.*CTRSigma);
  p1_CTR_vs_beam_Y -> SetMaximum(CTRMean+2.*CTRSigma);
  p1_CTR_vs_beam_Y -> SetTitle(";beam Y (mm);#LT #Deltat #GT (ns)");
  p1_CTR_vs_beam_Y -> Draw();
  p1_CTR_vs_beam_Y_cut -> SetMarkerColor(kBlue);
  p1_CTR_vs_beam_Y_cut -> SetLineColor(kBlue);
  p1_CTR_vs_beam_Y_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_xsel -> Draw("same");
  latexLabel12 -> Draw("same");
  gPad -> Update();
  c1 -> cd(3);
  p2_CTR_vs_beam_Y_vs_X -> SetMinimum(CTRMean-3.*CTRSigma);
  p2_CTR_vs_beam_Y_vs_X -> SetMaximum(CTRMean+2.*CTRSigma);
  p2_CTR_vs_beam_Y_vs_X -> SetTitle(";beam X (mm);beam Y (mm);#LT #Deltat #GT (ns)");
  p2_CTR_vs_beam_Y_vs_X -> Draw("colz");
  xBand_low -> Draw("same");
  xBand_hig -> Draw("same");
  yBand_lef -> Draw("same");
  yBand_rig -> Draw("same");
  latexLabel12 -> Draw("same");
  gPad -> Update();
  c1 -> Print(Form("%s/c__deltaTProfile__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_ampCorr_beam_X_Y","amp-correctd DeltaT profile",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  gPad -> SetGridy();
  p1_CTR_ampCorr_vs_beam_X -> SetMinimum(CTRMean-3.*CTRSigma);
  p1_CTR_ampCorr_vs_beam_X -> SetMaximum(CTRMean+2.*CTRSigma);
  p1_CTR_ampCorr_vs_beam_X -> SetTitle(";beam X (mm);#LT amp-corrected #Deltat #GT (ns)");
  p1_CTR_ampCorr_vs_beam_X -> Draw();
  p1_CTR_ampCorr_vs_beam_X_cut -> SetMarkerColor(kRed);
  p1_CTR_ampCorr_vs_beam_X_cut -> SetLineColor(kRed);
  p1_CTR_ampCorr_vs_beam_X_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_ysel -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(2);
  gPad -> SetGridy();
  p1_CTR_ampCorr_vs_beam_Y -> SetMinimum(CTRMean-3.*CTRSigma);
  p1_CTR_ampCorr_vs_beam_Y -> SetMaximum(CTRMean+2.*CTRSigma);
  p1_CTR_ampCorr_vs_beam_Y -> SetTitle(";beam Y (mm);#LT amp-corrected #Deltat #GT (ns)");
  p1_CTR_ampCorr_vs_beam_Y -> Draw();
  p1_CTR_ampCorr_vs_beam_Y_cut -> SetMarkerColor(kBlue);
  p1_CTR_ampCorr_vs_beam_Y_cut -> SetLineColor(kBlue);
  p1_CTR_ampCorr_vs_beam_Y_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_xsel -> Draw("same");
  latexLabel12 -> Draw("same");
  gPad -> Update();
  c1 -> cd(3);
  p2_CTR_ampCorr_vs_beam_Y_vs_X -> SetMinimum(CTRMean-3.*CTRSigma);
  p2_CTR_ampCorr_vs_beam_Y_vs_X -> SetMaximum(CTRMean+2.*CTRSigma);
  p2_CTR_ampCorr_vs_beam_Y_vs_X -> SetTitle(";beam X (mm);beam Y (mm);#LT amp-corrected #Deltat #GT (ns)");
  p2_CTR_ampCorr_vs_beam_Y_vs_X -> Draw("colz");
  xBand_low -> Draw("same");
  xBand_hig -> Draw("same");
  yBand_lef -> Draw("same");
  yBand_rig -> Draw("same");
  latexLabel12 -> Draw("same");
  gPad -> Update();
  c1 -> Print(Form("%s/c__deltaTProfile_ampCorr__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_ampCorrEff_beam_X_Y","eff. amp-corrected DeltaT profile",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  gPad -> SetGridy();
  p1_CTR_ampCorrEff_vs_beam_X -> SetMinimum(CTRMean-3.*CTRSigma);
  p1_CTR_ampCorrEff_vs_beam_X -> SetMaximum(CTRMean+2.*CTRSigma);
  p1_CTR_ampCorrEff_vs_beam_X -> SetTitle(";beam X (mm);#LT eff. amp-corrected #Deltat #GT (ns)");
  p1_CTR_ampCorrEff_vs_beam_X -> Draw();
  p1_CTR_ampCorrEff_vs_beam_X_cut -> SetMarkerColor(kRed);
  p1_CTR_ampCorrEff_vs_beam_X_cut -> SetLineColor(kRed);
  p1_CTR_ampCorrEff_vs_beam_X_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_ysel -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(2);
  gPad -> SetGridy();
  p1_CTR_ampCorrEff_vs_beam_Y -> SetMinimum(CTRMean-3.*CTRSigma);
  p1_CTR_ampCorrEff_vs_beam_Y -> SetMaximum(CTRMean+2.*CTRSigma);
  p1_CTR_ampCorrEff_vs_beam_Y -> SetTitle(";beam Y (mm);#LT eff. amp-corrected #Deltat #GT (ns)");
  p1_CTR_ampCorrEff_vs_beam_Y -> Draw();
  p1_CTR_ampCorrEff_vs_beam_Y_cut -> SetMarkerColor(kBlue);
  p1_CTR_ampCorrEff_vs_beam_Y_cut -> SetLineColor(kBlue);
  p1_CTR_ampCorrEff_vs_beam_Y_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_xsel -> Draw("same");
  latexLabel12 -> Draw("same");
  gPad -> Update();
  c1 -> cd(3);
  p2_CTR_ampCorrEff_vs_beam_Y_vs_X -> SetMinimum(CTRMean-3.*CTRSigma);
  p2_CTR_ampCorrEff_vs_beam_Y_vs_X -> SetMaximum(CTRMean+2.*CTRSigma);
  p2_CTR_ampCorrEff_vs_beam_Y_vs_X -> SetTitle(";beam X (mm);beam Y (mm);#LT eff. amp-corrected #Deltat #GT (ns)");
  p2_CTR_ampCorrEff_vs_beam_Y_vs_X -> Draw("colz");
  xBand_low -> Draw("same");
  xBand_hig -> Draw("same");
  yBand_lef -> Draw("same");
  yBand_rig -> Draw("same");
  latexLabel12 -> Draw("same");
  gPad -> Update();
  c1 -> Print(Form("%s/c__deltaTProfile_ampCorrEff__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_ampCorrPerBin_beam_X_Y","per-bin amp-corrected DeltaT profile",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  gPad -> SetGridy();
  p1_CTR_ampCorrPerBin_vs_beam_X -> SetMinimum(CTRMean-3.*CTRSigma);
  p1_CTR_ampCorrPerBin_vs_beam_X -> SetMaximum(CTRMean+2.*CTRSigma);
  p1_CTR_ampCorrPerBin_vs_beam_X -> SetTitle(";beam X (mm);#LT per-bin amp-corrected #Deltat #GT (ns)");
  p1_CTR_ampCorrPerBin_vs_beam_X -> Draw();
  p1_CTR_ampCorrPerBin_vs_beam_X_cut -> SetMarkerColor(kRed);
  p1_CTR_ampCorrPerBin_vs_beam_X_cut -> SetLineColor(kRed);
  p1_CTR_ampCorrPerBin_vs_beam_X_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_ysel -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(2);
  gPad -> SetGridy();
  p1_CTR_ampCorrPerBin_vs_beam_Y -> SetMinimum(CTRMean-3.*CTRSigma);
  p1_CTR_ampCorrPerBin_vs_beam_Y -> SetMaximum(CTRMean+2.*CTRSigma);
  p1_CTR_ampCorrPerBin_vs_beam_Y -> SetTitle(";beam Y (mm);#LT per-bin amp-corrected #Deltat #GT (ns)");
  p1_CTR_ampCorrPerBin_vs_beam_Y -> Draw();
  p1_CTR_ampCorrPerBin_vs_beam_Y_cut -> SetMarkerColor(kBlue);
  p1_CTR_ampCorrPerBin_vs_beam_Y_cut -> SetLineColor(kBlue);
  p1_CTR_ampCorrPerBin_vs_beam_Y_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_xsel -> Draw("same");
  latexLabel12 -> Draw("same");
  gPad -> Update();
  c1 -> cd(3);
  p2_CTR_ampCorrPerBin_vs_beam_Y_vs_X -> SetMinimum(CTRMean-3.*CTRSigma);
  p2_CTR_ampCorrPerBin_vs_beam_Y_vs_X -> SetMaximum(CTRMean+2.*CTRSigma);
  p2_CTR_ampCorrPerBin_vs_beam_Y_vs_X -> SetTitle(";beam X (mm);beam Y (mm);#LT per-bin amp-corrected #Deltat #GT (ns)");
  p2_CTR_ampCorrPerBin_vs_beam_Y_vs_X -> Draw("colz");
  xBand_low -> Draw("same");
  xBand_hig -> Draw("same");
  yBand_lef -> Draw("same");
  yBand_rig -> Draw("same");
  latexLabel12 -> Draw("same");
  gPad -> Update();
  c1 -> Print(Form("%s/c__deltaTProfile_ampCorrPerBin__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_posCorr_beam_X_Y","pos-correctd DeltaT profile",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  gPad -> SetGridy();
  p1_CTR_posCorr_vs_beam_X -> SetMinimum(CTRMean-3.*CTRSigma);
  p1_CTR_posCorr_vs_beam_X -> SetMaximum(CTRMean+2.*CTRSigma);
  p1_CTR_posCorr_vs_beam_X -> SetTitle(";beam X (mm);#LT pos-corrected #Deltat #GT (ns)");
  p1_CTR_posCorr_vs_beam_X -> Draw();
  p1_CTR_posCorr_vs_beam_X_cut -> SetMarkerColor(kRed);
  p1_CTR_posCorr_vs_beam_X_cut -> SetLineColor(kRed);
  p1_CTR_posCorr_vs_beam_X_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_ysel -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(2);
  gPad -> SetGridy();
  p1_CTR_posCorr_vs_beam_Y -> SetMinimum(CTRMean-3.*CTRSigma);
  p1_CTR_posCorr_vs_beam_Y -> SetMaximum(CTRMean+2.*CTRSigma);
  p1_CTR_posCorr_vs_beam_Y -> SetTitle(";beam Y (mm);#LT pos-corrected #Deltat #GT (ns)");
  p1_CTR_posCorr_vs_beam_Y -> Draw();
  p1_CTR_posCorr_vs_beam_Y_cut -> SetMarkerColor(kBlue);
  p1_CTR_posCorr_vs_beam_Y_cut -> SetLineColor(kBlue);
  p1_CTR_posCorr_vs_beam_Y_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_xsel -> Draw("same");
  latexLabel12 -> Draw("same");
  gPad -> Update();
  c1 -> cd(3);
  p2_CTR_posCorr_vs_beam_Y_vs_X -> SetMinimum(CTRMean-3.*CTRSigma);
  p2_CTR_posCorr_vs_beam_Y_vs_X -> SetMaximum(CTRMean+2.*CTRSigma);
  p2_CTR_posCorr_vs_beam_Y_vs_X -> SetTitle(";beam X (mm);beam Y (mm);#LT pos-corrected #Deltat #GT (ns)");
  p2_CTR_posCorr_vs_beam_Y_vs_X -> Draw("colz");
  xBand_low -> Draw("same");
  xBand_hig -> Draw("same");
  yBand_lef -> Draw("same");
  yBand_rig -> Draw("same");
  latexLabel12 -> Draw("same");
  gPad -> Update();
  c1 -> Print(Form("%s/c__deltaTProfile_posCorr__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_posCorrAmpCorr_beam_X_Y","pos+amp-correctd DeltaT profile",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  gPad -> SetGridy();
  p1_CTR_posCorrAmpCorr_vs_beam_X -> SetMinimum(CTRMean-3.*CTRSigma);
  p1_CTR_posCorrAmpCorr_vs_beam_X -> SetMaximum(CTRMean+2.*CTRSigma);
  p1_CTR_posCorrAmpCorr_vs_beam_X -> SetTitle(";beam X (mm);#LT pos + amp-corrected #Deltat #GT (ns)");
  p1_CTR_posCorrAmpCorr_vs_beam_X -> Draw();
  p1_CTR_posCorrAmpCorr_vs_beam_X_cut -> SetMarkerColor(kRed);
  p1_CTR_posCorrAmpCorr_vs_beam_X_cut -> SetLineColor(kRed);
  p1_CTR_posCorrAmpCorr_vs_beam_X_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_ysel -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(2);
  gPad -> SetGridy();
  p1_CTR_posCorrAmpCorr_vs_beam_Y -> SetMinimum(CTRMean-3.*CTRSigma);
  p1_CTR_posCorrAmpCorr_vs_beam_Y -> SetMaximum(CTRMean+2.*CTRSigma);
  p1_CTR_posCorrAmpCorr_vs_beam_Y -> SetTitle(";beam Y (mm);#LT pos + amp-corrected #Deltat #GT (ns)");
  p1_CTR_posCorrAmpCorr_vs_beam_Y -> Draw();
  p1_CTR_posCorrAmpCorr_vs_beam_Y_cut -> SetMarkerColor(kBlue);
  p1_CTR_posCorrAmpCorr_vs_beam_Y_cut -> SetLineColor(kBlue);
  p1_CTR_posCorrAmpCorr_vs_beam_Y_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_xsel -> Draw("same");
  latexLabel12 -> Draw("same");
  gPad -> Update();
  c1 -> cd(3);
  p2_CTR_posCorrAmpCorr_vs_beam_Y_vs_X -> SetMinimum(CTRMean-3.*CTRSigma);
  p2_CTR_posCorrAmpCorr_vs_beam_Y_vs_X -> SetMaximum(CTRMean+2.*CTRSigma);
  p2_CTR_posCorrAmpCorr_vs_beam_Y_vs_X -> SetTitle(";beam X (mm);beam Y (mm);#LT pos + amp-corrected #Deltat #GT (ns)");
  p2_CTR_posCorrAmpCorr_vs_beam_Y_vs_X -> Draw("colz");
  xBand_low -> Draw("same");
  xBand_hig -> Draw("same");
  yBand_lef -> Draw("same");
  yBand_rig -> Draw("same");
  latexLabel12 -> Draw("same");
  gPad -> Update();
  c1 -> Print(Form("%s/c__deltaTProfile_posCorrAmpCorr__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_ampCorrEffPosCorr_beam_X_Y","eff. amp + pos-correctd DeltaT profile",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  gPad -> SetGridy();
  p1_CTR_ampCorrEffPosCorr_vs_beam_X -> SetMinimum(CTRMean-3.*CTRSigma);
  p1_CTR_ampCorrEffPosCorr_vs_beam_X -> SetMaximum(CTRMean+2.*CTRSigma);
  p1_CTR_ampCorrEffPosCorr_vs_beam_X -> SetTitle(";beam X (mm);#LT eff. amp + pos-corrected #Deltat #GT (ns)");
  p1_CTR_ampCorrEffPosCorr_vs_beam_X -> Draw();
  p1_CTR_ampCorrEffPosCorr_vs_beam_X_cut -> SetMarkerColor(kRed);
  p1_CTR_ampCorrEffPosCorr_vs_beam_X_cut -> SetLineColor(kRed);
  p1_CTR_ampCorrEffPosCorr_vs_beam_X_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_ysel -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(2);
  gPad -> SetGridy();
  p1_CTR_ampCorrEffPosCorr_vs_beam_Y -> SetMinimum(CTRMean-3.*CTRSigma);
  p1_CTR_ampCorrEffPosCorr_vs_beam_Y -> SetMaximum(CTRMean+2.*CTRSigma);
  p1_CTR_ampCorrEffPosCorr_vs_beam_Y -> SetTitle(";beam Y (mm);#LT eff. amp + pos-corrected #Deltat #GT (ns)");
  p1_CTR_ampCorrEffPosCorr_vs_beam_Y -> Draw();
  p1_CTR_ampCorrEffPosCorr_vs_beam_Y_cut -> SetMarkerColor(kBlue);
  p1_CTR_ampCorrEffPosCorr_vs_beam_Y_cut -> SetLineColor(kBlue);
  p1_CTR_ampCorrEffPosCorr_vs_beam_Y_cut -> Draw("same");
  latexLabel_all -> Draw("same");
  latexLabel_xsel -> Draw("same");
  latexLabel12 -> Draw("same");
  gPad -> Update();
  c1 -> cd(3);
  p2_CTR_ampCorrEffPosCorr_vs_beam_Y_vs_X -> SetMinimum(CTRMean-3.*CTRSigma);
  p2_CTR_ampCorrEffPosCorr_vs_beam_Y_vs_X -> SetMaximum(CTRMean+2.*CTRSigma);
  p2_CTR_ampCorrEffPosCorr_vs_beam_Y_vs_X -> SetTitle(";beam X (mm);beam Y (mm);#LT eff. amp + pos-corrected #Deltat #GT (ns)");
  p2_CTR_ampCorrEffPosCorr_vs_beam_Y_vs_X -> Draw("colz");
  xBand_low -> Draw("same");
  xBand_hig -> Draw("same");
  yBand_lef -> Draw("same");
  yBand_rig -> Draw("same");
  latexLabel12 -> Draw("same");
  gPad -> Update();
  c1 -> Print(Form("%s/c__deltaTProfile_ampCorrEffPosCorr__config%d.png",plotDir.c_str(),configuration));
  
  
  c1 = new TCanvas("c1_CTR_vs_amp","DeltaT vs amp",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  TH1F* hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT #Deltat #GT (ns)",xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_vs_amp1 -> Draw("same");
  p1_CTR_vs_amp1_center -> SetMarkerSize(0.5);
  p1_CTR_vs_amp1_center -> SetMarkerColor(kRed);
  p1_CTR_vs_amp1_center -> SetLineColor(kRed);
  p1_CTR_vs_amp1_center -> Draw("same");
  p1_CTR_vs_amp1_border -> SetMarkerSize(0.5);
  p1_CTR_vs_amp1_border -> SetMarkerColor(kBlue);
  p1_CTR_vs_amp1_border -> SetLineColor(kBlue);
  p1_CTR_vs_amp1_border -> Draw("same");
  if( !isMCP0 ) fitFunc_corrAmp -> Draw("same");
  latexLabel1 -> Draw("same");
  c1 -> cd(2);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT #Deltat #GT (ns)",xLabel2_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_vs_amp2 -> Draw("same");
  p1_CTR_vs_amp2_center -> SetMarkerSize(0.5);
  p1_CTR_vs_amp2_center -> SetMarkerColor(kRed);
  p1_CTR_vs_amp2_center -> SetLineColor(kRed);
  p1_CTR_vs_amp2_center -> Draw("same");
  p1_CTR_vs_amp2_border -> SetMarkerSize(0.5);
  p1_CTR_vs_amp2_border -> SetMarkerColor(kBlue);
  p1_CTR_vs_amp2_border -> SetLineColor(kBlue);
  p1_CTR_vs_amp2_border -> Draw("same");
  if( !isMCP1 ) fitFunc_corrAmp -> Draw("same");
  latexLabel2 -> Draw("same");
  c1 -> cd(3);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,5.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} / %s amp_{max} (mV);#LT #Deltat #GT (ns)",xLabel2_en.c_str(),xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_vs_ampRatio -> Draw("same");
  p1_CTR_vs_ampRatio_center -> SetMarkerSize(0.5);
  p1_CTR_vs_ampRatio_center -> SetMarkerColor(kRed);
  p1_CTR_vs_ampRatio_center -> SetLineColor(kRed);
  p1_CTR_vs_ampRatio_center -> Draw("same");
  p1_CTR_vs_ampRatio_border -> SetMarkerSize(0.5);
  p1_CTR_vs_ampRatio_border -> SetMarkerColor(kBlue);
  p1_CTR_vs_ampRatio_border -> SetLineColor(kBlue);
  p1_CTR_vs_ampRatio_border -> Draw("same");
  if( !isMCP0 && !isMCP1 ) fitFunc_corrAmp -> Draw("same");
  latexLabel2 -> Draw("same");  
  c1 -> Print(Form("%s/c__deltaT_vs_amp__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_ampCorr_vs_amp","amp-corrected DeltaT vs amp",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT amp-corrected #Deltat #GT (ns)",xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_ampCorr_vs_amp1 -> Draw("same");
  latexLabel1 -> Draw("same");
  c1 -> cd(2);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT amp-corrected #Deltat #GT (ns)",xLabel2_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_ampCorr_vs_amp2 -> Draw("same");
  latexLabel2 -> Draw("same");
  c1 -> cd(3);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,5.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} / %s amp_{max} (mV);#LT amp-corrected #Deltat #GT (ns)",xLabel2_en.c_str(),xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_ampCorr_vs_ampRatio -> Draw("same");
  latexLabel2 -> Draw("same");  
  c1 -> Print(Form("%s/c__deltaT_ampCorr_vs_amp__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_ampCorrEff_vs_amp","eff. amp-corrected DeltaT vs amp",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT eff. amp-corrected #Deltat #GT (ns)",xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_ampCorrEff_vs_amp1 -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(2);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT eff. amp-corrected #Deltat #GT (ns)",xLabel2_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_ampCorrEff_vs_amp2 -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(3);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,5.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} / %s amp_{max} (mV);#LT eff. amp-corrected #Deltat #GT (ns)",xLabel2_en.c_str(),xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_ampCorrEff_vs_ampRatio -> Draw("same");
  latexLabel12 -> Draw("same");  
  c1 -> Print(Form("%s/c__deltaT_ampCorrEff_vs_amp__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_ampCorrPerBin_vs_amp","per-bin amp-corrected DeltaT vs amp",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT per-bin amp-corrected #Deltat #GT (ns)",xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_ampCorrPerBin_vs_amp1 -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(2);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT per-bin amp-corrected #Deltat #GT (ns)",xLabel2_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_ampCorrPerBin_vs_amp2 -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(3);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,5.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} / %s amp_{max} (mV);#LT per-bin amp-corrected #Deltat #GT (ns)",xLabel2_en.c_str(),xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_ampCorrPerBin_vs_ampRatio -> Draw("same");
  latexLabel12 -> Draw("same");  
  c1 -> Print(Form("%s/c__deltaT_ampCorrPerBin_vs_amp__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_posCorr_vs_amp","pos-corrected DeltaT vs amp",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT pos-corrected #Deltat #GT (ns)",xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_posCorr_vs_amp1 -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(2);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT pos-corrected #Deltat #GT (ns)",xLabel2_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_posCorr_vs_amp2 -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(3);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,5.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} / %s amp_{max} (mV);#LT pos-corrected #Deltat #GT (ns)",xLabel2_en.c_str(),xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_posCorr_vs_ampRatio -> Draw("same");
  latexLabel12 -> Draw("same");  
  c1 -> Print(Form("%s/c__deltaT_posCorr_vs_amp__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_posCorrAmpCorr_vs_amp","pos + amp-corrected DeltaT vs amp",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT pos + amp-corrected #Deltat #GT (ns)",xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_posCorrAmpCorr_vs_amp1 -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(2);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT pos + amp-corrected #Deltat #GT (ns)",xLabel2_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_posCorrAmpCorr_vs_amp2 -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(3);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,5.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} / %s amp_{max} (mV);#LT pos + amp-corrected #Deltat #GT (ns)",xLabel2_en.c_str(),xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_posCorrAmpCorr_vs_ampRatio -> Draw("same");
  latexLabel12 -> Draw("same");  
  c1 -> Print(Form("%s/c__deltaT_posCorrAmpCorr_vs_amp__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_ampCorrEffPosCorr_vs_amp","eff. amp + pos-corrected DeltaT vs amp",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT eff. amp + pos-corrected #Deltat #GT (ns)",xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_ampCorrEffPosCorr_vs_amp1 -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(2);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT eff. amp + pos-corrected #Deltat #GT (ns)",xLabel2_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_ampCorrEffPosCorr_vs_amp2 -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> cd(3);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,5.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} / %s amp_{max} (mV);#LT eff. amp + pos-corrected #Deltat #GT (ns)",xLabel2_en.c_str(),xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_ampCorrEffPosCorr_vs_ampRatio -> Draw("same");
  latexLabel12 -> Draw("same");  
  c1 -> Print(Form("%s/c__deltaT_ampCorrEffPosCorr_vs_amp__config%d.png",plotDir.c_str(),configuration));
  
  
  c1 = new TCanvas("c1_CTR_chess","CTR chess");
  c1 -> cd();
  float minCTR = 999.;
  float maxCTR = -999.;
  float avgCTR = 0.;
  float nEvents = 0.;
  for(int binX = 0; binX < nBinsX; ++binX)
    for(int binY = 0; binY < nBinsY; ++binY)
    {
      std::string label(Form("%d-%d",binX,binY));
      if( map_h_CTR[label]->GetEntries() < 100 ) continue;
      
      TH1F* histo = map_h_CTR[label];
      float* vals = new float[4];
      FindSmallestInterval(vals,histo,0.68,true); 
      
      float min = vals[2];
      float max = vals[3];
      float delta = max-min;
      float sigma = 0.5*delta;
      float effSigma = sigma;      
      float val;
      if( (isMCP0 && !isMCP1) || (!isMCP0 && isMCP1) ) val = sqrt(effSigma*1000.*effSigma*1000.-MCPIntrinsic*1000.*MCPIntrinsic*1000.);
      else val = effSigma*1000./sqrt(2);
      avgCTR += val*histo->GetEntries();
      nEvents += histo->GetEntries();
      
      h2_CTR_chess -> SetBinContent( binX+1,binY+1,val );
      
      if( val < minCTR ) minCTR = val;
      if( val > maxCTR ) maxCTR = val;
    }
  h2_CTR_chess -> SetTitle(";beam X (mm);beam Y (mm);time resolution (ps)");
  h2_CTR_chess -> SetMinimum(minCTR-0.05*(maxCTR-minCTR));
  h2_CTR_chess -> SetMaximum(maxCTR+0.05*(maxCTR-minCTR));
  h2_CTR_chess -> Draw("colz,text");
  TLatex* latexLabelCTR = new TLatex(0.13,0.90,Form("#LT time res. #GT = %.1f ps",avgCTR/nEvents));
  latexLabelCTR -> SetNDC();
  latexLabelCTR -> SetTextFont(42);
  latexLabelCTR -> SetTextSize(0.04);
  latexLabelCTR -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> Print(Form("%s/c__CTR_chess__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_chess_ampCorr","CTR chess_ampCorr");
  c1 -> cd();
  minCTR = 999.;
  maxCTR = -999.;
  avgCTR = 0.;
  nEvents = 0.;
  for(int binX = 0; binX < nBinsX; ++binX)
    for(int binY = 0; binY < nBinsY; ++binY)
    {
      std::string label(Form("%d-%d",binX,binY));
      if( map_h_CTR_ampCorr[label]->GetEntries() < 100 ) continue;
      
      TH1F* histo = map_h_CTR_ampCorr[label];
      float* vals = new float[4];
      FindSmallestInterval(vals,histo,0.68,true); 
      
      float min = vals[2];
      float max = vals[3];
      float delta = max-min;
      float sigma = 0.5*delta;
      float effSigma = sigma;      
      float val;
      if( (isMCP0 && !isMCP1) || (!isMCP0 && isMCP1) ) val = sqrt(effSigma*1000.*effSigma*1000.-MCPIntrinsic*1000.*MCPIntrinsic*1000.);
      else val = effSigma*1000./sqrt(2);
      avgCTR += val*histo->GetEntries();
      nEvents += histo->GetEntries();
      
      h2_CTR_chess_ampCorr -> SetBinContent( binX+1,binY+1,val );
      
      if( val < minCTR ) minCTR = val;
      if( val > maxCTR ) maxCTR = val;
    }
  h2_CTR_chess_ampCorr -> SetTitle(";beam X (mm);beam Y (mm);amp-corrected time resolution (ps)");
  h2_CTR_chess_ampCorr -> SetMinimum(minCTR-0.05*(maxCTR-minCTR));
  h2_CTR_chess_ampCorr -> SetMaximum(maxCTR+0.05*(maxCTR-minCTR));
  h2_CTR_chess_ampCorr -> Draw("colz,text");
  latexLabelCTR = new TLatex(0.13,0.90,Form("#LT time res. #GT = %.1f ps",avgCTR/nEvents));
  latexLabelCTR -> SetNDC();
  latexLabelCTR -> SetTextFont(42);
  latexLabelCTR -> SetTextSize(0.04);
  latexLabelCTR -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> Print(Form("%s/c__CTR_chess_ampCorr__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_chess_ampCorrEff","CTR chess_ampCorrEff");
  c1 -> cd();
  minCTR = 999.;
  maxCTR = -999.;
  avgCTR = 0.;
  nEvents = 0.;
  for(int binX = 0; binX < nBinsX; ++binX)
    for(int binY = 0; binY < nBinsY; ++binY)
    {
      std::string label(Form("%d-%d",binX,binY));
      if( map_h_CTR_ampCorrEff[label]->GetEntries() < 100 ) continue;
      
      TH1F* histo = map_h_CTR_ampCorrEff[label];
      float* vals = new float[4];
      FindSmallestInterval(vals,histo,0.68,true); 
      
      float min = vals[2];
      float max = vals[3];
      float delta = max-min;
      float sigma = 0.5*delta;
      float effSigma = sigma;      
      float val;
      if( (isMCP0 && !isMCP1) || (!isMCP0 && isMCP1) ) val = sqrt(effSigma*1000.*effSigma*1000.-MCPIntrinsic*1000.*MCPIntrinsic*1000.);
      else val = effSigma*1000./sqrt(2);
      avgCTR += val*histo->GetEntries();
      nEvents += histo->GetEntries();
      
      h2_CTR_chess_ampCorrEff -> SetBinContent( binX+1,binY+1,val );
      
      if( val < minCTR ) minCTR = val;
      if( val > maxCTR ) maxCTR = val;
    }
  h2_CTR_chess_ampCorrEff -> SetTitle(";beam X (mm);beam Y (mm);eff. amp-corrected time resolution (ps)");
  h2_CTR_chess_ampCorrEff -> SetMinimum(minCTR-0.05*(maxCTR-minCTR));
  h2_CTR_chess_ampCorrEff -> SetMaximum(maxCTR+0.05*(maxCTR-minCTR));
  h2_CTR_chess_ampCorrEff -> Draw("colz,text");
  latexLabelCTR = new TLatex(0.13,0.90,Form("#LT time res. #GT = %.1f ps",avgCTR/nEvents));
  latexLabelCTR -> SetNDC();
  latexLabelCTR -> SetTextFont(42);
  latexLabelCTR -> SetTextSize(0.04);
  latexLabelCTR -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> Print(Form("%s/c__CTR_chess_ampCorrEff__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_chess_ampCorrPerBin","CTR chess_ampCorrPerBin");
  c1 -> cd();
  minCTR = 999.;
  maxCTR = -999.;
  avgCTR = 0.;
  nEvents = 0.;
  for(int binX = 0; binX < nBinsX; ++binX)
    for(int binY = 0; binY < nBinsY; ++binY)
    {
      std::string label(Form("%d-%d",binX,binY));
      if( map_h_CTR_ampCorrPerBin[label]->GetEntries() < 100 ) continue;
      
      TH1F* histo = map_h_CTR_ampCorrPerBin[label];
      float* vals = new float[4];
      FindSmallestInterval(vals,histo,0.68,true); 
      
      float min = vals[2];
      float max = vals[3];
      float delta = max-min;
      float sigma = 0.5*delta;
      float effSigma = sigma;      
      float val;
      if( (isMCP0 && !isMCP1) || (!isMCP0 && isMCP1) ) val = sqrt(effSigma*1000.*effSigma*1000.-MCPIntrinsic*1000.*MCPIntrinsic*1000.);
      else val = effSigma*1000./sqrt(2);
      avgCTR += val*histo->GetEntries();
      nEvents += histo->GetEntries();
      
      h2_CTR_chess_ampCorrPerBin -> SetBinContent( binX+1,binY+1,val );
      
      if( val < minCTR ) minCTR = val;
      if( val > maxCTR ) maxCTR = val;
    }
  h2_CTR_chess_ampCorrPerBin -> SetTitle(";beam X (mm);beam Y (mm);per-bin amp-corrected time resolution (ps)");
  h2_CTR_chess_ampCorrPerBin -> SetMinimum(minCTR-0.05*(maxCTR-minCTR));
  h2_CTR_chess_ampCorrPerBin -> SetMaximum(maxCTR+0.05*(maxCTR-minCTR));
  h2_CTR_chess_ampCorrPerBin -> Draw("colz,text");
  latexLabelCTR = new TLatex(0.13,0.90,Form("#LT time res. #GT = %.1f ps",avgCTR/nEvents));
  latexLabelCTR -> SetNDC();
  latexLabelCTR -> SetTextFont(42);
  latexLabelCTR -> SetTextSize(0.04);
  latexLabelCTR -> Draw("same");
  latexLabel12 -> Draw("same");
  c1 -> Print(Form("%s/c__CTR_chess_ampCorrPerBin__config%d.png",plotDir.c_str(),configuration));
  
  
  c1 = new TCanvas("c1_CTR","CTR");
  c1 -> cd();
  drawCTRPlot(h_CTR,c1,rebin,isMCP0,isMCP1,MCPIntrinsic,xLabel1_time,xLabel2_time,"",latexLabel12,h_CTR_center,h_CTR_border);
  c1 -> Print(Form("%s/c__CTR__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_ampCorr","CTR_ampCorr");
  c1 -> cd();
  drawCTRPlot(h_CTR_ampCorr,c1,rebin,isMCP0,isMCP1,MCPIntrinsic,xLabel1_time,xLabel2_time,"amp-corrected ",latexLabel12,h_CTR_ampCorr_center,h_CTR_ampCorr_border);
  c1 -> Print(Form("%s/c__CTR_ampCorr__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_ampCorrEff","CTR_ampCorrEff");
  c1 -> cd();
  drawCTRPlot(h_CTR_ampCorrEff,c1,rebin,isMCP0,isMCP1,MCPIntrinsic,xLabel1_time,xLabel2_time,"eff. amp-corrected ",latexLabel12,h_CTR_ampCorrEff_center,h_CTR_ampCorrEff_border);
  c1 -> Print(Form("%s/c__CTR_ampCorrEff__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_ampCorrPerBin","CTR_ampCorrPerBin");
  c1 -> cd();
  drawCTRPlot(h_CTR_ampCorrPerBin,c1,rebin,isMCP0,isMCP1,MCPIntrinsic,xLabel1_time,xLabel2_time,"per-bin amp-corrected ",latexLabel12,h_CTR_ampCorrPerBin_center,h_CTR_ampCorrPerBin_border);
  c1 -> Print(Form("%s/c__CTR_ampCorrPerBin__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_posCorr","CTR_posCorr");
  c1 -> cd();
  drawCTRPlot(h_CTR_posCorr,c1,rebin,isMCP0,isMCP1,MCPIntrinsic,xLabel1_time,xLabel2_time,"pos-corrected ",latexLabel12,h_CTR_posCorr_center,h_CTR_posCorr_border);
  c1 -> Print(Form("%s/c__CTR_posCorr__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_posCorrAmpCorr","CTR_posCorrAmpCorr");
  c1 -> cd();
  drawCTRPlot(h_CTR_posCorrAmpCorr,c1,rebin,isMCP0,isMCP1,MCPIntrinsic,xLabel1_time,xLabel2_time,"pos + amp-corrected ",latexLabel12,h_CTR_posCorrAmpCorr_center,h_CTR_posCorrAmpCorr_border);
  c1 -> Print(Form("%s/c__CTR_posCorrAmpCorr__config%d.png",plotDir.c_str(),configuration));
  
  c1 = new TCanvas("c1_CTR_ampCorrEffPosCorr","CTR_ampCorrEffPosCorr");
  c1 -> cd();
  drawCTRPlot(h_CTR_ampCorrEffPosCorr,c1,rebin,isMCP0,isMCP1,MCPIntrinsic,xLabel1_time,xLabel2_time,"eff. amp + pos-corrected ",latexLabel12,h_CTR_ampCorrEffPosCorr_center,h_CTR_ampCorrEffPosCorr_border);
  c1 -> Print(Form("%s/c__CTR_ampCorrEffPosCorr__config%d.png",plotDir.c_str(),configuration));
  
  
  if( popupPlots ) theApp -> Run();
  return 0;
}

//  LocalWords:  histograms
