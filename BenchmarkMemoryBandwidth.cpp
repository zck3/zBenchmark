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

extern "C" {
	#include <string.h>
	#include <stdint.h>

	extern void seq_memory_reader (uint64_t*, unsigned long, unsigned long loops);
	extern void seq_memory_writer (uint64_t*, unsigned long, unsigned long loops);
	extern void macos_seq_memory_reader (uint64_t*, unsigned long, unsigned long loops);
	extern void macos_seq_memory_writer (uint64_t*, unsigned long, unsigned long loops);
}

#include "defs.h"
#include "Utility.h"
#include "BenchmarkMemoryBandwidth.h"

static unsigned long chunk_sizes[] = {
	128,
	256,
	384,
	512,
	768,
	1024,
	2048,
	4096,
	8192,
	16384,
	32768,
	65536,
	131072,
	256 * 1024,
	512 * 1024,
	1024 * 1024,
	2 * 1024 * 1024,
	4 * 1024 * 1024,
	8 * 1024 * 1024,
	16 * 1024 * 1024,
	32 * 1024 * 1024,
	64 * 1024 * 1024,
	96 * 1024 * 1024,
	128 * 1024 * 1024,
	256 * 1024 * 1024,
	384 * 1024 * 1024,
	512 * 1024 * 1024,
};
#define N_CHUNK_SIZES sizeof(chunk_sizes)/sizeof(unsigned long)

//----------------------------------------------------------------------------
// Name:	generatePlotImage
// Purpose:	Create graph from bandwidth stats using gnuplot and load image.
// Returns:	QImage of the graph.
//----------------------------------------------------------------------------
QImage *BenchmarkMemoryBandwidth::generatePlotImage (double* seqReadData, 
				                     double* seqWriteData,
				                     int nData)
{
	if (!seqReadData) {
		return NULL;
	}

	size_t readCount = nData;
	size_t writeCount = 0;

	if (seqWriteData) {
		writeCount = nData;
	}

#define READ_POINTS_DATA_PATH "/tmp/zBenchmark_read_bandwidth.dat"
#define WRITE_POINTS_DATA_PATH "/tmp/zBenchmark_write_bandwidth.dat"
#define PLOT_FILEPATH_SVG "/tmp/gnuplot.svg"

	FILE *readPointsFile = fopen (READ_POINTS_DATA_PATH, "wb");
	if (!readPointsFile) {
		perror("fopen");
		return NULL;
	}

	int xmin = INT_MAX;
	int xmax = -INT_MAX;
	double ymax = -MAXFLOAT;

	for (size_t i = 0; i < readCount; i += 2) {
		double readRate = seqReadData[i];
		double chunkSize = seqReadData[i+1];

		if (readRate > ymax) 
			ymax = (int) ceil(readRate);

		double x = log2(chunkSize);
		if (x < xmin) 
			xmin = (int) floor(x);
		if (x > xmax) 
			xmax = (int) ceil(x);

		fprintf (readPointsFile, "%.1f %.1f\n", log2(chunkSize), readRate);
	}
	fclose (readPointsFile);

	if (seqWriteData) {
		FILE *writePointsFile = fopen (WRITE_POINTS_DATA_PATH, "wb");
		if (!writePointsFile) {
			perror("fopen");
			unlink (READ_POINTS_DATA_PATH);
			return NULL;
		}
		for (size_t i = 0; i < writeCount; i += 2) {
			double writeRate = seqWriteData[i];
			double chunkSize = seqWriteData[i+1];

			if (writeRate > ymax) 
				ymax = (int) ceil(writeRate);

			fprintf (writePointsFile, "%.1f %.1f\n", log2(chunkSize), writeRate);
		}
		fclose (writePointsFile);
	}

	char plotCommand[256];
	if (!seqWriteData) {
		snprintf (plotCommand, sizeof(plotCommand), "plot '%s' using 1:2 title 'Sequential Read' with lines;", READ_POINTS_DATA_PATH);
	}
	else {
		snprintf (plotCommand, sizeof(plotCommand), "plot '%s' using 1:2 title 'Sequential Read' with lines, '%s' using 1:2 title 'Sequential Write' with lines;", READ_POINTS_DATA_PATH, WRITE_POINTS_DATA_PATH);
	}

	char command[PATH_MAX];

	QImage *result = nullptr;
	snprintf (command, sizeof(command), "gnuplot -e \"set xlabel 'log2(chunkSize)'; set ylabel 'MB/sec'; set terminal svg; set output '%s'; set title 'zBenchmark bandwidth graph'; set yrange [0:%g]; set xrange [%d:%d]; %s\"", PLOT_FILEPATH_SVG, ymax, xmin, xmax, plotCommand);

	int retval = system (command);
	if (retval) {
		unlink (READ_POINTS_DATA_PATH);
		unlink (WRITE_POINTS_DATA_PATH);
		return NULL;
	} else {
		puts("GNUPLOT success generating SVG");
		result = new QImage(PLOT_FILEPATH_SVG);
	}

	unlink (READ_POINTS_DATA_PATH);
	unlink (WRITE_POINTS_DATA_PATH);
	unlink (PLOT_FILEPATH_SVG);
	return result;
}

