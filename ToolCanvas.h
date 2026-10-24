#include "ToolColors.h"

#ifndef TOOLCANVAS_H
#define TOOLCANVAS_H

#include <TBox.h>
#include <TCanvas.h>
#include <TList.h>
#include <TPad.h>
#include <TPaveText.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TText.h>
#include <cstdlib>

std::string SAVE_CANVAS_AS = "png";
static const int hd_pad_x = /* 250; // */ 1024;
static const int hd_pad_y = /* 250; // */ 1024;
float hd_line_width = 0.0045*hd_pad_y;
std::map<std::string, std::unique_ptr<TCanvas>> hd_canvas;
namespace {
const bool hd_canvas_cleanup_registered = [] {
    std::atexit([] { hd_canvas.clear(); });
    return true;
}();
}

int GetNofPadsIn(TCanvas *canvas = nullptr){
    if (canvas == nullptr){ return -1; }

    // Get the list of primitives in the canvas
    TList* primitives = canvas->GetListOfPrimitives();

    // Count the number of pads in the list of primitives
    int n_of_pads = 0;
    TIter next(primitives);
    TObject* obj;
    while ((obj = next())) {
        if (obj->IsA() == TPad::Class()) {
            n_of_pads++;
        }
    }
    return n_of_pads;
}

void SetPadMargins(TCanvas* c, int pad = 1, std::vector<float> margin = {0.15,0.15,0.15,0.15}){
    if (c == nullptr || margin.size() < 4){ return; }
    if (pad > 0){
        if (pad > GetNofPadsIn(c) && GetNofPadsIn(c) > 0) return;
        c->cd(pad);
        if (gPad == nullptr) return;
        gPad->SetLeftMargin  (margin[0]);
        gPad->SetRightMargin (margin[1]);
        gPad->SetTopMargin   (margin[2]);
        gPad->SetBottomMargin(margin[3]);
    }else{
        for (int i = 1; i <= GetNofPadsIn(c); i++){
            c->cd(i);
            if (gPad == nullptr) continue;
            gPad->SetLeftMargin  (margin[0]);
            gPad->SetRightMargin (margin[1]);
            gPad->SetTopMargin   (margin[2]);
            gPad->SetBottomMargin(margin[3]);
        }
    }
}

TCanvas* MakeHDCanvas(
    /*     canvas name      */ std::string c_name = "c0",
    /* number of x division */ int n_div_x = 1,
    /* number of y division */ int n_div_y = 1,
    /*    aspect ratio      */ float pad_ratio = 1
    ){

    gStyle->SetOptStat("");

    if (hd_canvas.count(c_name)){
        // std::cout << "An object was already created under this name, and it will be replaced!\n";
        hd_canvas.erase(c_name);
    }
    std::cout << Form("Creating HD canvas  - %s - with subpads (%u,%u)\n", c_name.c_str(), n_div_x, n_div_y);
    hd_canvas[c_name] = std::make_unique<TCanvas>(c_name.c_str(), c_name.c_str(), pad_ratio*n_div_x*hd_pad_x, n_div_y*hd_pad_y);

    if (n_div_x != 1 || n_div_y != 1){
        hd_canvas[c_name]->Divide(n_div_x, n_div_y);
    }

    // Setting pad margins
    for (int pad = 1; pad <= n_div_x*n_div_y; pad++){
        hd_canvas[c_name]->cd(pad);
        gPad->SetLeftMargin(0.15);
        gPad->SetBottomMargin(0.12);
        gPad->SetTopMargin(0.05);
        gPad->SetRightMargin(0.12);
    }

    if (DARK_THEME_ACTIVE) gStyle->SetPadColor(bkg_color_idx);

    hd_canvas[c_name]->cd();
    return hd_canvas[c_name].get();
}

