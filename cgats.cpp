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

#include "cgats.h"
#include <fstream>
#include <utility>
#include "statistics.h"

#pragma warning(disable : 4996)

namespace cgats_utilities {
    // Read CGATs file RGB values and optional LAB values.
    // Structure is read but not used for easy extension.
    vector<V6> read_cgats_rgblab(string filename, bool include_lab)
    {
        vector<string> cgats;
        std::regex word_regex("(\\S+)");
        std::ifstream infile(filename);
        if (infile.fail())
            throw std::invalid_argument("File Read Open failed");
        enum State { Main, InDataFormat, InDataSet } state = Main;
        int field_cnt = 0;
        int rgb_start = 0;
        int lab_start = 0;
        unsigned int number_of_sets = 0;
        vector<V6> rgb_lab;

        string s;
        while (getline(infile, s))
        {
            auto words_begin = std::sregex_iterator(s.begin(), s.end(), word_regex);
            auto words_end = std::sregex_iterator();
            if (std::distance(words_begin, words_end) > 0) {
                string first_word = words_begin->str();
                if (first_word == "NUMBER_OF_FIELDS")
                    cgats.push_back(first_word);
                else if (first_word == "NUMBER_OF_SETS")
                    cgats.push_back(first_word);
                else if (state == InDataFormat) {
                    if (first_word == "END_DATA_FORMAT")
                        cgats.push_back(first_word);
                }
                else if (state == InDataSet) {
                    if (first_word == "END_DATA")
                        cgats.push_back(first_word);
                }
                else
                    cgats.push_back(s);


                if (state == Main)
                {
                    if (first_word == "NUMBER_OF_FIELDS")
                    {
                        field_cnt = stoi((++words_begin)->str());
                    }
                    else if (first_word == "BEGIN_DATA_FORMAT")
                    {
                        state = InDataFormat;
                    }
                    else if (first_word == "BEGIN_DATA")
                    {
                        state = InDataSet;
                    }
                    else if (first_word == "NUMBER_OF_SETS")
                    {
                        number_of_sets = stoi((++words_begin)->str());
                    }
                }
                else if (state == InDataFormat)
                {
                    for (int i = 0; i < field_cnt; i++, words_begin++)
                    {
                        if (words_begin->str() == "RGB_R")
                            rgb_start = i;
                        else if (words_begin->str() == "LAB_L")
                            lab_start = i;
                    }
                    if (include_lab && !lab_start || !rgb_start)	// TODO fix this. message not needed for rgb
                    {
                        if (!include_lab)
                            throw std::runtime_error("no RGB values found");
                        else
                            throw std::runtime_error("no LAB values found");
                    }
                    state = Main;
                }
                else if (state == InDataSet)
                {
                    if (rgb_lab.size() >= number_of_sets)
                        throw std::runtime_error("Malformed CGATs file.Too  many data set entries");
                    V6 entry{};
                    for (int i = 0; i < std::max(rgb_start, lab_start)+3; i++, words_begin++)
                    {
                        if (i >= rgb_start && i < rgb_start + 3)
                            entry[i - rgb_start] = stoi(words_begin->str());
                        if (include_lab && lab_start && i >= lab_start && i < lab_start + 3)
                            entry[i - lab_start + 3] = stod(words_begin->str());
                    }
                    rgb_lab.push_back(entry);
                    if (rgb_lab.size() == number_of_sets)
                        state = Main;
                }
            }
        }
        return rgb_lab;
    }


    // This reads only the RGB components of a CGATs file
    vector<V3> read_cgats_rgb(string filename)
    {
        vector<V6> rgblab = read_cgats_rgblab(filename, false);
        vector<V3> ret;
        for (auto x : rgblab)
            ret.emplace_back(V3{ x[0], x[1], x[2] });
        return ret;
    }

    // Writes minimalist RGB CGATs file
    bool write_cgats_rgb(vector<V3> rgb, string filename)
    {
        FILE* fp = fopen(filename.c_str(), "w");
        if (!fp)
            throw std::invalid_argument("File Open for WRite Failed.");
        fprintf(fp, "NUMBER_OF_FIELDS 4\n"
            "BEGIN_DATA_FORMAT\n"
            "SampleID	RGB_R	RGB_G	RGB_B\n"
            "END_DATA_FORMAT\n\n"
            "NUMBER_OF_SETS	%d\n"
            "BEGIN_DATA\n", static_cast<int>(rgb.size()));
        for (unsigned int i = 0; i < rgb.size(); i++)
            fprintf(fp, "%d\t%6.2f\t%6.2f\t%6.2f\n", i + 1, rgb[i][0], rgb[i][1], rgb[i][2]);
        fprintf(fp, "END_DATA\n");
        fclose(fp);
        return true;
    }

