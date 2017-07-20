

/*
SMO_250517_100746_736132.root

SMO_250517_101700_736134.root

SMO_250517_101836_736135.root

SMO_250517_102024_736136.root

SMO_250517_102034_736137.root

SMO_250517_102137_736141.root

SMO_250517_102156_736142.root

SMO_250517_102212_736145.root

SMO_250517_102225_736146.root

SMO_250517_102331_736147.root

SMO_250517_102442_736150.root

SMO_250517_102514_736151.root

SMO_250517_102646_736153.root

SMO_250517_102744_736154.root

SMO_250517_102951_736174.root

SMO_250517_103032_736178.root

SMO_250517_103038_736179.root

SMO_250517_103047_736180.root

SMO_250517_103054_736181.root

SMO_250517_103105_736182.root

SMO_250517_103138_736183.root

SMO_250517_103150_736184.root
*/

void Fitter(){

  //  FitACase(240517, 184321,736185);

  //  FitACaseT1T0(250517, 0, 736185, "T1");
    FitACaseT1T0(250517, 101700, 736134, "T1");

  FitACaseT1T0(250517,100352,736110, "T1");
  
  FitACaseT1T0(250517,101836,736135, "T1");

FitACaseT1T0(250517,102024,736136, "T1");

FitACaseT1T0(250517,102034,736137, "T1");

FitACaseT1T0(250517,102137,736141, "T1");

FitACaseT1T0(250517,102156,736142, "T1");

FitACaseT1T0(250517,102212,736145, "T1");

FitACaseT1T0(250517,102225,736146, "T1");

FitACaseT1T0(250517,102331,736147, "T1");

 FitACaseT1T0(250517,102442,736150, "T1");


    FitACaseT1T0(250517, 101700, 736134, "T2");

  FitACaseT1T0(250517,100352,736110, "T2");
  
  FitACaseT1T0(250517,101836,736135, "T2");

FitACaseT1T0(250517,102024,736136, "T2");

FitACaseT1T0(250517,102034,736137, "T2");

FitACaseT1T0(250517,102137,736141, "T2");

FitACaseT1T0(250517,102156,736142, "T2");

FitACaseT1T0(250517,102212,736145, "T2");

FitACaseT1T0(250517,102225,736146, "T2");

FitACaseT1T0(250517,102331,736147, "T2");

 FitACaseT1T0(250517,102442,736150, "T2");
  

 
  
  //  FitACase(102034, 736137);
  //  FitACase(102137, 736141);
  //FitACase(100746,736132);
  /*
  FitACase(101700,736134);

  FitACase(100352,736110);
  
  FitACase(101836,736135);

FitACase(102024,736136);

FitACase(102034,736137);

FitACase(102137,736141);

FitACase(102156,736142);

FitACase(102212,736145);

FitACase(102225,736146);

FitACase(102331,736147);

FitACase(102442,736150);
  */
  /*
FitACase(102514,736151);

FitACase(102646,736153);

FitACase(102744,736154);

FitACase(102951,736174);

FitACase(103032,736178);

FitACase(103038,736179);

FitACase(103047,736180);

FitACase(103054,736181);

FitACase(103105,736182);

FitACase(103138,736183);

FitACase(103150,736184);
  */
  /*
PrintMultiplicity(103054,736181);

PrintMultiplicity(103138,736183);

PrintMultiplicity(103150,736184); 

PrintMultiplicity(103105,736182);
 
PrintMultiplicity(103047,736180);
  */








  
}
void PrintMultiplicity(int database, int run) {

  TFile *_file0 = TFile::Open(Form("/data/NAS/RPCH4/SMO/SMO_%.6d_%d.root", database, run));

  cout << "Run " << run << endl;

  TH1F* ClusterSize = (TH1F*) _file0->Get("Clusters/Cluster_Multiplicity_Side0_2;1");
  cout << "Side 0 1.4/1.4: " << ClusterSize->GetRMS() << endl;

  TH1F* ClusterSize = (TH1F*) _file0->Get("Clusters/Cluster_Multiplicity_Side1_2;1");
  cout << "Side 1 1.4/1.4: " << ClusterSize->GetRMS() << endl;

  TH1F* ClusterSize = (TH1F*) _file0->Get("Clusters/Cluster_Multiplicity_Both_Side_2;1");
  cout << " ========== Both Side 1.4/1.4: " << ClusterSize->GetRMS()/2 << endl;

  
  TH1F* ClusterSize = (TH1F*) _file0->Get("Clusters/Cluster_Multiplicity_Side0_1;1");
  cout << "Side 0 1.6/1.6: " << ClusterSize->GetRMS() << endl;

  TH1F* ClusterSize = (TH1F*) _file0->Get("Clusters/Cluster_Multiplicity_Side1_1;1");
  cout << "Side 1 1.6/1.6: " << ClusterSize->GetRMS() << endl;


  TH1F* ClusterSize = (TH1F*) _file0->Get("Clusters/Cluster_Multiplicity_Both_Side_1;1");
  cout << " ========== Both Side 1.6/1.6: " << ClusterSize->GetRMS()/2 << endl;
}

