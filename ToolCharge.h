#include "ToolCanvas.h"
#include "ToolHistogram.h"

#ifndef TOOLCHARGE_H
#define TOOLCHARGE_H

#include <TF1.h>
#include <TFitResult.h>
#include <TH1.h>
#include <TLegend.h>
#include <TMath.h>
#include <TVirtualFitter.h>

Double_t BackgroundFit (Double_t* x, Double_t* par){
    Double_t p0 = par[0];
    Double_t p1 = par[1];
    Double_t p2 = par[2];

    // return p0 + p1*x[0] + p2*x[0]*x[0];
    return p0 + p1*p2*TMath::Exp(-p2*x[0]);
}

Double_t LandauBkg(Double_t* x, Double_t* par) {
    Double_t A = par[0];   // Amplitude of Landau
    Double_t x0 = par[1];  // Peak position of Landau
    Double_t Gamma = par[2]; // Width of Landau

    // Landau part
    Double_t Landau = A * (Gamma*Gamma) / ( (x[0] - x0)*(x[0] - x0) + Gamma*Gamma );

    // Total function: Landau + background
    return Landau + BackgroundFit(x, &par[3]);
}

std::vector<TF1*> FitCharge(TH1* h_ptr,
    double x0 = 0, double x1 = 0,
    std::string image_name = "",
    bool log_scale_y = false,
    bool log_scale_x = false){

    if (h_ptr == nullptr || h_ptr->GetEntries() == 0) return {nullptr, nullptr, nullptr};
    TH1* h = static_cast<TH1*>(h_ptr->Clone());

    TFitResultPtr fit;
    double  fit_range = 10.*FWHM(h);
    double x_min = x0 == x1 ? h->GetBinCenter(h->GetMaximumBin()) - fit_range : x0 ;
    double x_max = x0 == x1 ? h->GetBinCenter(h->GetMaximumBin()) + fit_range : x1;

    TH1* h_peak = static_cast<TH1*>(h->Clone());
    h_peak->GetXaxis()->SetRangeUser(20e+3, 40e+3);
    double peak_posit = h_peak->GetBinCenter(h->GetMaximumBin());
    double sigma_init = h_peak->GetRMS();

    // Define the function with 6 parameters
    TF1* fitFcn = new TF1(Form("fitFcn_%s", h->GetName()), LandauBkg, x_min, x_max, 6);

    // Set parameter names (optional) and initial guesses for parameters
    fitFcn->SetParNames("Amplitude", "PeakPos", "Width", "P0", "P1", "P2");

    // Initial parameter guesses (user can adjust based on the data)
    fitFcn->SetParameters(0, 0.12);                                                   // Amplitude
    fitFcn->SetParameters(1, peak_posit);                                             // PeakPos
    fitFcn->SetParameters(2, sigma_init);                                             // Width
    fitFcn->SetParameters(4, 1);                                                     // P1
    fitFcn->SetParameters(5, 3e-4);                                                 // P2

    fitFcn->SetParLimits(0, 1e-3, 1e+2);                                              // Amplitude
    fitFcn->SetParLimits(1, 20e+3, 40e+3);                                            // PeakPos
    fitFcn->SetParLimits(2, 2e+3, 6e+3);                                             // Width
    // fitFcn->SetParLimits(4, 10);                                                   // P1
    // fitFcn->SetParLimits(5, 1e-);                                                  // P2

    fitFcn->SetParameters(3, 1e-9);                                                      // P0
    fitFcn->SetParLimits (3, 1e-12, 1e-1);                                                    // P0

    TVirtualFitter::SetMaxIterations(1e+4);

    std::cout << "Attempting fit ...\n";
    fit = h->Fit(Form("fitFcn_%s", h->GetName()), "S0+", "", x_min, x_max);

    if (fit /* || !fit->IsValid() || fit->Prob() > 0.001 */){
        std::cout << "*** Fit failed ***\n";
        return {nullptr, nullptr, nullptr};
    }

    std::unique_ptr<TF1> backFcn = std::make_unique<TF1>(Form("backFcn_%s", h->GetName()),BackgroundFit,x_min,x_max,3);
    std::unique_ptr<TF1> signalFcn = std::make_unique<TF1>(Form("signalFcn_%s", h->GetName()),
        "[0]*[2]*[2]/((x-[1])*(x-[1])+[2]*[2])", x_min, x_max);
    Double_t par[6];

    fitFcn->GetParameters(par);
    signalFcn->SetParameters(par);
    backFcn->SetParameters(&par[3]);

    if (image_name.length()){
        auto data_color = Colors1D[0];
        auto bkgd_color = Colors1D[2];
        auto sign_color = Colors1D[4];
        auto glob_color = Colors1D[3];

        h->SetMaximum(1.5*h->GetMaximum());
        h->SetTitle("");
        h->SetLineWidth(hd_line_width);
        h->SetLineColor(data_color);

        fitFcn->SetTitle("");
        fitFcn->SetLineWidth(hd_line_width);
        fitFcn->SetLineColorAlpha(glob_color, 0.8);

        backFcn->SetTitle("");
        backFcn->SetLineWidth(hd_line_width);
        backFcn->SetLineColorAlpha(bkgd_color, 0.8);

        signalFcn->SetTitle("");
        signalFcn->SetLineWidth(0.6*hd_line_width);
        signalFcn->SetLineColorAlpha(sign_color, 0.8);
        // ----------------------------------------------

        MakeHDCanvas(image_name);
        if (log_scale_x) gPad->SetLogx(1);
        if (log_scale_y) gPad->SetLogy(1);

        h->DrawClone("histo");
        backFcn->DrawClone("same");
        signalFcn->DrawClone("same");
        fitFcn->DrawClone("same");

        // draw the legend
        TLegend *legend=new TLegend(0.60, 0.60, 0.88, 0.88);
        legend->SetBorderSize(0);
        legend->AddEntry((TH1F*)h->Clone(),  "Data",        "lpe");
        legend->AddEntry(backFcn->Clone(),   "Background",  "l");
        legend->AddEntry(signalFcn->Clone(), "Signal",  "l");
        legend->AddEntry(fitFcn->Clone(),    "Global",  "l");

        legend->DrawClone();

        std::unique_ptr<TPaveText> text_box = std::make_unique<TPaveText>(0.60, 0.50, 0.88, 0.60, "NB NDC");
        text_box->SetFillColor(0);
        // text_box->AddText("Signal Fit");
        text_box->AddText(Form("MPV = %.0f", signalFcn->GetParameter(1)));
        // text_box->AddText(Form("Scale = %0.3f", signalFcn->GetParameter(2)));
        // text_box->AddText(Form("S/B = %0.3f", signalFcn->Eval(signalFcn->GetParameter(1)) / backFcn->Eval(signalFcn->GetParameter(1)) ) );
        text_box->DrawClone();

        hd_canvas[image_name]->SaveAs(Form("images/%s.%s", image_name.c_str(), SAVE_CANVAS_AS.c_str()));
        hd_canvas.erase(image_name);
    }
    return {(TF1*)backFcn->Clone(), (TF1*)signalFcn->Clone(), (TF1*)fitFcn->Clone()};

}

