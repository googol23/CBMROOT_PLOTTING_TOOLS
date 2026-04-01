#include "../tools/ToolCanvas.h"
#include "../tools/ToolHistogram.h"

#include "../tools/ToolFuncDef.h"


#ifndef TOOLFIT1D_H
#define TOOLFIT1D_H

struct FitPar{
    int index;
    double x_ini, x_min, x_max;

};
struct FitConfig{
    std::vector<FitPar> par_config;

    FitConfig(std::initializer_list<FitPar> list) : par_config(list) {}
};

/* Fit 1D distribution at the peak located at h->GetMaximumBin +- range */
TF1* Fit1D( TH1* h_ptr = nullptr,
            const char* func = "gaus",
            const char* mode = "SQM0",
            float range = 0.4,
            std::string c_name = "",
            std::string x_label = "",
            std::string y_label = "",
            bool log_scale_x = false,
            bool log_scale_y = false,
            bool draw_grid = false
            ){

    // Skip invalid or empty histograms
    if (h_ptr == nullptr || !h_ptr->GetEntries()){
        std::cout << " - WARNIGN - [Fit1D] Invalid histogram\n";
        return nullptr;
    }

    auto h = (TH1D*)h_ptr->Clone();
    TFitResultPtr fit;
    int max_bin = h->GetMaximumBin();
    if (max_bin == 0 || max_bin > h->GetNbinsX()){ return nullptr; } // no peak, just under/over flow entries
    float peak_x = h->GetBinCenter(h->GetMaximumBin());   // Fit around Peak position (global maximum)
    int bin_x_lower;
    int bin_x_upper;
    float range_min;
    float range_max;

    if (range){
        bin_x_lower = h->FindBin(peak_x - range);         // Lower bin range
        bin_x_upper = h->FindBin(peak_x + range);         // Upper bin range
        range_min = peak_x - range;
        range_max = peak_x + range;
    }
    else{
        bin_x_lower = 1;
        bin_x_upper = h->GetNbinsX();
        range_min = h->GetBinCenter(bin_x_lower);
        range_max = h->GetBinCenter(bin_x_upper);
    }


    if (!h->Integral(bin_x_lower, bin_x_upper)){
        std::cout << "- ERROR - Histogram is empty in the fitting range" << std::endl;
        return nullptr;
    }

    std::unique_ptr<TF1> fit_f;

    /* Fit function definitions */
    if (std::string(func) == "lorentz") {
        fit_f = std::make_unique<TF1>(func, Lorentz, range_min, range_max, 3);
        fit_f->SetParNames("norm", "x0", "fwhm");
    }

    fit = h->Fit(func, mode, "", range_min, range_max);



    if (c_name.length()){
        // ----------------------------------------------
        // Cosmetics
        // ----------------------------------------------
        auto data_color = Colors1D[0];
        auto sign_color = Colors1D[3];

        h->GetXaxis()->SetRangeUser(range_min, range_max);
        h->SetTitle("");
        h->SetLineWidth(hd_line_width);
        h->SetLineColor(data_color);
        SetAxisLabels(h, x_label, y_label);
        // ----------------------------------------------

        std::unique_ptr<TLegend> legend = std::make_unique<TLegend>(0.25, 0.95, 0.75, 0.99);
        legend->SetBorderSize(0);
        legend->AddEntry((TH1F*)h->Clone(), "Data", "lpe");
        legend->SetNColumns(2);

        MakeHDCanvas(c_name);
        gPad->SetGrid(draw_grid, draw_grid);
        gPad->SetLogx(log_scale_x);
        gPad->SetLogy(log_scale_y);

        h->DrawClone("histo");

        std::unique_ptr<TPaveText> text_box = std::make_unique<TPaveText>(0.18, 0.70, 0.47, 0.90, "NB NDC");
        text_box->SetFillColor(0);
        if (!fit && fit->IsValid()){
            TF1* f = h->GetFunction(func);
            f->SetTitle("");
            f->SetLineWidth(hd_line_width);
            f->SetLineColorAlpha(sign_color, 0.8);
            f->DrawClone("same");
            legend->AddEntry(f->Clone(), "Fit",  "l");

            text_box->AddText(Form("%s = %.3e", f->GetParName(1), f->GetParameter(1)));
            text_box->AddText(Form("%s = %.3e", f->GetParName(2), f->GetParameter(2)));
        }

        text_box->AddText(Form("RMS = %.3e", h->GetRMS()));
        text_box->AddText(Form("FWHM = %.3e", FWHM(h)));

        text_box->DrawClone();
        legend->DrawClone();

        hd_canvas[c_name]->SaveAs(Form("images/%s.%s",c_name.c_str(), SAVE_CANVAS_AS.c_str()));

    }
    return h->GetFunction(func);
    // std::cout << " - WARNING - [Fit1D] Fit is not valid\n";
    // return nullptr;
}



