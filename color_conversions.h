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
#include <array>
#include <vector>

namespace color_conversions {
    // Any double triplet
    using V3 = std::array<double, 3>;
    using std::vector;
    using std::array;

    // Convert L* to sRGB (0:255) value for neutrals (a*=0, b*=0)
    double L_to_sG(double L);

    // sRGB to Lab, sRGB scaled 0:255
    V3 sRGB_to_Lab(const V3& rgb);

    //// multiply 3x3 matrix by 3 vector
    //V3 multM3xV(const array<V3, 3> & matx, const V3& vec);

    // XYZ to L*a*b*  XYZ Y=0:1
    V3 XYZ_to_Lab(const V3& xyz);

    // XYZ to sRGB
    V3 XYZ_to_sRGB_Clipped(const V3& xyz);

    V3 ppRGB_to_XYZ(V3 rgb);
    V3 ppRGB_to_XYZ(unsigned int rgbu);

    V3 aRGB_to_XYZ(V3 rgb);
    V3 aRGB_to_XYZ(unsigned int rgbu);

    V3 sRGB_to_XYZ(V3 rgb);


    // vector operations
    // Convert sRGB (0:255) to lab
    vector<V3> sRGB_to_Lab(const vector<V3> rgb);


    double deltaE2000(const V3& lab_std, const V3& lab_sample);
    void test();
}
