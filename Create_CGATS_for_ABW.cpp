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
        "  L generates 256 RGB patches 0:1:255 and n (default 2) is number\n"
        "  of pattern repeats\n\n"

        "     ---------- Step 2 -----------\n"
        "ABWProfilePatches MeasurementFilename ProfileName\n"
        "  MeasurementFilename and ProfileName\n"
        "  Reads a ABW CGATS measurement file of neutral patches and creates\n"
        "  synthetic RGBLAB CGATs files named \"ProfileName.txt\" and \"ProfileName_adj.txt\"\n"
        "  from which Argyll or I1Profiler can createICC profiles.\n"
        "  Then make profiles from these two files.\n\n"

        "     ---------- Step 3 -----------\n"
        "ABWProfilePatches Profile\n"
        "  Where profile is the name of base profile with a suffix of \".icm\"\n"
        "  There must be two profiles from the previos step. The second profile has the\n"
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

    cout << "-----ABWProfilePatches V1.1-----\n\n";
    if (argc == 1)
        usage();
    try {
        if (args[0] == "S")             //ABWProfilePatches S [n]
            {
            int rept{};
            if (args.size() == 2) try { rept = std::stoi(args[1]); } catch (std::exception e) { }
            make_RGB_for_ABW("Neutrals_52.txt", 52, rept);
            if (rept==0)
                std::cout << "Making 52 patch set\n" << '\n';
            else
                std::cout << "Making randomized 52 patch set repeated " << rept << " times\n";
        }
        else if (args[0] == "L")        //ABWProfilePatches L [n]
        {
            int rept{};
            if (args.size() == 2) try { rept = std::stoi(args[1]); } catch (std::exception e) {}
            make_RGB_for_ABW("Neutrals_256.txt", 256, rept);
            if (rept == 0)
                std::cout << "Making 256 patch set\n" << '\n';
            else
                std::cout << "Making randomized 256 patch set repeated " << rept << " times\n";
        }
       else if (args.size()==2 && is_suffix_txt(args[0]) && is_suffix_txt(args[1]))
        {
            cout << "Creating synthetic patch sets\n  From: " << args[0] << "\n"
                "  To:   " << args[1] << "\nas well as an _adj file.\n\n"
                "Use these two CGATs files to create ICC profiles.\n";

            LabStats stats = make_RGBLAB_CGATS_for_ABW(args[0], args[1]);
            cout << "This is the distribution of dE00s between each RGB location and the average\n"
                "of the RGB locations 5 steps to either side of it in percentages of patches at\n"
                "or below a dE00.\n\n";
            for (const auto x : stats.distribution_5)
            {
                static double d = 0;
                printf("    At or below %3.1f:  %4.1f%%\n", d+=.1, 100*x);
            }
            cout << "\nThis is the distribution of dE00s between each RGB location and the average\n"
                "of the RGB locations 15 steps to either side of it in percentages of patches at\n"
                "or below a dE00.\n\n";
            for (const auto x : stats.distribution_15)
            {
                static double d = 0;
                printf("    At or below %3.1f:  %4.1f%%\n", d += .1, 100 * x);
            }
            cout << "\nThis is the distribution of standard deviation of L* of the same\n"
                "RGB patches when RGB duplicates exist.\n\n";
                for (const auto x : stats.distribution_std_L)
                {
                    static double d = 0;
                    printf("    At or below %3.1f:  %4.1f%%\n", d += .1, 100 * x);
                }
                cout << "\nThis is the distribution of standard deviation of a* of the same\n"
                    "RGB patches when RGB duplicates exist.\n\n";
                for (const auto x : stats.distribution_std_a)
                {
                    static double d = 0;
                    printf("    At or below %3.1f:  %4.1f%%\n", d += .1, 100 * x);
                }
                cout << "\nThis is the distribution of standard deviation of b* of the same\n"
                    "RGB patches when RGB duplicates exist.\n\n";
                for (const auto x : stats.distribution_std_b)
                {
                    static double d = 0;
                    printf("    At or below %3.1f:  %4.1f%%\n", d += .1, 100 * x);
                }
        }
       else if (args.size() == 1 && is_suffix_icc(args[0]))
        {
            if (is_suffix_icc(args[0]))
                replace_icc1_A2B1_with_icc2_A2B1(args[0], replace_suffix(args[0],".icm", "_adj.icm"));
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


