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

#include <array>
#include <vector>
#include <algorithm>
#include "color_conversions.h"
#include "PatchFilter.h"

namespace color_conversions {
    // Any double triplet
    using V3 = std::array<double, 3>;
    using std::vector;
    using std::array;

    inline V3 operator+(V3 arg1, V3 arg2) { return V3{ arg1[0] + arg2[0], arg1[1] + arg2[1], arg1[2] + arg2[2] }; }
    inline V3 operator-(V3 arg1, V3 arg2) { return V3{ arg1[0] - arg2[0], arg1[1] - arg2[1], arg1[2] - arg2[2] }; }
    inline V3 operator*(V3 arg, double s) { return V3{ s * arg[0], s * arg[1], s * arg[2] }; }
    inline V3 operator*(double s, V3 arg) { return V3{ s * arg[0], s * arg[1], s * arg[2] }; }
    inline V3 operator+(V3 arg, double s) { return V3{ s + arg[0], s + arg[1], s + arg[2] }; }
    inline V3 operator+(double s, V3 arg) { return V3{ s + arg[0], s + arg[1], s + arg[2] }; }

    // Conversion matrixes to and from (sRGB only) D50 adaptated XYZ when required
    // RGB spaces must be linear (gamma=1) and scaled 0-1
    // Adobe RGB M (Adapted to D50)
    // Conversion matrixes are from www.brucelindblooom.com
    array<V3, 3> MaRGB{
        0.6097559,  0.2052401,  0.1492240,
        0.3111242,  0.6256560,  0.0632197,
        0.0194811,  0.0608902,  0.7448387
    };

    // ProPhoto RGB M
    array<V3, 3> MppRGB{
        0.7976749,  0.1351917,  0.0313534,
        0.2880402,  0.7118741,  0.0000857,
        0.0000000,  0.0000000,  0.8252100
    };

    // sRGB M (Adapted to D50)
    array<V3, 3> MsRGB{
        0.4360747,  0.3850649,  0.1430804,
        0.2225045,  0.7168786,  0.0606169,
        0.0139322,  0.0971045,  0.7141733
    };

    // Inverse sRGB M (Adapted from D50)
    array<V3, 3> MsIRGB{
         3.1338561, -1.6168667, -0.4906146,
        -0.9787684,  1.9161415,  0.0334540,
         0.0719453, -0.2289914,  1.4052427
    };

    /**************** REENTRANT **************/
    // Multiply 3x3 Matrix by Vector producing Vector
    V3 multM3xV(const array<V3, 3> & matx, const V3& vec)
    {
        V3 prod{};
        for (int i = 0; i < 3; i++)
            for (int ii = 0; ii < 3; ii++)
                prod[i] += matx[i][ii] * vec[ii];
        return prod;
    }



    // L* to R of sRGB. Useful to convert L* neutrals to sRGB
    double L_to_sG(double L)
    {
        double v = L > 0.008856 * 903.3 ? pow((L + 16.) / 116., 3) : L / 903.3;
        double g = v < .0031308 ? 12.92 * v : 1.055 * pow(v, 1.0 / 2.4) - .055;
        return 255. * g;
    }

    V3 Lab_to_XYZ(const V3& lab)
    {
        double fy = (lab[0] + 16) / 116;
        double fz = fy - lab[2] / 200;
        double fx = lab[1] / 500 + fy;
        double xr = fx * fx * fx > .008856 ? fx * fx * fx : (116 * fx - 16) / 903.3;
        double yr = lab[0] > .008856 * 903.3 ? pow((lab[0] + 16) / 116, 3) : lab[0] / 903.3;
        double zr = fz * fz * fz > .008856 ? fz * fz * fz : (116 * fz - 16) / 903.3;
        return V3{ xr*0.964220, yr, zr*0.825210 };
    }


    vector<V3> Lab_to_XYZ(const vector<V3>& labv)
    {
        vector<V3> ret;
        for (auto& lab : labv)
            ret.push_back(Lab_to_XYZ(lab));
        return ret;
    }

    vector<V3> XYZ_to_Lab(const vector<V3>& XYZv)
    {
        vector<V3> ret;
        for (auto& XYZ : XYZv)
            ret.push_back(XYZ_to_Lab(XYZ));
        return ret;
    }

    vector<V3> sRGB_to_Lab(const vector<V3> rgb)
    {
        vector<V3> ret; ret.reserve(rgb.size());
        for (auto x : rgb)
            ret.push_back(color_conversions::sRGB_to_Lab(x));
        return ret;
    }