//----------------------------------------------------------------------------
// Name:	Test_memory_sequential_read
// Purpose:	Measures memory bandwidth using sequential reads.
// Returns:	Array filled with rates in even positions, chunk sizes in odd.
//		Returns total values in array that were set.
//----------------------------------------------------------------------------

#ifdef __GNUC__
#pragma GCC push_options
#pragma GCC optimize ("O0")
#elif __clang__
#pragma clang optimize off
#endif

static int Test_memory_sequential_read (double *results_return, int max_results)
{
	if (!results_return || !max_results) {
		return BENCHMARK_ERROR;
	}

	memset (results_return, 0, sizeof(double) * max_results);

	int resultIndex = 0;

	void *chunk = NULL;
	void *previousChunk = NULL;

	for (size_t i=0; i < N_CHUNK_SIZES; i++) {
		long long chunkSize = chunk_sizes[i];
		if (!chunkSize) {
			puts ("Zero chunk size.");
			break;
		}

		if (chunk) {
			previousChunk = chunk;
		}
		chunk = calloc(chunkSize, 1);
		if (!chunk) {
			return BENCHMARK_ERROR;
		}

		// For redundancy. On Linux, reading from uninitialized memory is a no-op.
		memset (chunk, 255, chunkSize);

		long long totalLoops = 0;
		long long n_loops = (1LU<<29) / chunkSize;
		
		double t0 = getPreciseTime();

		while (getPreciseTime() - t0 < 1.0) {

			totalLoops += n_loops;
#if defined(IS_64BIT) 
			uint64_t *ptr = (uint64_t*)chunk;
  #ifdef __x86_64__
			seq_memory_reader (ptr, chunkSize, n_loops);
  #elif defined(__aarch64__)
    #ifdef __APPLE__
			macos_seq_memory_reader (ptr, chunkSize, n_loops);
    #else
			seq_memory_reader (ptr, chunkSize, n_loops);
    #endif
  #else
			for (unsigned n=0; n < n_loops; n++) {
				uint64_t length = chunkSize;
				uint64_t bits = 0;
				while (length > 0) {
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					length -= 128;
				}
				if (bits != 0) {
					puts ("Memory corruption");
				}
			}
  #endif
#else 
			for (unsigned n=0; n < n_loops; n++) {
				uint32_t *ptr = (uint32_t*)chunk;
				uint32_t length = chunkSize;

				uint32_t bits = 0;
				while (length > 0) {
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					bits |= *ptr++;
					length -= 64;
				}
				if (bits != 0) {
					puts ("Memory corruption");
				}
			}
#endif
		}

		double dt = getPreciseTime() - t0;
		double rate = chunkSize;
		rate *= totalLoops;
		rate /= dt;
		rate /= ONE_MEGABYTE;
		printf ("Reader: chunk size %lu, loops %lu, dt %.1f -> %.1f MB/s\n", (unsigned long)chunkSize, (unsigned long)totalLoops, dt, (float)rate);
		fflush (NULL);

		if (resultIndex >= max_results-1) {
			return BENCHMARK_ERROR;
		}
		results_return[resultIndex++] = rate;
		results_return[resultIndex++] = chunkSize;

		if (previousChunk) {
			free(previousChunk);
			previousChunk = NULL;
		}
	}

	free(chunk);

	return resultIndex;
}

