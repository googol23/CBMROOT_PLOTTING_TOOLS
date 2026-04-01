
#ifndef TOOLGRAP_H
#define TOOLGRAP_H

void SwapGraphAxis(TGraph * gr){
    Double_t x,y;
    for (int i = 0 ; i < gr->GetN(); i++){
        gr->GetPoint(i,x,y);
        gr->SetPoint(i,y,x);
    }
}

#endif