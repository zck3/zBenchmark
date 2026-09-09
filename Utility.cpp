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

#include "Utility.h"

extern "C" {
	#include <stdio.h>
	#include <stdlib.h>
	#include <sys/stat.h>
	#include <sys/time.h>
	#include <ctype.h> // isspace
}

//----------------------------------------------------------------------------
// Name:	getPreciseTime
// Purpose:	Determine precise time down to the microsecond, since 1970.
// Returns:	Time in seconds.
//----------------------------------------------------------------------------
double getPreciseTime ()
{
	struct timeval tv;
	gettimeofday (&tv, NULL);
	long long value = tv.tv_sec;
	value *= 1000000LU;
	value += tv.tv_usec;
	double t = (double)value;
	t /= 1000000.;
	return t;
}

//----------------------------------------------------------------------------
// Name:	programIsInstalled
// Purpose:	Determine if particular program is installed on this computer.
// Returns:	True if it is, false if not.
//----------------------------------------------------------------------------
bool programIsInstalled (const char *name)
{
	if (!name || !*name)
		return false;

	char command[256];
	snprintf (command, sizeof(command), "which %s > /dev/null", name);
	return 0 == system(command);
}

//----------------------------------------------------------------------------
// Name:	fileSize
// Purpose:	Obtain the size of a file.
// Returns:	Size in bytes or -1 on error.
//----------------------------------------------------------------------------
long long fileSize (const char *path)
{
	struct stat st;
	if (stat (path, &st)) {
		perror ("stat");
		return -1;
	}
	return (long long) st.st_size;
}

//----------------------------------------------------------------------------
// Name:	fileExists
// Purpose:	Determine if a file exists at specified path.
// Returns:	True if it exists, false if not.
//----------------------------------------------------------------------------
bool fileExists (const char *path)
{
	struct stat st;
	if (!stat (path, &st)) {
		return true;
	}
	return false;
}

static const char *assetDirectoryPaths[3] = {
	"/usr/share/zBenchmark/",
	"/usr/local/share/zBenchmark/",
	"./",
};

//----------------------------------------------------------------------------
// Name:	findAsset
// Purpose:	Locate asset (test file) to be used in some test.
// Returns:	Path string if found (caller frees), else NULL.
//----------------------------------------------------------------------------
char *
findAsset (const char *filename)
{
	bool found = false;
	struct stat st;

	char path[2*PATH_MAX];

	for (unsigned i=0; i < sizeof(assetDirectoryPaths)/sizeof(char*); i++) {
		snprintf (path, sizeof(path), "%s%s", assetDirectoryPaths[i], filename);

		if (!stat (path, &st)) {
printf("FOUND ASSET: %s\n",path);
			found = true;
			break;
		}
	}

	return found ? strdup(path) : NULL;
}

//----------------------------------------------------------------------------
// Name:	create_randomized_file 
// Purpose:	Creates a file of random bytes of specified size at given path.
// Returns:	True if success.
//----------------------------------------------------------------------------
bool 
create_randomized_file (const char *path, size_t size)
{
	if (!path || !size)
		return false;

	srand (0xDEADF00D);
	
	FILE *f = fopen (path, "wb");
	if (!f) {
		perror ("fopen");
		return false;
	}

#define RANDOMIZE_FILE_BUFSIZE (1<<20)
	char *buffer = (char*) malloc(RANDOMIZE_FILE_BUFSIZE);
	if (buffer) {
		setbuffer (f, buffer, RANDOMIZE_FILE_BUFSIZE);
	}

	uint32_t value;
	bool success = true;
	for (unsigned long i = 0; i < size/4; i++) {
		value = (uint32_t) rand();
		if (4 != fwrite (&value, 1, 4, f)) {
			perror ("fwrite");
			success = false;
			break;
		}
	}
	fflush (f);
	fclose (f);

	if (buffer)
		free(buffer);

	return success;
}

//----------------------------------------------------------------------------
// Name:	readTrimmedLineFromFile
// Purpose:	Reads first line of a file, removes whitespace from start+end.
// Returns:	Line length.
//----------------------------------------------------------------------------
unsigned readTrimmedLineFromFile (const char *path, char *result, unsigned maxlen)
{
	if (!path || !result || !maxlen)
		return 0;

	FILE *f = fopen (path, "r");
	if (!f) {
		perror ("fopen");
		return 0;
	}

	int ch;
	unsigned index = 0;
	bool found_nonwhitespace = false;
	while (EOF != (ch = fgetc(f))) {
		if (ch == '\n')
			break;

		// Trim the start.
		if (!found_nonwhitespace && isspace(ch))
			continue;

		found_nonwhitespace = true;
		result[index++] = ch;

		if (index == maxlen-1)
			break;
	}
	result[index] = 0;

	// Trim the end.
	unsigned length = index;
	while (index > 0) {
		index--;
		if (isspace(result[index])) {
			result[index] = 0;
			length--;
		}
		else {
			break;
		}
	}

	fclose (f);
	return length;
}

