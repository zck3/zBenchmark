/*=========================================================================
 * This file is part of zBenchmark, which is a system benchmarking tool.
 * (C) 2021, 2024, 2026 Zack T Smith.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * The author may be reached at 3 at zs3 dot m3.
 *=======================================================================*/

#ifndef _DEFS_H
#define _DEFS_H

#define PROGRAM_NAME "zBenchmark"
#define PROGRAM_RELEASE "0.35"

//-----------------------------------------------------------------------------
// CHANGES
// 0.1	Basic UI with menus.
// 0.2	Basic classes.
// 0.3	Added benchmarks which are run in worker thread.
// 0.4	Run all benchmarks vs single-benchmark run.
// 0.5	Worker threads send text to main GUI thread.
// 0.6	Disk I/O benchmarking improvements.
// 0.7	Improved text sending to UI. Registration of benchmarks. 
// 0.8	Added C-based prime numbers finder benchmark.
// 0.9	Added bzip2 file compression/decompression benchmark.
// 0.10	Added C-based sequential memory bandwidth benchmark.
// 0.11	Added POVRay multi-core CPU raytracing benchmark.
// 0.12	Measurements for 1.5 GHz Rpi 4b 32- and 64-bit.
// 0.13	Abandoned use of Imagemagick, switched to Qt-based image manipulations.
// 0.14	Improved sequential memory bandwidth benchmarks.
// 0.15	Now using gnuplot to graph memory brandwidth performance.
// 0.16	Code cleanup and refactor. Benchmarks registered from main().
// 0.17	Improved file compression benchmark to use 4 compressors.
// 0.18	Improved score reporting.
// 0.19	Improved display of system information.
// 0.20	Improved detection of missing required programs.
// 0.21 Added audio compression benchmark.
// 0.22 Added C code compilation benchmark.
// 0.23 Added AES encryption benchmark.
// 0.24 Added megabit Fibonacci benchmark.
// 0.25	Added aarch64 assembly code for memory bandwidth test & big number add.
// 0.26	Added icon.
// 0.27	Added 32/64-bit floating point benchmarks. Shortened compilation test.
// 0.28	Updated baseline to be 1.8 GHz Raspberry Pi 4 64-bit.
// 0.29	Updated for Qt6. Added keyboad shortcuts.
// 0.30	Added x86_64 assembly code for memory bandwidth test & big number add.
// 0.31	Added x86_64 assembly code for prime number generation.
// 0.32	Revised installer script to work around gnuplot's dependency on Qt5.
// 0.33 - Reduced WAV and MP4 file sizes to 50MB, the upper limit for Github.
//	- Revised SHA code to use EVP API.
//	- Recalibrated so that 1.0 is the Macbook Pro 2013 with Core i5 4278U.
//	- Memory bandwidth test now mirrors `bandwidth` code.
// 0.34	Added text to speech benchmark using PiperTTS.
// 0.35	Added object detection benchmark using TorchVision.
//-----------------------------------------------------------------------------
// TO DO
// * Improve "stop run" feature to kill any child process.
// * Add AI benchmark.
// * Add OpenGL benchmark.
// * More assembly code.
//-----------------------------------------------------------------------------

#define MINIMUM_WINDOW_WIDTH 640
#define MINIMUM_WINDOW_HEIGHT 480
#define PREFERRED_WINDOW_WIDTH 1000
#define PREFERRED_WINDOW_HEIGHT 680

#define ONE_MEGABYTE (1048576LU)
#define ONE_MILLION (1000000LU)

#if __WORDSIZE == 64 || defined(__aarch64__) || defined(__x86_64__) || defined(WIN64) || defined(RISCV64)
	#define IS_64BIT
#else
	#define IS_32BIT
#endif

extern bool forceStop;

#endif

// NOTE: zBenchmark compares against the Raspberry Pi 4 8GB running the 64-bit RpiOS at 1.8 GHz.
// NOTE: Disk I/O test measured against Samsung 512 GB without encryption enabled.