    // Writes CGATs file containing RGB and LAB fields only - No spectral data
    bool write_cgats_rgblab(vector<V6> rgblab, string filename, string descriptor)
    {
        FILE* fp = fopen(filename.c_str(), "w");
        if (!fp)
            throw std::invalid_argument("File Open for WRite Failed.");
        fprintf(fp,
            "CGATS.17\n"
            "NUMBER_OF_FIELDS 7\n"
            "BEGIN_DATA_FORMAT\n"
            "SAMPLE_ID RGB_R RGB_G RGB_B LAB_L LAB_A LAB_B\n"
            "END_DATA_FORMAT\n"
            "NUMBER_OF_SETS %d\n"
            "BEGIN_DATA\n", static_cast<int>(rgblab.size()));

        for (unsigned int i = 0; i < rgblab.size(); i++)
            fprintf(fp, "%d\t%6.2f\t%6.2f\t%6.2f\t%6.2f\t%6.2f\t%6.2f\n", i + 1,
                rgblab[i][0], rgblab[i][1], rgblab[i][2],
                rgblab[i][3], rgblab[i][4], rgblab[i][5]);
        fprintf(fp, "END_DATA\n");
        fclose(fp);
        return true;
    }

    // separate out RGB and LAB vectors, use auto [rgb, lab] = separate_rgb_lab(rgblab)
    pair<vector<V3>, vector<V3>> separate_rgb_lab(const vector<V6>& rgblab)
    {
        pair<vector<V3>, vector<V3>> ret;
        ret.first.reserve(rgblab.size()); ret.second.reserve(rgblab.size());
        for (auto x : rgblab)
        {
            ret.first.emplace_back(V3{ x[0], x[1], x[2] });
            ret.second.emplace_back(V3{ x[3], x[4], x[5] });
        }
        return ret;
    }

    // Combine rgb and lab vectors
    vector<V6> combine_rgb_lab(const vector<V3>& rgb, const vector<V3>& lab)
    {
        if (rgb.size() != lab.size())
            throw std::runtime_error("rgb and lab vectors must be same size");
        vector<V6> ret;
        ret.reserve(rgb.size());
        for (unsigned int i = 0; i < rgb.size(); i++)
            ret.emplace_back(V6{ rgb[i][0], rgb[i][1], rgb[i][2], lab[i][0], lab[i][1] , lab[i][2] });
        return ret;
    }

    // add the last 3 values in V6 (Lab values) to average later, check the first 3 are the same
    inline V6 add_lab(V6 arg1, V6 arg2) {
        for (int i = 0; i < 3; i++)
            if (arg1[i] != arg2[i])
                throw std::runtime_error("averaging LAB values requyires RGB values to be identical");
        for (int i = 3; i < 6; i++)
            arg1[i] += arg2[i];
        return arg1;
    }


    // predicate for sort by RGB values
    bool less_than(const V6& arg1, const V6& arg2) {
        for (int i = 0; i < 3; i++)
        {
            if (arg1[i] < arg2[i])
                return true;
            else if (arg1[i] > arg2[i])
                return false;
        }
        return false;
    }

    // Overrides operator== for array<> to look at only the first 3 values
    bool operator==(V6 arg1, V6 arg2) {
        for (int i = 0; i < 3; i++)
            if (arg1[i] != arg2[i])
                return false;
        return true;
    }

    // Sorts b&W patches, averages Labs, and removes duplicates
    vector<DuplicateStats> remove_duplicates(vector<V6> vals)
    {
        vector<DuplicateStats> ret;
        sort(vals.begin(), vals.end(), less_than);
        for (unsigned int i = 0; i < vals.size(); i++)
        {
            DuplicateStats same;
            same.rgb_lab = vals[i];
            same.lab[0].clk(vals[i][3]);
            same.lab[1].clk(vals[i][4]);
            same.lab[2].clk(vals[i][5]);
            for (; i < vals.size() - 1 && vals[i][0] == vals[i + 1][0]; i++)
            {
                same.lab[0].clk(vals[i + 1][3]);
                same.lab[1].clk(vals[i + 1][4]);
                same.lab[2].clk(vals[i + 1][5]);
            }
            same.rgb_lab[3] = same.lab[0].ave();
            same.rgb_lab[4] = same.lab[1].ave();
            same.rgb_lab[5] = same.lab[2].ave();
            ret.push_back(same);
        }
        return ret;
    }
}
