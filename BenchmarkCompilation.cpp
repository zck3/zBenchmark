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
	#include <stdio.h>
	#include <stdlib.h>
};

#include "defs.h"
#include "Utility.h"
#include "BenchmarkCompilation.h"

static const char *optimizations[2] = {
	"-O3",
	"-O0"
};

//----------------------------------------------------------------------------
// Name:	generateRandomCSourceCode
// Purpose:	Provides randomized C code that will befuddle any compiler.
// Returns:	True on success.
//----------------------------------------------------------------------------
bool generateRandomCSourceCode (const char *path, unsigned nRounds, unsigned long seed)
{
	if (!path || !nRounds) {
		return false;
	}

	srand (seed);
	const char *operations[] = {
		"+", "-", "*", "|", "^", "&",
	};
#define N_OPTYPES sizeof(operations)/sizeof(char*)

	FILE *f = fopen (path, "wb");
	if (!f) {
		perror ("fopen");
		return false;
	}

	fprintf (f, "#include <stdio.h>\n");
	fprintf (f, "#include <stdlib.h>\n");
	fprintf (f, "#include <stdint.h>\n");
	fprintf (f, "#include <math.h>\n");
	fprintf (f, "static inline uint64_t min2(uint64_t a, uint64_t b) { \n if (a<b) return a; else return b; }\n");
	fprintf (f, "static inline uint64_t max2(uint64_t a, uint64_t b) { \n if (a>b) return a; else return b; }\n");
	fprintf (f, "int main() {\n");
	fprintf (f, "double temp;\n");
	fprintf (f, "srand (0xDEADF00D);\n");

#define N_VARS 20
	int i;
	for (i=0; i < N_VARS; i++) {
		fprintf (f, "uint64_t %c = %luLU + (%luLU << 32);\n", i + 'a', 0xffffffff & (unsigned long)rand(), 0xffff & (unsigned long) rand());
	}

	// Some extra data
	fprintf (f, "float extras[%i] = {0};\n", N_VARS);
	for (i=0; i < N_VARS; i++) {
		fprintf (f, "extras[%d] = 0.001f * (float)%luLU;\n", i, 0xffffffff & (unsigned long)rand());
	}

	for (unsigned round=0; round < nRounds; round++) {
		for (i=0; i < N_VARS; i++) {
			unsigned op = rand() % N_OPTYPES;
			const char *opString = operations[op];
			fprintf (f, "%c %s= %luL;\n", i + 'a', opString, (unsigned long)rand());
		}
		
		unsigned var0, var1, var2, var3, var4, var5;
		for (i=0; i < N_VARS; i++) {
			var0 = 'a' + (rand() % N_VARS);
			var1 = 'a' + (rand() % N_VARS);
			unsigned var1 = 'a' + (rand() % N_VARS);
			fprintf (f, "%c ^= %c;\n", var0, var1);
		}

		var0 = 'a' + (rand() % N_VARS);
		var1 = 'a' + (rand() % N_VARS);
		var2 = 'a' + (rand() % N_VARS);
		var3 = 'a' + (rand() % N_VARS);
		var4 = 'a' + (rand() % N_VARS);
		var5 = 'a' + (rand() % N_VARS);
		if (var0 == var1 || var2 == var0 || var3 == var0 || var4 == var0 || var2 == var1 || var3 == var1 || var4 == var1) {
			var0 = 'a';
			var1 = 'b';
			var2 = 'c';
			var3 = 'd';
			var4 = 'e';
			var5 = 'f';
		}
		fprintf (f, "for (int i = (0xfff & min2(%c,%c)); i <= (0xfff & max2(%c,%c)); i++) {\n", var0, var1, var0, var1);
		fprintf (f, "  %c += (uint64_t) (%c * %c + (long long)ceil(sin(%c)));\n", var2, var3, var4, var5);
		fprintf (f, "}\n");
		
		unsigned swappable0 = 'a' + (rand() % N_VARS);
		unsigned swappable1 = 'a' + (rand() % N_VARS);
		fprintf (f, "temp = %c; %c = %c; %c = (uint64_t)fabs(temp);\n", swappable0, swappable0, swappable1, swappable1);
		
		swappable0 = rand() % N_VARS;
		swappable1 = 'a' + (rand() % N_VARS);
		fprintf (f, "temp = extras[%i]; extras[%i] = %c; %c = (uint64_t)fabs(temp);\n", swappable0, swappable0, swappable1, swappable1);
	}

	fprintf (f, "uint64_t sum = ");
	for (i=0; i < N_VARS; i++) {
		fprintf (f, "%c+", i + 'a');
	}
	fprintf (f, "0;\n");
	fprintf (f, "return sum;\n");
	fprintf (f, "}\n");
	fclose (f);
	fflush (NULL);

	return true;
}

