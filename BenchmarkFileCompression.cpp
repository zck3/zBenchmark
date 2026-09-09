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
#include "BenchmarkFileCompression.h"

enum {
	COMPRESSOR_GZIP = 0,
	COMPRESSOR_BZIP2 = 1,
	COMPRESSOR_ZIP = 2,
	COMPRESSOR_7ZIP = 3,
};

static const char *compressorNames[] = {
	"gzip", "bzip2", "zip", "7z"
};
static const char *compressorParameters[] = {
	"--best -c", "--best -c", "-9", "a"
};
static const char *compressorOutputFiles[] = {
	"random.gz", "random.bz2", "random.zip", "random.7z"
};
#define N_COMPRESSORS sizeof(compressorNames)/sizeof(char*)

//----------------------------------------------------------------------------
// Method:	measure 
// Purpose:	Perform file compression, report live results.
// Returns:	Time in seconds to run.
// Note:	Compression is using various compressors.
//----------------------------------------------------------------------------
float BenchmarkFileCompression::measure ()
{
	char inputFilePath[PATH_MAX];
	snprintf (inputFilePath, sizeof(inputFilePath), "/tmp/random.dat");

	char decompressionFilePath[PATH_MAX] = {0};

#define COMPRESSION_FILE_LENGTH (COMPRESSION_TEST_FILE_SIZE_IN_MEGS * ONE_MEGABYTE)

	if (!create_randomized_file (inputFilePath, COMPRESSION_FILE_LENGTH)) {
		return BENCHMARK_ERROR;
	}

	char message[256];
	snprintf (message, sizeof(message), "Wrote %luMB randomized file.\n", COMPRESSION_TEST_FILE_SIZE_IN_MEGS);
	emit sendTextToUI(strdup (message));

	double totalCompressionTime = 0.;

	for (unsigned whichCompressor = 0; whichCompressor < N_COMPRESSORS; whichCompressor++) {
		const char *compressorName = compressorNames[whichCompressor];

		if (!programIsInstalled (compressorName)) {
			snprintf (message, sizeof(message), "Compressor %s is not installed.\n", compressorName);
			emit sendTextToUI(strdup (message));
			return BENCHMARK_ERROR;
		}

		const char *parameters = compressorParameters[whichCompressor];

		snprintf (message, sizeof(message), "Compressing with %s... ", compressorName);
		emit sendTextToUI(strdup (message));

		char outputFilePath[PATH_MAX];
		snprintf (outputFilePath, sizeof(outputFilePath), "/tmp/%s", compressorOutputFiles[whichCompressor]);

		// Delete in case last test run killed by user before finishing.
		unlink (outputFilePath);

		char compressionCommand[PATH_MAX * 3];
		switch (whichCompressor) {
		case COMPRESSOR_GZIP:
		case COMPRESSOR_BZIP2:
			snprintf (compressionCommand, sizeof(compressionCommand), "nice -20 %s %s %s > %s", 
				compressorName, 
				parameters, 
				inputFilePath,
				outputFilePath);
			break;
		case COMPRESSOR_ZIP:
		case COMPRESSOR_7ZIP:
			snprintf (compressionCommand, sizeof(compressionCommand), "nice -20 %s %s %s %s > /dev/null", 
				compressorName, 
				parameters, 
				outputFilePath,
				inputFilePath);
			break;
		default:
			break;
		}

		int retval = 0;

		double t0 = getPreciseTime();
		retval = system(compressionCommand);
		double compressionTime = getPreciseTime() - t0;

		totalCompressionTime += compressionTime;

		long long compressedSize = fileSize (outputFilePath);
		if (compressedSize < 0) {
			emit sendTextToUI(strdup ("\nCompressed file is missing. \n"));
			unlink(inputFilePath);
			return BENCHMARK_ERROR;
		}

		if (retval) {
			emit sendTextToUI(strdup ("\nA compression error occurred. \n"));
			unlink(inputFilePath);
			return BENCHMARK_ERROR;
		}

		double compressionRate = 1.0 - (compressedSize / (double)COMPRESSION_FILE_LENGTH);
		compressionRate *= 100.0;

		// RULE: Only one compressed file will be decompressed, since 
		// decompression is usually fast and this is mainly a test of compression times.

		if (whichCompressor == 0) {
			strncpy (decompressionFilePath, outputFilePath, sizeof(decompressionFilePath));
		} else {
			unlink (outputFilePath);
		}

		// printf ("%s compression achieved on random data: %.3g%%\n", compressorName, compressionRate);

		snprintf (message, sizeof(message), "%.1f seconds.\n", compressionTime);
		emit sendTextToUI(strdup (message));
	}

	char decompressionCommand[PATH_MAX * 2];
	snprintf (decompressionCommand, sizeof(decompressionCommand), "nice -20 gunzip -c %s > /dev/null", decompressionFilePath);

	double t0 = getPreciseTime();
	int retval = system(decompressionCommand);
        double decompressionTime = getPreciseTime() - t0;

	if (retval) {
		emit sendTextToUI(strdup ("A decompression error occurred."));
		unlink(inputFilePath);
		unlink(decompressionFilePath);
		return BENCHMARK_ERROR;
	}

	snprintf (message, sizeof(message), "Decompressed gzipped file in %.1f seconds.\n", decompressionTime);
	emit sendTextToUI(strdup (message));

	unlink (inputFilePath);
	unlink (decompressionFilePath);

	double totalTime = totalCompressionTime + decompressionTime;

	snprintf (message, sizeof(message), "Total run time = %.1f seconds.\n", totalTime);
	emit sendTextToUI(strdup (message));

	return totalTime;
}

