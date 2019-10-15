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
#include <array>

using std::vector, std::array;
using V3=array<double, 3>;
using V6=array<double, 6>;

class PatchFilter {
public:
    PatchFilter(const vector<V3>& vin);     // filters Lab values based on spread size (ND)
    PatchFilter(const vector<V6>& vin);
    vector<double> get_dE00_split(int five_or_15, bool zero_ab); // Must be 5, or 15
    vector<double> get_dE00_vals();     // returns array of dE00 point v surround
    vector<V6> get_rgblab5(bool zero_ab);
private:
    const int ND;           // distance between RGB: 1, 5
    vector<V3> lab;         // Lab (L*a*b*) values of sorted, ave, same, rgb patches
    vector<V3> labf;        // low pass filtered Lab
    vector<V3> labfx;       // low pass filtered Lab excluding center
};

vector<int> histogram(vector<double> v, double step, double last);
vector<double> distribution(vector<int>, bool accumulate=true);

V3 find_lab_interpolation(const vector<V6> & v, int x); // returns estimate 