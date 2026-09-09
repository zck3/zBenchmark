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

#include "defs.h"
#include "Utility.h"
#include "BenchmarkDiskIO.h"

extern "C" {
	#include <sys/types.h>
	#include <fcntl.h>
	#include <sys/param.h> // MAX
}

//----------------------------------------------------------------------------
// Name:	createTemporaryFile
// Purpose:	Creates a temporary file and tells Linux to not cache it in RAM.
// Returns:	True on success, false on failure.
//----------------------------------------------------------------------------
static bool createTemporaryFile (const char *path, size_t fileSize, bool randomize)
{
	uint8_t *writeBuffer = (uint8_t*) malloc(ONE_MEGABYTE);
	if (!writeBuffer) {
		perror("malloc");
		return false;
	}

	int fd = open (path, O_CREAT | O_RDWR |  O_SYNC, S_IRWXU);
	if (!fd) {
		perror("open");
		free (writeBuffer);
		return false;
	}

	if (randomize) {
		unsigned value = 197;
		for (size_t i=0; i < ONE_MEGABYTE; i++) {
			value *= 7;
			value ^= 0x37;
			writeBuffer[i] = (uint8_t) value;
		}
	}
	else {
		memset (writeBuffer, 0x87, ONE_MEGABYTE);
	}

	long remaining = fileSize;
	while (remaining > 0) {
		unsigned amountToWrite = MAX(remaining, ONE_MEGABYTE);
		long totalWrote = write (fd, writeBuffer, amountToWrite);
		if (totalWrote < 0) {
			perror ("write");
			break;
		}
		remaining -= totalWrote;
	}

	// Tell the kernel we don't want this kept in memory
	posix_fadvise (fd, 0, fileSize, POSIX_FADV_DONTNEED);
	fsync (fd);
	close (fd);
	free (writeBuffer);
	return true;
}

//----------------------------------------------------------------------------
// Name:	Test_drive_read_speed
// Purpose:	Creates a non-cached temporary file and measures time to read it.
// Returns:	Rate in MB/sec.
//----------------------------------------------------------------------------
static float Test_drive_read_speed(size_t fileSize)
{
	char path[PATH_MAX];
	snprintf (path, sizeof(path), "/tmp/%s_readtest.%luMB", PROGRAM_NAME, (unsigned long) fileSize >> 20);
	unlink (path);

	if (!createTemporaryFile (path, fileSize, false))
		return BENCHMARK_ERROR;

	double t0 = getPreciseTime();

	FILE *f = fopen(path, "rb");
	if (!f) {
		perror("fopen");
		return BENCHMARK_ERROR;
	}

#ifdef __APPLE__
	int fd = fileno (f);
	fcntl (fd, F_NOCACHE, 1);
	fcntl (fd, F_GLOBAL_NOCACHE, 1);
	fcntl (fd, F_FULLFSYNC, -1);
#endif

	uint8_t *readBuffer = (uint8_t*) malloc(ONE_MEGABYTE);
	if (!readBuffer) {
		perror("malloc");
		return BENCHMARK_ERROR;
	}

	uint64_t bytesRead = 0;

	while (!feof(f)) {
		long got = fread (readBuffer, 1, ONE_MEGABYTE, f);
		if (got <= 0)
			break;
		bytesRead += got;
	}

	double t = getPreciseTime() - t0;
	double rate = (double)bytesRead / t;
	rate /= (double)ONE_MEGABYTE;

	free (readBuffer);
	unlink (path);
	system ("sync");

	return rate;
}

//----------------------------------------------------------------------------
// Name:	Test_drive_write_speed
// Purpose:	Creates a non-cached temporary file and times it.
// Returns:	Rate in MB/sec.
//----------------------------------------------------------------------------
static float Test_drive_write_speed(size_t fileSize)
{
	char path[1000];
	snprintf (path, sizeof(path), "/tmp/%s_writetest.%lx", PROGRAM_NAME, (unsigned long) fileSize);
	unlink (path);
	system ("sync");

	double t0 = getPreciseTime();

	if (!createTemporaryFile (path, fileSize, true)) {
		return BENCHMARK_ERROR;
	}

	system ("sync");

	double t = getPreciseTime() - t0;

	unlink (path);
	system ("sync");

	double bytesPerSecond = (double)fileSize / (double)t;
	double megabytesPerSecond = bytesPerSecond / (double)ONE_MEGABYTE;

	return megabytesPerSecond;
}

//----------------------------------------------------------------------------
// Method:	measure 
// Purpose:	Perform disk read and write tests, reports live results.
// Returns:	Average of read and write speeds.
//----------------------------------------------------------------------------
float BenchmarkDiskIO::measure ()
{
	int nFilesWritten = 0;
	int nFilesRead = 0;
	char message[256];

	//-----------------------
	// Large file writer test
	//
	float averageWriteRate = 0;
	bool success = true;

	for (size_t fileSize = ONE_MEGABYTE; fileSize <= MAX_DISKIO_WRITE_TEST_FILE_SIZE_IN_MEGS * ONE_MEGABYTE; fileSize *= 2) {
		float rate;
		if (0 > (rate = Test_drive_write_speed(fileSize))) {
			success = false;
			break;
		}

		snprintf (message, sizeof(message), "Wrote %luMB at %.2f MB/s\n", (unsigned long) fileSize/ONE_MEGABYTE, rate);
		emit sendTextToUI(strdup(message));
		averageWriteRate += rate;
		nFilesWritten++;
	}
	if (!success) {
		emit sendTextToUI(strdup("Unable to perform file write test."));
		return BENCHMARK_ERROR;
	}

	// RULE: The write score is determined by the average of the disk write speeds
	// for various file sizes.

	averageWriteRate /= nFilesWritten;
	snprintf (message, sizeof(message), "Average write speed %.2f MB/s\n", averageWriteRate);
	emit sendTextToUI(strdup(message));

	//-----------------------
	// Large file reader test
	//
	float averageReadRate = 0;
	static size_t fileSizes[3] = { 17, 37, 43 };
	success = true;

	const int nRuns = sizeof(fileSizes)/sizeof(size_t);
	for (int i=0; i < nRuns; i++) {
		size_t fileSize = fileSizes[i] * ONE_MEGABYTE;
		float rate;
		if (0 > (rate = Test_drive_read_speed(fileSize))) {
			success = false;
			break;
		}

		snprintf (message, sizeof(message), "Read %luMB at %.2f MB/s\n", (unsigned long) fileSize/ONE_MEGABYTE, rate);
		emit sendTextToUI(strdup(message));
		averageReadRate += rate;
		nFilesRead++;
	}
	if (!success) {
		emit sendTextToUI(strdup("Unable to perform file read test."));
		return BENCHMARK_ERROR;
	}

	// RULE: The read score is determined by the average of the disk read speeds
	// for various file sizes.

	averageReadRate /= (float)nRuns;
	snprintf (message, sizeof(message), "Average read speed %.2f MB/s\n", averageReadRate);
	emit sendTextToUI(strdup(message));

	float overallRate = (averageWriteRate + averageReadRate) / 2;

	printf ("Overall rate for writing and reading was %.1f MB/s\n", overallRate);

	return overallRate;
}

