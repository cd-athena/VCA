#*****************************************************************************
 # Copyright (C) 2026 Christian Doppler Laboratory ATHENA
 #
 # Authors: Vignesh V Menon <vignesh.menon@aau.at>
 #
 # This program is free software; you can redistribute it and/or modify
 # it under the terms of the GNU General Public License as published by
 # the Free Software Foundation; either version 3 of the License, or
 # (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with this program.
 #****************************************************************************/

cmake_minimum_required(VERSION 3.13)

# Prefer Release for single-config generators; harmless for multi-config (MSVC)
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

project(VCA LANGUAGES C CXX)

# Global settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # static libs ok in shared libs

# Extra module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Hard-stop for 32-bit builds
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR "Building for 32 bit targets is not supported.")
endif()

# Feature toggles (cached so subdirs see them)
option(ENABLE_SIMD                 "Enable SIMD optimizations (auto-disables if assembler/toolchain missing)" ON)
option(ENABLE_NASM                 "Enable use of nasm assembly on x86 if available" ON)
option(VCA_ARM_INTRINSICS_ONLY     "On ARM, use NEON intrinsics only (no ARM assembler files)" ON)
option(ENABLE_TEST                 "Enable build of Tests" OFF)
option(ENABLE_PERFORMANCE_TEST     "Enable Performance Test" OFF)

# FetchContent settings (silence CMP0135 warning from gtest etc.)
set(FETCHCONTENT_DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

# Subdirectories
add_subdirectory(source/lib)
add_subdirectory(source/apps/vca)

option(ENABLE_PERFORMANCE_TEST "Enable Performance Test" OFF)
option(ENABLE_TEST "Enable build of Tests" OFF)

if(ENABLE_PERFORMANCE_TEST)
  add_subdirectory(source/apps/vcaPerformanceTest)
endif(ENABLE_PERFORMANCE_TEST)
