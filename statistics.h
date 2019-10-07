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
#include <cmath>

//----------------------- Statistics ----------------
// Gather statistics on data. This accumulates info on a single pass
// at the cost of increased errors for small deviations
class Statistics {
private:
    int scount;
    float vmax;
    float vmin;
    double sum;
    double sum2;
public:
	void reset(){scount=0; sum=sum2=0.0f; vmax=-999.9f, vmin=999.9f;};	// clear accumulators
    Statistics(){reset();};
    void clk(float f);			// clock in data value
    void clk(double f) { clk((float)f);};			// Accept doubles though precision is float
    float ave();				// get mean
    float stdp();				// get standard deviation, population
    float std();				// get standard deviation
    float min();				// get minimum
    float max();				// get maximum
	int n(){return scount;};
	Statistics operator+(Statistics &a);	// accumulate statistics
};

inline Statistics Statistics::operator+(Statistics &a)
{
	Statistics p;
	p.sum = a.sum+sum;
	p.sum2 = a.sum2+sum2;
	p.vmax = (a.vmax > vmax ? a.vmax : vmax);
	p.vmin = (a.vmin < vmin ? a.vmin : vmin);
	p.scount = a.scount+scount;
	return p;
}

inline void Statistics::clk(float f)
{
    scount++;
    sum += f;
    sum2 += f*f;
    if (f < vmin)
        vmin = f;
    if (f > vmax)
        vmax = f;
}

inline float Statistics::ave()
{
    return float(sum/scount);
}

inline float Statistics::stdp()
{
    return (float)sqrt((double)(sum2/scount - (sum/scount)*(sum/scount)));
}

inline float Statistics::std()
{
	return (float)sqrt((double)((sum2 - sum*(sum/scount))/(scount-1)));
}

inline float Statistics::min()
{
    return vmin;
}

inline float Statistics::max()
{
    return vmax;
}
//----------------------- Statistics  END ----------------

template <class T>
Statistics get_collection_stats(const T& v)
{
    Statistics ret;
    for (auto x : v)
        ret.clk(x);
    return ret;

}