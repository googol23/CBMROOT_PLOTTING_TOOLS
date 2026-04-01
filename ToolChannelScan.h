#ifndef TOOLCHANNELSCAN_H
#define TOOLCHANNELSCAN_H

#include <yaml-cpp/yaml.h>
#include "ToolIO.h"
#include "ToolStatistic.h"

enum IssueCode{
    kInactive = -1, // No digi were ever processed
    kActive = 0,      // No statistical deviation
    kCaution = 1,   // Statistical deviation for ADC
    kBroken = 2,    // Broken channel (extensive analysis)
    kNoisy = 3     // Noisy channel S/B (On_spill/Off_spill) above analysis threshold
};

std::map<IssueCode, std::string> issue_code_str{
    {kInactive, "kInactive"},
    {kActive,     "kActive"},
    {kCaution,  "kCaution"},
    {kBroken,   "kBroken"}
};
std::map<IssueCode, int> issue_code_threshold = {
    {kInactive, 1},
    {kCaution,  1},
    {kBroken,   6}
};

IssueCode ScanChannel(  TH2D* h = nullptr,
                        int channel = 0){
    assert(h != nullptr);

    int n_of_adc = 31;
    int start   = -1;
    int end     = -1;
    int n_of_holes =  0;
    int n_of_dead_adc = 0;
    int entries = 0;

    for (int i = 1 ; i <= n_of_adc; i++ ){
        int bin_content = h->GetBinContent(channel, i);
        entries += bin_content;

        if (i > 1){
            if (!bin_content){
                n_of_dead_adc++;
                if (start == -1){
                    start = i;
                }
            }else{
                if (start != -1){
                    start = -1;
                    end = i;
                    n_of_holes++;
                }
            }
        }
    }

    if (entries < issue_code_threshold[IssueCode::kInactive]){
        return IssueCode::kInactive;
    }

    if (n_of_dead_adc > issue_code_threshold[IssueCode::kBroken]){
        return IssueCode::kBroken;
    }

    if (n_of_holes > issue_code_threshold[IssueCode::kCaution]){
        // Further analysis to the given channel is required (manual inspection might be required)

        // ---- Entries comparison with neighbor channels ----
        // Statistic<double> avg_entries_neighbor;
        // auto px = h->ProjectionX();
        // if (channel > 1 && ScanChannel(h, channel-1) == IssueCode::kActive){
        //     avg_entries_neighbor.Add(px->GetBinContent(channel-1));
        // }
        // if (channel < px->GetNbinsX() && ScanChannel(h, channel+1) == IssueCode::kActive){
        //     avg_entries_neighbor.Add(px->GetBinContent(channel+1));

        // }

        // if (entries < 0.5*avg_entries_neighbor.Mean()){
        //     return IssueCode::kBroken;
        // }
        // ---- End Entries comparison with neighbor channels ----

        return IssueCode::kCaution;
    }

    return IssueCode::kActive;
}

std::vector<IssueCode> ScanModule(TH2D* h = nullptr){
    assert(h != nullptr);
    std::vector<IssueCode> channel_codes;
    for (int i = 1 ; i <= 2048 ; i++ ){
        IssueCode code = ScanChannel(h, i);
        channel_codes.push_back(code);
    }
    return channel_codes;
}

/**
 * Scan channel and check for noisy condition
 * @param bkg Background channel distribution (OFF_spill)
 * @param sgn Signal channel distribution (ON_spill - OFF_spill)
 * @param sb_threshold  Noisy channel condition sb_threshold < S/B
 * @return list channels with boolean flag for noisy
 * **/
std::vector<bool> ScanForNoisy( TH1* bkg = nullptr,
                                TH1* sgn = nullptr,
                                double sb_threshold = 3
){
    assert(bkg != nullptr);
    assert(sgn != nullptr);
    assert(sb_threshold > 0);

    std::vector<bool> channel_flag (2048, false);

    for (int bin = 1 ; bin <= 2048 ; bin++ ){
        double bkg_entries = bkg->GetBinContent(bin);
        double sgn_entries = sgn->GetBinContent(bin);

        if (bkg_entries && sgn_entries < (bkg_entries * sb_threshold)){
            channel_flag[bin-1] = true;
        }

    }
    return channel_flag;
}

void WriteAdcCutMapYAML(std::map<int32_t, std::vector<int>>& data, const std::string& filename) {
    // Create a YAML node
    YAML::Node root;

    // Create a sequence node to hold all the address and adc_cut entries
    YAML::Node asic_par_set(YAML::NodeType::Sequence);

    for (const auto& [address, values] : data) {

        // Convert the address to hex
        std::ostringstream oss;
        oss << std::hex << std::uppercase << address;
        std::string hex_address = "0x" + oss.str();

        // Create a map node for this entry
        YAML::Node entry;
        entry["address"] = hex_address;

        // Create a flow-style sequence node for adc_cut values
        YAML::Node adc_cut(YAML::NodeType::Sequence);
        for (const int value : values) {
            adc_cut.push_back(value);
        }
        adc_cut.SetStyle(YAML::EmitterStyle::Flow);

        // Add adc_cut to the entry
        entry["adc_cut"] = adc_cut;

        // Append the entry to the asic_par_set sequence
        asic_par_set.push_back(entry);
    }

    // Assign the sequence to the root node
    root["asic_par_set"] = asic_par_set;

    // Write the YAML node to a file
    std::ofstream outFile(filename);
    outFile << "---\n";
    outFile << root;
    outFile << "\n...";
}

template <typename T>
void WriteYAML(std::map<int32_t, std::vector<T>>& data, T code, const std::string& filename) {
    // Create a YAML node
    YAML::Node root;

    // Add data to the YAML node
    for (const auto& [address, values] : data) {

        std::ostringstream oss;
        oss << std::hex << std::uppercase << address;
        std::string hex_add = "0x" + oss.str();

        std::vector<int> channels;
        for (size_t i = 0; i < values.size(); ++i) {
            if (values[i] == code) {
                channels.push_back(i);
            }
        }

        std::cout << std::hex << address << ": " << std::dec << channels.size() << std::endl;


        // Create a sequence node for values and set it to flow style
        YAML::Node valuesNode(YAML::NodeType::Sequence); // Sequence for the values

        // Push the values into the sequence node
        for (const int value : channels) {
            valuesNode.push_back(value); // Push each value to the sequence
        }
        valuesNode.SetStyle(YAML::EmitterStyle::Flow);

        // Assign the flow style sequence to the key
        root[hex_add] = valuesNode; // Assign directly to the key

    }

    // Write the YAML node to a file
    std::ofstream outFile(filename);
    outFile << "---\n";
    outFile << root;
    outFile << "\n...";
}
#endif