    vector<V3> operator+(double arg, vector<V3> v)
    {
        std::transform(v.begin(), v.end(), v.begin(), [arg](V3 a) {return arg + a; });
        return v;
    }

    vector<V3> operator*(double arg, vector<V3> v)
    {
        std::transform(v.begin(), v.end(), v.begin(), [arg](V3 a) {return arg * a; });
        return v;
    }


    /**************** REENTRANT **************/
    // Get L*a*b* from XYZ
    V3 XYZ_to_Lab(const V3& xyz)
    {
        V3 lab;
        double xr = xyz[0] / 0.964220f;
        double yr = xyz[1];
        double zr = xyz[2] / 0.825210f;
        double e = .008856f;
        double k = 903.3f;
        double fx = xr > e ? pow(xr, 1.f / 3.f) : (k * xr + 16.f) / 116.f;
        double fy = yr > e ? pow(yr, 1.f / 3.f) : (k * yr + 16.f) / 116.f;
        double fz = zr > e ? pow(zr, 1.f / 3.f) : (k * zr + 16.f) / 116.f;
        lab[0] = 116.f * fy - 16;
        lab[1] = 500.f * (fx - fy);
        lab[2] = 200.f * (fy - fz);
        return lab;
    }


    /**************** REENTRANT **************/
    // Convert XYZ to sRGB, clip negatives and over 1, and then back to XYZ
    V3 XYZ_to_sRGB_Clipped(const V3& xyz)
    {
        // When converting to sRGB check under and overflow
        V3 sRGB = multM3xV(MsIRGB, xyz);
        for (int i = 0; i < 3; i++)
        {
            if (sRGB[i] < 0) sRGB[i] = 0;
            if (sRGB[i] > 1) sRGB[i] = 1;
        }
        return multM3xV(MsRGB, sRGB);
    }

    // sRGB to Lab, sRGB scaled 0 255]
    V3 sRGB_to_Lab(const V3& rgb)
    {
        return XYZ_to_Lab(sRGB_to_XYZ(rgb));
    }


    /**************** REENTRANT **************/
    // Get XYZ (scaled 1.0) from Adobe RGB or ProPhoto RGB
    V3 ppRGB_to_XYZ(unsigned int rgbu)
    {
        auto gexp = 1.8f;
        auto gamma = [gexp](unsigned int arg) {
            V3 retV;
            retV[0] = pow(((arg) & 255) / 255.0f, gexp);
            retV[1] = pow(((arg >> 8) & 255) / 255.0f, gexp);
            retV[2] = pow(((arg >> 16) & 255) / 255.0f, gexp);
            return retV;
        };

        auto rgb = gamma(rgbu);
        V3 XYZ;
        XYZ = multM3xV(MppRGB, rgb);
        return XYZ;
    }

    /**************** REENTRANT **************/
    // Get XYZ (scaled 1.0) from Adobe RGB or ProPhoto RGB
    V3 aRGB_to_XYZ(unsigned int rgbu)
    {
        auto gexp = 2.2f;
        auto gamma = [gexp](unsigned int arg) {
            V3 retV;
            retV[0] = pow(((arg) & 255) / 255.0f, gexp);
            retV[1] = pow(((arg >> 8) & 255) / 255.0f, gexp);
            retV[2] = pow(((arg >> 16) & 255) / 255.0f, gexp);
            return retV;
        };

        auto rgb = gamma(rgbu);

        V3 XYZ;
        XYZ = multM3xV(MaRGB, rgb);
        return XYZ;
    }


    
    /**************** REENTRANT **************/
    // Get XYZ (scaled 1.0) from Adobe RGB or ProPhoto RGB
    V3 ppRGB_to_XYZ(V3 rgb)
    {
        V3 rgb_linear{ pow(rgb[0] / 255, 1.8), pow(rgb[1] / 255, 1.8), pow(rgb[2] / 255, 1.8) };
        V3 XYZ = multM3xV(MppRGB, rgb_linear);
        return XYZ;
    }

    /**************** REENTRANT **************/
    // Get XYZ (scaled 1.0) from Adobe RGB or ProPhoto RGB
    V3 aRGB_to_XYZ(V3 rgb)
    {
        V3 rgb_linear{ pow(rgb[0] / 255, 2.2), pow(rgb[1] / 255, 2.2), pow(rgb[2] / 255, 2.2) };
        V3 XYZ= multM3xV(MaRGB, rgb);
        return XYZ;
    }
    
