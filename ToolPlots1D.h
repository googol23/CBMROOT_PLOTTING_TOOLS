#include "../tools/ToolCanvas.h"
#include "../tools/ToolHistogram.h"
#include "../tools/ToolChannelScan.h"
#include "../tools/ToolStatistic.h"


#ifndef TOOLPLOTS1D_H
#define TOOLPLOTS1D_H


void MakeHdPlot1D(TH1* h,                             // Hsitogram to be draw
                  std::string x_label = "x-axis",     // label for x-axis
                  std::string y_label = "y-axis",     // label for y-axis
                  std::string c_name = "noname",      // canvas name for the images to be saved
                  bool log_x = false,                 // Enable log scale for z-axis
                  bool log_y = false,                 // Enable log scale for z-axis
                  bool draw_grid = false,             // Enable grid
                  std::vector<TObject*> features = {} // List of additional TObject to be draw in the canvas
){
    if (h == nullptr) return;
    MakeHDCanvas(c_name, 1, 1, 1.2);
    gPad->SetLogx(log_x);
    gPad->SetLogy(log_y);
    gPad->SetGrid(draw_grid, draw_grid);
    h->SetTitle("");
    SetAxisLabels(h, x_label, y_label);
    h->SetLineColor(kBlack);
    h->SetLineWidth(hd_line_width);
    h->Draw("histo");

    for (auto &f : features){
        std::cout << " - MakeHdPlot2D - Drawing additional feature: " << f->GetName() << std::endl;
        f->DrawClone();
    }

    hd_canvas[c_name]->SaveAs(Form("images/%s.%s", c_name.c_str(), SAVE_CANVAS_AS.c_str()));
}

template <typename H>
void PlotContribution(std::vector<H*> v, std::vector<std::string> titles, std::string c_name = "", std::string x_label = "", std::string y_label = ""){
    H* sum = (H*)v[0]->Clone();
    sum->Reset();
    sum->SetLineWidth(hd_line_width);
    sum->SetLineColor(color_blind_safe[0]);
    sum->SetFillColorAlpha(color_blind_safe[0], 0.2);
    SetAxisLabels(sum, x_label, y_label);
    sum->GetXaxis()->SetMaxDigits(3);
    sum->GetXaxis()->SetNdivisions(10);

    MakeHDCanvas(c_name);

    for (auto h : v){
        sum->Add(h);
        h->GetYaxis()->SetMaxDigits(3);
        h->GetYaxis()->SetNdivisions(10);
        h->GetXaxis()->SetMaxDigits(3);
        h->GetXaxis()->SetNdivisions(10);
    }
    double scale = 1./sum->Integral();
    sum->Scale(scale);
    sum->Draw("histo same");

    int legend_columns = 0;
    std::unique_ptr<TLegend> legend_x = std::make_unique<TLegend>(0.25, 0.95, 0.75, 0.99);
    legend_x->SetBorderSize(0);
    legend_x->AddEntry((H*)sum->Clone(), "Total", "f"); legend_columns++;

    int contribution = 1;
    for (auto h : v){
        h->Scale(scale);
        h->GetXaxis()->SetMaxDigits(3);
        h->GetXaxis()->SetNdivisions(10);

        h->SetLineWidth(0.8*hd_line_width);
        h->SetFillColorAlpha(color_blind_safe[contribution], 0.2);
        h->SetLineColor(color_blind_safe[contribution]);
        h->SetLineStyle(kDotted);
        SetAxisLabels(h, x_label, y_label);

        h->Draw("histo same");
        legend_x->AddEntry((H*)h->Clone(), titles[contribution-1].c_str(), "f"); legend_columns++;

        contribution++;
    }

    legend_x->SetNColumns(legend_columns);
    legend_x->DrawClone();

    hd_canvas[c_name]->SaveAs(Form("images/%s.%s", c_name.c_str(), SAVE_CANVAS_AS.c_str()));
}