//----------------------------------------------------------------------------
// Name:	Test_memory_sequential_write
// Purpose:	Measures memory bandwidth using sequential write.
// Returns:	Array filled with rates in even positions, chunk sizes in odd.
//		Returns total values in array that were set.
//----------------------------------------------------------------------------
static int Test_memory_sequential_write (double *results_return, int max_results)
{
	if (!results_return || !max_results) {
		return BENCHMARK_ERROR;
	}

	memset (results_return, 0, sizeof(double) * max_results);

	int resultIndex = 0;

	for (size_t i=0; i < N_CHUNK_SIZES; i++) {
		long long chunkSize = chunk_sizes[i];
		if (!chunkSize) {
			puts ("Zero chunk size.");
			break;
		}

		void *chunk = malloc(chunkSize);
		if (!chunk) {
			return BENCHMARK_ERROR;
		}

		long long totalLoops = 0;
		long long n_loops = (1L<<29) / chunkSize;
		double t0 = getPreciseTime();

		while (getPreciseTime() - t0 < 1.0) {
			totalLoops += n_loops;
#if defined(IS_64BIT) 
 #ifdef __x86_64__
			seq_memory_writer ((uint64_t*) chunk, chunkSize, n_loops);
 #elif defined(__aarch64__)
 #ifdef __APPLE__
			macos_seq_memory_writer ((uint64_t*) chunk, chunkSize, n_loops);
 #else
			seq_memory_writer ((uint64_t*) chunk, chunkSize, n_loops);
 #endif
 #else
			for (long long j=0; j < n_loops; j++) {
				memset (chunk, 1, chunkSize);
			}
 #endif
#else 
			for (long long j=0; j < n_loops; j++) {
				memset (chunk, 1, chunkSize);
			}
#endif
		}

		double t = getPreciseTime() - t0;
		double rate = chunkSize * totalLoops;
		rate /= t;
		rate /= ONE_MEGABYTE;
		printf ("Writer: chunk size %lu, loops %lu -> %.1f MB/s\n", (unsigned long)chunkSize, (unsigned long)n_loops, (float)rate);
		fflush (NULL);
		if (resultIndex >= max_results-1) {
			return BENCHMARK_ERROR;
		}
		results_return[resultIndex++] = rate;
		results_return[resultIndex++] = chunkSize;
	}

	return resultIndex;
}

#ifdef __GNUC__
#pragma GCC pop_options
#elif __clang__
#pragma clang optimize on
#endif

//----------------------------------------------------------------------------
// Method:	measure 
// Purpose:	Perform memory read and write tests, reports live results.
// Returns:	Average of read and write speeds.
//----------------------------------------------------------------------------
float BenchmarkMemoryBandwidth::measure () 
{
	bool haveGnuplot = true;
	if (!programIsInstalled("gnuplot")) {
		emit sendTextToUI(strdup("Cannot find gnuplot, will not display graph."));
		haveGnuplot = false;
	}

#if !defined(__aarch64__) && !defined(__x86_64__)
	emit sendTextToUI(strdup("Note! Bandwidth numbers are only approximate, as core routines are not written in assembly language.\n"));
#endif

	emit sendTextToUI(strdup("Sequential memory read performance score = "));

#define MAX_BANDWIDTH_DATA 256
	double seqReadValues[MAX_BANDWIDTH_DATA];
	int nValues = Test_memory_sequential_read (seqReadValues, MAX_BANDWIDTH_DATA);
	if (nValues < 0) {
		return BENCHMARK_ERROR;
	}

	if (haveGnuplot) {
		QImage *graph = generatePlotImage (seqReadValues, NULL, nValues);
		if (graph) {
			emit sendImageToUI(graph);
		}
	}

	double readIntegral = 0;
	for (int i=0; i < nValues; i+=2) {
		readIntegral += seqReadValues[i];
	}

	printf ("Area under sequential read bandwidth curve = %g\n", readIntegral);
	

	char message[128];
	snprintf (message, sizeof(message), "%f\n", readIntegral / REFERENCE_SYSTEM_64BIT_MEMORY_BANDWIDTH_SEQ_READ_PERFORMANCE);
	emit sendTextToUI(strdup (message));

	emit sendTextToUI(strdup("Sequential memory write performance score = "));

	double seqWriteValues[MAX_BANDWIDTH_DATA];
	nValues = Test_memory_sequential_write (seqWriteValues, MAX_BANDWIDTH_DATA);
	if (nValues < 0) {
		return BENCHMARK_ERROR;
	}

	if (haveGnuplot) {
		QImage *graph = generatePlotImage (seqReadValues, seqWriteValues, nValues);
		if (graph) {
			emit sendImageToUI(graph);
		}
	}

	double writeIntegral = 0;
	for (int i=0; i < nValues; i+=2) {
		writeIntegral += seqWriteValues[i];
	}
	printf ("Area under sequential write bandwidth curve = %g\n", writeIntegral);

	snprintf (message, sizeof(message), "%f\n", writeIntegral / REFERENCE_SYSTEM_64BIT_MEMORY_BANDWIDTH_SEQ_WRITE_PERFORMANCE);
	emit sendTextToUI(strdup (message));

	// Total performance is total area for both read and write.
	return readIntegral + writeIntegral;
}

