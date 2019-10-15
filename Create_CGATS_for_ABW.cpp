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

#include <iostream>
#include <string>
#include <locale>
#include "Create_CGATS_for_ABW_Routines.h"

using std::cout;
using std::string;

void usage()
{
    const char* message = {
        "     ---------- Step 1 -----------\n"
        "ABWProfilePatches [S|L] [n]\n"
        "  Creates RGB CGATS file Where S generates 52 RGB patches 0:5:255,\n"
        "  L generates 256 RGB patches 0:1:255 and the optional [n] is number\n"
        "  of pattern repeats\n\n"

        "     ---------- Step 2 -----------\n"
        "ABWProfilePatches MeasurementFilename [ProfileName]\n"
        "  MeasurementFilename and ProfileName\n"
        "  Reads a ABW CGATS measurement file of neutral patches and optionally creates\n"
        "  synthetic RGBLAB CGATs files named \"ProfileName.txt\" and \"ProfileName_adj.txt\"\n"
        "  from which Argyll or I1Profiler can createICC profiles.\n"
        "  Then make profiles from these two files.\n"
        "  If only MeasurementFilename is given, just display statistics\n\n"

        "     ---------- Step 3 -----------\n"
        "ABWProfilePatches Profile\n"
        "  Where profile is the name of base profile with a suffix of \".icm\"\n"
        "  There must be two profiles from the previous step. The second profile has the\n"
        "  same name with \"_adj\" added. The A2B1 tables inside the Profile_adj.icm\n"
        "  will replace the A2B1 table inside Profile.icm.\n\n"
        "" };
    std::cout << message;
    exit(0);
}

int main(int argc, char** argv)
{
    vector<string> args;
    for (size_t i = 1; i < argc; i++)       // copy arguments into string vector and make suffixes lower case
    {
        string data = argv[i];
        auto p = find(data.begin(), data.end(), '.');
        for (; p < data.end(); p++)
            *p = std::tolower(*p);
        args.push_back(data);
    }

    cout << "-----ABWProfileMaker V1.2-----\n";
    if (argc == 1)
        usage();
    try {
        if (args[0] == "S")             //ABWProfilePatches S [n]
            {
            int rept{};
            if (args.size() == 2) try { rept = std::stoi(args[1]); } catch (std::exception e) { }
            make_RGB_for_ABW("Neutrals_52.txt", 52, rept);
        }
        else if (args[0] == "L")        //ABWProfilePatches L [n]
        {
            int rept{};
            if (args.size() == 2) try { rept = std::stoi(args[1]); } catch (std::exception e) {}
            make_RGB_for_ABW("Neutrals_256.txt", 256, rept);
        }
        else if (args.size() == 1 && is_suffix_txt(args[0]))
        {
            cout << "Statistics for: " << args[0] << "\n\n";
            LabStats stats = process_cgats_measurement_file(args[0]);
            void print_stats(const LabStats & stats);
            print_stats(stats);
        }
        else if (args.size()==2 && is_suffix_txt(args[0]) && is_suffix_txt(args[1]))
        {
            cout << "Creating synthetic patch sets\n  From: " << args[0] << "\n"
            "  To:   " << args[1] << "\nas well as an _adj file.\n\n"
            "Use these two CGATs files to create ICC profiles.\n\n\n";

            LabStats stats = process_cgats_measurement_file(args[0]);
            cgats_utilities::write_cgats_rgblab(stats.rgblab_neutral, args[1]);
            cgats_utilities::write_cgats_rgblab(stats.rgblab_tint, replace_suffix(args[1], ".txt", "_adj.txt"));

            void print_stats(const LabStats & stats);
            print_stats(stats);
        }
       else if (args.size() == 1 && is_suffix_icc(args[0]))
        {
            if (is_suffix_icc(args[0]))
            {
                cout << "Replacing A2B1 table in " << args[0] << " with " << replace_suffix(args[0], ".icm", "_adj.icm") << "\n\n";
                replace_icc1_A2B1_with_icc2_A2B1(args[0], replace_suffix(args[0], ".icm", "_adj.icm"));
            }

            else
                usage();
        }
        else
        {
            usage();
        }
    }
    catch (std::exception e) {
        std::cout << e.what() << "\n\n\n";
        usage();
    }
    return 0;
}


void print_stats(const LabStats& stats)
{
    printf("White Point L*a*b*:%6.2f %5.2f %5.2f\n"
        "Black Point L*a*b*:%6.2f %5.2f %5.2f\n"
        "At RGB130   L*a*b*:%6.2f %5.2f %5.2f\n\n",
        stats.white_point[0], stats.white_point[1], stats.white_point[2],
        stats.black_point[0], stats.black_point[1], stats.black_point[2],
        stats.lab_rgb130[0], stats.lab_rgb130[1], stats.lab_rgb130[2]);
    printf("      ---Patch deltaE2000 variations---\n"
        "These are deltaE2000 variations from the averages of RGB patches\n"
        "comparing patch values with those of adjacent patches either\n"
        "5 RGB steps or 15 RGB steps away.  Also shown are the deltaE200\n"
        "variations but with a* and b* ignored.  This is useful to evaluate\n"
        "Luminance without color shifts from neutral. These variations are much\n"
        "smaller since a* and b* contribute heavily to deltaE2000 calculations.\n"
        "Note: L* a* and b* are standard deviations of individual patches, not\n"
        "dE2000, and are only printed when the charts have duplicated RGB patches\n\n"
        "Steps (with ab zeroed)       5    15      5z   15z       L*    a*    b*\n");

    for (size_t i = 0; i < stats.percents.size(); i++)
    {
        if (stats.repeats >= 2)
            printf("%3.0f Percent of dE00s <=  %5.2f %5.2f   %5.2f %5.2f    %5.2f %5.2f %5.2f\n", stats.percents[i],
                stats.distributionp_5[i],
                stats.distributionp_15[i],
                stats.distributionp_ab0_5[i],
                stats.distributionp_ab0_15[i],
                stats.distributionp_std_L[i],
                stats.distributionp_std_a[i],
                stats.distributionp_std_b[i]);
        else
            printf("%3.0f Percent of dE00s <=  %5.2f %5.2f   %5.2f %5.2f\n", stats.percents[i],
                stats.distributionp_5[i],
                stats.distributionp_15[i],
                stats.distributionp_ab0_5[i],
                stats.distributionp_ab0_15[i]);
    }
    printf("\n\n");
}
