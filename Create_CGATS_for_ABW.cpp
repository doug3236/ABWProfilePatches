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
        "ABWProfilePatches S|L [n]\n"
        "  Creates RGB CGATS file Where S generates 52 RGB patches 0:5:255,\n"
        "  L generates 256 RGB patches 0:1:255 and the optional [n] is number\n"
        "  of pattern repeats\n\n"

        "     ---------- Step 2 -----------\n"
        "ABWProfilePatches MeasurementFilename.txt ProfileName\n"
        "  If only MeasurementFilename is given, just display statistics, otherwise\n"
        "  Reads a ABW CGATS measurement file of neutral patches and creates\n"
        "  synthetic RGBLAB CGATs files named \"ProfileName.txt\" and \"ProfileName_adj.txt\"\n"
        "  from which one creates ICC profiles. Then make profiles from these two files\n"
        "  using an automatically created batch file \"make_argyll_abw_profile.bat\"\n"
        "  if you have Argyll's software installed or manually with a program like I1Profiler\n\n"

        "     ---------- Step 3 -----------\n"
        "ABWProfilePatches Profile\n"
        "  Where profile is the name of base profile with a suffix of \".icm\"\n"
        "  There must be two profiles from the previous step. The second profile has the\n"
        "  same name with \"_adj\" added. The A2B1 tables inside the Profile_adj.icm\n"
        "  will replace the A2B1 table inside Profile.icm. Discard the \"_adj\" profile.\n"
        "  and install the first.\n\n"
        "" };
    std::cout << message;
    exit(0);
}

int main(int argc, char** argv)
{
    vector<string> args;
    for (int i = 1; i < argc; i++)       // copy arguments into string vector and make suffixes lower case
    {
        string data = argv[i];
        auto p = find(data.begin(), data.end(), '.');
        for (; p < data.end(); p++)
            *p = std::tolower(*p);
        args.push_back(data);
    }

    cout << "-----ABWProfileMaker V2.0-----\n";
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
        // print CGATs measurement statistics
        else if (args.size() == 1 && is_suffix_txt(args[0]))
        {
            cout << "Statistics for: " << args[0] << "\n\n";
            LabStats stats = process_cgats_measurement_file(args[0]);
            print_stats(stats);
        }
        // print CGATs measurement statistics and generate pseudo CGATs files for B&W
        // first file simulates printing tracking L* but zeroing a* and b* for accurate BtoA table
        // second file simulates color by providing accurate L*, a*, and b*
        // A useable ABW profile with soft proofing requires generating ICC profiles and replacing
        // the color info containing AtoB table.
        else if (args.size() == 2 && is_suffix_txt(args[0]))
        {
            string second_arg = remove_suffix(args[1]);
            cout << "Creating synthetic patch sets\n  From: " << args[0] << "\n"
                "  To:   " << second_arg + ".txt" << "\n"
                "  And:  " << second_arg + "_adj.txt" << "\n"
                "And a batch file to make Argyll profiles: " << "make_argyll_abw_profile.bat" << "\n\n";

            LabStats stats = process_cgats_measurement_file(args[0]);
            cgats_utilities::write_cgats_rgblab(stats.rgblab_neutral, second_arg + ".txt");
            cgats_utilities::write_cgats_rgblab(stats.rgblab_tint, second_arg + "_adj.txt");
            print_stats(stats);

            // Make a Windows batch command file to automate Argyll
            print_argyll_batch_command_file("make_argyll_abw_profile.bat", second_arg.data());
        }
        else if (args.size() == 1 && is_suffix_icm(args[0]))
        {
            if (is_suffix_icm(args[0]))
            {
                replace_icc1_A2B1_with_icc2_A2B1(args[0], replace_suffix(args[0], ".icm", "_adj.icm"));
                cout << "Replaced A2B1 table in " << args[0] << " with " << replace_suffix(args[0], ".icm", "_adj.icm")
                    << "\n" << args[0] << " may now be used to print and softproof ABW.\n\n";
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


