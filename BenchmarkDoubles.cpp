/*=========================================================================
 * This file is part of zBenchmark, which is a system benchmarking tool.
 * (C) 2022, 2026 Zack T Smith.
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
#include "BenchmarkDoubles.h"

typedef double Number;

//----------------------------------------------------------------------------
// Name:	calculations
// Purpose:	Perform howMany (multiple of 8) loops of operations on doubles.
// Returns:	Time to run in seconds.
//----------------------------------------------------------------------------
static float calculations (unsigned long howMany)
{
	if (howMany & 7)
		return BENCHMARK_ERROR;

	Number *values = (Number*) malloc (sizeof(Number) * howMany);
	if (!values) {
		perror("malloc");
		return BENCHMARK_ERROR;
	}

	srand(time(NULL));

	double t0 = getPreciseTime();

	for (unsigned long i=0; i < howMany; i++) {
		Number value = 1.0 + 0.0001 * (Number) rand();
		values[i] = value;
	}

	Number divisor1 = 1.0 + 0.01 * (Number) rand();
	Number divisor2 = 1.0 + 0.01 * (Number) rand();
	Number divisor3 = 1.0 + 0.01 * (Number) rand();
	Number divisor4 = 1.0 + 0.01 * (Number) rand();
	if (divisor1 == 0.)
		divisor1 = 3.14;
	if (divisor2 == 0.)
		divisor2 = 1.73;
	if (divisor3 == 0.)
		divisor3 = 2.24;
	if (divisor4 == 0.)
		divisor4 = 1.41;

	// 36 floating point operations per loop

	Number sum = 0.0;
	for (unsigned long i=0; i < howMany; i+=8) {
		Number a,b,c,d,e,f,g,h;
		a = values[i];
		b = values[i+1];
		c = values[i+2];
		d = values[i+3];
		e = values[i+4];
		f = values[i+5];
		g = values[i+6];
		h = values[i+7];

		// 8 additions, 8 multiplications
		a = (b + c) * d;
		b = (c + d) * e;
		c = (d + e) * f;
		d = (e + f) * g;
		e = (f + g) * h;
		f = (g + h) * a;
		g = (h + a) * b;
		h = (a + b) * c;

		// 8 additions, 8 divisions
		a = (b + c) / divisor1;
		b = (c + d) / divisor2;
		c = (d + e) / divisor3;
		d = (e + f) / divisor4;
		e = (f + g) / divisor1;
		f = (g + h) / divisor2;
		g = (h + a) / divisor3;
		h = (a + b) / divisor4;

		// 2 trigonometry, 2 exponentials
		e = 1.0f + cos(f);
		f = 1.0f + sin(f);
		g = pow(g, 3.14159);
		h = sqrt(h);

		sum += h;

		values[i] = h != 0.f ? h : 1.1;
		values[i+1] = g != 0.0 ? g : 1.2;
		values[i+2] = f != 0.0 ? f : 1.3;
		values[i+3] = e != 0.0 ? e : 1.4;
		values[i+4] = d != 0.0 ? d : 1.5;
		values[i+5] = c != 0.0 ? c : 1.6;
		values[i+6] = b != 0.0 ? b : 1.7;
		values[i+7] = a != 0.0 ? a : 1.8;
	}

	// Prevent compiler from optimizing out all of the above.
	if (sum == M_PI) {
		puts (".");
	}

	double t = getPreciseTime() - t0;

	free (values);

	return t;
}
//----------------------------------------------------------------------------
// Method:	measure 
// Purpose:	Perform millions of floating point operations, report result.
// Returns:	Time in seconds to run.
//----------------------------------------------------------------------------
float BenchmarkDoubles::measure () 
{
	unsigned long n_ops = MAX_DOUBLE_OPS * 36LU;

	char message[256];
	snprintf (message, sizeof(message), "Performing %lu 64-bit floating point operations...\n", n_ops);
	emit sendTextToUI(strdup (message));

	float runTime = calculations(MAX_DOUBLE_OPS);

	snprintf (message, sizeof(message), "Run time was %.1f seconds.\n", runTime);
	emit sendTextToUI(strdup (message));

	return runTime;
}

