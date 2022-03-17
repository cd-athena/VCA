# Index

## Introduction

For online prediction in live streaming applications, selecting low-complexity features is critical to ensure low-latency video streaming without disruptions. For each frame/ video/ video segment, two features, i.e., the average texture energy and the average gradient of the texture energy are determined. A DCT-based energy function is introduced to determine the block-wise texture of each frame. The spatial and temporal features of the video/ video segment is derived from the DCT-based energy function. The Video Complexity Analyzer (VCA) project is launched in 2022, aiming to provide the most efficient, highest performance spatial and temporal complexity prediction of each frame/ video/ video segment which can be used for a variety of applications like shot/scene detection, online per-title encoding.

## About VCA

The primary objective of VCA is to become the best spatial and temporal complexity predictor for every video/ video segment which aids in predicting encoding parameters for applications like online per-title encoding. VCA is available as an open source library, published under the GPLv3 license. VCA leverages x86 SIMD and multi-threading optimizations for effective performance.
While VCA is primarily designed as a video complexity analyzer library, a command-line executable is provided to facilitate testing and development. We expect VCA to be utilized in many leading video encoding solutions in the coming years.

 - [How to build?](build.md)
 - [CLI options](cli.md)
 - [API functions](api.md)
 - [Related Publications](related_publications.md)
 - [Licensing FAQ](licensing_faq.md)