TF1* FitLorentzPeak(
    TH1* h,
    std::string c_name = "noname",
    bool log_scale_y = false,
    bool log_scale_x = false
    ){

    TFitResultPtr fit;
    float  fwhm = FWHM(h);
    float  fit_range = 2.7*FWHM(h);
    float x_min = h->GetBinCenter(h->GetMaximumBin()) - fit_range;
    float x_max = h->GetBinCenter(h->GetMaximumBin()) + fit_range;

    std::unique_ptr<TF1> fitFcn = std::make_unique<TF1>(Form("fitFcn_%s", h->GetName()), Lorentz, x_min, x_max, 3);
    fitFcn->SetParameter(1,h->GetBinCenter(h->GetMaximumBin()));
    fitFcn->SetParameter(2,fwhm);

    Double_t par[3];
    fit = h->Fit(Form("fitFcn_%s", h->GetName()), "SQ0+", "", x_min, x_max);

    // ----------------------------------------------
    // Cosmetics
    // ----------------------------------------------
    auto data_color = Colors1D[0];
    auto ffit_color = Colors1D[3];
    h->SetTitle("");
    h->SetLineColor(data_color);
    h->GetXaxis()->SetRangeUser(2.5*x_min, 2.5*x_max);
    // ----------------------------------------------
    MakeHDCanvas("temp");
    TLegend *legend=new TLegend(0.60, 0.60, 0.88, 0.88);
    legend->SetBorderSize(0);
    legend->AddEntry((TH1F*)h->Clone(),  "Data",        "lpe");

    int n_of_lines = 1;
    std::unique_ptr<TPaveText> text_box;

    if (log_scale_x) gPad->SetLogx(1);
    if (log_scale_y) gPad->SetLogy(1);
    h->DrawClone("histo");

    bool fit_status = !fit && fit->IsValid() && !(fit->Prob() > 0.05);
    const char* axis_unit = "cm";
    if (fit_status){
        fitFcn->GetParameters(par);
        fitFcn->SetTitle("");
        fitFcn->SetLineWidth(hd_line_width);
        fitFcn->SetLineColorAlpha(ffit_color, 0.8);
        fitFcn->DrawClone("same");
        legend->AddEntry(fitFcn->Clone(), "Lorentz fit", "l");

        n_of_lines += 2;
        text_box = std::make_unique<TPaveText>(0.2, 0.90-0.04*n_of_lines, 0.47, 0.90, "NB NDC");
        text_box->AddText(Form("%10s%0.f %s", "x_0 =  ", fitFcn->GetParameter(1), axis_unit));
        text_box->AddText(Form("%10s%0.f %s", "FWHM = ", fitFcn->GetParameter(2), axis_unit));
    }

    if (!text_box){
        text_box = std::make_unique<TPaveText>(0.2, 0.90-0.04*n_of_lines, 0.47, 0.90, "NB NDC");
        text_box->AddText(Form("FWHM = %0.f %s", fwhm, axis_unit));
    }
    text_box->SetFillColor(0);
    text_box->DrawClone();

    legend->DrawClone();

    gPad->SaveAs(c_name.c_str());

    return (TF1*)fitFcn->Clone();
}

template<typename H>
std::vector<std::shared_ptr<TF1>> FitGausP2(
    H* h_ptr,
    double fit_x_min,
    double fit_x_max,
    FitConfig fit_config
    ){

    if (h_ptr == nullptr){
        std::cout << "- WARNING - Invalid object to fit\n";
        return {};
    }
    H* h = (H*)h_ptr->Clone();

    std::shared_ptr<TF1> f_x    = std::make_shared<TF1>("f_x", "[0]*TMath::Gaus(x, [1], [2]) + [3] + [4]*x + [5]*x*x", fit_x_min, fit_x_max);
    std::shared_ptr<TF1> f_peak = std::make_shared<TF1>("f_peak", "[0]*TMath::Gaus(x, [1], [2])", fit_x_min, fit_x_max);
    std::shared_ptr<TF1> f_bkgd = std::make_shared<TF1>("f_bkgd", "[0] + [1]*x + [2]*x*x", fit_x_min, fit_x_max);

    f_x->SetParNames("norm", "mean", "sigma", "p0", "p1", "p2");
    f_peak->SetParNames("norm", "mean", "sigma");
    f_bkgd->SetParNames("p0", "p1", "p2");

    for (auto &par : fit_config.par_config){
        f_x->SetParameter(par.index, par.x_ini);
        f_x->SetParLimits(par.index, par.x_min, par.x_max);
    }


    TFitResultPtr fit_ptr = h->Fit("f_x", "LSN0", "", fit_x_min, fit_x_max);
    /* Invalid ptr */
    if (fit_ptr) return {};

    /* Invalid fit */
    if (!fit_ptr->IsValid()) return {};

    /* Invalid did not converged */
    if (fit_ptr->Status() != 0) return {};

    /* Erros to large */
    // if (std::abs(1 - fit_ptr->Chi2() / fit_ptr->Ndf()) > 0.5) return {};

    f_x->SetParameters(fit_ptr->GetParams());
    f_peak->SetParameter(0, fit_ptr->Parameter(0));
    f_peak->SetParameter(1, fit_ptr->Parameter(1));
    f_peak->SetParameter(2, fit_ptr->Parameter(2));
    f_bkgd->SetParameter(0, fit_ptr->Parameter(3));
    f_bkgd->SetParameter(1, fit_ptr->Parameter(4));
    f_bkgd->SetParameter(2, fit_ptr->Parameter(5));

    return {f_x, f_peak, f_bkgd};

}
#endif