//----------------------------------------------------------------------------
// Method:	measure 
// Purpose:	Measure compilation of random C code at various optimizations.
// Returns:	Time in seconds to run.
//----------------------------------------------------------------------------
float BenchmarkCompilation::measure () 
{
	char message[512] = {0};
	char command[1024] = {0};
	char line [256] = {0};

	if (!ensureThatProgramIsInstalled ("clang"))
		return BENCHMARK_ERROR;

#define TEMP_SOURCE_FILE "/tmp/.zB.c"
#define TEMP_EXE_FILE "/tmp/.zB"
#define TEMP_LINECOUNT_FILE "/tmp/.zB.nlines"
#define TEMP_VERSION_FILE "/tmp/.zB.clang"

	unlink (TEMP_SOURCE_FILE);
	unlink (TEMP_EXE_FILE);
	unlink (TEMP_LINECOUNT_FILE);
	unlink (TEMP_VERSION_FILE);

	system ("clang -v 2>&1 | grep version | sed 's/^.*version //' > " TEMP_VERSION_FILE);
	float clangVersion = 0;
	if (readTrimmedLineFromFile (TEMP_VERSION_FILE, line, sizeof(line))) {
		// Parse the version number. 
		if (isdigit(line[0])) 
			clangVersion = atof(line);
	}

	if (clangVersion > 0.f) {
		snprintf (message, sizeof(message), "Found Clang compiler version %g\n", clangVersion);
		emit sendTextToUI(strdup (message));
	}

	//-----------------------
	// Generate random C code.
	//
	snprintf (message, sizeof(message), "Randomly generating C source code...\n");
	emit sendTextToUI(strdup (message));

	if (!generateRandomCSourceCode (TEMP_SOURCE_FILE, 2221, 314159236))
		return BENCHMARK_ERROR;

	long lineCount = fileLineCount(TEMP_SOURCE_FILE);
	if (lineCount < 0)
		return BENCHMARK_ERROR;

	snprintf (message, sizeof(message), "Source file has %ld lines.\n", lineCount);
	emit sendTextToUI(strdup (message));

	emit sendTextToUI(strdup("This test may take several minutes.\n"));

	system ("sync");

	double totalTime = 0;
	for (unsigned round = 0; round < N_COMPILATION_ROUNDS; round++) { 
		const char *optimizationLevel = optimizations[round];

		snprintf (message, sizeof(message), "Compiling at %s optimization level...", optimizationLevel);
		emit sendTextToUI(strdup (message));

		//------------------
		// Clang compilation.
		//
		double t0 = getPreciseTime();
		snprintf (command, sizeof(command), "sync; nice -20 clang -lm %s %s -o %s", optimizationLevel, TEMP_SOURCE_FILE, TEMP_EXE_FILE);
		if (system (command)) {
			return BENCHMARK_ERROR;
		}
		double t = getPreciseTime() - t0;
		totalTime += t;
		snprintf (message, sizeof(message), " That took %.1f seconds.\n", t);
		emit sendTextToUI(strdup (message));
	}

	totalTime /= N_COMPILATION_ROUNDS;
	
	snprintf (message, sizeof(message), "Average compilation time was %.1f seconds.\n", totalTime);
	emit sendTextToUI(strdup (message));

	unlink (TEMP_SOURCE_FILE);
	unlink (TEMP_EXE_FILE);
	unlink (TEMP_LINECOUNT_FILE);
	unlink (TEMP_VERSION_FILE);

	return totalTime;
}

