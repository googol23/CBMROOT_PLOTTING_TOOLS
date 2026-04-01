#ifndef TOOLCOLORS_H
#define TOOLCOLORS_H

std::vector<int> Colors1D = {
    TColor::GetColor(  0,   0,   0),     // kBlack
    TColor::GetColor( 23,  57,  92),     // #17395C
    TColor::GetColor(239, 183,  88),     // #EFB758 "Yellow"
    TColor::GetColor(194,  66,  41),     // #C24229 "Red"
    TColor::GetColor(015, 103, 183),     // "Blue"
};

struct RGB{
    int r;
    int g;
    int b;
};
std::vector<RGB> petroff_color_cycle_06 = {
    {150,  74, 139}, // #964A8B
    {228,  37,  54}, // #e42536
    {248, 156,  32}, // #F89C20
    { 87, 144, 252}, // #5790FC
    {156, 156, 161}, // #9C9CA1
    {122,  33, 221}  // #7A21DD
};


std::vector<int> color_blind_safe = {
    TColor::GetColor(  0,   0,   0),     // kBlack
    TColor::GetColor(238, 119,  51),    // Orange
    TColor::GetColor(  0, 119, 187),    // Blue
    TColor::GetColor(238,  51, 119),    // Magenta
    TColor::GetColor( 51, 187, 238),    // Light blue
    TColor::GetColor(204,  51,  17),    // Maroon
    TColor::GetColor(  0, 153, 136),    // Teal
    TColor::GetColor(187, 187, 187)     // Gray
};

std::vector<int> color_large_set = {
    TColor::GetColor(  0,   0,   0),    // Black
    TColor::GetColor(238, 119,  51),    // Orange
    TColor::GetColor(  0, 119, 187),    // Blue
    TColor::GetColor(238,  51, 119),    // Magenta
    TColor::GetColor( 51, 187, 238),    // Light Blue
    TColor::GetColor(  0, 153,  51),    // Green
    TColor::GetColor(204, 187,  68),    // Yellow
    TColor::GetColor(187, 119,  51),    // Brown
    TColor::GetColor(153,  51, 204),    // Purple
    TColor::GetColor(119, 119, 119),    // Gray
    TColor::GetColor(  0,  68, 136),    // Dark Blue
    TColor::GetColor(102,  51, 153),    // Dark Purple
    TColor::GetColor(136, 204, 238),    // Light Sky Blue
    TColor::GetColor(221, 170,  51),    // Mustard Yellow
    TColor::GetColor(153, 221, 255)     // Pale Cyan
};

Int_t MyPalette[256];
void LoadCustomColormap(const char* colormapFile = "inferno.txt") {
    const int n_of_colors = 256;

    // Create arrays to hold color stops and their RGB values
    Int_t colors[n_of_colors];
    Double_t stops[n_of_colors];
    Double_t red[n_of_colors];
    Double_t green[n_of_colors];
    Double_t blue[n_of_colors];

    // Read RGB values from the colormap file
    FILE* file = fopen(colormapFile, "r");
    if (file) {
        for (int i = 0; i < n_of_colors; ++i) {
            int r, g, b;
            if (fscanf(file, "%d %d %d\n", &r, &g, &b) == 3) {
            stops[i] = i / (n_of_colors - 1.0);
            red[i] = r / 256.0;
            green[i] = g / 256.0;
            blue[i] = b / 256.0;
            }
        }
        fclose(file);
    }

    // Create the custom gradient color table
    Int_t FI = TColor::CreateGradientColorTable(n_of_colors, stops, red, green, blue, n_of_colors );

    // Save the custom pelette
    for (int i=0;i<100;i++){ MyPalette[i] = FI+i; }
}

/* Call function before getting objs */
Int_t frg_color_idx = 0;
Int_t bkg_color_idx = TColor::GetFreeColorIndex();
TColor* bkg_color = new TColor(bkg_color_idx, 38./256, 38./256, 38./256);
bool DARK_THEME_ACTIVE = false;
void UseDarkTheme(TCanvas *canvas = nullptr){
    gStyle->SetOptStat("");

    // background color
    gStyle->SetCanvasColor(bkg_color_idx);
    gStyle->SetTitleFillColor(bkg_color_idx);
    gStyle->SetStatColor(bkg_color_idx);

    // foreground color
    gStyle->SetFrameLineColor(frg_color_idx);
    gStyle->SetGridColor(frg_color_idx);
    gStyle->SetStatTextColor(frg_color_idx);
    gStyle->SetTitleTextColor(frg_color_idx);
    gStyle->SetLabelColor(frg_color_idx,"xyz");
    gStyle->SetTitleColor(frg_color_idx,"xyz");
    gStyle->SetAxisColor(frg_color_idx,"xyz");

    gROOT->ForceStyle();
    DARK_THEME_ACTIVE = true;
}


#endif