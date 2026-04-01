
#ifndef TOOLSHAPES_H
#define TOOLSHAPES_H

std::vector<TObject*> DrawLabeledBox(double x1, double y1, double x2, double y2, const std::string &txt, int text_color = kBlack, int box_color = kYellow, double box_size_x = 0.1, double box_size_y = 0.1) {
    // Define box dimensions
    double txt_size = 0.03;
    double box_xmin = x2 - 0.5*box_size_x;
    double box_xmax = x2 + 0.5*box_size_x;
    double box_ymin = y2 - 0.5*box_size_y;
    double box_ymax = y2 + 0.5*box_size_y;

    // Draw line connecting (x1, y1) to (x2, y2)
    TLine *line = new TLine(x1, y1, x2, y2);
    line->SetLineWidth(2);
    line->SetLineColor(box_color);
    line->DrawClone();

    // Create and draw box
    TBox *box = new TBox(box_xmin, box_ymin, box_xmax, box_ymax);
    box->SetFillColor(box_color);
    box->SetLineColor(kBlack);
    box->DrawClone();

    // Draw label inside box
    TLatex *label = new TLatex(x2, y2, txt.c_str());
    label->SetTextAlign(22); // Center the text
    label->SetTextSize(txt_size);
    label->SetTextColor(text_color);
    label->DrawClone();


    return {line->Clone(), box->Clone(), label->Clone()};

}

#endif