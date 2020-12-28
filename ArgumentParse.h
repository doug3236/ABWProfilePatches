#pragma once
// process command line options
// example of two argument option:
//   procFlag("-x", arglist, option1, option2);  // arglist=vector<strings), option1= first variable, option2=second variable
// variables may be either strings, ints, doubles, or bools. Bools do not remove an argument but set option variable to true
// other variables are set with converted argument strings which are then removed

#include <vector>
#include <string>
#include <type_traits>

namespace ArgHelp {
	static void fixup(const std::string &argcmd, bool& arg) {}	// dummy, bool variable set by procFlag(), no arg consumed
    static void fixup(const std::string &argcmd, std::string &arg) { arg = argcmd; }
	template<class T>
	std::enable_if_t<std::is_floating_point<T>::value>
        static fixup(const std::string &argcmd, T& arg) { arg = static_cast<T>(stod(argcmd)); }
	template<class T>
	std::enable_if_t<std::is_integral<T>::value>
        static fixup(const std::string &argcmd, T& arg) { arg = stoi(argcmd); }

	template<class T>
    static void procVal(std::vector<std::string>& arglist, size_t idx, T& arg)
	{
		fixup(arglist[idx], arg);
		arglist.erase(arglist.begin() + idx);
	}
    static void procVal(std::vector<std::string>&, size_t) {}  // nothing more to do, end option search

	template<class T, class ...TA>
    static void procVal(std::vector<std::string>& arglist, int idx, T& arg, TA&...argv)
	{
		procVal(arglist, idx, arg);     // process an argument
		procVal(arglist, idx, argv...); // then process another argument
	}
}

template<class T, class ...TA>
static bool procFlag(const char *pc, std::vector<std::string>& arglist, T& arg1, TA&...argv)
{
	using namespace ArgHelp;
	std::string flag(pc);
	if (flag[0] != '-' || flag.length() != 2)
		throw std::invalid_argument(flag);
    
    // bools may be concatenated, remove if a match is found
	if (std::is_same<bool, T>::value)
	{
        for (size_t i = 0; i < arglist.size(); i++)
        {
            if (arglist[i][0] == '-')   // may be flag or negative numerical argument
            {
                for (size_t ii = 1; ii < arglist[i].length(); ii++)
                {
                    if (arglist[i][ii] == flag[1])
                    {
                        arg1 = true;
                        if (arglist[i].length() == 2)
                            arglist.erase(arglist.begin() + i);
                        else
                            arglist[i].erase(ii,1);
                        return true;
                    }
                }
            }
        }
	}
	for (size_t i = 0; i < arglist.size(); i++)
	{
		if (arglist[i] == flag)
		{
			arglist.erase(arglist.begin() + i);
			procVal(arglist, i, arg1);		// process one argument after flag
			procVal(arglist, i, argv...);	// process more arguments, if any, after flag
			return true;
		}
	}
	return false;
}

#if 0
// ArgumentParse Test
bool procFlagTest()
{
    int pass = 0;
    {
        vector<string> args{
            "-abc",
            "-x", "3.0",
            "-de",
            "-s", "Hello There",
            "-y", "4.5", "4.6",
            "-z", "100",
            "-f"
        };
        bool a, b, c, d, e, f;
        int z1;
        float x1;
        double y1, y2;
        string s;
        procFlag("-a", args, a);
        procFlag("-b", args, b);
        procFlag("-c", args, c);
        procFlag("-d", args, d);
        procFlag("-s", args, s);
        procFlag("-e", args, e);
        procFlag("-x", args, x1);
        procFlag("-f", args, f);
        procFlag("-y", args, y1, y2);
        procFlag("-z", args, z1);
        pass++;
    }
    {
        vector<string> args{
            "-s", "Hello There",
            "-abc",
            "-x", "3.0",
            "-de",
            "-y", "4.5", "4.6",
            "-z", "100",
            "-f"
        };
        bool a, b, c, d, e, f;
        int z1;
        float x1;
        double y1, y2;
        string s;
        procFlag("-y", args, y1, y2);
        procFlag("-z", args, z1);
        procFlag("-b", args, b);
        procFlag("-s", args, s);
        procFlag("-c", args, c);
        procFlag("-a", args, a);
        procFlag("-d", args, d);
        procFlag("-e", args, e);
        procFlag("-x", args, x1);
        procFlag("-f", args, f);
        pass++;
    }
    return true;
}
#endif

