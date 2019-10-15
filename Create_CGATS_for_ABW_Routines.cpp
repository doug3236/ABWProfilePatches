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

// Disable errors for fread and fwrite posix functions
#define _CRT_SECURE_NO_WARNINGS

#include <array>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <regex>
#include <tuple>
#include <random>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <stdexcept>
#include "Create_CGATS_for_ABW_Routines.h"
#include "cgats.h"
#include "color_conversions.h"
#include "PatchFilter.h"

using std::string;
using std::vector;
using std::array;
using std::cout;
using std::pair;

using namespace color_conversions;
using namespace cgats_utilities;

std::string replace_suffix(std::string name, string suffix, std::string replacement)
{
    if (name.size() <= suffix.size())
        throw std::invalid_argument("Suffix longer than name.");
    if (name.substr(name.size() - suffix.size(), suffix.size()) != suffix)
        throw std::invalid_argument("suffix doesn't match.");
    string sname = name.substr(0, name.size() - suffix.size());
    return sname + replacement;
}

double mult_vec(const vector<double>& v, const vector<double>& f, size_t loc)
{
    assert(loc > f.size() / 2 && loc < v.size() - f.size() / 2);
    double s = 0;
    for (int i = 0; i < f.size(); i++)
        s += f[i] * v[i + f.size() / 2];
    return s;
}
vector<double> convolve(vector<double> v1, vector<double> v2)
{
    vector<double> ret(v1.size() + v2.size() - 1);
    for (size_t i = 0; i < v1.size(); i++)
        for (size_t ii = 0; ii < v2.size(); ii++)
            ret[i + ii] += v1[i] * v2[ii];
    return ret;
}
vector<double> make_lowpass(size_t n)
{
    vector<double> ret{ 1 };
    vector<double> f1{ 1, 1 };
    for (int i = 1; i < n; i++)
        ret = convolve(ret, f1);
    auto s = std::accumulate(ret.begin(), ret.end(), 0.0);
    std::transform(ret.begin(), ret.end(), ret.begin(), [s](double x) {return x / s; });
    return ret;
}
vector<double> smooth(const vector<double>& v, size_t n)
{
    assert((n & 1) == 1); // n must be odd, degree determines levels of smoothing
    vector<double> ret(v.size());
    auto f = make_lowpass(n);
    for (size_t i = 0; i < v.size() - n + 1; i++)
        ret[i + n / 2] = std::inner_product(f.begin(), f.begin() + n, v.begin() + i, 0.0);

    for (size_t i = 1; i < n / 2+1; i++) // use decreasing low pass filters to average near end points
    {
        auto f = make_lowpass(i * 2 - 1);
        ret[i-1] = std::inner_product(f.begin(), f.begin() + f.size(), v.begin(), 0.0);
        ret[ret.size() - i] = std::inner_product(f.begin(), f.begin() + f.size(), v.end() - f.size(), 0.0);
    }
    return ret;
}