//----------------------------------------------------------------------------
// Name:	readTrimmedLinesFromFile
// Purpose:	Reads n lines of a file, removes whitespace from start.
// Returns:	Total lines read.
//----------------------------------------------------------------------------
unsigned readTrimmedLinesFromFile (const char *path, char **result, unsigned maxlines)
{
	if (!path || !result || !maxlines)
		return 0;

	FILE *f = fopen (path, "r");
	if (!f) {
		perror ("fopen");
		return 0;
	}

	int ch;
	int index = 0;
	bool found_nonwhitespace = false;

	char line[1024];
	unsigned line_number = 0;

	while (EOF != (ch = fgetc(f))) {
		// Trim the starts of lines.
		if (!found_nonwhitespace && (ch == ' ' || ch == '\t'))
			continue;

		found_nonwhitespace = true;

		if (ch == '\n' || index == sizeof(line)-1) {
			line[index] = 0;
			result[line_number++] = strdup(line);
			index = 0;
			found_nonwhitespace = false;

			if (line_number == maxlines)
				break;
		} else {
			line[index++] = ch;
		}
	}
	if (index > 0 && line_number < maxlines) {
		line[index] = 0;
		result[line_number++] = strdup(line);
	}

	fclose (f);
	return line_number;
}

//----------------------------------------------------------------------------
// Name:	readFloatFromFile
// Purpose:	Reads a single floating-point number from a file.
// Returns:	Stores float in result, returns true if success, else false.
//----------------------------------------------------------------------------
bool readFloatFromFile (const char *path, float *return_value)
{
	if (!path || !return_value)
		return false;

	FILE *f = fopen (path, "r");
	if (!f) {
		perror ("fopen");
		return false;
	}

	int count = fscanf (f, "%f", return_value);
	fclose (f);
	return count == 1;
}

//----------------------------------------------------------------------------
// Name:	readLongFromFile
// Purpose:	Reads a single floating-point number from a file.
// Returns:	Stores long in result, returns true if success, else false.
//----------------------------------------------------------------------------
bool readLongFromFile (const char *path, long *return_value)
{
	if (!path || !return_value)
		return false;

	FILE *f = fopen (path, "r");
	if (!f) {
		perror ("fopen");
		return false;
	}

	int count = fscanf (f, "%ld", return_value);
	fclose (f);
	return count == 1;
}

//----------------------------------------------------------------------------
// Name:	fileLineCount
// Purpose:	Counts the number of text lines in a text file.
// Returns:	Count, or -1 on error.
//----------------------------------------------------------------------------
long fileLineCount (const char *path)
{
	FILE *f = fopen (path, "r");
	if (!f) {
		perror ("fopen");
		return -1;
	}

	long lineCount = 0;

#define BUFSIZE 4096
	char buffer[BUFSIZE];
	char lastChar = 0;
	while (!feof (f)) {
		long len = fread (buffer, 1, BUFSIZE, f);
		if (len <= 0)
			break;
		for (long i=0; i < len; i++) {
			lastChar = buffer[i];
			if (lastChar == '\n')
				lineCount++;
		}
	}
	if (lastChar != '\n')
		lineCount++; // Partial last line.

	return lineCount;
}

//----------------------------------------------------------------------------
// Name:	hasPrefix
// Purpose:	Check whether C string has specified prefix, case-sensitive.
// Returns:	True if it has the prefix.
//----------------------------------------------------------------------------
bool hasPrefix (const char *string, const char *prefix)
{
	if (!string || !prefix)
		return false;

	size_t stringLength = strlen(string);
	size_t prefixLength = strlen(prefix);
	if (prefixLength > stringLength)
		return false;

	return 0 == strncmp (string, prefix, prefixLength);
}

//----------------------------------------------------------------------------
// Name:	hasSuffix
// Purpose:	Check whether C string has specified suffix, case-sensitive.
// Returns:	True if it has the suffix.
//----------------------------------------------------------------------------
bool hasSuffix (const char *string, const char *suffix)
{
	if (!string || !suffix)
		return false;

	size_t stringLength = strlen(string);
	size_t suffixLength = strlen(suffix);
	if (suffixLength > stringLength)
		return false;

	char *ending = (char*)string + stringLength - suffixLength;
	return 0 == strcmp (ending, suffix);
}

char *contentsOfFileAtPath (const char *path)
{
	struct stat st;
	if (stat (path, &st)) {
		perror ("stat");
		return NULL;
	}

	size_t size = st.st_size;
	if (size == 0) {
		return NULL;
	}

	FILE *f = fopen (path, "rb");
	if (!f) {
		// We can stat the file but not read it e.g. /sys/devices/cpu/rdpmc.
		return NULL;
	}

	char *buffer = (char*) malloc (size+1);
	if (!buffer) {
		perror ("malloc");
		return NULL;
	}

	size_t amount = fread (buffer, 1, size, f);
	fclose (f);

	buffer[size] = 0;

	if (amount != size) {
		perror ("fread");
		free (buffer);
		return NULL;
	}

	return buffer;
}