void FitACase(int date, int database, int run) {
  //  TFile *_file0 = TFile::Open("/data/NAS/RPCH4/SMO/SMO_250517_083830_735821.root");

  //  int database = 84435, run = 735824;
  
  TFile *_file0 = TFile::Open(Form("/data/NAS/RPCH4/SMO/SMO_%d_%.6d_%d.root", date, database, run));
  


  TCanvas* Canvas = new TCanvas("Canvas", "Canvas", 1600, 1600);
  Canvas->Divide(4, 4);

  TCanvas* Canvas_zoom = new TCanvas("Canvas_zoom", "Canvas_zoom", 500, 500);

  
  for (int i = 0; i < 16; i++){

    Canvas->cd(i+1);
    
    TH1F* T2mT1 = (TH1F*) _file0->Get(Form("T1-T2_onehit_2%.2d;1", 8+i));

    if (i == 13) {

      TFile *_file1 = new TFile(Form("TimingFitsRun_%d_histos.root", run), "RECREATE");
      _file1->cd();
      T2mT1->Write("DeltaT");
     _file1->Close();

    }
    

    T2mT1->Rebin();
    //    T2mT1->Rebin();
    T2mT1->SetStats(0);
    //T2mT1->SetOptFit(1);
    T2mT1->Draw();

    T2mT1->GetXaxis()->SetRangeUser(-15, 15);

    cout << "Menaq = " << T2mT1->GetMean() << " get RMS = " << T2mT1->GetRMS() << endl;
    double mean = T2mT1->GetMean(), rms = T2mT1->GetRMS();

    
    TF1* Signal = new TF1("Signal", "[0]*[1]*TMath::Gaus(x, [2], [3],1) + (1-[0])*[1]*TMath::Gaus(x, [2], [4],1)", mean-3, mean+5);
    Signal->SetParNames("Narrow Frac.", "Norm.", "Mean", "Resolution", "Sigma outlayers");
 


    T2mT1->SetTitle(Form("HPL 1.4/1.4 mm chamber, PETIROC electronics, strip %d; T2-T1 (ns); ",i+8));
    int min =  T2mT1->FindBin(mean - 4*rms);
    int max =  T2mT1->FindBin(mean + 4*rms);

    cout << "min = " << min << " max = " << max << endl;

    Signal->SetParameters(0.6, T2mT1->Integral(min, max), T2mT1->GetMean(), 0.2,  T2mT1->GetRMS());
    
    //Signal->FixParameter(1, T2mT1->Integral(min, max));
    
    T2mT1->Fit(Signal, "", "", mean-3, mean+5);

   TLatex latex;
   latex.SetTextSize(0.06);
   latex.SetTextAlign(13);  //align at top
   latex.DrawLatex(-13,T2mT1->GetMaximum()/3,Form("#mu = %.2f ns", Signal->GetParameter(2)));
   latex.DrawLatex(-13,T2mT1->GetMaximum()/3.9,Form("#sigma = %.3f ns", Signal->GetParameter(3)));
   //   latex.DrawLatex(-13,T2mT1->GetMaximum()/3.9,Form("#sigma L = %.2f ns", Signal->GetParameter(4)));


   if (i == 13) {
     Canvas_zoom->cd();
     T2mT1->Draw();

     T2mT1->SetMaximum(1.2*T2mT1->GetMaximum());
     
     latex.DrawLatex(-13,T2mT1->GetMaximum()/3,Form("#mu = %.2f ns", Signal->GetParameter(2)));
     latex.DrawLatex(-13,T2mT1->GetMaximum()/3.9,Form("#sigma = %.3f ns", Signal->GetParameter(3)));



     
   }
   
  }

  Canvas->SaveAs(Form("TimingFitsRun_%d.png", run));
  Canvas->SaveAs(Form("TimingFitsRun_%d.pdf", run));

  Canvas_zoom->SaveAs(Form("TimingFitsRun_zoom_%d.png", run));
  Canvas_zoom->SaveAs(Form("TimingFitsRun_zoom_%d.pdf", run));

  Canvas_zoom->SaveAs(Form("TimingFitsRun_zoom_%d.C", run));
  Canvas_zoom->SaveAs(Form("TimingFitsRun_zoom_%d.root", run));
  
  
}


