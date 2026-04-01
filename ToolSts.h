#ifndef TOOLSTS_H
#define TOOLSTS_H

static const double sts_dx = 6. / 1024;
static const double sts_dy = sts_dx / std::tan(7.5*TMath::Pi()/180);
static const double err_sys_x = sts_dx / sqrt(12);
static const double err_sys_y = sts_dy / sqrt(12);

#endif