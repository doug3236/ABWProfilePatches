/*
Copyright (c) <2019> <doug gray>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <array>
#include <string>
#include <vector>
#include "color_conversions.h"
#include "PatchFilter.h"

using std::vector;
using std::string;

struct LabStats {
    vector<double> distribution_ab0_15;
    vector<double> distribution_ab0_5;
    vector<double> distribution_15;
    vector<double> distribution_5;

    vector<double> distribution_std_L;
    vector<double> distribution_std_a;
    vector<double> distribution_std_b;
    int repeats;
};

LabStats make_RGBLAB_CGATS_for_ABW(const string& filename, const string& filenameout);
void make_RGB_for_ABW(const string& filename, int count, int randomize_and_repeat=0);

vector<V6> make_rgb_synth(PatchFilter& pf, bool color = false);
void replace_icc1_A2B1_with_icc2_A2B1(string iccpath1, string iccpath2);
string replace_suffix(string name, string suffix, string replacement);
bool is_suffix_icc(string fname);
bool is_suffix_txt(string fname);
