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

#include "Benchmark.h"
#include "ReferenceSystem.h"

// This is a test of big number integer math.

#define N_FIBONACCI_BITS (1LU << 20) // Must be divisible by 256 due to ARM64 arm code.
#define N_FIBONACCI_BYTES (N_FIBONACCI_BITS >> 3) 
#define N_FIBONACCI_DWORDS (N_FIBONACCI_BITS >> 5)
#define N_FIBONACCI_QWORDS (N_FIBONACCI_BITS >> 6)

class BenchmarkFibonacci : public Benchmark
{
public:
	float measure ();

	const char *name () {
		return "Fibonacci Overflow";
	}

	float referenceMeasurement () {
		return REFERENCE_SYSTEM_64BIT_FIBONACCI_PERFORMANCE;
	}
};

