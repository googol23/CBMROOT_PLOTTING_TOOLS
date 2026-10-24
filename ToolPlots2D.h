#include "ToolCanvas.h"
#include "ToolHistogram.h"
#include "ToolChannelScan.h"
#include "ToolSetupAddress.h"
#include "ToolFit1D.h"
#include "ToolFit2D.h"
#include "ToolGraph.h"

#ifdef CBMROOTLIB
  #include "ToolSetupAddress.h"
#endif

#ifndef TOOLPLOTS2D_H
#define TOOLPLOTS2D_H

#include <TProfile.h>
#include <regex>

void MakeHdPlot2D(TH2* h,                            // Histogram to be draw
                  std::string x_label,               // label for x-axis
                  std::string y_label,               // label for y-axis
                  std::string c_name = "noname",     // canvas name for the images to be saved
                  bool logz = false,                 // Enable log scale for z-axis
                  bool draw_grid = false,            // Enable grid
                  std::vector<TObject*> features = {} // List of additional TObject to be draw in the canvas
){
    if (h == nullptr){
        std::cout << " - MakeHdPlot2D - Invalid histogram\n";
        return;
    }
    MakeHDCanvas(c_name, 1, 1, 1);
    gPad->SetLogz(logz);
    gPad->SetGrid(draw_grid, draw_grid);
    h->SetTitle("");
    SetAxisLabels(h, x_label, y_label);
    h->Draw("colz");

    for (auto &f : features){
        if (f == nullptr) continue;
        std::cout << " - MakeHdPlot2D - Drawing additional feature: " << f->GetName() << std::endl;
        f->DrawClone("same");
    }

    hd_canvas[c_name]->SaveAs(Form("images/%s.%s", c_name.c_str(), SAVE_CANVAS_AS.c_str()));
}

