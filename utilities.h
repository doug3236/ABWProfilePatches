#pragma once
#include <vector>
#include <array>
#include <algorithm>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <optional>
#include <utility>
#include <chrono>
#include <thread>
#include <string>

//#include "ArgumentParse.h"
using namespace std;

using RGB = array<double, 3>;

extern "C"
{
	// Use littleCMS to process LAB array to RGB device values, returns false if no profile
	bool convert_lab_to_device_rgb(const char* profile, const double lab[][3], double rgb[][3], int len, int colorimetric_mode);
}
struct Options {
	bool TestRun = false;                       // -t                   Test run. Don't create output. Use to get statistics
	bool EmitFractionals = false;               // -f                   Emit fractional RGB values, ints are default
	bool RemoveGridNeutrals = false;            // -r                   Remove neutrals in grids (use when adding neutrals separately)
	bool DontRemoveRGBDuplicates = false;		// -d					Don't Remove Duplicates in RGB grids
	bool Randomize = false;                     // -v                   Scramble Patches
	int MGrid = 8;                              // -m MajorGridDim      Major Grid dimension
	int RptCntMgrid = 1;                        // -M repeatcount       Major Grid Repeat Count
	int LGrid = 0;                              // -l LowGridDim        Low Grid Dimension (scales and places half spacing packed grid at low RGB values)
	int RptCntLgrid = 1;                        // -L repeatcount       Low Grid Repeat Count
	int HGrid = 0;                              // -h HighGridDim       High Grid Dimension (scales and places half spacing packed grid at high RGB values)
	int RptCntHgrid = 1;                        // -H repeatcount       Hiogh Grid Repeat Count
	int StaggerVal = 2;                         // -s staggerVal        Stagger alternate RGB neutrals by this magnitude
	int Neutrals = 100;                         // -n Counts            Counts neutrals (device or colorimetric) are added
	int ColorimetricMode = 1;                   // -C Color mode        1 (Colorimetric) or 3 (Absolute Colorimetric)
	string OutCGATsFile = "TargetPatches.txt";  // -o CGATsFilename     The name of the ICC profile used to generaete tracking neutrals
	string Profile = "";                        // -i ICC profile       Profile used to adapt neutrals to actuals
	string Creator = "D. Gray";                 // -c creator           Who dunnit
};

string descriptor();

template<typename T>
T powi(T v, int exponent) { auto r = v;  for (int i = 2; i <= exponent; i++) r *= v; return r; };

bool different(const RGB& a, const RGB& b, double thresh = .5);

template<typename T>
void get_console_value(T & val)
{
	string s;
	std::getline(cin, s);
	if constexpr (is_arithmetic_v<T>)
		val = std::stoi(s);
	else
		val = s;
}

//inline RGB operator-(const RGB & a, const RGB & b) { return RGB{ a[0] - b[0], a[1] - b[1], a[2] - b[2] }; }
//inline RGB operator+(const RGB & a, const RGB & b) { return RGB{ a[0] + b[0], a[1] + b[1], a[2] + b[2] }; }


vector<RGB> append(const vector<RGB>& rgb1, const vector<RGB>& rgb2);

vector<RGB> add_stagger(vector<RGB> rgb, int stagger, bool fill_no_tracking=false);

// scale a RGB vector
void scale(vector<RGB>& rgb, double factor);

template<typename T>
vector<RGB> operator+(const vector<RGB> & rgb, const T & arg)
{
	vector<RGB> ret = rgb;
	if constexpr (is_same<vector<RGB>, T>::value)
	{
		if (rgb.size() != arg.size())
			throw std::invalid_argument("vectors must have the same size");
		for (auto i = 0; i < ret.size(); i++)
			ret[i] = ret[i] + arg[i];
	}
	else if constexpr (is_same<RGB, T>::value)
	{
		for (auto i = 0; i < ret.size(); i++)
			ret[i] = ret[i] + arg;
	}
	else
		for (auto i = 0; i < ret.size(); i++)
			for (int ii = 0; ii < 3; ii++)
				ret[i][ii] = ret[i][ii] + arg;
	return ret;
}


// Make a vector<RGB> 3D grid N elements per side. Optionally space by "spacing" units, offset [0:1)
vector<RGB> make_grid(int N, double spacing = 0, double offset = 0);

// Get size of packed grids with and without neutrals
int gridSize(int N, bool excludeNeutrals = false);

