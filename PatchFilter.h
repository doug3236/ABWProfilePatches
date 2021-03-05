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
#include <vector>
#include <array>

using std::vector, std::array;
using V3=array<double, 3>;
using V6=array<double, 6>;


inline V3 operator+(V3 arg1, V3 arg2) { return V3{ arg1[0] + arg2[0], arg1[1] + arg2[1], arg1[2] + arg2[2] }; }
inline V3 operator-(V3 arg1, V3 arg2) { return V3{ arg1[0] - arg2[0], arg1[1] - arg2[1], arg1[2] - arg2[2] }; }
inline V3 operator*(V3 arg, double s) { return V3{ s * arg[0], s * arg[1], s * arg[2] }; }
inline V3 operator*(double s, V3 arg) { return V3{ s * arg[0], s * arg[1], s * arg[2] }; }
inline V3 operator+(V3 arg, double s) { return V3{ s + arg[0], s + arg[1], s + arg[2] }; }
inline V3 operator+(double s, V3 arg) { return V3{ s + arg[0], s + arg[1], s + arg[2] }; }
inline V3 operator/(V3 arg1, V3 arg2) { return V3{ arg1[0] / arg2[0], arg1[1] / arg2[1], arg1[2] / arg2[2] }; }
inline V3 operator*(V3 arg1, V3 arg2) { return V3{ arg1[0] * arg2[0], arg1[1] * arg2[1], arg1[2] * arg2[2] }; }

class PatchFilter {
public:
    PatchFilter(const vector<V6>& vin);
    PatchFilter() = default;
    vector<double> get_dE00_split(int five_or_15, bool zero_ab); // Must be 5, or 15
    vector<double> get_dE00_vals();     // returns array of dE00 point v surround
    vector<V6> get_rgblab5(bool zero_ab);   // synthesized rgblab sets
    vector<V3> lab5;                // measured and filtered
    vector<double> L_projected;     // calculated L*
    vector<double> L_sRGB;          // L* of sRGB (Absolute)
    enum class Intent { REL, RELBPC, ABS } intent;
private:
    int ND;                 // distance between RGB: 1, 5
    vector<V3> lab;         // Lab (L*a*b*) values of sorted, ave, same, rgb patches
    vector<V3> labf;        // low pass filtered Lab
    vector<V3> labfx;       // low pass filtered Lab excluding center
    vector<V3> sRGB_xyz;    // XYZ from sRGB
};

vector<int> histogram(vector<double> v, double step, double last);
vector<double> distribution(vector<int>, bool accumulate=true);

V3 find_lab_interpolation(const vector<V6> & v, int x); // returns estimate 