#include "ToolCanvas.h"
#include "ToolColors.h"

#ifndef TOOLHISTOGRAM_H
#define TOOLHISTOGRAM_H

#include <TAxis.h>
#include <TBox.h>
#include <TGraph.h>
#include <TH1.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLine.h>
#include <TPaveText.h>

template<typename TObj>
void SetAxisLabels(TObj* h, std::string x_label = "", std::string y_label = "", float txt_size = 0.05){
    if (h == nullptr){ return; }
    h->GetXaxis()->CenterTitle(true);
    h->GetYaxis()->CenterTitle(true);

    h->GetXaxis()->SetTitleSize(txt_size);
    h->GetYaxis()->SetTitleSize(txt_size);

    if (x_label.length())    h->GetXaxis()->SetTitle(x_label.c_str());
    if (y_label.length())    h->GetYaxis()->SetTitle(y_label.c_str());
}

template <typename H>
TH2D* RotateTH2(const H* i_h, double phi_rad, double x_center = 0, double y_center = 0) {

    // Create a new TH2D histogram with the same binning as the original
    int nx = i_h->GetNbinsX();
    int ny = i_h->GetNbinsY();

    TH2D* o_h = (TH2D*)i_h->Clone();
    o_h->Reset();

    // Rotation matrix components
    double cos_alpha = cos(phi_rad);
    double sin_alpha = sin(phi_rad);

    // Loop over all bins of the original histogram
    for (int ix = 1; ix <= nx; ix++) {
        for (int iy = 1; iy <= ny; iy++) {
            double x = i_h->GetXaxis()->GetBinCenter(ix) - x_center;
            double y = i_h->GetYaxis()->GetBinCenter(iy) - y_center;

            // Apply the rotation matrix
            double x_rot = x*cos_alpha - y*sin_alpha + x_center;
            double y_rot = x*sin_alpha + y*cos_alpha + y_center;

            // Find the corresponding bin in the rotated histogram
            int bin_rot = o_h->FindBin(x_rot, y_rot);

            // Set the bin content and error in the new histogram
            double content = i_h->GetBinContent(ix, iy);
            double error = i_h->GetBinError(ix, iy);
            o_h->SetBinContent(bin_rot, o_h->GetBinContent(bin_rot) + content);
            // o_h->SetBinError(bin_rot, sqrt(pow(o_h->GetBinError(bin_rot), 2) + pow(error, 2)));
        }
    }

    return o_h;
}


template <typename H>
TH2D* FlipAxisTH2(const H* i_h) {

    // Create a new TH2D histogram with the same binning as the original
    int nx = i_h->GetNbinsX();
    int ny = i_h->GetNbinsY();

    TH2D o_h (Form("%s_flip", i_h->GetName()), "",
        ny, i_h->GetYaxis()->GetBinLowEdge(1), i_h->GetYaxis()->GetBinUpEdge(ny),
        nx, i_h->GetXaxis()->GetBinLowEdge(1), i_h->GetXaxis()->GetBinUpEdge(nx)
        );

    // Loop over all bins of the original histogram
    for (int ix = 1; ix <= nx; ix++) {
    for (int iy = 1; iy <= ny; iy++) {
        o_h.SetBinContent(iy, ix, i_h->GetBinContent(ix, iy));
    }}

    return (TH2D*)o_h.Clone();
}

template <typename H>
double FWHM(H* h, std::string image_name = ""){
    if (h == nullptr)
    return -1;

    int bin1 = h->FindFirstBinAbove(h->GetMaximum()/2);
    int bin2 = h->FindLastBinAbove(h->GetMaximum()/2);
    double fwhm = h->GetXaxis()->GetBinUpEdge(bin2) - h->GetXaxis()->GetBinLowEdge(bin1);

    if (image_name.length()){
        MakeHDCanvas(image_name);
        h->SetTitle("");
        h->SetLineWidth(hd_line_width);
        h->SetLineColor(Colors1D[0]);
        h->DrawClone("histo");
        std::unique_ptr<TPaveText> text_box = std::make_unique<TPaveText>(0.2, 0.70, 0.47, 0.90, "NB NDC");
        text_box->SetFillColor(0);
        text_box->AddText(Form("FWHM = %0.3f cm", fwhm));
        text_box->DrawClone();
        hd_canvas[image_name]->SaveAs(Form("images/%s.%s", image_name.c_str(), SAVE_CANVAS_AS.c_str()));
        hd_canvas.erase(image_name);
    }

    return fwhm;
}


