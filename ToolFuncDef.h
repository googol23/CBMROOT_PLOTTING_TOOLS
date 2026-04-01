#ifndef TOOLFUNCDEF_H
#define TOOLFUNCDEF_H

Double_t Lorentz(Double_t *x, Double_t *par) {
    return (0.5*par[0]*par[1]/TMath::Pi()) / TMath::Max(1.e-10,(x[0]-par[2])*(x[0]-par[2]) + .25*par[1]*par[1]);
}

#endif