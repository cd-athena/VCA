*********************
Command Line Options
*********************

.. _string-options-ref:

Note that unless an option is listed as **CLI ONLY** the option is also
supported by vca_param_parse(). The CLI uses getopt to parse the
command line options so the short or long versions may be used and the
long options may be truncated to the shortest unambiguous abbreviation.
Users of the API must pass vca_param_parse() the full option name.

The API user must call vca_param_default_preset() with the preset and
tune parameters they wish to use, prior to calling vca_param_parse() to
set any additional fields. The CLI does this for the user implicitly, so
all CLI options are applied after the user's preset and tune choices,
regardless of the order of the arguments on the command line.

Generally, when an option expects a string value from a list of strings
the user may specify the integer ordinal of the value they desire. ie:
:option:`--log-level` 3 is equivalent to :option:`--log-level` debug.

Executable Options
==================

.. option:: --help, -h

	Display help text

	**CLI ONLY**

.. option:: --version, -V

	Display version details

	**CLI ONLY**

Command line executable return codes::

	0. encode successful
	1. unable to parse command line
	2. unable to open analyzer
	3. unable to generate stream headers
	4. analyzer abort

Logging/Statistic Options
=========================

.. option:: --log-level <integer|string>

	Controls the level of information displayed on the console.
	
	0. error
	1. warning
	2. info **(default)**
	3. debug
	4. full

.. option:: --no-progress

	Disable periodic progress reports from the CLI

	**CLI ONLY**

.. option:: --complexity-csv <filename>

	Write the spatial (E) and temporal complexity (h) statistics to a Comma
	Separated Values log file. Creates the file if it doesn't already exist.

	The following statistics are available:
	
	**POC** Picture Order Count - The display order of the frames. 
	
	**E** Spatial complexity for the frame. 
	
	**h** Temporal complexity for the frame.
	
	**epsilon** Applicable only when shot detection is enabled. This is used
	to determine the IDR-frame position of the new shot.

.. option:: --shot-csv <filename>

	Write the shot id, the first frame and last frame of every shot to a Comma
	Separated Values log file. Creates the file if it doesn't already exist.

Performance Options
===================

.. option:: --asm <integer:false:string>, --no-asm

	VCA will use all detected CPU SIMD architectures by default. You can
	disable all assembly by using :option:`--no-asm` or you can specify
	a comma separated list of SIMD architectures to use, matching these
	strings: MMX2, SSE, SSE2, SSE3, SSSE3, SSE4, SSE4.1, SSE4.2, AVX, XOP, FMA4, AVX2, FMA3, AVX512

	Some higher architectures imply lower ones being present, this is
	handled implicitly.

	One may also directly supply the CPU capability bitmap as an integer.
	
	Default: auto-detected SIMD architectures

Input/Output File Options
=========================

These options all describe the input video sequence or, in the case of
:option:`--dither`, operations that are performed on the sequence prior
to analyze. All options dealing with files (names, formats, offsets or
frame counts) are only applicable to the CLI application.

.. option:: --input <filename>

	Input filename, only raw YUV or Y4M supported. Use single dash for
	stdin.

	**CLI ONLY**

.. option:: --y4m

	Parse input stream as YUV4MPEG2 regardless of file extension,
	primarily intended for use with stdin (ie: :option:`--input` -
	:option:`--y4m`).  This option is implied if the input filename has
	a ".y4m" extension

	**CLI ONLY**

.. option:: --input-depth <integer>

	YUV only: Bit-depth of input file or stream

	**Values:** any value between 8 and 16. Default is internal depth.

	**CLI ONLY**

.. option:: --frames <integer>

	The number of frames intended to be analyzed.  It may be left
	unspecified.

.. option:: --dither

	Enable high quality downscaling to the analyzer's internal bitdepth. 
	Dithering is based on the diffusion	of errors from one row of pixels 
	to the next row of pixels in a picture. Only applicable when the 
	input bit depth is larger than 8bits. Default disabled

	**CLI ONLY**

.. option:: --input-res <wxh>

	YUV only: Source picture size [w x h]

	**CLI ONLY**

.. option:: --input-csp <integer|string>

	Chroma Subsampling (YUV only):  Only 4:0:0(monochrome), 4:2:0, 4:2:2, and 4:4:4 are supported at this time.

	0. i400 (4:0:0 monochrome) - Not supported by Main or Main10 profiles
	1. i420 (4:2:0 default)    - Supported by all HEVC profiles
	2. i422 (4:2:2)            - Not supported by Main, Main10 and Main12 profiles
	3. i444 (4:4:4)            - Supported by Main 4:4:4, Main 4:4:4 10, Main 4:4:4 12, Main 4:4:4 16 Intra profiles
	4. nv12
	5. nv16

.. option:: --fps <integer|float|numerator/denominator>

	YUV only: Source frame rate

	**Range of values:** positive int or float, or num/denom

.. option:: --seek <integer>

	Number of frames to skip at start of input file. Default 0

	**CLI ONLY**

.. option:: --frames, -f <integer>

	Number of frames of input sequence to be analyzed. Default 0 (all)

	**CLI ONLY**

Analyzer configuration options
========================

.. option:: --max-blocksize <16/32/64>

	The size of the non-overlapping blocks used to determine the E, h features. Default 64
	
.. option:: --shot-detect <0/1>

	The size of the non-overlapping blocks used to determine the E, h features. Default 0
	
.. option:: --min-thresh <float>

	The minimum threshold of epsilon for shot detection. Default 10

.. option:: --max-thresh <float>

	The maximum threshold of epsilon for shot detection. Default 50	