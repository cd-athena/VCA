# Command Line Options

## General

- `--help, -h`

	Display Help Text

- `--version, -v`

	Display version details

## Logging/Statistic Options

- `--complexity-csv <filename>`

	Write the spatial (E) and temporal complexity (h), epsilon, brightness (L) statistics to a Comma Separated Values log file. Creates the file if it doesn't already exist. The following statistics are available:

	- `POC` Picture Order Count - The display order of the frames
	- `E` Spatial complexity of the frame
	- `h` Temporal complexity of the frame
	- `epsilon` Gradient of the temporal complexity of the frame
	- `L` Brightness of the frame

	Unless option:`--no-chroma` is used, the following chroma statistics are also available:

	- `avgU` Average U chroma component of the frame
	- `energyU` Average U chroma texture of the frame
	- `avgV` Average V chroma component of the frame
	- `energyV` Average V chroma texture of the frame

- `--shot-csv < filename>`

	Write the shot id, the first POC of every shot to a Comma Separated Values log file. Creates the file if it doesn't already exist.

- `--yuview-stats <filename>`

	Write the per block results (L, E, h) to a stats file that can be visualized using YUView.

## Performance Options

- `--no-lowpass`

	Disable lowpass DCT analysis (which is enabled by default).

- `--no-chroma`

	Disable analysis of chroma planes (which is enabled by default).
	
- `--no-simd`

	VCA will use all detected CPU SIMD architectures by default. This will disable that detection.

- `--no-dctenergy`

	Disable analysis of DCT-energy-based features (which is enabled by default).

- `--no-entropy`

	Disable analysis of entropy-based features (which is enabled by default).
	
- `--threads <integer>`

	Specify the number of threads to use. Default: 0 (autodetect).
	
## Input/Output

- `--input <filename>`

	Input filename. Raw YUV or Y4M supported. Use `stdin` for stdin. For example piping input from ffmpeg works like this:

	```
	ffmpeg.exe -i Sintel.2010.1080p.mkv -f yuv4mpegpipe - | vca.exe --y4m --input stdin
	```

- `--y4m`

	Parse input stream as YUV4MPEG2 regardless of file extension. Primarily intended for use with stdin. This option is implied if the input filename has a ".y4m" extension

- `--input-depth <integer>`
 
	Bit-depth of input file or stream. Any value between 8 and 16. Default is 8. For Y4M files, this is read from the Y4M header.

- `--input-res <wxh>`

	Source picture size [w x h]. For Y4M files, this is read from the Y4M header.

- `--input-csp <integer or string>`

	Chroma Subsampling. 4:0:0(monochrome), 4:2:0, 4:2:2, and 4:4:4 are supported. For Y4M files, this is read from the Y4M header.

- `--input-fps <double>`

	The framerate of the input. For Y4M files, this is read from the Y4M header.

- `--skip <integer>`

	Number of frames to skip at start of input file. Default 0.

- `--frames, -f <integer>`
 
	Number of frames of input sequence to be analyzed. Default 0 (all).

## Analyzer Configuration

- `--block-size <8/16/32>` 

	Size of the non-overlapping blocks used to determine the E, h features. Default: 32.

- `--min-epsthresh <double>` 

	Minimum threshold of epsilon for shot detection.

- `--max-epsthresh <double>`
 
	Maximum threshold of epsilon for shot detection.

- `--max-sadthresh <double>`
 
	Maximum threshold of h for shot detection.
