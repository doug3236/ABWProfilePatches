/*
Copyright (c) <2020> <doug gray>

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
#include "cgats.h"

using std::vector;
using std::string;

using V3 = array<double, 3>;  // typically used to hold RGB, LAB or XYZ values

struct LabStats {
    // synthesized RGBLAB values for use in profiling application
    vector<V6> rgblab_neutral;
    vector<V6> rgblab_tint;
    
    // All vectors the same size (percents.size()) containing fraction
    // of samples > corresponding percents
    array<double,6> percents = { 50, 75, 90, 95, 98, 100 };
    vector<double> distributionp_ab0_15;
    vector<double> distributionp_ab0_5;
    vector<double> distributionp_15;
    vector<double> distributionp_5;
    // These used only if repeats > 1 since they are individual sample stats
    vector<double> distributionp_std_L;
    vector<double> distributionp_std_a;
    vector<double> distributionp_std_b;

    V3 white_point;
    V3 black_point;
    V3 lab_average;
    int repeats;

    vector<V3> lab;
    vector<double> rgb;
    PatchFilter patch_filter;
};

LabStats process_cgats_measurement_file(const string& filename);
void make_RGB_for_ABW(const string& filename, int count, int randomize_and_repeat=0);

vector<V6> make_rgb_synth(PatchFilter& pf, bool color = false);
void replace_icc1_A2B1_with_icc2_A2B1(string iccpath1, string iccpath2);
string replace_suffix(string name, string suffix, string replacement);
bool is_suffix_icm(string fname);
bool is_suffix_txt(string fname);
string remove_suffix(string fname);
void print_stats(const LabStats& stats, bool extended=false);
void print_argyll_batch_command_file(const char* batch_file_name, const char* pc);