template <typename H>
void Compare1D( H* h_0_ptr,
                H* h_1_ptr,
                std::string title_0,
                std::string title_1,
                std::string x_label = "",
                std::string y_label = "",
                std::string c_name = "",
                bool log_x = false,
                bool log_y = false,
                bool grid  = false,
                bool norma = false
    ){

    double y_max_0 = h_0_ptr ? h_0_ptr->GetMaximum() : -1;
    double y_max_1 = h_1_ptr ? h_1_ptr->GetMaximum() : -1;
    double y_max = std::max(y_max_0, y_max_1);

    if (y_max < 0){
        std::cout << "- WARNING - Input histograms are emtpy or invalid\n";
        return;
    }
    MakeHDCanvas(c_name, 1, 1, 1.2);
    gPad->SetLogx(log_x);
    gPad->SetLogy(log_y);
    gPad->SetTopMargin(0.09);
    gPad->SetGrid(grid, grid);

    std::unique_ptr<TLegend> legend = std::make_unique<TLegend>(0.20, 0.92, 0.75, 0.99);
    legend->SetBorderSize(0);
    int legend_n_col = 0;

    // ---- Cosmetics ----
    auto h_configure = [&](H* ptr, std::string title, int color){
        ptr->SetTitle("");
        ptr->SetLineWidth(0.8*hd_line_width);
        ptr->SetLineColor(color);
        ptr->SetMaximum(1.05* y_max);
        SetAxisLabels(ptr, x_label, y_label);
        ptr->Draw("P same");
        legend->AddEntry((H*)ptr->Clone(), title.c_str(), "f");
        legend_n_col++;
    };
    // ---- Finish Cosmetics ----


    if (h_0_ptr == nullptr) {
        std::cout << " - WARNING - Input parameter h_0: Invalid histogram\n";
        return;
    }else{
        H* h_0 = (H*)h_0_ptr->Clone();
        h_configure(h_0, title_0, Colors1D[3]);

    }
    if (h_1_ptr == nullptr) {
        std::cout << " - WARNING - Input parameter h_1: Invalid histogram\n";
        return;
    }else{
        H* h_1 = (H*)h_1_ptr->Clone();
        h_configure(h_1, title_1, Colors1D[4]);
    }


    legend->SetNColumns(legend_n_col);
    legend->DrawClone();

    hd_canvas[c_name]->SaveAs(Form("images/%s.%s", c_name.c_str(), SAVE_CANVAS_AS.c_str()));
}

template <typename H>
void CompareMulti1D(
    std::vector<H*> v,
    std::vector<std::string> titles,
    std::string c_name = "",
    std::string x_label = "",
    std::string y_label = "",
    std::string draw_opt = "histo",
    std::vector<int> line_style = {},
    std::vector<int> line_color = {},
    std::vector<int> fill_color = {},
    bool log_x = false,
    bool log_y = false,
    bool grid = false,
    std::vector<TObject*> features = {}
    ){

    assert(v.size() != 0 && "Nothing to plot");
    if (!titles.size()){
        std::cout << " - WARNING - [CompareMulti1D] No titles provided. Using TObject name\n";
        for (auto h : v) {
            if (h) {
                titles.push_back(h->GetName());
            }else{
                titles.push_back("No title");
            }
        }

    }
    int n_obj = v.size();


    MakeHDCanvas(c_name,1, 1, 1.2);
    gPad->SetLogx(log_x);
    gPad->SetLogy(log_y);
    gPad->SetGrid(grid,grid);
    gPad->SetLeftMargin(0.15);
    gPad->SetRightMargin(0.08);

    int legend_columns = 0;
    double legend_dy = 0.7 / n_obj > 0.1 ? 0.1 : 0.8 / n_obj;
    double legend_lower_edge = 0.70 - 0.5*legend_dy * n_obj;
    double legend_upper_edge = 0.70 + 0.5*legend_dy * n_obj;
    std::unique_ptr<TLegend> legend_x = std::make_unique<TLegend>(0.60, legend_lower_edge,
                                                                  0.92, legend_upper_edge);
    legend_x->SetBorderSize(0);

    bool is_graph = std::is_base_of<TGraph, H>::value;
    if (is_graph)
        std::cout << "Obj list contains TGraph based" << std::endl;

    double y_max = 0;
    for (H* h : v){
        if (h == nullptr){
            std::cout << "- WARNING - [CompareMulti1D] Invalid object to compare\n";
            continue;
        }
        y_max = std::max(y_max, h->GetMaximum());
    }

    for (int idx = 0 ; idx < v.size() ; idx++){
        H* h = v[idx];
        if (h == nullptr) continue;
        // ---- Cosmetics ----
        h->SetTitle("");
        if (y_max) h->SetMaximum(1.05 * y_max);
        h->SetLineWidth(0.8*hd_line_width);

        if (line_style.size() > idx){
            h->SetLineStyle(line_style[idx]);
        }

        if (line_color.size() > idx){
            h->SetLineColor(line_color[idx]);
        }else{
            h->SetLineColor(color_large_set[idx]);
        }


        SetAxisLabels(h, x_label, y_label);
        // ---- Finish Cosmetics ----

        std::string current_draw_opt = is_graph && idx == 0 ? "A" + draw_opt : draw_opt;
        current_draw_opt = idx != 0 ? current_draw_opt + "same" : current_draw_opt;
        h->DrawClone(current_draw_opt.c_str());

        legend_x->AddEntry((H*)h->Clone(), titles[idx].c_str(), "f"); legend_columns++;
    }

    legend_x->DrawClone();

    for (auto f : features){
        if (f == nullptr) continue;
        f->DrawClone();
    }

    hd_canvas[c_name]->SaveAs(Form("images/%s.%s", c_name.c_str(), SAVE_CANVAS_AS.c_str()));
}

