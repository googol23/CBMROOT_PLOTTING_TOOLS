#ifndef TOOLS_TH_COMPARISON_H
#define TOOLS_TH_COMPARISON_H

#include <TAxis.h>
#include <TClass.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TH1.h>
#include <TKey.h>

#include <cmath>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace ToolTHCompare
{

  struct HistogramPaths {
    std::vector<std::string> th1;
    std::vector<std::string> th2;
  };

  inline HistogramPaths GetTHPaths(TDirectory* directory)
  {
    HistogramPaths paths;
    if (!directory) return paths;
    if (auto* file = dynamic_cast<TFile*>(directory)) {
      if (file->IsZombie()) return paths;
    }

    auto scan = [&paths](auto&& self, TDirectory* dir, const std::string& prefix) -> void {
      TIter next(dir->GetListOfKeys());
      while (auto* key = static_cast<TKey*>(next())) {
        TClass* type = TClass::GetClass(key->GetClassName());
        if (!type) continue;
        const std::string name = key->GetName();
        const std::string path = prefix.empty() ? name : prefix + "/" + name;
        if (type->InheritsFrom(TDirectory::Class())) {
          if (auto* child = dir->GetDirectory(name.c_str())) self(self, child, path);
        }
        else if (type->InheritsFrom(TH1::Class())) {
          TH1* histogram = nullptr;
          dir->GetObject(name.c_str(), histogram);
          if (!histogram) continue;
          if (histogram->GetDimension() == 1) paths.th1.push_back(path);
          if (histogram->GetDimension() == 2) paths.th2.push_back(path);
        }
      }
    };
    scan(scan, directory, "");
    return paths;
  }

  inline double NormalizedDifference(double first, double second)
  {
    const double sum = first + second;
    return sum == 0.0 ? 0.0 : (first - second) / sum;
  }

  struct Result {
    double chi2 = 0.0;
    int ndof    = 0;  // Number of bins with positive combined error variance; no parameters are fitted.
    std::unique_ptr<TH1> comparison;
    std::unique_ptr<TH1> chi2Map;

    double Chi2PerBin() const { return ndof ? chi2 / ndof : 0.0; }

    void Write(TDirectory& directory) const
    {
      if (!directory.IsWritable())
        throw std::invalid_argument("ToolsTHComparison::Result::Write: directory is not writable");
      TDirectory::TContext context(&directory);
      if (comparison) comparison->Write("comparison");
      if (chi2Map) chi2Map->Write("chi2_map");
    }
  };

  inline void CheckAxis(const TAxis& first, const TAxis& second, const char* name)
  {
    if (first.GetNbins() != second.GetNbins())
      throw std::invalid_argument(std::string("CompareTH: different ") + name + " bin counts");
    for (int bin = 1; bin <= first.GetNbins() + 1; ++bin) {
      if (first.GetBinLowEdge(bin) != second.GetBinLowEdge(bin))
        throw std::invalid_argument(std::string("CompareTH: incompatible ") + name + "-axis binning");
    }
  }

  inline Result CompareTH(const TH1& first, const TH1& second, const std::function<double(double, double)>& operation)
  {
    if (!operation) throw std::invalid_argument("CompareTH: empty operation");
    const int dimension = first.GetDimension();
    if (dimension != second.GetDimension()) throw std::invalid_argument("CompareTH: different dimensions");
    if (dimension != 1 && dimension != 2)
      throw std::invalid_argument("CompareTH: only 1D and 2D histograms are supported");
    CheckAxis(*first.GetXaxis(), *second.GetXaxis(), "X");
    if (dimension == 2) CheckAxis(*first.GetYaxis(), *second.GetYaxis(), "Y");

    Result result;
    result.comparison.reset(static_cast<TH1*>(first.Clone("comparison")));
    result.chi2Map.reset(static_cast<TH1*>(first.Clone("chi2_map")));
    if (!result.comparison || !result.chi2Map) throw std::runtime_error("CompareTH: cannot clone histogram");
    result.comparison->SetDirectory(nullptr);
    result.chi2Map->SetDirectory(nullptr);
    result.comparison->Reset("ICES");
    result.chi2Map->Reset("ICES");
    result.comparison->SetTitle("Bin-by-bin comparison");
    result.chi2Map->SetTitle("#chi^{2} contribution per bin");

    for (int ix = 1; ix <= first.GetNbinsX(); ++ix) {
      const int ny = dimension == 2 ? first.GetNbinsY() : 1;
      for (int iy = 1; iy <= ny; ++iy) {
        const int bin      = dimension == 2 ? first.GetBin(ix, iy) : first.GetBin(ix);
        const double a     = first.GetBinContent(bin);
        const double b     = second.GetBinContent(bin);
        const double value = operation(a, b);
        if (!std::isfinite(value)) throw std::domain_error("CompareTH: operation returned a non-finite value");
        result.comparison->SetBinContent(bin, value);
        const double ea       = first.GetBinError(bin);
        const double eb       = second.GetBinError(bin);
        const double variance = ea * ea + eb * eb;
        if (variance <= 0.0 || !std::isfinite(variance)) continue;
        const double contribution = (a - b) * (a - b) / variance;
        if (!std::isfinite(contribution)) throw std::domain_error("CompareTH: non-finite chi-square contribution");
        result.chi2Map->SetBinContent(bin, contribution);
        result.chi2 += contribution;
        ++result.ndof;
      }
    }
    return result;
  }

  inline std::string SanitizeName(std::string name)
  {
    for (char& character : name)
      if (character == '/') character = '_';
    return name;
  }

}  // namespace ToolsTHComparison

#endif  // TOOLS_TH_COMPARISON_H