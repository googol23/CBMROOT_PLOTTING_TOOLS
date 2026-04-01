#ifndef TOOLRUNINFO_H
#define TOOLRUNINFO_H

#include "ToolSetupAddress.h"

struct RunInfo {
	int year;
	std::string setup_name;
	std::vector<int32_t> sts_address;

};

std::map<int, RunInfo> mcbm_run_info = {
	{2391, RunInfo{2022, "mcbm_beam_2022_05_23_nickel", sts_address_mcbm2022}},
	{2984, RunInfo{2024, "mcbm_beam_2024_05_08_nickel", sts_address_mcbm2024}},
	{3105, RunInfo{2024, "mcbm_beam_2024_05_08_nickel", sts_address_mcbm2024}}
};

void PopulateRunInfo(){
	
}

#endif