std::tuple<TProfile*, TFitResultPtr> Make2DWithProfile( TH2* h,                            // Histogram to be draw
                        std::string x_label,               // label for x-axis
                        std::string y_label,               // label for y-axis
                        std::string c_name = "noname",     // canvas name for the images to be saved
                        bool logz = false,                 // Enable log scale for z-axis
                        bool draw_grid = false,            // Enable grid
                        std::vector<TObject*> features = {}, // List of additional TObject to be draw in the canvas
                        bool draw_rms = true,
                        bool draw_fit = true
){
    if (h == nullptr) return std::make_tuple(nullptr,nullptr);

    MakeHDCanvas(c_name, 1, 1, 1.2);
    gPad->SetLogz(logz);
    gPad->SetGrid(draw_grid, draw_grid);
    gPad->SetTopMargin(0.08);

    h->SetTitle("");
    SetAxisLabels(h, x_label, y_label);
    h->Draw("colz");

    for (auto &f : features){
        if (f == nullptr) continue;
        std::cout << " - MakeHdPlot2D - Drawing additional feature: " << f->GetName() << std::endl;
        f->DrawClone("same");
    }


    /* Draw profile */
    double alpha = 1;
    double h_mean_x = h->GetMean(1);
    double h_rms_x = h->GetRMS(1);
    int fst_bin = h->GetXaxis()->FindBin(h_mean_x - alpha*h_rms_x);
    int lst_bin = h->GetXaxis()->FindBin(h_mean_x + alpha*h_rms_x);

    TLine lower_bound(h_mean_x - alpha*h_rms_x, h->GetYaxis()->GetBinLowEdge(1), h_mean_x - alpha*h_rms_x, h->GetYaxis()->GetBinUpEdge(h->GetNbinsY()));
    TLine upper_bound(h_mean_x + alpha*h_rms_x, h->GetYaxis()->GetBinLowEdge(1), h_mean_x + alpha*h_rms_x, h->GetYaxis()->GetBinUpEdge(h->GetNbinsY()));
    lower_bound.SetLineColor(kBlue);
    upper_bound.SetLineColor(kBlue);
    lower_bound.SetLineWidth(hd_line_width);
    upper_bound.SetLineWidth(hd_line_width);
    if (draw_rms){
        lower_bound.DrawClone();
        upper_bound.DrawClone();
    }


    auto profile_y = h->ProfileY(Form("%s_profile", h->GetName()), fst_bin, lst_bin, "");

    std::unique_ptr<TGraphErrors> gr_profile = std::make_unique<TGraphErrors>();
    for (int bin = 1; bin <= profile_y->GetNbinsX(); bin++){
        double axis = profile_y->GetBinCenter(bin);
        double mean = profile_y->GetBinContent(bin);
        double rms = profile_y->GetBinError(bin);
        double bin_size = profile_y->GetXaxis()->GetBinWidth(bin);

        if (rms != 0 && rms < 0.015){
            gr_profile->SetPoint(gr_profile->GetN(), mean, axis);
            gr_profile->SetPointError(gr_profile->GetN() - 1, rms, bin_size/2);
        }
    }
    gr_profile->SetMarkerStyle(kDot);
    gr_profile->SetMarkerSize(0.1*hd_line_width);
    gr_profile->SetMarkerColor(kGreen);
    gr_profile->SetLineWidth(0.8* hd_line_width);
    gr_profile->SetLineColorAlpha(kGreen, 0.8);
    gr_profile->Draw("same p");

    auto fit_ptr = profile_y->Fit("pol1", "SQ0");
    if (fit_ptr || !fit_ptr->IsValid()) {
        hd_canvas[c_name]->SaveAs(Form("images/%s.%s", c_name.c_str(), SAVE_CANVAS_AS.c_str()));
        return std::make_tuple(profile_y, fit_ptr);
    }
    double p0 = fit_ptr->Parameter(0);
    double p1 = fit_ptr->Parameter(1);
    double y0 = h->GetYaxis()->GetBinLowEdge(1);
    double y1 = h->GetYaxis()->GetBinUpEdge(h->GetNbinsY());
    double x0 = y0 * p1 + p0;
    double x1 = y1 * p1 + p0;

    TLine fit_line(x0, y0, x1, y1);
    fit_line.SetLineColor(kRed);
    fit_line.SetLineWidth(hd_line_width);
    if (draw_fit) {
        fit_line.DrawClone();
    }

    std::string y_title = h->GetYaxis()->GetTitle();

    int n_cols = 1;
    std::unique_ptr<TLegend> legend = std::make_unique<TLegend>(0.05,0.93, 0.98, 0.99, nullptr, "NDC");
    legend->SetBorderSize(0);
    if (draw_rms){
        legend->AddEntry(&upper_bound, "\\mu_{dx} \\pm RMS","f"); n_cols++;
    }
    if (gr_profile){
        legend->AddEntry(gr_profile.get(), Form("\\mu_{dx}^i=\\mu_{dx}(%s)", y_title.substr(0,y_title.find("[")).c_str()),"f"); n_cols++;
    }
    if (draw_fit) {
        legend->AddEntry(&fit_line, "P_{1}(\\mu_{dx}^i)","f"); n_cols++;
    }
    legend->SetNColumns(n_cols);
    legend->DrawClone();

    hd_canvas[c_name]->SaveAs(Form("images/%s.%s", c_name.c_str(), SAVE_CANVAS_AS.c_str()));

    return std::make_tuple(profile_y, fit_ptr);
}

void MakeChargeChannel( TH2* h = nullptr,
                        std::string c_name = "noname",
                        std::string x_label = "Channel",
                        std::string y_label = "Charge [ADC]",
                        bool logz = false,
                        std::vector<IssueCode> issue_list = {}){
    if (h == nullptr) return;

    MakeHDCanvas(c_name, 1, 1, 2.5);
    gPad->SetLogz(logz);
    gPad->SetLeftMargin(0.10);
    gPad->SetRightMargin(0.10);

    // ---- Cosmetics ----
    h->SetTitle("");
    h->GetXaxis()->SetNdivisions(16, kFALSE);;
    SetAxisLabels(h, x_label, y_label);
    // ---- Finish Cosmetics ----

    h->Draw("colz");

    // Draw a line kCaution
    double max = h->GetYaxis()->GetBinUpEdge(h->GetYaxis()->GetNbins());
    for (int chn = 0 ; chn < issue_list.size(); chn++){
        if (issue_list[chn] == IssueCode::kBroken){
            TBox chn_box = TBox(chn + 0.2, 1.005 * max, chn + 0.8, 1.05 * max);
            chn_box.SetLineColor(kBlack);
            chn_box.SetFillColor(kBlack);
            chn_box.DrawClone();
        }
    }


    hd_canvas[c_name]->SaveAs(Form("images/%s.%s", c_name.c_str(), SAVE_CANVAS_AS.c_str()));
}


