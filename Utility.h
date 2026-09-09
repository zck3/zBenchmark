/*=========================================================================
 * This file is part of zBenchmark, which is a system benchmarking tool.
 * (C) 2021, 2026 Zack T Smith.
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

#ifndef _UTILITY_H
#define _UTILITY_H

extern "C" {
	#include <stdio.h>
	#include <string.h>
	#include <stdint.h>
	#include <stdbool.h>
	#include <limits.h>
	#include <unistd.h>
	#include <time.h>
	#include <sys/time.h>
};

extern double getPreciseTime ();

extern bool programIsInstalled (const char *name);
extern bool fileExists (const char *path);
extern long long fileSize (const char *path);
extern long fileLineCount (const char *path);
extern bool readFloatFromFile (const char *path, float *);
extern bool readLongFromFile (const char *path, long *);
extern unsigned readTrimmedLineFromFile (const char *path, char *result, unsigned maxlen);
extern unsigned readTrimmedLinesFromFile (const char *path, char **result, unsigned maxlines);
extern char *findAsset (const char *filename);
extern bool create_randomized_file (const char *path, size_t size);
extern bool hasPrefix (const char *string, const char *prefix);
extern bool hasSuffix (const char *string, const char *suffix);
extern char *contentsOfFileAtPath (const char *path);

#endif
