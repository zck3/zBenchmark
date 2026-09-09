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
	#include <stdbool.h>
	#include <stdint.h>
	#include <string.h>

	extern int bignum_add (uint64_t *a, uint64_t *b, uint64_t *sum, uint64_t n_qwords);
	extern int macos_bignum_add (uint64_t *a, uint64_t *b, uint64_t *sum, uint64_t n_qwords);
};

#include "defs.h"
#include "Utility.h"
#include "BenchmarkFibonacci.h"

//-----------------------------------------------------------------------------
// The Fibonacci sequence is 0, 1, 1, 2, 3, 5 ....
// The algorithm is:
// int a=0
// int b=1
// repeat(rounds) {
//   int c = a + b
//   a = b
//   b = c
// }
//-----------------------------------------------------------------------------

static void print (uint64_t *num, FILE *output)
{
	bool found_nonzero = false;

	for (int i=N_FIBONACCI_QWORDS-1; i >= 0; i--) {
		uint64_t number = num[i];
		if (!number && !found_nonzero)
			continue;
		fprintf (output, "%016llx", (long long unsigned int) num[i]);
		found_nonzero = true;
	}

	if (!found_nonzero)
		fputc ('0', output);

	fputc ('\n', output);
}

#ifndef __aarch64__
static bool add (uint32_t *result, uint32_t *a, uint32_t *b)
{
	uint32_t carry = 0;
	for (unsigned i=0; i < N_FIBONACCI_DWORDS; i++) {
		uint64_t sum = a[i];
		sum += b[i];
		sum += carry;
		carry = 0;
		if (sum >> 32) {
			carry = 1;
		}
		result[i] = 0xffffffff & sum;
	}
	return carry != 0;
}
#endif

static void fibonacci () 
{
	int n_printed = 0;

	uint64_t *a = (uint64_t*) calloc(N_FIBONACCI_BYTES, 1);
	uint64_t *b = (uint64_t*) calloc(N_FIBONACCI_BYTES, 1);
	uint64_t *sum = (uint64_t*) malloc(N_FIBONACCI_BYTES);
	b[0] = 1;
	int i = 0;
	while (true) {
		// sum = a + b
		bool overflow = false;
#ifndef __aarch64__
		overflow = add ((uint32_t*) sum, (uint32_t*) a, (uint32_t*) b);
#else
  #ifdef __APPLE__
		overflow = macos_bignum_add (a, b, sum, N_FIBONACCI_QWORDS);
  #else
		overflow = bignum_add (a, b, sum, N_FIBONACCI_QWORDS);
  #endif
#endif
		if (overflow) {
			puts ("OVERFLOW");
#ifdef DIAGNOSTIC_OUTPUT
			FILE *f = fopen("check.out", "w");
			print (a, f);
			fclose (f);
#endif
			break;
		}

		if (n_printed < 30) {
			print (sum, stdout);
			n_printed++;
		}

		// Rotate
		uint64_t *tmp = a;
		a = b;
		b = sum;
		sum = tmp;

		i++;
	}
}

//----------------------------------------------------------------------------
// Method:	measure 
// Purpose:	Measure running time of my megabit variant of Fibonacci.
// Returns:	Time in seconds to run.
//----------------------------------------------------------------------------
float BenchmarkFibonacci::measure () 
{
	char message[256];
	snprintf (message, sizeof(message), "Finding all Fibonacci numbers that fit within %lu bits.\nThis test ends when it encounters an arithmetic overflow.\n", (unsigned long)N_FIBONACCI_BITS);
	emit sendTextToUI(strdup(message));

	double t0 = getPreciseTime();
	fibonacci();
	double runtime = getPreciseTime() - t0;

	snprintf (message, sizeof(message), "Total run time = %.2f seconds\n", runtime);
	emit sendTextToUI(strdup(message));

	return runtime;
}

