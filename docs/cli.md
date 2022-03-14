Command Line Options
======================

## **Executable Options** (**CLI ONLY**)

- option:: **--help, -h**
		
	``` 
	 Display Help Text ```

- option:: **--version, -v** 

	``` 
	Display version details ```
		
******************
## **Logging/Statistic Options** (**CLI ONLY**)

- option:: **--complexity-csv < filename>** 

	``` 	
	Write the spatial (E) and temporal complexity (h), epsilon, brightness (L) statistics to a Comma Separated Values log file. Creates the file if it doesn't already exist. The following statistics are available: ```
 
	>  **POC** Picture Order Count - The display order of the frames
 
	>  **E** Spatial complexity of the frame

	>  **h** Temporal complexity of the frame

	>  **epsilon** Gradient of the temporal complexity of the frame
	
	>  **L** Brightness of the frame
	
	>  **avgU** Average U chroma component of the frame
	
	>  **energyU** Average U chroma texture of the frame
	
	>  **avgV** Average V chroma component of the frame
	
	>  **energyV** Average V chroma texture of the frame

- option:: **--shot-csv < filename>** 

	``` 
	Write the shot id, the first POC of every shot to a Comma Separated Values log file. Creates the file if it doesn't already exist. ```
	
- option:: **--yuvview-stats < filename>** 

	``` 
	Write the per block results (L, E, h) to a stats file that can be visualized using YUView. ```
	
******************

## **Performance Options**

- option:: **--no-chroma** 

	```
	VCA will analyze chroma planes by default. You can disable chroma analysis by using :option:`--no-chroma`. ```
	
- option:: **--no-simd** 

	```
	VCA will use all detected CPU SIMD architectures by default. You can disable all SIMD by using :option:`--no-simd`. ```
	
- option:: **--threads < integer>** 

	```
	Specify the number of threads to use. Default: 0 (autodetect) ```
	
******************

## **Input/Output File Options** (**CLI ONLY**)

These options describe the input video sequence or operations that are performed on the sequence prior to analyze. All options dealing with files (names, formats, offsets or
frame counts) are only applicable to the CLI application.

- option:: **--input < filename>**

	``` 
	Input filename, only raw YUV or Y4M supported. Use single dash for stdin. ```

- option:: **--y4m**

	``` 
	Parse input stream as YUV4MPEG2 regardless of file extension,	primarily intended for use with stdin (ie: :option:`--input` -
	:option:`--y4m`).  This option is implied if the input filename has a ".y4m" extension ```

- option:: **--input-depth < integer>**
 
	``` 
	YUV only: Bit-depth of input file or stream. Any value between 8 and 16. Default is 8 ```

- option:: **--input-res < wxh>**

	``` 
	YUV only: Source picture size [w x h] ```

- option:: **--input-csp < integer or string>**

	```
	Chroma Subsampling (YUV only):  Only 4:0:0(monochrome), 4:2:0, 4:2:2, and 4:4:4 are supported at this time. ```

	>  0 or i400 (4:0:0 monochrome) - Not supported by Main or Main10 profiles

	>  1 or i420 (4:2:0 default)    - Supported by all HEVC profiles

	>  2 or i422 (4:2:2)            - Not supported by Main, Main10 and Main12 profiles

	>  3 or i444 (4:4:4)            - Supported by Main 4:4:4, Main 4:4:4 10, Main 4:4:4 12, Main 4:4:4 16 Intra profiles

- option:: **--input-fps < double>**

	```
	The framerate of the input. If the input is a Y4M file, it will be read from there but can be overriden with this. ```

- option:: **--skip < integer>**

	``` 
	Number of frames to skip at start of input file. Default 0 ```

- option:: **--frames, -f < integer>**
 
	```
	Number of frames of input sequence to be analyzed. Default 0 (all) ```

******************

## **Analyzer Configuration options**

- option:: **--block-size <8/16/32>** 

	```
	Size of the non-overlapping blocks used to determine the E, h features. Default: 32 ```

- option:: **--min-thresh < double>** 

	``` 
	Minimum threshold of epsilon for shot detection ```

- option:: **--max-thresh < double>**
 
	```
	Maximum threshold of epsilon for shot detection	```
