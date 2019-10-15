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


#include <numeric>
#include "PatchFilter.h"
#include "color_conversions.h"
using namespace color_conversions;

static inline V3 operator+(V3 arg1, V3 arg2) { return V3{ arg1[0] + arg2[0], arg1[1] + arg2[1], arg1[2] + arg2[2] }; }
static inline V3 operator-(V3 arg1, V3 arg2) { return V3{ arg1[0] - arg2[0], arg1[1] - arg2[1], arg1[2] - arg2[2] }; }
static inline V3 operator*(V3 arg, double s) { return V3{ s * arg[0], s * arg[1], s * arg[2] }; }
static inline V3 operator*(double s, V3 arg) { return V3{ s * arg[0], s * arg[1], s * arg[2] }; }

static double mult_vec(const vector<double>& v, const vector<double>& f, int loc)
{
    double s = 0;
    for (int i = 0; i < f.size(); i++)
        s += f[i] * v[i + f.size() / 2];
    return s;
}
static vector<double> convolve(vector<double> v1, vector<double> v2)
{
    vector<double> ret(v1.size() + v2.size() - 1);
    for (int i = 0; i < v1.size(); i++)
        for (int ii = 0; ii < v2.size(); ii++)
            ret[i + ii] += v1[i] * v2[ii];
    return ret;
}
static vector<double> make_lowpass(size_t n, bool xcenter)
{
    vector<double> ret{ 1 };
    vector<double> f1{ 1, 1 };
    for (int i = 1; i < n; i++)
        ret = convolve(ret, f1);
    if (xcenter)
        ret[n / 2] = .001;  // fix corner case where filter has length 1. Forces a no change at ends
    auto s = std::accumulate(ret.begin(), ret.end(), 0.0);
    std::transform(ret.begin(), ret.end(), ret.begin(), [s](double x) {return x / s; });
    return ret;
}
static vector<double> smooth(const vector<double>& v, size_t n, bool xcenter)
{
    vector<double> ret(v.size());
    auto f = make_lowpass(n, xcenter);
    for (size_t i = 0; i < v.size() - n + 1; i++)
        ret[i + n / 2] = std::inner_product(f.begin(), f.begin() + n, v.begin() + i, 0.0);

    for (size_t i = 1; i < n / 2 + 1; i++) // use decreasing low pass filters to average near end points
    {
        auto f = make_lowpass(i * 2 - 1, xcenter);
        ret[i - 1] = std::inner_product(f.begin(), f.begin() + f.size(), v.begin(), 0.0);
        ret[ret.size() - i] = std::inner_product(f.begin(), f.begin() + f.size(), v.end() - f.size(), 0.0);
    }
    return ret;
}

static vector<V3> smooth(const vector<V3>& v3, int n, bool xcenter)
{
    vector<double> ret1(v3.size());
    vector<V3> ret(v3.size());
    auto f = make_lowpass(n, xcenter);
    for (size_t ii = 0; ii < 3; ii++)
    {
        vector<double> v;
        for (auto x : v3)
            v.push_back(x[ii]);
        for (size_t i = 0; i < v.size() - n + 1; i++)
            ret[i + n / 2][ii] = std::inner_product(f.begin(), f.begin() + n, v.begin() + i, 0.0);

        for (size_t i  = 1; i < n / 2u + 1; i++) // use decreasing low pass filters to average near end points
        {
            auto f = make_lowpass(i * 2 - 1, xcenter);
            ret[i - 1][ii] = std::inner_product(f.begin(), f.begin() + f.size(), v.begin(), 0.0);
            ret[ret1.size() - i][ii] = std::inner_product(f.begin(), f.begin() + f.size(), v.end() - f.size(), 0.0);
        }
    }
    return ret;
}


// Test constructor to check filtering
PatchFilter::PatchFilter(const vector<V3>& vin) :
    ND{ 255 / (int)(vin.size() - 1) }, lab{vin}
{
    labf = smooth(lab, ND > 1 ? 3 : 9, false);
    labfx = smooth(lab, ND > 1 ? 3 : 9, true);
}

