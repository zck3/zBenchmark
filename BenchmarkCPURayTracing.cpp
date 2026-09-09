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
#include "BenchmarkCPURayTracing.h"

#define POVRAY_FILE "benchmark.pov"

static const char *possibleRaytracerNames[] = {
	"povray",
	// On some OSes a more specific name may be needed.
	"povray3.7",
	"povray3.8",
};

//----------------------------------------------------------------------------
// Method:	measure 
// Purpose:	Perform CPU-based raytracing benchmark, report result.
// Returns:	Time in seconds to run.
//----------------------------------------------------------------------------
float BenchmarkCPURayTracing::measure () 
{
	const char *programName = NULL;
	for (unsigned i=0; i < sizeof(possibleRaytracerNames)/sizeof(char*); i++) {
		const char *name = possibleRaytracerNames[i];
		if (programIsInstalled (name)) {
			programName = name;
			break;
		}
	}
	if (!programName) {
		emit sendTextToUI(strdup("Cannot find POVRay executable."));
		return BENCHMARK_ERROR;
	}

	char *povFile = findAsset (POVRAY_FILE);
	if (!povFile) {
		emit sendTextToUI(strdup("Cannot find POVRay test file."));
		return BENCHMARK_ERROR;
	}

	emit sendTextToUI(strdup ("This is a multicore raytracing benchmark.\n"));

	char raytraceCommand[PATH_MAX];
	snprintf (raytraceCommand, sizeof(raytraceCommand), "nice -20 %s -F %s", programName, povFile);

        double t0 = getPreciseTime();
	int retval = system(raytraceCommand);
        double dt = getPreciseTime() - t0;

	char message[128];
	if (retval) {
		snprintf (message, sizeof(message), "The raytracer returned error %d.\n", retval);
		emit sendTextToUI(strdup(message));
		return BENCHMARK_ERROR;
	} else {
		snprintf (message, sizeof(message), "Raytracing took %.1f seconds.\n", dt);
		emit sendTextToUI(strdup(message));
		return dt;
	}
}

