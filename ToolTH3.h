#ifndef TOOLTH3_H
#define TOOLTH3_H

#include "TH2D.h"
#include "TF2.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TFile.h"
#include "TRandom3.h"
#include <stdexcept>

std::shared_ptr<TH2D> SliceTH3(TH3* h3, double value, int axs = 0){
  if (!h3) {
    throw std::invalid_argument("Input Nullptr");
  }
  
  std::string h_name;
  TAxis* slice_axs = nullptr;
  TAxis* h2_x_axs = nullptr;
  TAxis* h2_y_axs = nullptr;
  
  if (axs == 0){
    h_name = "SliceX";
    slice_axs = h3->GetXaxis();
    h2_x_axs = h3->GetYaxis();
    h2_y_axs = h3->GetZaxis();
  }
  else if (axs == 1){
    h_name = "SliceY";
    slice_axs = h3->GetYaxis();
    h2_x_axs = h3->GetXaxis();
    h2_y_axs = h3->GetZaxis();
  }
  else {
    h_name = "SliceZ";
    slice_axs = h3->GetZaxis();
    h2_x_axs = h3->GetXaxis();
    h2_y_axs = h3->GetYaxis();
  }
  
  auto h2 = std::make_shared<TH2D>(h_name.c_str(), h_name.c_str(),
    h2_x_axs->GetNbins(), h2_x_axs->GetXmin(), h2_x_axs->GetXmax(),
    h2_y_axs->GetNbins(), h2_y_axs->GetXmin(), h2_y_axs->GetXmax());
  
  h2->GetXaxis()->SetTitle(h2_y_axs->GetTitle());
  h2->GetYaxis()->SetTitle(h2_y_axs->GetTitle());
  
  int silice_bin = slice_axs->FindBin(value);
  
  auto getBinContent = [h3, axs, silice_bin](int xi, int yi) -> double {
    if (axs == 0) {
      // Slicing X axis: indices are (silice_bin, yi, zi) -> (fixed, Y, Z)
      return h3->GetBinContent(silice_bin, xi, yi);
    } else if (axs == 1) {
      // Slicing Y axis: indices are (xi, silice_bin, yi) -> (X, fixed, Z)
      return h3->GetBinContent(xi, silice_bin, yi);
    } else {
      // Slicing Z axis: indices are (xi, yi, silice_bin) -> (X, Y, fixed)
      return h3->GetBinContent(xi, yi, silice_bin);
    }
  };
  
  for (auto xi = 1; xi <= h2_x_axs->GetNbins(); xi++){
  for (auto yi = 1; yi <= h2_y_axs->GetNbins(); yi++){
   h2->SetBinContent(xi, yi, getBinContent(xi, yi)); 
  }}
  
  
  return h2;
}

#endif