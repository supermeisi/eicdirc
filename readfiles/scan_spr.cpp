#include <TFile.h>
#include <TTree.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TAxis.h>
#include <TStyle.h>
#include <TGaxis.h>
#include <TLegend.h>
#include <iostream>
#include <vector>

void SetupCustomStyle() {
    TStyle* customStyle = new TStyle("CustomStyle", "Custom Style for Plots");

    customStyle->SetCanvasColor(0);
    customStyle->SetPadColor(0);
    customStyle->SetFrameFillColor(0);
    customStyle->SetHistLineWidth(2);

    customStyle->SetLabelFont(132, "x");
    customStyle->SetLabelFont(132, "y");
    customStyle->SetLabelFont(132, "z");
    customStyle->SetLabelSize(0.04, "x");
    customStyle->SetLabelSize(0.04, "y");
    customStyle->SetLabelSize(0.04, "z");
    customStyle->SetTitleFont(130, "x");
    customStyle->SetTitleFont(130, "y");
    customStyle->SetTitleFont(130, "z");
    customStyle->SetTitleSize(0.05, "x");
    customStyle->SetTitleSize(0.05, "y");
    customStyle->SetTitleSize(0.05, "z");

    customStyle->SetLegendFont(132);
    customStyle->SetLegendTextSize(0.04);

    TGaxis::SetMaxDigits(3);
    TGaxis::SetExponentOffset(0.09, -0.09, "y");

    customStyle->SetPadGridX(true);
    customStyle->SetPadGridY(true);
    customStyle->SetGridWidth(1);
    customStyle->SetGridColor(kGray+2);
    customStyle->SetGridStyle(2);
    
    gROOT->SetStyle("CustomStyle");
    gStyle->cd();
    gStyle->SetPalette(84);
    gStyle->SetNumberContours(99);

    std::cout << "Custom style applied globally." << std::endl;
}

void scan_spr() {
    SetupCustomStyle();

    std::vector<double> misalignment_values;
    std::vector<double> sep_gr_values;
    std::vector<double> sep_gr_errors;

for (int i = 0; i < 100; i++) {  // Increase to 100 iterations
  //double misalignment = i * 0.00009;  // Adjust step size to fit in [0, 0.009]
  double misalignment = i * 0.001; //offset
    misalignment_values.push_back(misalignment);
       std::string filename = Form("/home/afafwasili/Datat_Muu/onebarlens/shiftx_%d/reco.root",i);
    //   std::string filename = Form("data30shifty/shifty_%d/reco.root", i);
    //         std::string filename = Form("data30shiftx/shiftx_%d/reco.root", i);
    TFile *file = TFile::Open(filename.c_str());

        if (!file || file->IsZombie()) {
            std::cerr << "Error: Cannot open " << filename << std::endl;
            sep_gr_values.push_back(0);
            sep_gr_errors.push_back(0);
            continue;
        }

        TTree *tree = (TTree*) file->Get("reco;1");
        if (!tree) {
            std::cerr << "Error: No tree found in " << filename << std::endl;
            sep_gr_values.push_back(0);
            sep_gr_errors.push_back(0);
            continue;
        }

        double sep_gr, sep_gr_err;
        tree->SetBranchAddress("sep_gr", &sep_gr);
        tree->SetBranchAddress("sep_gr_err", &sep_gr_err);
        tree->GetEntry(0);

        sep_gr_values.push_back(sep_gr);
        sep_gr_errors.push_back(sep_gr_err);
        file->Close();
    }

    std::cout << "Misalignment Rotation (rad) | SEP_GR | SEP_GR_ERR" << std::endl;
    for (size_t i = 0; i < misalignment_values.size(); i++) {
        std::cout << misalignment_values[i] << " | " 
                  << sep_gr_values[i] << " +/- " 
                  << sep_gr_errors[i] << std::endl;
    }

    TGraphErrors *graph = new TGraphErrors(
        misalignment_values.size(), &misalignment_values[0], &sep_gr_values[0],
        nullptr, &sep_gr_errors[0]
    );

    // **Removed plot title**
    graph->SetTitle("");
    graph->SetMarkerStyle(29);
    graph->SetMarkerSize(1.5);
    graph->SetMarkerColor(kBlack);
    //  graph->SetLineColor(kBlue);
    graph->SetLineWidth(1);

    graph->GetYaxis()->SetTitle("separation power");
    //graph->GetXaxis()->SetTitle("misalignment magnitude: rot_z [rad]");
graph->GetXaxis()->SetTitle("misalignment magnitude: offset in x [mm]");
    // **Set Y-axis title offset to match X-axis**
    graph->GetYaxis()->SetTitleOffset(1.0);  // Same as X-axis
    graph->GetXaxis()->SetTitleOffset(1.0);

    graph->GetYaxis()->SetLabelSize(0.03);
    graph->GetXaxis()->SetLabelSize(0.03);


graph->GetYaxis()->SetTitleFont(132);  // Correct: Sets font for Y-axis title
graph->GetXaxis()->SetTitleFont(132);  // Correct: Sets font for X-axis title

graph->GetYaxis()->SetLabelFont(132);  // Correct: Sets font for Y-axis labels
graph->GetXaxis()->SetLabelFont(132);  // Correct: Sets font for X-axis labels
//graph->GetXaxis()->SetLimits(-0.001, 0.010);  // Expands X range slightly
//graph->GetXaxis()->SetLimits(0.0001, 0.100);
    graph->GetXaxis()->SetTitleSize(0.03);
graph->GetYaxis()->SetTitleSize(0.03);
 
// **Increased Canvas Size**
    TCanvas *c1 = new TCanvas("c1", "SEP_GR vs. Misalignment Rotation", 1200, 900);
    c1->SetGridx();
    c1->SetGridy();
    graph->Draw("AP");

    // **Updated Legend with More Precise Information**
    TLegend *legend = new TLegend(0.43, 0.75, 0.85, 0.90);

    legend->SetBorderSize(0); 
    legend->SetFillStyle(0);
    legend->SetTextSize(0.025);
    legend->SetTextFont(132);

    legend->AddEntry((TObject*)0, "DIRC at ePIC: Misalignment Bars (Work in Progress)", "");
    legend->AddEntry((TObject*)0, "Particle ID: #pi/K", "");
    legend->AddEntry((TObject*)0, "Track Angle: #theta = 30 [deg]", "");
    legend->AddEntry((TObject*)0, "Momentum: 6 GeV/c", "");

    legend->Draw();
    c1->SaveAs("Plots/sep_gr_vs_misalignment.png");

    std::cout << "Plot saved as sep_gr_vs_misalignment.png" << std::endl;
}
