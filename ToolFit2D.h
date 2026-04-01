#include "ToolCanvas.h"
#include "ToolHistogram.h"
#include "ToolFit1D.h"
#include "ToolFuncDef.h"


#ifndef TOOLFIT2D_H
#define TOOLFIT2D_H

#include "TH2D.h"
#include "TF2.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TFile.h"
#include "TRandom3.h"

using namespace std;

// Define a rotated bivariate Gaussian function
double BivariateGaussian(double* xy, double* par) {
    double x = xy[0];
    double y = xy[1];

    // Parameters
    double mu_x = par[0];       // Mean in X
    double mu_y = par[1];       // Mean in Y
    double sigma_x = par[2];    // Sigma in X
    double sigma_y = par[3];    // Sigma in Y
    double theta = par[4];      // Rotation angle (in radians)
    double norm = par[5];       // Normalization factor

    // Rotation matrix
    double cosTheta = TMath::Cos(theta);
    double sinTheta = TMath::Sin(theta);

    // Coordinates in rotated frame
    double x_prime = cosTheta * (x - mu_x) + sinTheta * (y - mu_y);
    double y_prime = -sinTheta * (x - mu_x) + cosTheta * (y - mu_y);

    // Bivariate Gaussian function
    double exponent = -0.5 * (TMath::Power(x_prime / sigma_x, 2) + TMath::Power(y_prime / sigma_y, 2));
    return norm * TMath::Exp(exponent);
}

TF2* FitBivariateGaussian(  TH2* h_ptr,
                            FitConfig fit_config,
                            double x_min = 0,
                            double x_max = 0,
                            double y_min = 0,
                            double y_max = 0,
                            std::string c_name = "",
                            std::string x_label = "x_label",
                            std::string y_label = "y_label",
                            bool logz = false,
                            bool draw_grid = false,
                            bool debug = false
) {
    if (h_ptr == nullptr) return nullptr;

    TH2D* h2 = (TH2D*)h_ptr->Clone();

    double fit_x_min = x_min == x_max ? h2->GetXaxis()->GetBinLowEdge(1) : x_min;
    double fit_y_min = y_min == y_max ? h2->GetYaxis()->GetBinLowEdge(1) : y_min;
    double fit_x_max = x_min == x_max ? h2->GetXaxis()->GetBinUpEdge(h2->GetNbinsX()) : x_max;
    double fit_y_max = y_min == y_max ? h2->GetYaxis()->GetBinUpEdge(h2->GetNbinsY()) : y_max;

    // Define the fitting function
    std::shared_ptr<TF2> f_2 = std::make_shared<TF2>("f_2", BivariateGaussian, fit_x_min, fit_x_max, fit_y_min, fit_y_max, 6);

    for (auto &par : fit_config.par_config){
        if (par.x_min == par.x_max) {
            f_2->FixParameter(par.index, par.x_ini);
        }else{
            f_2->SetParameter(par.index, par.x_ini);
            f_2->SetParLimits(par.index, par.x_min, par.x_max);
        }
    }
    f_2->SetParNames("mu_x", "mu_y", "sigma_x", "sigma_y", "theta", "norm");

    TFitResultPtr fit_ptr = h2->Fit("f_2", "LSN0");

    if (!fit_ptr && fit_ptr->IsValid()){
        if (debug){
            cout << "Fit Results:" << endl;
            cout << "mu_x = " << f_2->GetParameter(0) << endl;
            cout << "mu_y = " << f_2->GetParameter(1) << endl;
            cout << "sigma_x = " << f_2->GetParameter(2) << endl;
            cout << "sigma_y = " << f_2->GetParameter(3) << endl;
            cout << "theta (rad) = " << f_2->GetParameter(4) << endl;
            cout << "norm = " << f_2->GetParameter(5) << endl;

        }

        if (c_name.size()){
            MakeHDCanvas(c_name, 1, 1, 1.2);
            gPad->SetLogz(logz);
            gPad->SetGrid(draw_grid, draw_grid);
            h2->SetTitle("");
            SetAxisLabels(h2, x_label, y_label);
            h2->Draw("colz");


            // Draw the fit function
            f_2->SetNpx(5000); // Resolution in X
            f_2->SetNpy(5000); // Resolution in Y
            f_2->SetLineWidth(0.8*hd_line_width);
            f_2->SetLineColor(kGreen+1);
            f_2->DrawClone("CONT3 SAME");

            std::unique_ptr<TPaveText> text_box = std::make_unique<TPaveText>(0.25, 0.75, 0.75, 0.94, "NB NDC");
            // text_box->SetFillColor(0);
            text_box->AddText(Form(" mean_x = %0.3f cm", f_2->GetParameter(0)))->SetTextColor(kGreen+2);
            text_box->AddText(Form(" mean_y = %0.3f cm", f_2->GetParameter(1)))->SetTextColor(kGreen+2);
            text_box->AddText(Form("sigma_x = %0.3f cm", f_2->GetParameter(2)))->SetTextColor(kGreen+2);
            text_box->AddText(Form("sigma_y = %0.3f cm", f_2->GetParameter(3)))->SetTextColor(kGreen+2);
            // text_box->AddText(Form("  theta = %0.3f dg", f_2->GetParameter(4)))->SetTextColor(kGreen+2);
            text_box->DrawClone();

            hd_canvas[c_name]->SaveAs(Form("images/%s.%s", c_name.c_str(), SAVE_CANVAS_AS.c_str()));
        }
        return (TF2*)f_2->Clone();

    }
    return nullptr;
}

#endif