std::vector<double> eff_contours = {
  0.00,
  0.50,
  0.90,
  0.97
};

void DrawSensorEfficiency(  TH2* found,
                            TH2* track,
                            std::string c_name = "noname",
                            TBox* sampling_area = nullptr,
                            bool logz = false,
                            bool draw_grid = false,
                            std::vector<double> sensor_limits ={}
                        ){

    if (found == nullptr || track == nullptr) return;

    TH2D* eff = (TH2D*)found->Clone();
    eff->Divide(track);
    eff->SetContour(4, eff_contours.data());

    double pad_ratio = 1;
    std::unique_ptr<TPaveText> area_label;
    std::unique_ptr<TBox> sensor_area;
    if (sensor_limits.size()){
        sensor_area = std::make_unique<TBox>(sensor_limits[0], sensor_limits[1], sensor_limits[2], sensor_limits[3]);
        sensor_area->SetLineWidth(hd_line_width);
        sensor_area->SetLineColor(kBlack);
        sensor_area->SetLineStyle(kSolid);
        sensor_area->SetFillStyle(0);
        pad_ratio = (sensor_limits[2]-sensor_limits[0]) / (sensor_limits[3]-sensor_limits[1]);

        double x_size = sensor_limits[2] - sensor_limits[0];
        double y_size = sensor_limits[3] - sensor_limits[1];
        area_label = std::make_unique<TPaveText>(sensor_limits[2] - 0.30*x_size, sensor_limits[3],
                                                 sensor_limits[2],               sensor_limits[3] + 0.05*y_size
                                                );
        area_label->AddText("sensor edges");
        area_label->SetTextColor(kBlack);
        area_label->SetBorderSize(0);
        area_label->SetFillStyle(0);
    }

    MakeHDCanvas(c_name, 3, 1, pad_ratio);
    /* ---- Cosmetics ---- */
    std::string x_label = "X [cm]";
    std::string y_label = "Y [cm]";

    SetAxisLabels(found, x_label, y_label);
    SetAxisLabels(track, x_label, y_label);
    SetAxisLabels(eff, x_label, y_label);

    found->SetTitle("Attachable hit");
    track->SetTitle("Extrapolation");
    eff->SetTitle("Efficiency");
    /* ---- --------- ---- */

    /* ----  Drawing  ---- */
    auto draw_decorator = [&](){
        if (sensor_area) sensor_area->DrawClone();
        if (area_label) area_label->DrawClone();
        if (sampling_area) sampling_area->DrawClone("same");
    };

    hd_canvas[c_name]->cd(1);   gPad->SetLogz(logz);    gPad->SetGrid(draw_grid, draw_grid);
    found->DrawClone("colz");
    draw_decorator();

    hd_canvas[c_name]->cd(2);   gPad->SetLogz(logz);    gPad->SetGrid(draw_grid, draw_grid);
    track->DrawClone("colz");
    draw_decorator();

    hd_canvas[c_name]->cd(3);   gPad->SetLogz(logz);    gPad->SetGrid(draw_grid, draw_grid);
    eff->DrawClone("colz");
    draw_decorator();
    /* ---- --------- ---- */

    hd_canvas[c_name]->SaveAs(Form("images/%s.%s", c_name.c_str(), SAVE_CANVAS_AS.c_str()));
}