std::string FindAxisUnit(const TAxis* axis){
    std::string label = axis->GetTitle();
    std::cout << label << std::endl;
    size_t start = label.find('[');
    size_t end = label.find(']', start);
    std::string unit;
    size_t str_size = label.size();
    if (start != std::string::npos && end != std::string::npos && start < end) {
        unit = label.substr(str_size - start + 2).c_str();
    } else {
        unit = "";
    }

    std::cout << unit.length() << std::endl;

    return unit;
}

int GetFirstBinFilled(TH1* h){
  for (int i = 1; i < h->GetXaxis()->GetNbins(); i++){
    if (h->GetBinContent(i)){
      return i;
    }
  }
  return 0;
}

int GetLastBinFilled(TH1* h){
  for (int i = h->GetXaxis()->GetNbins(); i > 0; i--){
    if (h->GetBinContent(i)){
      return i;
    }
  }
  return -1;
}

struct H1Error {
    TH1* mean;
    TH1* lower_error;
    TH1* upper_error;

    void Draw(const char * opt = "histo"){

        if (mean == nullptr) {
            std::cout << " - WARNING - [TH1Error::Draw] Invalid input (nullptr)\n";
            return;
        }

        mean->DrawClone(opt);

        int color_idx = mean->GetLineColor();

        /* Draw error boxes */
        for (int bin = 1; bin <= mean->GetNbinsX(); bin++){
            double x0 = mean->GetXaxis()->GetBinLowEdge(bin);
            double x1 = mean->GetXaxis()->GetBinUpEdge(bin);
            double y0 = mean->GetBinContent(bin) - lower_error->GetBinContent(bin);
            double y1 = mean->GetBinContent(bin) + upper_error->GetBinContent(bin);

            y1 = y1 > 1 ? 1 : y1;

            // std::cout << "Drawing Error Box: " << x0 << "\t" << x1 << "\t" << y0 << " |" << mean->GetBinContent(bin) << "| " << y1 << std::endl;

            TBox bin_err_band = TBox(x0,y0,x1,y1);
            bin_err_band.SetLineColorAlpha(color_idx, 0.4);
            bin_err_band.SetFillColorAlpha(color_idx, 0.4);
            bin_err_band.DrawClone();
        }
    }
};

TH1D* TGraph2TH1(const TGraph *g){
    if (g == nullptr){
        std::cout << " - WARNING - [TGraph2TH1] Invalid input (nullptr)\n";
        return nullptr;
    }
    auto n_of_pts = g->GetN();
    double x_min = g->GetPointX(0);
    double x_max = g->GetPointX(n_of_pts-1);
    std::cout << x_min << "\t" << x_max << std::endl;
    std::unique_ptr<TH1D> h =
        std::make_unique<TH1D>(g->GetName(), g->GetTitle(), n_of_pts, x_min, x_max);

    for(int i = 0 ; i < n_of_pts ; i++) {
        double y = g->GetPointY(i);
        h->SetBinContent(i, y);
    }
    return (TH1D*)h->Clone();
}

// Same range and binning
TH1D* Involute(std::vector<TH1D*> set){
    TH1D* involute = (TH1D*)set[0]->Clone();
    involute->Reset();

    for (auto h : set){
        for (int i = 0 ; i < involute->GetXaxis()->GetNbins() ; i++) {
            involute->SetBinContent(i, std::max(h->GetBinContent(i), involute->GetBinContent(i)));
        }
    }
    return involute;
}

#endif