Command Line Options
======================

******************
**Executable Options (CLI ONLY)**

- option:: **--help, -h**

	> Display help text
	

- option:: **--version, -v**

	> Display version details

******************
**Logging/Statistic Options (CLI ONLY)**

- option:: **--complexity-csv < filename>**

	> Write the spatial (E) and temporal complexity (h) statistics to a Comma
	Separated Values log file. Creates the file if it doesn't already exist.
	> The following statistics are available:
		
	> 1. **POC** Picture Order Count - The display order of the frames.
	
	> 2. **E** Spatial complexity for the frame. 
	
	> 3. **h** Temporal complexity for the frame.
	
	> 4. **epsilon** Gradient of the temporal complexity for the frame.

- option:: **--shot-csv < filename>**

	> Write the shot id, the first frame and last frame of every shot to a Comma
	Separated Values log file. Creates the file if it doesn't already exist.

- option:: **--yuvview-stats < filename>**

    > Write the per block results (energy, sad) to a stats file that can be 
	visualized using YUView.

******************
**Performance Options**

- option:: **--[no-]asm**

	> VCA will use all detected CPU SIMD architectures by default. You can disable all assembly by using :option:`--no-asm`. Some higher architectures imply lower ones being present, this is handled implicitly. Default: enabled

- option:: **--threads < integer>**

    > Specify the number of threads to use.
	Default: 0 (autodetect)

******************
**Input/Output File Options (CLI ONLY)**

These options describe the input video sequence or operations that are performed on the sequence prior to analyze. All options dealing with files (names, formats, offsets or
frame counts) are only applicable to the CLI application.

- option:: **--input < filename>**

	> Input filename, only raw YUV or Y4M supported. Use single dash for stdin.

- option:: --y4m

	> Parse input stream as YUV4MPEG2 regardless of file extension,	primarily intended for use with stdin (ie: :option:`--input` -
	:option:`--y4m`).  This option is implied if the input filename has a ".y4m" extension

- option:: **--input-depth < integer>**

	> YUV only: Bit-depth of input file or stream

	> **Values:** any value between 8 and 16. Default is 8

- option:: **--input-res < wxh>**

	> YUV only: Source picture size [w x h]

- option:: **--input-csp <integer|string>**

	> Chroma Subsampling (YUV only):  Only 4:0:0(monochrome), 4:2:0, 4:2:2, and 4:4:4 are supported at this time.

	> 0. i400 (4:0:0 monochrome) - Not supported by Main or Main10 profiles
	> 1. i420 (4:2:0 default)    - Supported by all HEVC profiles
	> 2. i422 (4:2:2)            - Not supported by Main, Main10 and Main12 profiles
	> 3. i444 (4:4:4)            - Supported by Main 4:4:4, Main 4:4:4 10, Main 4:4:4 12, Main 4:4:4 16 Intra profiles

- option:: **--skip < integer>**

	> Number of frames to skip at start of input file. Default 0

- option:: **--frames, -f < integer>**

	> Number of frames of input sequence to be analyzed. Default 0 (all)


Analyzer configuration options
========================

- option:: **--block-size <8/16/32>**

	> Size of the non-overlapping blocks used to determine the E, h features. Default: 32
	
- option:: **--min-thresh < float>**

	> Minimum threshold of epsilon for shot detection

- option:: **--max-thresh < float>**

	> Maximum threshold of epsilon for shot detection	