void MergeStationSensors(   std::vector<TH2D*> h_sensor,
                            std::string x_label = "X [cm]",
                            std::string y_label = "Y [cm]",
                            std::string c_name = "noname",
                            bool logz = false,
                            bool draw_grid = false,
                            bool draw_sensor_edges = false,
                            double margin = 0
                        ){
    // Get binning from first sensor
    if (h_sensor.empty() || std::any_of(h_sensor.begin(), h_sensor.end(), [](TH2D* h) { return h == nullptr; })) {
        std::cout << " - WARNING - [MergeStationSensors] Empty or invalid sensor list\n";
        return;
    }
    double bin_dx = h_sensor[0]->GetXaxis()->GetBinWidth(1);
    double bin_dy = h_sensor[0]->GetYaxis()->GetBinWidth(1);

    // Scan range for station histogram
    double station_x_min = h_sensor[0]->GetXaxis()->GetBinLowEdge(1);
    double station_x_max = h_sensor[0]->GetXaxis()->GetBinUpEdge(h_sensor[0]->GetNbinsX());
    double station_y_min = h_sensor[0]->GetYaxis()->GetBinLowEdge(1);
    double station_y_max = h_sensor[0]->GetYaxis()->GetBinUpEdge(h_sensor[0]->GetNbinsY());

    for (auto h : h_sensor){
        station_x_min = std::min(station_x_min, h->GetXaxis()->GetBinLowEdge(1));
        station_x_max = std::max(station_x_max, h->GetXaxis()->GetBinUpEdge(h->GetNbinsX()));
        station_y_min = std::min(station_y_min, h->GetYaxis()->GetBinLowEdge(1));
        station_y_max = std::max(station_y_max, h->GetYaxis()->GetBinUpEdge(h->GetNbinsY()));
    }

    double range_x = station_x_max - station_x_min;
    double range_y = station_y_max - station_y_min;
    station_x_min -= 0.1 * range_x;
    station_y_min -= 0.1 * range_y;
    station_x_max += 0.1 * range_x;
    station_y_max += 0.1 * range_y;

    // Create station histogram
    std::unique_ptr<TH2D> station = std::make_unique<TH2D>(c_name.c_str(), c_name.c_str(),
        (station_x_max - station_x_min) / bin_dx, station_x_min, station_x_max,
        (station_y_max - station_y_min) / bin_dy, station_y_min, station_y_max
        );
    SetAxisLabels(station.get(), x_label, y_label);

    // Copy sensor histogram content to station histogram
    for (auto h : h_sensor){
        for (int bin_x = 1 ; bin_x <= h->GetNbinsX(); bin_x++){
        for (int bin_y = 1 ; bin_y <= h->GetNbinsY(); bin_y++){
            double x = h->GetXaxis()->GetBinCenter(bin_x);
            double y = h->GetYaxis()->GetBinCenter(bin_y);

            int station_bin_x = station->GetXaxis()->FindBin(x);
            int station_bin_y = station->GetYaxis()->FindBin(y);

            // Update bin content
            double update_entries = station->GetBinContent(station_bin_x, station_bin_y) + h->GetBinContent(bin_x, bin_y);
            station->SetBinContent(station_bin_x, station_bin_y, update_entries);
        }}
    }   // End histograms loop

    /* ---- Drawing ---- */
    MakeHdPlot2D(station.get(), x_label, y_label, c_name, logz);

    std::unique_ptr<TBox> sensor_area;
    if (draw_sensor_edges){
        hd_canvas[c_name]->cd();
        for (auto h : h_sensor){
            double x_min = h->GetXaxis()->GetBinLowEdge(1) + margin;
            double y_min = h->GetYaxis()->GetBinLowEdge(1) + margin;
            double x_max = h->GetXaxis()->GetBinUpEdge(h->GetNbinsX()) - margin;
            double y_max = h->GetYaxis()->GetBinUpEdge(h->GetNbinsY()) - margin;

            TBox sensor_area (x_min, y_min, x_max, y_max);
            sensor_area.SetLineWidth(hd_line_width);
            sensor_area.SetLineColor(kBlack);
            sensor_area.SetLineStyle(kSolid);
            sensor_area.SetFillStyle(0);

            sensor_area.DrawClone();

            // Find address in histogram name
            std::regex hex_regex("0[xX][0-8]{8}");
            std::smatch match;
            std::string name = h->GetName();
            if (std::regex_search(name, match, hex_regex)) {
                int32_t address = std::stoi(match.str(), nullptr, 16);
                double x_size = x_max - x_min;
                double y_size = y_max - y_min;
                TPaveText area_label (x_max - 0.30*x_size, y_max, x_max, y_max + 0.05*y_size);
                #ifdef CBMROOTLIB
                  area_label.AddText(NiceAddress(address, "ULM").c_str());
                #endif
                area_label.SetTextColor(kBlack);
                area_label.SetBorderSize(0);
                area_label.SetFillStyle(0);
                area_label.DrawClone();
            }
        }
    }
    hd_canvas[c_name]->SaveAs(Form("images/%s.%s", c_name.c_str(), SAVE_CANVAS_AS.c_str()));
}