void FitACaseT1T0(int date, int database, int run, char* time) {
  //  TFile *_file0 = TFile::Open("/data/NAS/RPCH4/SMO/SMO_250517_083830_735821.root");

  //  int database = 84435, run = 735824;
  
  TFile *_file0 = TFile::Open(Form("/data/NAS/RPCH4/SMO/SMO_%d_%.6d_%d.root", date, database, run));
  


  TCanvas* Canvas = new TCanvas("Canvas", "Canvas", 1600, 1600);
  Canvas->Divide(4, 4);

  TCanvas* Canvas_zoom = new TCanvas("Canvas_zoom", "Canvas_zoom", 500, 500);

  string stime(time);
  stime = stime + "-T0;1";
  
  TDirectory* T1T0 = _file0->Get(stime.c_str());
  
  for (int i = 0; i < 16; i++){

    Canvas->cd(i+1);

    
    
    TH1F* T2mT1 = (TH1F*) T1T0->Get(Form("%s-T0_2%.2d;1", time, 8+i));

    if (i == 13) {

      TFile *_file1 = new TFile(Form("TimingT1T0_%d_histos.root", run), "RECREATE");
      _file1->cd();
      T2mT1->Write("DeltaT");
     _file1->Close();

    }
    
    T2mT1->Rebin();
    T2mT1->Rebin();
    T2mT1->Rebin();
    T2mT1->SetStats(0);
    //T2mT1->SetOptFit(1);
    T2mT1->Draw();

    T2mT1->GetXaxis()->SetRangeUser(-630, -614);

    cout << "Menaq = " << T2mT1->GetMean() << " get RMS = " << T2mT1->GetRMS() << endl;
    //    double mean = -624, rms = 2;
    double mean = T2mT1->GetMean(), rms = T2mT1->GetRMS();

    
    TF1* Signal = new TF1("Signal", "[0]*TMath::Gaus(x, [1], [2],1)", -630, -616);
    Signal->SetParNames("Norm.", "Mean", "Resolution");
 


    T2mT1->SetTitle(Form("HPL 1.4/1.4 mm chamber, PETIROC electronics, strip %d; %s-T0 (ns); ",i+8, time));
    int min =  T2mT1->FindBin(mean - 3*rms);
    int max =  T2mT1->FindBin(mean + 3*rms);

    cout << "min = " << min << " max = " << max << endl;

    Signal->SetParameters(T2mT1->Integral(min, max), mean, rms);
    
    //Signal->FixParameter(1, T2mT1->Integral(min, max));
    
    T2mT1->Fit(Signal, "", "", mean - 3*rms, mean+3*rms);

   TLatex latex;
   latex.SetTextSize(0.05);
   latex.SetTextAlign(13);  //align at top
   latex.DrawLatex(mean-3,T2mT1->GetMaximum()/3,Form("#mu = %.0f ns", Signal->GetParameter(1)));
   latex.DrawLatex(mean-3,T2mT1->GetMaximum()/3.9,Form("#sigma = %.1f ns", Signal->GetParameter(2)));
   //   latex.DrawLatex(-13,T2mT1->GetMaximum()/3.9,Form("#sigma L = %.2f ns", Signal->GetParameter(4)));


   if (i == 13) {
     Canvas_zoom->cd();
     T2mT1->Draw();

     T2mT1->SetMaximum(1.2*T2mT1->GetMaximum());
     
     latex.DrawLatex(mean-2,T2mT1->GetMaximum()/3,Form("#mu = %.2f ns", Signal->GetParameter(2)));
     latex.DrawLatex(mean-2,T2mT1->GetMaximum()/3.9,Form("#sigma = %.3f ns", Signal->GetParameter(3)));



     
   }
   
  }

  Canvas->SaveAs(Form("TimingFitsRun%sT0_%d.png", time, run));
  Canvas->SaveAs(Form("TimingFitsRun%sT0_%d.pdf", time, run));

  Canvas_zoom->SaveAs(Form("TimingFitsRun%sT0_zoom_%d.png", time, run));
  Canvas_zoom->SaveAs(Form("TimingFitsRun%sT0_zoom_%d.pdf", time, run));

  Canvas_zoom->SaveAs(Form("TimingFitsRun%sT0_zoom_%d.C", time, run));
  Canvas_zoom->SaveAs(Form("TimingFitsRun%sT0_zoom_%d.root", time, run));
  
  
}
