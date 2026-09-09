/*=========================================================================
 * This file is part of zBenchmark, which is a system benchmarking tool.
 * (C) 2021-2022, 2026 Zack T Smith.
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
#include "BenchmarkVideoEncoding.h"

#define VIDEOFILE_NAME "LakeArrowhead.mp4"

//----------------------------------------------------------------------------
// Method:	measure 
// Purpose:	Perform video encoding test using ffmpeg, report live results.
// Returns:	Time in seconds to run.
//----------------------------------------------------------------------------
float BenchmarkVideoEncoding::measure () 
{
	if (!ensureThatProgramIsInstalled ("ffmpeg"))
		return BENCHMARK_ERROR;

	emit sendTextToUI(strdup ("This is a multicore encoding benchmark using ffmpeg.\n"));

	char *assetPath = findAsset(VIDEOFILE_NAME);
	if (!assetPath ) {
		emit sendTextToUI(strdup("Cannot find video file needed for the test."));
		return BENCHMARK_ERROR;
	}
		
	emit sendTextToUI(strdup("Encoding 1080p 60fps video with h264 video codec...\nThis may take several minutes.\n"));

	const char *outputPath = "/tmp/result.mp4";
	unlink(outputPath);

	char encodingCommand[PATH_MAX*2];

	snprintf (encodingCommand, sizeof(encodingCommand), "nice -20 ffmpeg -i %s -acodec mp2 -map_metadata -1 -b:a 160k -vcodec h264 -profile:v high -level:v 6.0 -preset slow %s", assetPath, outputPath); 

        double t0 = getPreciseTime();
	int retVal = system(encodingCommand);
        double timeToRun = getPreciseTime() - t0;

	unlink(outputPath);

	char message[128];
	if (retVal) {
		snprintf(message, sizeof(message), "ffmpeg exited with error code %d.\n", retVal);
	}
	else {
		snprintf(message, sizeof(message), "Encoding time was %.1f seconds\n", timeToRun);
	}
	emit sendTextToUI(strdup(message));

	free(assetPath);

	return 0 == retVal ? timeToRun : BENCHMARK_ERROR;
}

