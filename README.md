# ABWProfileMaker

## What problem does it solve:
Normally, black and white printing, when done with profiles, for instance using Quad Tone RIP, will
at best, not show in Photoshop the tint or deviation from neutral of the actual print.
This is problematic because prints are not perfectly neutral even when no tint is selcted.
Additionally, subtle tints can provide an artistic touch to black and white prints.
Tools for making ICC profiles, such as those from X-Rite and the open source Argyll require
data sets that contain a range of RGB values and corresponding Lab (more formally L\*a\*b\*) values
result from measuring the printed colors.

ABWProfileMaker provides a process for creating ICC complient profiles for use in Photoshop and other programs
that use ICC profiles to accurately display (aka softproof) and print black and white images using
black and white printing modes.

Black and White (and gray) modes are available on most printers designed to reproduce high
quality prints. Examples are Epson's "Advanced Black and White" mode and
Canon's Black and White Photo Print modes which are seledctable in the driver.

For example, this image from a B&W crop from one of Keith Copper's (North Light Images).

![Image](BandWTest.jpg)

When printed on Matte paper on an Epson 9800 using ABW mdoe with a maximum orange tint prints like
this using Relative Colorimetric Intent without using BPC (Black Point Compression):

![Image](BandWTestRelCol.jpg)

Using BPC lightens the overall image to bring out features in darker areas that are otherwise
not printable.

![Image](BandWTestRelColBPC.jpg)


## Overview

The ABWProfileMaker program accomplishes this in conjunction with regular tools for making
ICC profiles by taking B&W patch measuements, inclduing any tints, and making two
sets of RGB, Lab values in CGATs (an industry interchange format).
Profiles are made from each of these. The second profile
has the a\* and b\* values cleared. Both CGATs files contain additional, synthesized, non-neutral
RGB and Lab values. The synthesized colors are needed so that the profile software works correctly
since it expects a color gamut, not black and white data data.

As a result, one profile contains correct lookup tables for printing (B2A tables) while the
other contains correct values for converting the RGB values the printer sees (A2B tables)
to the colors actually printed. Both must be correct to get good prints and to see what
will be printed in Photoshop's View Soft Proof dialog.

## ABWProfileMaker use:

There are three uses of the program.

* The first creates sets of CGATs neutral RGB patches with
spacings of 5 (RGB (0,0,0; 5,5,5; ... 255,255,255). Additionally, the set can be
duplicated multiple times and scrambled to minimize any process variation such as measurement
noise. The file will be labeled: Neutrals_52.txt or Neutrals_256.txt.
The command to create this RGB CGATs file is:  
    `ABWProfileMaker [S|L] [n]`  
If a repeat count [n] is used the file will be Repeat_n_Neutrals_52.txt
or Repeat_n_Neutrals_256.txt. This can then be loaded 
dragged and dropped into the patches icon) into XRite's
I1Profiler to create charts to print. Save the printer's B&W settings as it's important
to use these same settings when printing future B&W images using this generated profile.
and measure with one of XRite'sspectrophotometers. The measurement file must be saved as CGATs file.

* Then run the program:  
    `ABWProfileMaker measurementfile.txt profilename.txt`  
This will create `profilename.txt` and `profilename_adj.txt`. with additional synthesized
colors. Now create the two profiles keeping the same names. They will be `profilename.icm`
and `profilename_adj.icm`. You can use I1Profiler if licensed to make profiles or Argyll's
programs which are free. I recommend high quality settings but the defaults are fine.
Just use the same settings for both.

* With both icm profiles in the same directory. Run:  
    `ABWProfileMaker profilename.icm`  
This copies the A2B1 table from `profilename_adj.icm` into `profilename.icm`'s A2B1 table
making a profile that correctly both prints and soft proofs black and white images.

## Installation
The provided C++17 code is portable and requires no libraries. Just load the
`.h` and `.cpp` files in the same directory, compile the source files,
and link into an executable. A Windows (7 or later) executable is provided.

A simple batch file is provided that executes the commands for those that wish
to use Argyll's profiling tools.


### notes:
The second step in making the syntheized patch sets has an additional feature. It prints out
a distribution tha shows how much dE20000 variation occurs between steps of 5 and 15.
This provides information on how smooth the neutral tone transitions are.  Additonally,
if duplicates were selected when creating the patch set, it prints out the statistical
distribution of L*, a* and b* between the same patches. This is useful for checking the
consistency the printer.

One easy way to see how much better ABW mode over just using standard color printing to
print black and white images is to compare ABW against printing the neutral patches using
standard color settings. I see over a 2 to 1 reduction in errors using ABW.

