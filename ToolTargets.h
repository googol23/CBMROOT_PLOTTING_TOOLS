#ifndef TOOLTARGETS_H
#define TOOLTARGETS_H

#include "ToolColors.h"

double l_t0 = 20.00;
double l_k0 = 38.45;
double l_b1 = l_k0 + 4.99 + 1.2 + 0.75;
double l_k1 = l_k0 + 0.86 + 11.8 + 1.3 + 0.7;


CbmTarget ni_target;
CbmTarget t0_target;
CbmTarget k0_target;
CbmTarget b1_target;
CbmTarget k1_target;

void LoadTargets(){
	double beam_rot_y = (25.)*TMath::DegToRad();

	ni_target.SetPosition(0, 0, 0);
	ni_target.SetRotation(beam_rot_y);

	t0_target.SetPosition(-l_t0*sin(beam_rot_y), 0, -l_t0*cos(beam_rot_y));
	t0_target.SetRotation(beam_rot_y);

	k0_target.SetPosition(-l_k0*sin(beam_rot_y), 0, -l_k0*cos(beam_rot_y));
	k0_target.SetRotation(beam_rot_y);

	b1_target.SetPosition(-l_b1*sin(beam_rot_y), 0, -l_b1*cos(beam_rot_y));
	b1_target.SetRotation(beam_rot_y);

	k1_target.SetPosition(-l_k1*sin(beam_rot_y), 0, -l_k1*cos(beam_rot_y));
	k1_target.SetRotation(beam_rot_y);

}



std::map<std::string, CbmTarget*> mcbm_targets = {
	{"K1", &k1_target},
	{"B1", &b1_target},
	{"K0", &k0_target},
	{"T0", &t0_target},
	{"Ni", &ni_target}
};

std::map<std::string, double> target_distance = {
	{"Ni", 0.00},
	{"T0", l_t0},
	{"K0", l_k0},
	{"B1", l_b1},
	{"K1", l_k1},
};

std::map<std::string, int> target_color = {
	{"K1", Colors1D[0]},
	{"K0", Colors1D[1]},
	{"T0", Colors1D[2]},
	{"Ni", Colors1D[3]},
	{"B1", TColor::GetColor(petroff_color_cycle_06[5].r, petroff_color_cycle_06[5].g, petroff_color_cycle_06[5].b)}
};

#endif