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
#include "BenchmarkBigNumbers.h"

//----------------------------------------------------------------------------
// Name:	Test_big_number_calculations 
// Purpose:	Perform predefined set of big-number operations.
// Returns:	Time in seconds to run.
//----------------------------------------------------------------------------
typedef enum {
	BIGNUM_PI,
	BIGNUM_EULER,
} BigNumberTestIdentifier;

static float 
Test_big_number_calculations (unsigned nDigits, BigNumberTestIdentifier whichTest)
{
	char path [PATH_MAX];
	snprintf (path, sizeof(path), "/tmp/.%s.pi.sh", PROGRAM_NAME);

	FILE *f = fopen (path, "w");
	if (!f)
		return BENCHMARK_ERROR;
	fprintf (f, "#/bin/sh\n");

	fprintf (f, "echo \"scale=%u; ", nDigits);

	switch (whichTest) {
	case BIGNUM_PI:
		fprintf (f, "4*a(1);");
		break;
	case BIGNUM_EULER:
		fprintf (f, "e(1);");
		break;
	}

	fprintf (f, "\" | bc -l -q > /dev/null\n");
	fclose (f);

	chmod (path, 0700);

	double t0 = getPreciseTime();
	int retval = system(path);
	double t = getPreciseTime() - t0;

	unlink(path);

	return 0 == retval ? t : BENCHMARK_ERROR;
}

//----------------------------------------------------------------------------
// Method:	measure 
// Purpose:	Perform big-number tests and report live results.
// Returns:	Time in seconds to run.
//----------------------------------------------------------------------------
float BenchmarkBigNumbers::measure () 
{
	if (!ensureThatProgramIsInstalled ("bc")) {
		return BENCHMARK_ERROR;
	}

	char message[256];
	snprintf (message, sizeof(message), "Performing %lu-digit floating-point calculations using 'bc':\n", N_BIGNUM_DIGITS);
	emit sendTextToUI(strdup(message));

	float totalRuntime = 0;
	float secondsToRun;

	emit sendTextToUI(strdup("Calculating pi... "));
	secondsToRun = Test_big_number_calculations(N_BIGNUM_DIGITS, BIGNUM_PI);
	if (secondsToRun < 0) {
		emit sendNewlineToUI();
		return BENCHMARK_ERROR;
	}
	snprintf (message, sizeof(message), "%.2f seconds\n", secondsToRun);
	emit sendTextToUI(strdup(message));
	totalRuntime += secondsToRun;

	emit sendTextToUI(strdup("Calculating Euler's number (e)... "));
	secondsToRun = Test_big_number_calculations(N_BIGNUM_DIGITS, BIGNUM_EULER);
	if (secondsToRun < 0) {
		emit sendNewlineToUI();
		return BENCHMARK_ERROR;
	}
	snprintf (message, sizeof(message), "%.2f seconds\n", secondsToRun);
	emit sendTextToUI(strdup(message));
	totalRuntime += secondsToRun;

printf ("Big numbers total time = %g\n", totalRuntime);
	return totalRuntime;
}

