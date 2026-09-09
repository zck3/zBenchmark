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
#include "BenchmarkAudioEncoding.h"

#define WAV_FILE "hike.wav"

//----------------------------------------------------------------------------
// Method:	measure 
// Purpose:	Perform big-number tests and report live results.
// Returns:	Time in seconds to run.
//----------------------------------------------------------------------------
float BenchmarkAudioEncoding::measure () 
{
	if (!ensureThatProgramIsInstalled ("lame"))
		return BENCHMARK_ERROR;

	char *wavFile = findAsset (WAV_FILE);
	if (!wavFile) {
		emit sendTextToUI(strdup("Cannot find WAV test file."));
		return BENCHMARK_ERROR;
	}

#define LAME_OUTPUT "/tmp/.lameOutput.mp3"
	unlink (LAME_OUTPUT);

	emit sendTextToUI(strdup("Converting WAV to MP3 using 'lame'... "));

	char command[1024];
	snprintf (command, sizeof(command), "lame -V0 -q0 %s %s", wavFile, LAME_OUTPUT);

	double t0 = getPreciseTime ();
	int retval = system (command);
	double t = getPreciseTime ();

	unlink (LAME_OUTPUT);

	if (retval) {
		emit sendTextToUI(strdup("The LAME encoder reported an error."));
		return -1;
	}
	
	double runtime = t - t0;
	char message[128];
	snprintf (message, sizeof(message), "\nTime to generate MP3 = %.2f seconds\n", runtime);
	emit sendTextToUI(strdup(message));

	return runtime;
}

