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
#include "declarationOfIndependence.h"
#include "BenchmarkTTS.h"

//----------------------------------------------------------------------------
// Method:	measure 
// Purpose:	Perform text to speech conversion, report live results.
// Returns:	Time in seconds to run.
// Note:	Using PiperTTS, which is faster than most.
//----------------------------------------------------------------------------
float BenchmarkTTS::measure ()
{
	if (!programIsInstalled("piper")) {
		emit sendTextToUI(strdup ("Can't locate the PiperTTS executable. Please run install.sh.\n"));
		return BENCHMARK_ERROR;
	}

#define OUTPUT_FILE "/tmp/TTS.wav"
	unlink (OUTPUT_FILE);

#define INPUT_FILE "/tmp/TTS.txt"
	FILE *file = fopen (INPUT_FILE, "w");
	if (!file) {
		return BENCHMARK_ERROR;
	}
	fwrite (declarationOfIndependence8192Bytes, 1, strlen(declarationOfIndependence8192Bytes), file);
	fclose (file);

	// Verify that the ONNX models are installed.
#define ONNX_FILE "/usr/share/zBenchmark/en_GB-alan-low.onnx"
	file = fopen (ONNX_FILE, "r");
	if (!file) {
		emit sendTextToUI(strdup ("Can't locate the ONNX model. Please run install.sh.\n"));
		return BENCHMARK_ERROR;
	}
	fclose (file);
#define ONNX_CONFIG "/usr/share/zBenchmark/en_GB-alan-low.onnx.json"
	file = fopen (ONNX_CONFIG, "r");
	if (!file) {
		emit sendTextToUI(strdup ("Can't locate the ONNX model config file. Please run install.sh.\n"));
		return BENCHMARK_ERROR;
	}
	fclose (file);

	char command[4*PATH_MAX];
	snprintf (command, sizeof(command), "piper -m %s -c %s -i %s -f %s",
		ONNX_FILE, ONNX_CONFIG, INPUT_FILE, OUTPUT_FILE);
		
	emit sendTextToUI(strdup ("Using PiperTTS to convert Declaration of Independence to WAV audio...\n"));

	double t0 = getPreciseTime();
	int retval = system(command);
        double dt = getPreciseTime() - t0;

	if (retval) {
		emit sendTextToUI(strdup ("An error occurred."));
		unlink(INPUT_FILE);
		unlink(OUTPUT_FILE);
		return BENCHMARK_ERROR;
	}

	char message[256];
	snprintf (message, sizeof(message), "Conversion time = %ld seconds.\n", (long) round(dt));
	emit sendTextToUI(strdup (message));

	unlink(INPUT_FILE);
	unlink(OUTPUT_FILE);

	return (float)dt;
}