void CompareSpill(  TH1* h_spill_0_ptr = nullptr, /* Off spill channel distribution */
                    TH1* h_spill_1_ptr = nullptr, /*  On spill channel distribution */
                    std::string c_name = "noname",
                    bool logy = false,
                    std::vector<bool> mark_noisy = {},
                    TH2* chg_vs_chn = nullptr){

    if (h_spill_0_ptr == nullptr || h_spill_1_ptr == nullptr) return;

    TH1D* h_spill_0 = (TH1D*)h_spill_0_ptr->Clone();
    TH1D* h_spill_1 = (TH1D*)h_spill_1_ptr->Clone();
    int spill_color_0 = Colors1D[0];
    int spill_color_1 = Colors1D[3];

    int n_pad_y = chg_vs_chn ? 2 : 1;
    MakeHDCanvas(c_name, 1, n_pad_y, 2.5);

    if (n_pad_y == 2){
        TPad* top_pad = (TPad*)hd_canvas[c_name]->GetPad(1);
        TPad* bot_pad = (TPad*)hd_canvas[c_name]->GetPad(2);
        top_pad->SetLeftMargin(0.10);
        bot_pad->SetLeftMargin(0.10);
        top_pad->SetRightMargin(0.10);
        bot_pad->SetRightMargin(0.10);
        top_pad->SetTopMargin(0.05);
        top_pad->SetBottomMargin(0.00);
        bot_pad->SetTopMargin(0.00);
        top_pad->SetLogy(1);
        bot_pad->SetLogz(1);

        top_pad->SetPad(0.0, 0.5, 1.0, 1.0);
        bot_pad->SetPad(0.0, 0.0, 1.0, 0.5);

        top_pad->SetGrid();
        bot_pad->SetGrid();
    }{
        gPad->SetLogy(logy);
        gPad->SetLeftMargin(0.10);
        gPad->SetRightMargin(0.10);
        gPad->SetGrid();
    }

    hd_canvas[c_name]->cd(1);
    // ---- Cosmetics ----
    h_spill_0->SetTitle("");
    h_spill_1->SetTitle("");

    SetAxisLabels(h_spill_0, "Channel", "Normalized Entries");
    SetAxisLabels(h_spill_1, "Channel", "Normalized Entries");

    h_spill_0->SetLineColor(spill_color_0);
    h_spill_1->SetLineColor(spill_color_1);

    h_spill_0->SetFillColorAlpha(spill_color_0, 0.6);
    h_spill_1->SetFillColorAlpha(spill_color_1, 0.6);

    h_spill_0->GetXaxis()->SetNdivisions(16, kFALSE);
    h_spill_1->GetXaxis()->SetNdivisions(16, kFALSE);

    double max = std::max(h_spill_0->GetMaximum(), h_spill_1->GetMaximum());
    h_spill_0->SetMaximum(1.5*max);
    h_spill_1->SetMaximum(1.5*max);
    // ---- Finish Cosmetics ----

    h_spill_1->Draw("histo");
    h_spill_0->Draw("histo same");

    std::unique_ptr<TLegend> legend_x = std::make_unique<TLegend>(0.10, 0.95, 0.90, 0.99);
    int n_of_columns = 0;
    legend_x->SetBorderSize(0);
    legend_x->AddEntry((TH1F*)h_spill_0->Clone(), "Off-spill", "f"); n_of_columns++;
    legend_x->AddEntry((TH1F*)h_spill_1->Clone(),  "On-spill", "f"); n_of_columns++;
    legend_x->SetNColumns(n_of_columns);
    legend_x->DrawClone();

    for (int chn = 0 ; chn < 2048; chn++){
        if (mark_noisy[chn]){
            TBox chn_box = TBox(chn + 0.2, 1.1 * max, chn + 0.8, 1.25 * max);
            chn_box.SetLineColor(kBlack);
            chn_box.SetFillColor(kBlack);
            chn_box.DrawClone();
        }
    }


    // ---- Plot charge vs channel distribution --------
    if (chg_vs_chn){
        hd_canvas[c_name]->cd(n_pad_y);
        TH2D* h = (TH2D*)chg_vs_chn->Clone();
        // ---- Cosmetics ----
        h->SetTitle("");
        h->GetXaxis()->SetNdivisions(16, kFALSE);;
        SetAxisLabels(h, "Channel", "Charge [ADC]");
        // ---- Finish Cosmetics ----

        h->Draw("colz");

        // Draw a line kCaution
        double max = h->GetYaxis()->GetBinUpEdge(h->GetYaxis()->GetNbins());
        for (int chn = 0 ; chn < mark_noisy.size(); chn++){
            if (mark_noisy[chn]){
                TBox chn_box = TBox(chn + 0.2, 1.005 * max, chn + 0.8, 1.05 * max);
                chn_box.SetLineColor(kBlack);
                chn_box.SetFillColor(kBlack);
                chn_box.DrawClone();
            }
        }
    }
    // ---- End Plot charge vs channel distribution ----
    hd_canvas[c_name]->SaveAs(Form("images/%s.%s", c_name.c_str(), SAVE_CANVAS_AS.c_str()));
}