void CompareCharge(TH1* h0_ptr, TH1* h1_ptr, std::string image = "", std::string x_label = "", std::string y_label = "", std::string h0_title = "", std::string h1_title = "", bool do_fit = false){
    if (h0_ptr == nullptr || h1_ptr == nullptr) return;
    TH1D* h0 = (TH1D*)h0_ptr->Clone();
    TH1D* h1 = (TH1D*)h1_ptr->Clone();

    std::string c_name = image;
    MakeHDCanvas(c_name);

    int color0 = Colors1D[3];
    int color1 = Colors1D[4];

    h0->Scale(1./ h0->Integral());
    h1->Scale(1./ h1->Integral());

    h0->SetLineWidth(hd_line_width);
    h1->SetLineWidth(hd_line_width);
    h0->SetLineColor(color0);
    h1->SetLineColor(color1);
    h0->SetTitle("");
    h1->SetTitle("");
    SetAxisLabels(h0, x_label, y_label);
    SetAxisLabels(h1, x_label, y_label);

    h0->GetXaxis()->SetMaxDigits(3);
    h1->GetXaxis()->SetMaxDigits(3);
    h0->GetXaxis()->SetNdivisions(10);
    h1->GetXaxis()->SetNdivisions(10);

    h0->GetXaxis()->SetRangeUser(h0->GetBinCenter(1), h0->GetBinCenter(31));
    h1->GetXaxis()->SetRangeUser(h1->GetBinCenter(1), h1->GetBinCenter(31));

    double y_max_val = 1.2 * std::max(h0->GetMaximum(), h1->GetMaximum());
    h0->GetYaxis()->SetRangeUser(0, y_max_val);
    h1->GetYaxis()->SetRangeUser(0, y_max_val);

    h0->Draw("histo");
    h1->Draw("histo same");

    int legend_columns = 0;
    std::unique_ptr<TLegend> legend_x = std::make_unique<TLegend>(0.25, 0.95, 0.75, 0.99);
    legend_x->SetBorderSize(0);
    legend_x->AddEntry((TH1D*)h0->Clone(), h0_title.c_str(), "f"); legend_columns++;
    legend_x->AddEntry((TH1D*)h1->Clone(), h1_title.c_str(), "f"); legend_columns++;

    if (do_fit){
        TF1* f0 = FitCharge(h0,
                            h0->GetXaxis()->GetBinCenter(1),
                            h0->GetXaxis()->GetBinCenter(31),
                            Form("fits/%s_fit.png", h0->GetName()))[2];
        TF1* f1 = FitCharge(h1,
                            h1->GetXaxis()->GetBinCenter(1),
                            h1->GetXaxis()->GetBinCenter(31),
                            Form("fits/%s_fit.png", h1->GetName()))[2];

        hd_canvas[c_name]->cd();

        if (f0){
            f0->SetLineWidth(0.5*hd_line_width);
            f0->SetLineStyle(kDashed);
            f0->SetLineColor(color0);
            f0->Draw("same");
            legend_x->AddEntry((TH1D*)f0->Clone(), Form("%s fit",h0_title.c_str()), "f"); legend_columns++;
        }
        if (f1){
            f1->SetLineWidth(0.5*hd_line_width);
            f1->SetLineStyle(kDashed);
            f1->SetLineColor(color1);
            f1->Draw("same");
            legend_x->AddEntry((TH1D*)f1->Clone(), Form("%s fit",h0_title.c_str()), "f"); legend_columns++;
        }
    }

    legend_x->SetNColumns(legend_columns);
    legend_x->DrawClone();
    hd_canvas[c_name]->SaveAs(Form("images/%s.%s", c_name.c_str(), SAVE_CANVAS_AS.c_str()));
}

#endif