    // sRGB to Lab, sRGB scaled 0 255]
    V3 sRGB_to_XYZ(const V3 rgb)
    {
        V3 linear_rgb;
        for (int i = 0; i < 3; i++)
        {
            double tmp = rgb[i] / 255.0;
            if (tmp < .04045)
                tmp = tmp / 12.92;
            else
                tmp = pow((tmp + .055) / 1.055, 2.4);
            linear_rgb[i] = tmp;
        }
        return multM3xV(MsRGB, linear_rgb);
    }


    /*
    Based on the article and modified from Matlab to C++:
    "The CIEDE2000 Color-Difference Formula: Implementation Notes,
    Supplementary Test Data, and Mathematical Observations, ", G. Sharma,
    W.Wu, E.N.Dalal, submitted to Color Research and Application, January 2004.
    available at http ://www.ece.rochester.edu/~/gsharma/ciede2000/
    */

    /**************** REENTRANT **************/
    // get the CIE Delta E 2000 Color Difference betweewn Two Lab Values
    double deltaE2000(const V3& lab_std, const V3& lab_sample)
    {
        const double pi = -2 * atan2(-1., 0.);
        double Lstd{ lab_std[0] }, astd{ lab_std[1] }, bstd{ lab_std[2] };
        double Lsample{ lab_sample[0] }, asample{ lab_sample[1] }, bsample{ lab_sample[2] };
        double cabarithmean = (sqrt(pow(astd, 2) + pow(bstd, 2)) + sqrt(pow(asample, 2) + pow(bsample, 2))) / 2.0f;
        double G = 0.5 * (1 - sqrt((pow(cabarithmean, 7) / (pow(cabarithmean, 7) + pow(25.f, 7)))));
        double apstd{ (1 + G) * astd };
        double apsample{ (1 + G) * asample };
        double Cpsample{ sqrt(pow(apsample,2) + pow(bsample, 2)) };
        double Cpstd{ sqrt(pow(apstd, 2) + pow(bstd, 2)) };
        double Cpprod = Cpsample * Cpstd;
        bool zcidx{ Cpprod == 0 };
        double hpstd = atan2(bstd, apstd);
        hpstd = hpstd + 2 * pi * (hpstd < 0);
        if (abs(apstd) + abs(bstd) == 0)
            hpstd = 0;
        double hpsample = atan2(bsample, apsample);
        hpsample = hpsample + 2 * pi * (hpsample < 0);
        if (abs(apsample) + abs(bsample) == 0)
            hpsample = 0;

        double dL = (Lsample - Lstd);
        double dC = (Cpsample - Cpstd);
        double dhp = (hpsample - hpstd);
        dhp = dhp - 2 * pi * (dhp > pi);
        dhp = dhp + 2 * pi * (dhp < (-pi));
        if (zcidx)
            dhp = 0;
        double dH = 2 * sqrt(Cpprod) * sin(dhp / 2);
        double Lp = (Lsample + Lstd) / 2;
        double Cp = (Cpstd + Cpsample) / 2;
        double hp = (hpstd + hpsample) / 2;
        hp = hp - (abs(hpstd - hpsample) > pi) * pi;
        hp = hp + (hp < 0) * 2 * pi;
        if (zcidx)
            hp = hpsample + hpstd;
        double Lpm502 = pow(Lp - 50, 2);
        double Sl = 1 + 0.015f * Lpm502 / sqrt(20 + Lpm502);
        double Sc = 1 + 0.045f * Cp;

        double Tx = 1 - 0.17f * cos(hp - pi / 6) + 0.24f * cos(2 * hp) + 0.32f * cos(3 * hp + pi / 30)
            - 0.20f * cos(4 * hp - 63 * pi / 180);

        double Sh = 1 + 0.015f * Cp * Tx;
        double delthetarad = (30 * pi / 180) * exp(-(pow((180 / pi * hp - 275) / 25, 2)));
        double Rc = 2 * sqrt((pow(Cp, 7)) / (pow(Cp, 7) + pow(25.f, 7)));
        double RT = -sin(2 * delthetarad) * Rc;

        const int kl = 1, kc = 1, kh = 1;
        double klSl = kl * Sl;
        double kcSc = kc * Sc;
        double khSh = kh * Sh;
        double de00 = sqrt(pow((dL / klSl), 2) + pow(dC / kcSc, 2) + pow(dH / khSh, 2) + RT * (dC / kcSc) * (dH / khSh));
        return de00;
    }

    void test() {	// Test sRGB to LAB
        vector<V3> test = { { 255.,255.,255. },{ 10.,10.,20. } };
        auto xxx = color_conversions::sRGB_to_Lab(test);  //2.9586, 1.3534, -4.9638
        vector<double> zz{ L_to_sG(5.0),L_to_sG(50) };  // L* with a=b=0) to sRGB vals 
    }
}