void NoiseVsAdcCut( TH2* h_spill_0_ptr = nullptr, /* Off spill channel distribution */
                    TH2* h_spill_1_ptr = nullptr, /*  On spill channel distribution */
                    std::string c_name = "noname",
                    std::vector<int>* adc_cut = nullptr){

    auto noise_func = [](double sgn, double bkg) -> double { return (sgn - bkg) / (sgn + bkg); };

    if (h_spill_0_ptr == nullptr || h_spill_1_ptr == nullptr) return;

    TH2D* h_spill_0 = (TH2D*)h_spill_0_ptr->Clone();
    TH2D* h_spill_1 = (TH2D*)h_spill_1_ptr->Clone();

    MakeHDCanvas(c_name, 4, 4);

    std::map<int,std::unique_ptr<TH1D>> asic_avg_noise;

    for (int asic = 0 ; asic < 16; asic++){
        std::string h_name = Form("ASIC_%d", asic);
        asic_avg_noise[asic] = std::make_unique<TH1D>(h_name.c_str(), h_name.c_str(), 31, 1, 32);

        for (int adc = 1; adc <= 31 ; adc++){
            Statistic<double> avg_noise;
            for (int chn = 128*asic + 1 ; chn <= 128*(asic+1); chn++){
                double bkg = 0;
                double sgn = 0;
                for (int bin_y = adc ; bin_y <= 32; bin_y++){
                    bkg += h_spill_0->GetBinContent(chn, bin_y);
                    sgn += h_spill_1->GetBinContent(chn, bin_y);
                }
                if (sgn || bkg){
                    avg_noise.Add(noise_func(sgn, bkg));
                }
            }

            asic_avg_noise[asic]->SetBinContent(adc, avg_noise.Mean());
            asic_avg_noise[asic]->SetBinError(adc, avg_noise.RMS());

        }
        // ---- Cosmetics ----
        asic_avg_noise[asic]->SetLineWidth(hd_line_width);
        SetAxisLabels(asic_avg_noise[asic].get(), "ADC threshold", "\\frac{S-B}{S+B}");
        // ---- End Cosmetics ----

        hd_canvas[c_name]->cd(asic+1);
        asic_avg_noise[asic]->DrawClone("histo");

        if(adc_cut != nullptr){
            assert(adc_cut->size() == 16);
            int first_above = asic_avg_noise[asic]->FindFirstBinAbove(0.5);
            adc_cut->at(asic) = first_above >= 1 ? first_above-1 : 0;
        }


        // ---- Draw second y-axis ----
        // double x_pos = asic_avg_noise[asic]->GetXaxis()->GetXmax();
        // double y_min = asic_avg_noise[asic]->GetYaxis()->GetXmin();
        // double y_max = asic_avg_noise[asic]->GetYaxis()->GetXmax();

        // TGaxis* right_axis = new TGaxis(x_pos, y_min,
        //                                 x_pos, y_max,
        //                                 0, 1,
        //                                 510,
        //                                 "+L");
        // right_axis->SetTitle("\\frac{S}{B}");
        // right_axis->CenterTitle(true);
        // right_axis->SetTitleSize(0.05);
        // right_axis->DrawClone();
        // ---- End Draw second y-axis ----
    }
    hd_canvas[c_name]->SaveAs(Form("images/%s.%s", c_name.c_str(), SAVE_CANVAS_AS.c_str()));
}

#endif