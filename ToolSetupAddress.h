#ifndef TOOLSETUPADDRESS_H
#define TOOLSETUPADDRESS_H

#include <CbmStsAddress.h>
#include <CbmStsAnaBase.h>

#define CBMROOTLIB

std::vector<int32_t> sts_address_mcbm2021 = {
	0x10008002,
	0x10018002,

	0x10008402,
	0x10018402,

	0x10008412,
	0x10018412,
};

std::vector<int32_t> sts_address_mcbm2022 = {
	0x10008002,
	0x10018002,

	0x10008402,
	0x10018402,

	0x10008012,
	0x10018012,

	0x10018412,
	0x10008412,

	0x10008812,
	0x10018812,
	0x10028812
};

std::vector<int32_t> sts_address_mcbm2024 = {
	0x10000002,

	0x10008012,
	0x10018012,

	0x10008022,
	0x10018022,

	0x10008412,
	0x10018412,

	0x10008422,
  0x10018422,

	0x10008822,
  0x10018822,
  0x10028822
};

#ifdef CBMROOTLIB
std::set<int> GetLaddersInUnit(std::vector<int32_t> address_list){
	std::unique_ptr<CbmStsAnaBase> ana = std::make_unique<CbmStsAnaBase>(); // Hack to access CbmStsAddress
	std::set<int> ladders;
	for (auto & address : address_list)
		ladders.insert(CbmStsAddress::GetElementId(address, kStsLadder));
	return ladders;
}
#endif

#ifdef CBMROOTLIB
int CountLadderInUnit(std::vector<int32_t> address_list, int ladder){
	std::unique_ptr<CbmStsAnaBase> ana = std::make_unique<CbmStsAnaBase>(); // Hack to access CbmStsAddress
	int sol = 0;
	for (auto & address : address_list)
		sol += CbmStsAddress::GetElementId(address, kStsLadder) == ladder;
	return sol;
}
#endif

#ifdef CBMROOTLIB
std::set<int> GetUnitsInSetup(std::vector<int32_t> address_list){
	std::unique_ptr<CbmStsAnaBase> ana = std::make_unique<CbmStsAnaBase>(); // Hack to access CbmStsAddress
	std::set<int> units;
	for (auto  & address : address_list)
		units.insert(CbmStsAddress::GetElementId(address, kStsUnit));
	return units;
}
#endif

#ifdef CBMROOTLIB
int CountModulesInUnit(std::vector<int32_t> address_list, int unit){
	std::unique_ptr<CbmStsAnaBase> ana = std::make_unique<CbmStsAnaBase>(); // Hack to access CbmStsAddress
	int sol = 0;
	for (auto & address : address_list)
		sol += CbmStsAddress::GetElementId(address, kStsUnit) == unit;
	return sol;
}
#endif

#ifdef CBMROOTLIB
std::vector<int> ModulesPerLadder(std::vector<int32_t> address_list){
	std::vector<int> modules_per_ladder;
	for (auto & address : address_list){
		int ladder = CbmStsAddress::GetElementId(address, kStsLadder);
		if (modules_per_ladder.size() <= ladder){
			modules_per_ladder.resize(ladder+1);
		}
		modules_per_ladder[ladder]++;
	}
	return modules_per_ladder;
}
#endif

#ifdef CBMROOTLIB
std::vector<int32_t> ModulesInUnit(std::vector<int32_t> address_list, int unit){
	std::unique_ptr<CbmStsAnaBase> ana = std::make_unique<CbmStsAnaBase>(); // Hack to access CbmStsAddress
	std::vector<int32_t> sol;
	for (auto & address : address_list){
		if (CbmStsAddress::GetElementId(address, kStsUnit) == unit){
			sol.push_back(address);
		}
	}
	return sol;
}
#endif

#ifdef CBMROOTLIB
std::vector<int32_t> ModulesInLadder(std::vector<int32_t> address_list, int ladder){
	std::unique_ptr<CbmStsAnaBase> ana = std::make_unique<CbmStsAnaBase>(); // Hack to access CbmStsAddress
	std::vector<int32_t> sol;
	for (auto & address : address_list){
		if (CbmStsAddress::GetElementId(address, kStsLadder) == ladder){
			sol.push_back(address);
		}
	}
	return sol;
}
#endif

#ifdef CBMROOTLIB
void SortAddress(std::vector<int32_t> &address_list) {
	// std::unique_ptr<CbmStsAnaBase> ana = std::make_unique<CbmStsAnaBase>(); // Hack to access CbmStsAddress

    std::sort(address_list.begin(), address_list.end(), [&](int32_t a, int32_t b) {
        // Extract unit, ladder, and module
        int unitA = CbmStsAddress::GetElementId(a, kStsUnit);
        int unitB = CbmStsAddress::GetElementId(b, kStsUnit);

        if (unitA != unitB) {
            return unitA < unitB; // Sort by unit
        }

        int ladderA = CbmStsAddress::GetElementId(a, kStsLadder);
        int ladderB = CbmStsAddress::GetElementId(b, kStsLadder);

        if (ladderA != ladderB) {
            return ladderA < ladderB; // Sort by ladder
        }

        int moduleA = CbmStsAddress::GetElementId(a, kStsModule);
        int moduleB = CbmStsAddress::GetElementId(b, kStsModule);

        return moduleA < moduleB; // Sort by module
    });
}
#endif

#ifdef CBMROOTLIB
std::string NiceAddress(int32_t address, std::string format = "ULM", bool short_fmt = true){
	std::unique_ptr<CbmStsAnaBase> ana = std::make_unique<CbmStsAnaBase>(); // Hack to access CbmStsAddress
	std::stringstream strm;
	int32_t unt = CbmStsAddress::GetElementId(address, kStsUnit);
	int32_t lad = CbmStsAddress::GetElementId(address, kStsLadder);
	int32_t hld = CbmStsAddress::GetElementId(address, kStsHalfLadder);
	int32_t mod = CbmStsAddress::GetElementId(address, kStsModule);

	std::transform(format.begin(), format.end(), format.begin(), [](unsigned char c) {      return std::tolower(c);});

	std::string unt_str = short_fmt ? "U" : "Unit ";
	std::string lad_str = short_fmt ? "L" : "Ladder ";
	std::string hld_str = short_fmt ? "H" : "Half-Ladder ";
	std::string mod_str = short_fmt ? "M" : "Module ";

	if (format.find("u") != std::string::npos){
		strm << unt_str << unt << " ";
	}
	if (format.find("l") != std::string::npos){
		strm << lad_str << lad << " ";
	}
	if (format.find("h") != std::string::npos){
		strm << hld_str << hld << " ";
	}
	if (format.find("m") != std::string::npos){
		strm << mod_str << mod << " ";
	}
	return strm.str().c_str();
}
#endif
#endif