void MakePadGrid(
    /*   number of rows  */ int n_rows = 2,
    /* number of columns */ int n_cols = 2,
    /*   data pad label  */ std::string default_data_label = "Empty pad",
    /*    top labels     */ std::vector<std::string> top_labels = {},
    /*    bot labels     */ std::vector<std::string> bot_labels = {},
    /*   left labels     */ std::vector<std::string> left_labels = {},
    /*  right labels     */ std::vector<std::string> right_labels = {},
    /*    canvas name    */std::string c_name = ""
){

    int n_div_x = n_cols + 2;
    int n_div_y = n_rows + 2;

    if (top_labels.size() != n_cols){
        for (int i = 0 ; i < n_cols ; i++)
            top_labels.push_back(Form("TOP COL %d", i));
    }
    if (bot_labels.size() != n_cols){
        for (int i = 0 ; i < n_cols ; i++)
            bot_labels.push_back(Form("BOT COL %d", i));
    }
    if (left_labels.size() != n_rows){
        for (int i = 0 ; i < n_rows ; i++)
            left_labels.push_back(Form("LFT ROW %d", i));
    }
    if (right_labels.size() != n_rows){
        for (int i = 0 ; i < n_rows ; i++)
            right_labels.push_back(Form("RGH ROW %d", i));
    }

    if (!c_name.size()) c_name = Form("PadGrid_%d_x_%d", n_rows, n_cols);

    if (!hd_canvas.count(c_name)){
        double pad_ratio = 1.;
        MakeHDCanvas(c_name, n_div_x, n_div_y, pad_ratio);
        gStyle->SetOptStat("");

        double c_size_x = pad_ratio*n_div_x*hd_pad_x;
        double c_size_y =           n_div_y*hd_pad_y;
        double edge_size_x = 0.03;
        double edge_size_y = edge_size_x*c_size_x / c_size_y;
        double central_pad_x = (1. - 2.*edge_size_x) / (n_div_x - 2.);
        double central_pad_y = (1. - 2.*edge_size_y) / (n_div_y - 2.);
        double txt_size = 0.5;


        for (int pad_i = 0; pad_i < GetNofPadsIn(hd_canvas[c_name].get()); pad_i++){
            bool edge_row = pad_i < n_div_x || pad_i >= n_div_x*(n_div_y-1);
            bool edge_col = pad_i%n_div_x == 0 || pad_i%n_div_x == n_div_x-1;

            int row = pad_i / n_div_x;
            int col = pad_i % n_div_x;

            /* ----  Configure pad  ---- */
            TPad *pad = (TPad*)hd_canvas[c_name]->GetPad(pad_i+1);

            double x_0 = col == 0 ? col*central_pad_x : edge_size_x + (col-1)*central_pad_x;
            double y_0 = row == 0 ? row*central_pad_y : edge_size_y + (row-1)*central_pad_y;
            double x_1   = col == 0 ? x_0 + edge_size_x : (col != n_div_x-1 ? x_0 + central_pad_x : x_0 + edge_size_x);
            double y_1   = row == 0 ? y_0 + edge_size_y : (row != n_div_y-1 ? y_0 + central_pad_y : y_0 + edge_size_y);

            pad->SetPad(x_0, y_0, x_1, y_1);
            pad->SetTopMargin   (.0);
            pad->SetRightMargin (.0);
            pad->SetBottomMargin(.0);
            pad->SetLeftMargin  (.0);
            /* ---- End Configure pad ---- */

            hd_canvas[c_name]->cd(pad_i+1);

            // Data pad
            if (!edge_col && !edge_row){
                TBox box (.01, .01, 0.99, 0.99);
                box.SetFillColor(kGray);
                box.SetLineColor(kBlack);
                box.SetLineWidth(0.5*hd_line_width);
                box.DrawClone();

                std::string txt = default_data_label;
                TText text(.5, .5, txt.c_str());
                text.SetNDC();
                text.SetTextSize(0.1);
                text.SetTextColor(kWhite);
                text.SetTextAngle(45);
                text.SetTextAlign(22);
                text.DrawClone();
            }

            // Right labels
            if (col == n_cols+1 && !edge_row){
                std::string text = right_labels[row-1];

                TPaveText key_str(0, 0, 1, 1, "NDC");
                key_str.SetBorderSize(0);
                key_str.SetFillColor(0);
                TText* key_txt = key_str.AddText(text.c_str());
                key_txt->SetTextAngle(90);
                key_txt->SetTextSize(txt_size);
                key_txt->SetTextAlign(23);
                key_str.DrawClone();
            }
            // Top labels
            if (row == n_rows+1 && !edge_col){
                std::string text = top_labels[col-1];

                TPaveText key_str(0, 0, 1, 1, "NDC");
                key_str.SetBorderSize(0);
                key_str.SetFillColor(0);
                TText* key_txt = key_str.AddText(text.c_str());
                key_txt->SetTextAngle(0);
                key_txt->SetTextSize(txt_size);
                key_txt->SetTextAlign(22);
                key_str.DrawClone();
            }

            // Left labels
            if (col == 0 && !edge_row){
                std::string text = left_labels[row-1];
                TPaveText key_str(0, 0, 1, 1, "NDC");
                key_str.SetBorderSize(0);
                key_str.SetFillColor(0);
                TText* key_txt = key_str.AddText(text.c_str());
                key_txt->SetTextAngle(90);
                key_txt->SetTextSize(txt_size);
                key_txt->SetTextAlign(21);
                key_str.DrawClone();

            }
            // Bot labels
            if (row == 0 && !edge_col){
                std::string text = bot_labels[col-1];
                TPaveText key_str(0, 0, 1, 1, "NDC");
                key_str.SetBorderSize(0);
                key_str.SetFillColor(0);
                TText* key_txt = key_str.AddText(text.c_str());
                key_txt->SetTextAngle(0);
                key_txt->SetTextSize(txt_size);
                key_txt->SetTextAlign(23);
                key_str.DrawClone();

            }
        }
    }

}
#endif