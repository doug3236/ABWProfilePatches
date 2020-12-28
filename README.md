# ABWProfileMaker
<!-- See [create an anchor](#Technical-Overview) -->

## Program to Create B&W ICC Profiles


**ABWProfileMaker** Is a Windows based tool used to create ICC Profiles that can be
used to accurately print black and white images using programs that impliment ICC color management
like Photoshop. It is unique in comparison to other tools in that it
creates a fully functional B&W profile for use in Epson Advanced Black and White
workflows and is used in the same manner as regular color managed printing. For instance,
letting Photoshop manage "colors" and selecting Relative Intent with/without BPC.
It produces accurate soft proofs including things like the subtle tones when printing B&W sepia.

A big reason to use the printer's ABW mode is that prints are more stable and blacks are printed
more smoothly. The printer uses only small amounts of CYM inks in black and white mode. One can
see the measured difference as smaller variations in patch colors in the comparison
[here](#abw-v-printing-black-and-white-in-full-color-mode).

#### Requirements
Windows x64, 7,8, or 10.
Spectrophotometer and software than can read printed patches and produce a CGATs file.
Software than creates ICC profiles from CGATs files. Tested with Argyll and I1Profiler.
Argyll is recommended but I1Profiler, which requires a license to make profiles, can be used.

#### Overview of how it's done
The program reads a CGATs measurement file of black and white patches. It then creates two
new CGATs file with synthesized colors added. The first tosses out the color info (Lab's a* and b*)
and adds synthesized colors so that an accurate printing profile can be constructed. The
second CGATs file retains the (a* and b*) of the neutral patches and then adds synthesized
colors around them. The profile created from this file is not accurate for printing but
it is accurate for the reverse lookup that soft proofing uses. Then the program splices in the
soft proof lookup tables from the second profile into the first profile creating
a profile that can be used for both printing and soft proofing.

This is the tone curve (L*) from my Epson 9800 using Absolute Col. Intent. It's from
actual measurements of neutral patches from L*=0 to 100. It's correctly shows clipping
at the paper's darkest black and white of unprinted paper. Elsewhere, the match between requested L*
and actual printed L* is nearly perfect. If printed with Relative and BPC
the line goes straight from L=0 to 100 since the L* is expanded to 0 and 100.

![Image](refs/Epson9800LStar.png)

To minimize configuration errors I highly recommend B&W images be in sRGB colorspace.
The sRGB tone curve, while suboptimal for color,
is actually better than other working spaces for black and white. This is because the deltaE
between RGB steps is almost always lower than that of other spaces like Adobe RGB or ProPhoto
RGB as a result of the lead in ramp sRGB has. sRGB more effectively spreads out the change in L*
over the full RGB range while its smaller overall color
gamut isn't relevant to black and white images.

#### The two kinds of profiles
Standard profiles are those made from patches printed w/o "color" management.
This can be done with Epson ABW printers but not others that I've looked at like Canon.
Standard profiles can be used for soft proofing and printing B&W using Relative or Absolute
colorimetric intents using the same process as color printing with "Photoshop Manages Colors."

For printers that don't allow "Photoshop Manages Colors" in B&W mode,
you must use "Printer Manages Colors" which disables profile selection to
print the charts. 
[See "Making a Printer Managed Profile"](#making-a-printer-managed-profile-alternate)
and their usage is slightly different.

Two kinds of profiles can be made. Some people like to print black and white directly,
which the default provides, and just want to be able to soft proof their work so
they can see what a print would look like before committing to print
and possibly tweak the tone curve a bit. This is called the "Printer Managed Profile".
Others want a conventional profile that prints exactly what is on the image. If the value
in the image is L*=35, then the print should be L*=35. The other is a "Printer Managed Profile"
which can be used to soft proof but isn't (normally) involved in the actual printing.


#### Making a Photoshop Color Managed (Standard) Profile
The standard approach works in Windows with
Epson printers that offer ABW (Advanced black and white). This operates in Photoshop
just like regular printing letting Photoshop manage color printing - but with a B&W profile!
Printing should be done with Relative Colorimetric Intent. BPC is optional. Precision grays
of known L* can be printed using Absolute Colorimetric.

First, to create a standard B&W profile print the chart in the *misc* directory
that matches your instrument (I1Pro, I1Pro2, or i1iSis). Here is an image of the i!Pro 2
260 patch chart included in the *misc* directory.


![Image](refs/Repeat_5x_Neutrals_52_I1Pro2.jpg)


Make sure you save your
black and white printer settings in the printer driver so you can load the same settings
when you print in the future. I name the profile and saved settings the same. Use Adobe
ACPU or other program that bypasses color management. Alternately, you can use I1Profiler
to print the provided chart. Load the appropriate chart *.txf*, also in directory *misc*.
For instance: *Repeat_5x_Neutrals_52_I1Pro2.txf*. The charts provided
contain 260 patches which are the RGB sequence (0,0,0), (5,5,5), ... to (255,255,255)
repeated 5 times and scrambled. Scan the print with a spectrophotometer using I1Profiler
and save the measurement data as a CGATs text file by selecting "Save" then entering
a file name and selecting "i1Profiler CGATS CIELab (*.txt)".

#### Making a Printer Managed Profile (alternate)
This type of profile is required when a printer's B&W mode disallows selection
of "Photoshop Manages Colors" such as on Canon printers. But you can still make
a B&W Profile!. This requires printing the
patch chart from Photoshop instead of using ACPU or I1Profiler's direct printing.
Use your desired printer driver black and white settings.
In Photoshop, let the printer manage "color" and select Relative Colorimetric. Print and
measure the chart as described above. Now you're ready to make the Printer Managed Profile.

#### Constructing the B&W Profile
Now just execute ABWProfilerMaker with the name of the measurement file and the desired
profile name. This will make the two special CGATs files as well as print metrics and statistical
information.

    ABWProfileMaker measurementfile.txt desiredprofilename

The last step is to execute the batch file that calls Argyll utilities to create the profile:

    make_argyll_abw_profile

That's it! You now have a B&W profile suitable for use in printing and soft proofing.

#### Using Photoshop Manages Color Profiles
FOR ABW printing and soft proofing use the same procedure you use for print and soft proofing color images.
Except, of course, selecting ABW.

#### Using Printer Managed profiles
If your printer has a B&W mode but doesn't allow Photoshop,
select the "Printer Managed Profile" in the view proof dialog and then
check "Preserve RGB Numbers."

![Image](refs/SoftProofPrinterManages.jpg)

This forces the soft proof to reflect the
image as it is actually printed but only applies when using "Printer Managed Profiles."  It will
show a soft proof that reflects what your print will look like when letting the printer manage color.


Here's what the image look like printed with Photoshop Manages Color

![Image](refs/BandWTest_RelCol.jpg)

And here's what the print and image softproof looks like when printed letting the printer manage color.
The first was with the 9800 set for "darkest" tone and the second with "lighter."

![Image](refs/BandWTestDarker_PM.jpg)
![Image](refs/BandWTestLighter_PM..jpg)


##### Warning: Advanced. Technique to print colorimetrically accurate B&W prints.
One might think that since Photoshop doesn't allow you to select a profile when printing B&W
on some printers that it's not possible to achieve a colorimetrically accurate print
that is easy on ABW Epson printers. Not so.
But it requires a few extra steps. First, create a copy of the image
then convert it to the "Printer Managed Profile" using Photoshop's Edit->Convert-to-Profile dialog.
Select the conversion desired such as Rel. Col. with BPC.
Then assign (not convert) the converted image using Edit->Assign-Profile and select sRGB.
It can now be printed with accurate colorimetry using "Printer Manages Colors"
and the driver set to the proper B&W mode.
Should you choose to use this approach, then you should also soft proof the initial image (before conversion)
without setting "Preserve RGB Numbers." This is because you will be printing a colorimetrically accurate print
using this workaround.

## Description in Depth
Normally, black and white printing, when done with profiles will,
at best, not show in Photoshop the tint or deviation from neutral of the actual print.
This is problematic because prints are not perfectly neutral even when no tint is selected.
Additionally, subtle tints can provide an artistic touch to black and white prints.
It's desirable for soft proofing to show this tint. For example
the Sepia or Cold settings in Epson ABW printers.

Tools for making ICC profiles, such as those from X-Rite and the open source Argyll
assume RGB color ans so require
data sets that contain a range of RGB values and corresponding Lab (more formally L\*a\*b\*) values
that result from measuring the printed colors.
ABWProfileMaker provides a process for creating ICC compliant profiles for use in
Photoshop and other programs that use ICC profiles to accurately display
(aka softproof) and print black and white images using black and white printing modes.

Black and White modes are available on most printers designed to reproduce high
quality prints. Examples are Epson's "Advanced Black and White" mode and
Canon's Black and White Photo Print modes which are selectable in the driver.

For example, this image from a B&W crop from one of Keith Copper's (North Light Images).

![Image](refs/BandWTest.jpg)

When printed on Matte paper on an Epson 9800 using ABW mode, PK ink, 
with a maximum yellow-orange tint prints like
this using Relative Colorimetric Intent without using BPC (Black Point Compression):

![Image](refs/BandWTestRelCol.jpg)

Using BPC lightens the overall image to bring out features in darker areas that are otherwise
not printable. This is especially noticeable on matte prints with low DMax (high black L*).

![Image](refs/BandWTestRelColBPC.jpg)


## Technical Overview

The ABWProfileMaker program accomplishes this in conjunction with regular tools for making
ICC profiles by taking B&W patch measurements, including any tints, and making two
sets of RGBLAB values in CGATs (an industry interchange format). The a* and b* values are
zeroed for the first created RGBLAB set while the second set uses measured a* and b*.
Profiles are made from each of these. The first set is zeroed because we want the profile to map
neutral RGB values to neutrals to send to the printer. This provides accurate L* for 
the B&W print. The second profile keeps the a* and b* values. This provides accurate
data for soft proofing. Since this data is in the AtoB profile tables, this table is
then copied into the first profile replacing its AtoB tables.
Both CGATs files contain additional, synthesized, non-neutral
RGB and Lab values. The synthesized colors are needed so that the profile software works correctly
since it expects a color gamut, not black and white data.

Note that the profile is for use only when printing B&W images using the printer's B&W mode.

### ABWProfileMaker Detailed use:

Several steps are required to create a ABW profile. First a set of neutral steps is generated and printed.
Second, the printed chart is scanned with a spectrophotometer like an XRite I1 Pro 2 or i1iSis and
the CGATs measurements including RGB and L*a*b* values are saved. Third, the measurement file
is read by ABWProfileMaker which creates 2 CGATs files as well as a batch file which uses Argyll to
create 2 profiles and ABWProfileMaker then splices the AtoB tables of the second profile into the first.

Target charts for printing and/or loading into I1Profiler are provided in the misc directory
for the supported spectrophotometers.
These were made from the command: **ABWProfileMaker -S 5**, which creates a 260 patch set.
Also provided are measurement files and the created profiles
in neutral, cool, and sepia ABW settings for the Epson 9800 using glossy paper. You can use the
measurement files to test out your Argyll installation.

Here is how ABWProfileMaker was used to create the profile used for the images above printed on an
Epson 9800 with ABW. 

Spectro measured CGATs files are provided in the "misc" directory. These files
are from a set of 260 patches which are (0,0,0), (5,5,5), ...(255,255,255) repeated 5 times
then scrambled to minimize spectro reading errors when creating the profile. The following command
reads in the black and white CGATs file and creates 2 synthetic CGATs files containing RGB and Lab
info. Synthetic colors are added where RGB are non neutral so that profiling software can
create profiles but accurate info for the RGB/Lab neutrals are retained. Part of the process
prints out statistical information,
including paper white and black ink L*, that is useful for looking at the "lumpiness" and random
variation of the measured patches. See
[B&W statistics](#these-are-the-statistics-printing-black-and-white-images-using-abw)

    ABWProfileMaker 9800_Y_ABW52x5Matte_M2.txt 9800_ABW_Y_Matte

Then you run the newly created batch file. It generates 2 profiles using Argyll profiling
tools then splices the AtoB table from the second profile into the first to enable soft proofing.
Temporary files are removed. This executes the batch file and creates the profile:
"9800_ABW_Y_Matte.icm".

    make_argyll_abw_profile

Alternately, if you have a licensed I1Profiler you can make profiles manually
from the two created CGATs files then execute ABWProfileMaker with the profile name in the same
order as the batch command file does it. Results are effectively identical. I prefer Argyll since
I1Profiler doesn't offer batch processing.

Now install the profile or copy to your computer's profile directory:
"C:\Windows\System32\spool\drivers\color"

### ABWProfileMaker command line options
You can also run the program in steps that create the patch set, process spectro measurements,
and splice reverse lookup color into the printing profile.
Here's the instructions you get just running the program with no arguments:

    -----ABWProfileMaker V2.0-----
         ---------- Step 1 -----------
    ABWProfilePatches S|L [n]
      Creates RGB CGATS file Where S generates 52 RGB patches 0:5:255,
      L generates 256 RGB patches 0:1:255 and the optional [n] is number
      of pattern repeats

         ---------- Step 2 -----------
    ABWProfilePatches MeasurementFilename.txt ProfileName
      If only MeasurementFilename is given, just display statistics, otherwise
      Reads a ABW CGATS measurement file of neutral patches and creates
      synthetic RGBLAB CGATs files named "ProfileName.txt" and "ProfileName_adj.txt"
      from which one creates ICC profiles. Then make profiles from these two files
      using an automatically created batch file "make_argyll_abw_profile.bat"
      if you have Argyll's software installed or manually with a program like I1Profiler

         ---------- Step 3 -----------
    ABWProfilePatches Profile
      Where profile is the name of base profile with a suffix of ".icm"
      There must be two profiles from the previous step. The second profile has the
      same name with "_adj" added. The A2B1 tables inside the Profile_adj.icm
      will replace the A2B1 table inside Profile.icm. Discard the "_adj" profile.
      and install the first.


## Installation
A Windows x86 (32 bit) executable is provided. For those that wish to use Linux or
iOS, there is standard source code.
The provided C++17 code is portable and requires no libraries. Just load the
`.h` and `.cpp` files in the same directory, compile the source files,
and link into an executable.


### ABW v Printing Black and White in Full Color Mode
The second step in making the synthesized patch sets has an additional feature. It prints out
a distribution that shows how much dE20000 variation occurs between steps of 5 and 15.
This provides information on how smooth the neutral tone transitions are.  Additionally,
if duplicates were selected when creating the patch set, it prints out the statistical
distribution of L*, a* and b* between the same patches. This is useful for checking the
consistency the printer.

One easy way to see how much better ABW mode over just using standard color printing to
print black and white images is to compare ABW against printing the neutral patches using
standard color settings. I see over a 2 to 1 reduction in errors using ABW.

Here is a comparison of the statistics for Glossy (PK Ink) on the Epson 9800 using
ABW and using the same, device neutral patch set with full color selected.

These are the statistics in color mode. This shows that adding CYM inks, even with
device neutral patches (R=G=B), which is required for color printing,
produces much higher measurement variations.

##### These are the statistics printing black and white images using full color mode


    -----ABWProfilePatches V2.0 in color printing mode-----

    Statistics for: 9800Neutrals52x5_dev_M2.txt

    White Point L*a*b*: 94.89 -1.54 -2.00
    Black Point L*a*b*:  3.59 -0.29 -0.91
    At RGB130   L*a*b*: 32.14 -1.07  0.55

    ---Patch deltaE2000 variations B&W printed using full color mode---
    These are deltaE2000 variations from the averages of RGB patches
    comparing patch values with those of adjacent patches either 5 RGB
    steps or 15 RGB steps away.  Also shown are the deltaE200 variations
    but with a* and b* ignored (eg, 5z, 15z).  This is useful to evaluate
    Luminance without color shifts from neutral. These variations are much
    smaller since a* and b* contribute heavily to deltaE2000 calculations.
    Note: L* a* and b* are standard deviations of individual patches, not
    dE2000, and are only printed when the charts have duplicated RGB patches

    Steps (with ab zeroed)       5    15      5z   15z       L*    a*    b*
     50 Percent of dE00s <=   0.11  0.67    0.03  0.31     0.13  0.08  0.10
     75 Percent of dE00s <=   0.18  0.82    0.08  0.38     0.18  0.12  0.12
     90 Percent of dE00s <=   0.23  0.96    0.11  0.54     0.24  0.16  0.14
     95 Percent of dE00s <=   0.33  1.22    0.14  0.72     0.25  0.21  0.15
     98 Percent of dE00s <=   0.38  1.65    0.17  0.80     0.34  0.25  0.18
    100 Percent of dE00s <=   0.52  2.12    0.19  0.96     0.37  0.27  0.26


##### These are the statistics printing black and white images using ABW


    -----ABWProfilePatches V2.0 in ABW mode -----

    Statistics for: 9800Neutrals52x5_ABWy_M2.txt

    White Point L*a*b*: 94.89 -1.53 -2.10
    Black Point L*a*b*:  2.87  0.27 -0.26
    At RGB130   L*a*b*: 54.54  6.06 11.52

    ---Patch deltaE2000 variations B&W printed using ABW---
    Steps (with ab zeroed)       5    15      5z   15z       L*    a*    b*
     50 Percent of dE00s <=   0.04  0.26    0.02  0.11     0.10  0.05  0.08
     75 Percent of dE00s <=   0.05  0.32    0.03  0.20     0.13  0.07  0.10
     90 Percent of dE00s <=   0.08  0.43    0.05  0.24     0.15  0.08  0.13
     95 Percent of dE00s <=   0.23  0.81    0.11  0.27     0.16  0.08  0.14
     98 Percent of dE00s <=   0.30  0.94    0.11  0.30     0.20  0.10  0.16
    100 Percent of dE00s <=   0.30  1.12    0.15  0.64     0.20  0.11  0.17



### Example: Making ABW Profile for Epson 9800
Example of Epson 9800 on Matte using PK inks (high black point L*). Files are in the
*misc* directory. PK ink has quite a high L* minimum (darkest black) but better shows
soft proofing. Here's the statistics.


##### Create Synthetic CGATs Files from B&W Patch File
    G:\test>ABWProfileMaker 9800_Y_ABW52x5Matte_M2.txt 9800_ABW_Y_Matte

    -----ABWProfileMaker V2.0-----
    Creating synthetic patch sets
      From: 9800_Y_ABW52x5Matte_M2.txt
      To:   9800_ABW_Y_Matte.txt
      And:  9800_ABW_Y_Matte_adj.txt
    And a batch file to make Argyll profiles: make_argyll_abw_profile.bat

    White Point L*a*b*: 92.89  0.95 -0.90
    Black Point L*a*b*: 24.32  0.36 -0.02

          ---Patch deltaE2000 variations---
    These are deltaE2000 variations from the averages of RGB patches
    comparing patch values with those of adjacent patches either
    5 RGB steps or 15 RGB steps away.  Also shown are the deltaE200
    variations but with a* and b* ignored.  This is useful to evaluate
    Luminance without color shifts from neutral. These variations are much
    smaller since a* and b* contribute heavily to deltaE2000 calculations.
    Note: L* a* and b* are standard deviations of individual patches, not
    dE2000, and are only printed when the charts have duplicated RGB patches

    Steps (with ab zeroed)       5    15      5z   15z       L*    a*    b*
     50 Percent of dE00s <=   0.03  0.20    0.02  0.10     0.10  0.03  0.07
     75 Percent of dE00s <=   0.04  0.25    0.03  0.20     0.12  0.04  0.08
     90 Percent of dE00s <=   0.06  0.33    0.04  0.24     0.14  0.05  0.10
     95 Percent of dE00s <=   0.07  0.45    0.04  0.25     0.15  0.06  0.10
     98 Percent of dE00s <=   0.07  0.52    0.05  0.29     0.16  0.07  0.11
    100 Percent of dE00s <=   0.08  0.55    0.05  0.30     0.17  0.08  0.15

##### Execute the Created Command File: *make_argyll_abw_profile*
    G:\test>make_argyll_abw_profile

    G:\test>set ARGYLL_CREATE_WRONG_VON_KRIES_OUTPUT_CLASS_REL_WP=1
    G:\test>txt2ti3 9800_ABW_Y_Matte.txt 9800_ABW_Y_Matte
    G:\test>colprof -r .1 -qh -D 9800_ABW_Y_Matte.icm -O 9800_ABW_Y_Matte.icm 9800_ABW_Y_Matte
    G:\test>txt2ti3 9800_ABW_Y_Matte_adj.txt 9800_ABW_Y_Matte_adj
    G:\test>colprof -r .1 -qh -D 9800_ABW_Y_Matte_adj.icm -O 9800_ABW_Y_Matte_adj.icm 9800_ABW_Y_Matte_adj
    G:\test>erase 9800_ABW_Y_Matte.ti3
    G:\test>erase 9800_ABW_Y_Matte_adj.ti3
    G:\test>erase 9800_ABW_Y_Matte.txt
    G:\test>erase 9800_ABW_Y_Matte_adj.txt

    G:\test>ABWProfileMaker 9800_ABW_Y_Matte.icm
    -----ABWProfileMaker V2.0-----
    Replaced A2B1 table in 9800_ABW_Y_Matte.icm with 9800_ABW_Y_Matte_adj.icm
    9800_ABW_Y_Matte.icm may now be used to print and softproof ABW.

    G:\test>erase 9800_ABW_Y_Matte_adj.icm
    G:\test>rem Install 9800_ABW_Y_Matte.icm in "C:\Windows\System32\spool\drivers\color"
    G:\test>