// Populate averages of Lab values for step sizes of either 1 or 5
// lab: average Lab of all same RGB samples
// labf: Smoothed (low pass filter) of lab;
// labx: Smoothed (low pass filter) of lab but excluding sample
PatchFilter::PatchFilter(const vector<V6>& vin) :
    ND{ 255 / (int)(vin.size() - 1) }
{
    for (auto x : vin)
        lab.push_back(*((V3*)&x+1));
    labf = smooth(lab, ND > 1 ? 3 : 9, false);
    labfx = smooth(lab, ND > 1 ? 3 : 9, true);
}

// Operates on potentially averaged values. Returns dE00 of sample vs. smoothed excluding sample
// Useful for evaluating printer smoothness
vector<double> PatchFilter::get_dE00_vals()
{
    vector<double> ret;
    for (int i = 0; i < labf.size(); i++)
        ret.push_back(deltaE2000(lab[i], labfx[i]));
    return ret;
}

// Returns dE00 of Lab for sample v ave(sample-spread, sample+spread)  w option to zero a* and b*
vector<double> PatchFilter::get_dE00_split(int spread, bool zero_ab)  // spread must be 5 or 15
{
    auto labfq = labf;
    if (zero_ab)
        for (auto &x : labfq)
            x[2] = x[1] = 0;
    if (lab.size() != 52 && lab.size() != 256)
        throw std::invalid_argument("B&W Must be either 52 or 256 evenly spaced RGB values from 0:255");
    vector<double> ret;
    if (lab.size() == 52)
        spread /= 5;
    for (int i = spread; i < lab.size() - spread; i++)
    {
        double de = deltaE2000((labfq[i - spread] + labfq[i + spread])*.5, labfq[i]);
        ret.push_back(de);
    }
    return ret;
}

// Get rgblab with spacing of 5 rgb units, Optionally clear a* and b*
vector<V6> PatchFilter::get_rgblab5(bool zero_ab)
{
    vector<V6> ret;
    for (int i = 0; i < labf.size(); i++)
    {
        double i_d = i*ND; // make all values in V6 doubles
        if (zero_ab)
            ret.push_back(V6{ i_d, i_d, i_d, labf[i][0], 0., 0. });
        else
            ret.push_back(V6{ i_d, i_d, i_d, labf[i][0], labf[i][1], labf[i][2] });
    }
    return ret;
}


vector<int> histogram(vector<double> v, double step, double last)
{
    vector<int> counts;
    int i = 0;
    for (; i * step < last; i++)
    {
        auto x = std::accumulate(v.begin(), v.end(), 0, [i, step](const auto& arg1, const auto& arg2)
            {return arg2 >= i * step && arg2 < (i + 1) * step ? arg1 + 1 : arg1; });
        counts.push_back(x);
    }
    auto x = std::accumulate(v.begin(), v.end(), 0, [i, step](const auto& arg1, const auto& arg2)
        {return arg2 >= i * step ? arg1 + 1 : arg1; });
    counts.push_back(x);
    for (size_t i = counts.size() - 1; counts[i] == 0; i--)
        counts.erase(counts.end() - 1);
    return counts;
}

vector<double> distribution(vector<int>v, bool accumulate)
{
    int count = std::accumulate(v.begin(), v.end(), 0);
    if (accumulate)
        std::partial_sum(v.begin(), v.end(), v.begin());
    vector<double> ret(v.size());
    std::transform(v.begin(), v.end(), ret.begin(), [count](auto x) {return 1.0 * x / count; });
    return ret;
}


// Interpolate to specific neutral RGB element and return Lab value
// v is RGBLAB vector of (3 for RGB, 3 for Lab)
V3 find_lab_interpolation(const vector<V6>& v, int x)
{
    V3 ret;
    if (x == 0)
        ret= *((V3*)&v[0] + 1);     // hack to get Lab component of V6
    else
    {
        auto low = std::find_if(v.begin(), v.end(), [x](V6 arg) {return arg[0] >= x; }) - 1;
        auto deltaLab = *((V3*)&low[1]+1) - *((V3*)&low[0]+1);
        auto deltaR = low[1][0] - low[0][0];        // Difference in RGB
        auto deltaLAdj = deltaLab * ((x - low[0][0])/deltaR);// Adjustment
        ret = *((V3*)&low[0] + 1) + deltaLAdj;
    }
    return ret;
}