LabStats process_cgats_measurement_file(const string& filename)
{
    LabStats ret{};
    vector<DuplicateStats> rgb_lab2 = remove_duplicates(read_cgats_rgblab(filename));      // sort, average, and remove duplicates
    {   // Check that the patch file has properly constructed neutrals
        auto n = rgb_lab2.size();
        if (n != 52 && n != 18 && n != 256)
            throw std::invalid_argument("Patch file must be 18, 52, or 256 in size");
        for (const auto& x : rgb_lab2)
            if (x.rgb_lab[0] != x.rgb_lab[1] || x.rgb_lab[0] != x.rgb_lab[2] || fmod(x.rgb_lab[0],255/(n-1)) != 0.0)
                throw std::invalid_argument("Patch file must be evenly spaced neutrals");

    }
    vector<V6> rgb_labz;
    for (auto& x : rgb_lab2)
        rgb_labz.push_back(x.rgb_lab);
    vector <V6> rgb_lab = rgb_labz;
    vector<V6> rgb_labs;
    auto pb = *rgb_lab.begin();  ret.black_point = V3{ pb[3], pb[4], pb[5] };
    auto pw = *(rgb_lab.end()-1);  ret.white_point = V3{ pw[3], pw[4], pw[5] };
    ret.lab_rgb130 = V3{ rgb_lab[26][3], rgb_lab[26][4], rgb_lab[26][5] };
    for (size_t i = 0; i < rgb_lab.size(); i++)
    {
        ret.lab_average[0] += rgb_lab[i][3] / rgb_lab.size();
        ret.lab_average[1] += rgb_lab[i][4] / rgb_lab.size();
        ret.lab_average[2] += rgb_lab[i][5] / rgb_lab.size();
    }

    PatchFilter pf(rgb_lab);
    ret.rgblab_neutral = make_rgb_synth(pf, false);
    ret.rgblab_tint = make_rgb_synth(pf, true);

    auto fill_percents = [&ret](vector<double> dE, auto& percent) {
        vector<double> result(ret.percents.size());
        sort(dE.begin(), dE.end());
        for (size_t i = 0; i < ret.percents.size(); i++) {
            auto loc = (int)((.01 * dE.size() * ret.percents[i]) + .5) - 1;
            result[i] = dE[loc];
        }
        return result;
    };
    ret.distributionp_5 = fill_percents(pf.get_dE00_split(5, false), ret.percents);
    ret.distributionp_15 = fill_percents(pf.get_dE00_split(15, false), ret.percents);
    ret.distributionp_ab0_5 = fill_percents(pf.get_dE00_split(5, true), ret.percents);
    ret.distributionp_ab0_15 = fill_percents(pf.get_dE00_split(15, true), ret.percents);

    // multiple samples on each patch ?
    bool multiple = std::all_of(rgb_lab2.begin(), rgb_lab2.end(), [](DuplicateStats& x) {return x.lab[0].n() >= 2; });
    ret.repeats = rgb_lab2[0].lab[0].n();
    if (multiple)
    {
        vector<double> std_L, std_a, std_b;
        for (auto& x : rgb_lab2)
        {
            std_L.push_back(x.lab[0].std());
            std_a.push_back(x.lab[1].std());
            std_b.push_back(x.lab[2].std());
        }
        ret.distributionp_std_L = fill_percents(std_L, ret.percents);
        ret.distributionp_std_a = fill_percents(std_a, ret.percents);
        ret.distributionp_std_b = fill_percents(std_b, ret.percents);
    }
    return ret;
}


void make_RGB_for_ABW(const string& filename, int count, int randomize_and_repeat) {
    if (count != 52 && count != 256)
        throw std::runtime_error("Illegal first argument.");
    vector<V3> neut;
    for (double i = 0; i <= 255; i += count == 52 ? 5 : 1)  // double used instead of int for portability
        if (randomize_and_repeat > 1)
            for (int ii = 0; ii < randomize_and_repeat; ii++)
                neut.push_back(V3{ i,i,i });
        else
            neut.push_back(V3{ i,i,i });
    if (randomize_and_repeat > 0)
    {
        std::shuffle(neut.begin(), neut.end(), std::mt19937());
        write_cgats_rgb(neut, "Repeat_" + std::to_string(randomize_and_repeat) + "x_" + filename);
        cout << "Creating " << "Repeat_" + std::to_string(randomize_and_repeat) + "x_" + filename << '\n';
    }
    else
    {
        write_cgats_rgb(neut, filename);
        cout << "Creating " << filename << '\n';
    }
}


vector<V6> make_rgb_synth(PatchFilter& pf, bool color)
{
    //auto dE = pf.get_dE00_split(5, false);
    //auto dEz = pf.get_dE00_split(5, true);
    auto rgblab_bw = pf.get_rgblab5(!color);
    auto rgblab = rgblab_bw;
    int N = 6;
    size_t index = rgblab.size();
    rgblab.resize(index + N * N * N - N);

    V6 sRGB_Steps;
    for (int i = 0; i < 6; i++)
    {
        auto lab_est = find_lab_interpolation(rgblab, i * 51);
        sRGB_Steps[i] = L_to_sG(lab_est[0]);		// find rgb B&W value for each L*
    }

    for (int i = 0; i < N; i++)
        for (int ii = 0; ii < N; ii++)
            for (int iii = 0; iii < N; iii++)
                if (i != ii || i != iii)            // Don't add in neutrals from syntheized colors
                {
                    V3 rgb{ i * 51., ii * 51., iii * 51. };
                    V3 rgbp{ sRGB_Steps[i], sRGB_Steps[ii], sRGB_Steps[iii] };		// synthesize sRGB colors
                    V3 lab = sRGB_to_Lab(rgbp);
                    V3 lab_offset { 0.,0.,0. };
                    if (color)
                    {
                        auto near = std::find_if(rgblab_bw.begin(), rgblab_bw.end(), [lab](auto x) {return x[3] > lab[0]; });
                        auto xoff = near - rgblab_bw.begin();
                        lab_offset = *((V3*)&near[0]+1);
                        lab[1] += lab_offset[1];
                        lab[2] += lab_offset[2];
                    }
                    V6 entry{ rgb[0], rgb[1], rgb[2], lab[0], lab[1], lab[2] };		// add synthesized colors
                    rgblab[index] = entry;
                    index++;
                }

    return rgblab;
}