void Draw2DProj(TH2 *h,
                std::string c_name = "",
                std::string x_label = "x_label",
                std::string y_label = "y_label",
                bool logz = false,
                bool draw_grid = false
                ){
    if (h == nullptr) {
        std::cout << "Draw2DProj - Histogram is nullptr" << std::endl;
        return;
    };

    int proj_color = kBlack;

    auto p_x = h->ProjectionX();
    p_x->SetLineWidth(hd_line_width);

    auto p_y = h->ProjectionY();
    p_y->SetLineWidth(hd_line_width);

    TGraph g_x = TGraph(p_x);
    g_x.SetLineWidth(hd_line_width);
    g_x.SetFillColorAlpha(proj_color, 0.5);
    g_x.SetLineColor(proj_color);
    SetAxisLabels(&g_x, x_label, "");

    TGraph g_y = TGraph(p_y);
    g_y.SetLineWidth(hd_line_width);
    g_y.SetFillColorAlpha(proj_color, 0.5);
    g_y.SetLineColor(proj_color);
    SetAxisLabels(&g_y, "", y_label);
    SwapGraphAxis(&g_y);

    auto canvas = MakeHDCanvas(c_name, 2, 2, 1);

    // Get the pointers to the pads
    TPad *pad_tl = (TPad*)canvas->GetPad(1); // Top left
    TPad *pad_tr = (TPad*)canvas->GetPad(2); // Top right
    TPad *pad_bl = (TPad*)canvas->GetPad(3); // Bottom left
    TPad *pad_br = (TPad*)canvas->GetPad(4); // Bottom right

    // Configure the pad sizes
    pad_tl->SetPad(0.0, 0.7, 0.7, 1.0); // Top left
    pad_tr->SetPad(0.7, 0.7, 1.0, 1.0); // Top right
    pad_bl->SetPad(0.0, 0.0, 0.7, 0.7); // Bottom left
    pad_br->SetPad(0.7, 0.0, 1.0, 0.7); // Bottom right

    pad_tl->SetTopMargin (.02);
    pad_tl->SetRightMargin (.001);
    pad_tl->SetBottomMargin(.001);

    pad_tr->SetLeftMargin  (.001);
    pad_tr->SetBottomMargin(.001);

    pad_bl->SetTopMargin  (.001);
    pad_bl->SetRightMargin(.001);

    pad_br->SetTopMargin (.001);
    pad_br->SetLeftMargin(.001);
    pad_br->SetRightMargin (.02);

    canvas->cd(1);
    g_x.Draw("APL f");

    canvas->cd(4);
    g_y.Draw("APL f");

    canvas->cd(3);
    gPad->SetLogz(logz);
    h->Draw("col");

    auto setup_line = [&](double x1, double y1, double x2, double y2){
        TLine line(x1,y1,x2,y2);
        line.SetLineColor(kBlack);
        line.SetLineWidth(hd_line_width);
        line.SetLineStyle(kDashed);
        line.DrawClone();
    };
    setup_line(h->GetXaxis()->GetBinLowEdge(h->GetXaxis()->GetFirst()), 0, h->GetXaxis()->GetBinUpEdge(h->GetXaxis()->GetLast()), 0);
    setup_line(0, h->GetYaxis()->GetBinLowEdge(h->GetYaxis()->GetFirst()), 0, h->GetYaxis()->GetBinLowEdge(h->GetYaxis()->GetLast()));


    // Draw stat box
    canvas->cd(2);
    std::unique_ptr<TPaveText> text_box = std::make_unique<TPaveText>(0.00, 0.0, 0.99, 0.99, "NDC");
    text_box->SetBorderSize(0);
    text_box->SetFillColorAlpha(0, 0.5);
    text_box->AddText(Form(" mean_x = %0.4f(%d) cm", h->GetMean(1), int(1e+4*h->GetMeanError(1))));
    text_box->AddText(Form(" mean_y = %0.4f(%d) cm", h->GetMean(2), int(1e+4*h->GetMeanError(2))));
    text_box->AddText(Form("sigma_x = %0.4f(%d) cm", h->GetRMS(1),  int(1e+4*h->GetRMSError(1))));
    text_box->AddText(Form("sigma_y = %0.4f(%d) cm", h->GetRMS(2),  int(1e+4*h->GetRMSError(2))));
    text_box->DrawClone();

    canvas->SaveAs(Form("images/%s.%s", c_name.c_str(), SAVE_CANVAS_AS.c_str()));

}

#endif