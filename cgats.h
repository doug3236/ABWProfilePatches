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

#include <vector>
#include <utility>
#include <string>
#include <array>
#include <fstream>
#include <iostream>
#include <regex>
#include <tuple>
#include <cassert>

#include "statistics.h"

using std::vector;
using std::string;
using std::pair;
using std::array;

namespace cgats_utilities {

    using V3=array<double, 3>;  // typically used to hold RGB, LAB or XYZ values
    using V6=array<double, 6>;  // typically used to hold RGBLAB value sets

    vector<V6> read_cgats_rgblab(string filename, bool include_lab = true);
    vector<V3> read_cgats_rgb(string filename);

    bool write_cgats_rgb(vector<V3> rgb, string filename);

    bool write_cgats_rgblab(vector<V6> rgblab, string filename, string descriptor = "RGBLAB");
    pair<vector<V3>, vector<V3>> separate_rgb_lab(const vector<V6>& rgblab);
    vector<V6> combine_rgb_lab(const vector<V3>& rgb, const vector<V3>& lab);

    // predicate for sort by RGB values
    bool less_than(const V6& arg1, const V6& arg2);
    // Overrides operator== for array<> to look at only the first 3 values
    bool operator==(V6 arg1, V6 arg2);
    
    // used to collect statistics when combining the same RGB patch values
    struct DuplicateStats {
        V6 rgb_lab;
        Statistics lab[3];
    };

    // sort and remove duplicates from patch sets
    vector<DuplicateStats> remove_duplicates(vector<V6> vals);
}