#include "../ToolCharge.h"
#include "../ToolIO.h"
#include "../ToolPlots1D.h"
#include "../ToolPlots2D.h"

#include <TCanvas.h>
#include <TColor.h>
#include <TFitResult.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>
#include <TROOT.h>
#include <TRandom3.h>
#include <TSystem.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

void test_plot_tools()
{
  gROOT->SetBatch(kTRUE);
  CheckOutputFolder("images");
  CheckOutputFolder("test_output_missing/subdir");
  assert(!gSystem->AccessPathName("test_output_missing/subdir"));

  TH1D first("test_first", "", 20, 0., 20.);
  TH1D second("test_second", "", 20, 0., 20.);
  for (int bin = 1; bin <= 20; ++bin) {
    first.SetBinContent(bin, bin);
    second.SetBinContent(bin, 2. * bin);
  }
  const double original_first  = first.GetBinContent(5);
  const double original_second = second.GetBinContent(5);

  MakeHdPlot1D(&first, "x", "entries", "test_basic_1d");
  Compare1D(&first, &second, "first", "second", "x", "entries", "test_compare_1d");
  Compare1D(
    &first, &second, "first", "second", [](double a, double b) { return a / b; }, "ratio", "x", "ratio",
    "test_compare_operation");
  MakeHdPlot1D(nullptr, "x", "entries", "test_null_1d");
  MakeHdPlot2D(nullptr, "x", "y", "test_null_2d");
  CompareMulti1D(std::vector<TH1D*>{&first, &second}, std::vector<std::string>{"first", "second"}, "test_compare_multi",
                 "x", "entries");
  CompareMulti1D(std::vector<TH1D*>(16, &first), std::vector<std::string>(16, "repeated"), "test_palette_wrap");
  PlotContribution(std::vector<TH1D*>{&first, &second}, std::vector<std::string>{"first", "second"},
                   "test_contributions", "x", "fraction");
  assert(first.GetBinContent(5) == original_first);
  assert(second.GetBinContent(5) == original_second);

  TH1D empty("test_empty", "", 20, 0., 20.);
  TRandom3 random(741);
  TH1D gaussian("test_gaussian", "", 100, -5., 5.);
  for (int i = 0; i < 10000; ++i)
    gaussian.Fill(random.Gaus(0., 1.));
  TF1* successful_fit = Fit1D(&gaussian);
  assert(successful_fit != nullptr);
  assert(Fit1D(&empty) == nullptr);
  const auto failed_charge_fit = FitCharge(nullptr);
  assert(failed_charge_fit.size() == 3 && failed_charge_fit[0] == nullptr);

  PlotContribution(std::vector<TH1D*>{&empty}, std::vector<std::string>{"empty"}, "test_zero_integral");
  CompareMulti1D(std::vector<TH1D*>{&first, nullptr}, std::vector<std::string>{"first", "null"}, "test_invalid_multi");
  CompareMulti1D(std::vector<TH1D*>{&first, &second}, std::vector<std::string>{"only one"}, "test_invalid_lengths");
  PlotContribution(std::vector<TH1D*>{&first, &second}, std::vector<std::string>{"only one"},
                   "test_invalid_contribution");
  CompareSpill(&first, &second, "test_short_noisy_list", false, std::vector<bool>{true});

  TH2D nonsquare("test_nonsquare", "", 4, 0., 4., 2, 0., 2.);
  for (int xbin = 1; xbin <= nonsquare.GetNbinsX(); ++xbin)
    for (int ybin = 1; ybin <= nonsquare.GetNbinsY(); ++ybin)
      nonsquare.SetBinContent(xbin, ybin, 10. * xbin + ybin);
  MakeHdPlot2D(&nonsquare, "x", "y", "test_nonsquare_2d");
  MergeStationSensors(std::vector<TH2D*>{&nonsquare}, "x", "y", "test_merge_sensors");

  TH2D profile_input("test_profile_input", "", 20, -2., 2., 30, -3., 3.);
  for (int i = 0; i < 30000; ++i) {
    const double x = random.Gaus(0., 0.8);
    profile_input.Fill(x, 0.5 * x + random.Gaus(0., 0.3));
  }
  auto profile_result = Make2DWithProfile(&profile_input, "x", "y", "test_profile", false, false, {}, true, true);
  assert(std::get<0>(profile_result) != nullptr);
  assert(std::get<1>(profile_result).Get() != nullptr);

  const auto palette_before = MyPalette[0];
  LoadCustomColormap("missing_colormap_file.txt");
  assert(MyPalette[0] == palette_before);
  LoadCustomColormap("inferno.txt");
  assert(MyPalette[255] != 0);
  const auto valid_palette_entry = MyPalette[0];
  {
    std::ofstream malformed("test_bad_colormap.txt");
    malformed << "12 34 56\\n";
  }
  LoadCustomColormap("test_bad_colormap.txt");
  assert(MyPalette[0] == valid_palette_entry);
  gSystem->Unlink("test_bad_colormap.txt");

  SetPadMargins(hd_canvas["test_basic_1d"].get(), 1, {0.2f});
  MakeHDCanvas("test_basic_1d");
  assert(hd_canvas["test_basic_1d"].get() != nullptr);
  assert(gROOT->GetListOfCanvases()->FindObject("test_basic_1d") == hd_canvas["test_basic_1d"].get());

  Statistic<double> empty_statistic;
  bool mean_threw     = false;
  bool variance_threw = false;
  try {
    (void) empty_statistic.Mean();
  }
  catch (const std::domain_error&) {
    mean_threw = true;
  }
  try {
    (void) empty_statistic.Variance();
  }
  catch (const std::domain_error&) {
    variance_threw = true;
  }
  assert(mean_threw && variance_threw);

  for (const char* name :
       {"test_basic_1d", "test_compare_1d", "test_compare_operation", "test_compare_multi", "test_contributions",
        "test_palette_wrap", "test_short_noisy_list", "test_nonsquare_2d", "test_merge_sensors", "test_profile"})
    assert(!gSystem->AccessPathName(Form("images/%s.png", name)));

  std::cout << "test_plot_tools: PASS\n";
}