uint32_t endian32(const char* pc)
{
    uint32_t ret{};
    ret = (uint8_t)pc[0] << 24;
    ret |= (uint8_t)pc[1] << 16;
    ret |= (uint8_t)pc[2] << 8;
    ret |= (uint8_t)pc[3];
    return ret;
}

struct WPandA2B1
{
    size_t wp_offset;
    size_t wp_size;
    size_t atob1_offset;
    size_t atob1_size;
};


struct Tags {
    std::string id;
    std::size_t offset;
    std::size_t size;
};

std::map<std::string, Tags> get_tags(const char* pstart)
{
    std::map<std::string, Tags> ret;
    auto tagcount = endian32(pstart + 128);
    for (size_t i = 0; i < tagcount; i++)
    {
        const char* p = pstart + 132 + 12 * i;
        Tags tag = { string(p, p + 4), endian32(p + 4), endian32(p + 8) };
        ret[tag.id] = tag;
    }
    return ret;
}

WPandA2B1 get_WP_and_A2B1_info(const char* pc)
{
    WPandA2B1 ret;
    auto tags = get_tags(pc);
    auto a2b1 = tags.at("A2B1");
    auto wtpt = tags.at("wtpt");
    //auto desc = tags.at("desc");

    ret.atob1_offset = a2b1.offset;
    ret.atob1_size = a2b1.size;
    ret.wp_offset = wtpt.offset;
    ret.wp_size = wtpt.size;
    return ret;
}

vector<char> read_binary_file(const string fname)
{
    FILE* fp = fopen(fname.c_str(), "rb");
    fseek(fp, 0l, SEEK_END);
    auto length = ftell(fp);
    fseek(fp, 0l, SEEK_SET);
    vector<char> ret(length);
    auto result = fread(ret.data(), 1, length, fp);
    if (result != length)
        throw std::invalid_argument("Unable to read profile" + fname);
    fclose(fp);
    return ret;
}


void write_binary_file(const string fname, vector<char> data)
{
    FILE* fp = fopen(fname.c_str(), "wb");
    auto result = fwrite(data.data(), 1, data.size(), fp);
    if (result != data.size())
        throw std::invalid_argument("Unable to write profile" + fname);
    fclose(fp);
}

void replace_icc1_A2B1_with_icc2_A2B1(string iccpath1, string iccpath2)
{

    auto buf1 = read_binary_file(iccpath1);
    auto info1 = get_WP_and_A2B1_info(buf1.data());
    auto buf2 = read_binary_file(iccpath2);
    auto info2 = get_WP_and_A2B1_info(buf2.data());
    if (info1.atob1_size != info2.atob1_size)
        throw std::invalid_argument("Profiles " + iccpath1 + " " + iccpath2 + " have different A2B1 sizes.");

    std::copy(buf2.data() + info2.atob1_offset, buf2.data() + info2.atob1_offset + info2.atob1_size, buf1.data() + info1.atob1_offset);
    std::copy(buf2.data() + info2.wp_offset, buf2.data() + info2.wp_offset + info2.wp_size, buf1.data() + info1.wp_offset);
    write_binary_file(iccpath1, buf1);
}

bool is_suffix_icc(std::string fname)
{
    return fname.substr(fname.size() - 4, 4) == ".icm" || fname.substr(fname.size() - 4, 4) == ".icc";
}

bool is_suffix_txt(std::string fname)
{
    return fname.substr(fname.size() - 4, 4) == ".txt";
}
