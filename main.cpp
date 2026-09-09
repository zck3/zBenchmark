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

#include <stdio.h>
#include <QApplication>
#include <QScreen>
#include <QIcon>

#include "defs.h"
#include "MainWindow.h"
#include "zBenchmark.xpm"

QApplication *app = nullptr;

#include "BenchmarkAES.h"
#include "BenchmarkAudioEncoding.h"
#include "BenchmarkBigNumbers.h"
#include "BenchmarkCompilation.h"
#include "BenchmarkCPURayTracing.h"
#include "BenchmarkDiskIO.h"
#include "BenchmarkDoubles.h"
#include "BenchmarkFibonacci.h"
#include "BenchmarkFileCompression.h"
#include "BenchmarkFloats.h"
#include "BenchmarkImageManipulation.h"
#include "BenchmarkMemoryBandwidth.h"
#include "BenchmarkObjectDetection.h"
#include "BenchmarkPrimes.h"
#include "BenchmarkSHA.h"
#include "BenchmarkTTS.h"
#include "BenchmarkVideoEncoding.h"

int main (int argc, char **argv)
{
	app = new QApplication(argc, argv);

	QPixmap pixmap(icon1024);
	QIcon icon(pixmap);
	app->setWindowIcon (icon);

	QScreen *screen = QGuiApplication::primaryScreen();
	QRect rect = screen->availableVirtualGeometry();
	int screenWidth = rect.size().width();
	int screenHeight = rect.size().height();
	int windowWidth = PREFERRED_WINDOW_WIDTH;
	int windowHeight = PREFERRED_WINDOW_HEIGHT;

	int x, y;

	// Layout for desktop UI idiom.
	x = (screenWidth - windowWidth) / 2;
	y = (screenHeight - windowHeight) / 2;
	const int yMinimum = 20;
	if (x < 0 || y < yMinimum) {
		x = 0;
		y = yMinimum;
		windowWidth = screenWidth;
		windowHeight = screenHeight;
	}

	MainWindow *window = new MainWindow();
	window->setWindowTitle(PROGRAM_NAME " " PROGRAM_RELEASE);
	window->setMinimumSize(MINIMUM_WINDOW_WIDTH, MINIMUM_WINDOW_HEIGHT);
	window->move(x, y);
	window->resize(windowWidth, windowHeight);

	BenchmarkAES aesEncryption;
	BenchmarkAudioEncoding audioEncoding;
	BenchmarkBigNumbers bigNumbers;
	BenchmarkCompilation compiler;
	BenchmarkCPURayTracing cpuRayTracing;
	BenchmarkDiskIO diskIO;
	BenchmarkDoubles doubles;
	BenchmarkFibonacci fibonacci;
	BenchmarkFileCompression fileCompression;
	BenchmarkFloats floats;
	BenchmarkImageManipulation imageManipulation;
	BenchmarkMemoryBandwidth memoryBandwidth;
	BenchmarkObjectDetection objectDetection;
	BenchmarkPrimes primes;
	BenchmarkSHA sha;
	BenchmarkTTS tts;
	BenchmarkVideoEncoding videoEncoding;

	window->addBenchmark (&aesEncryption);
	window->addBenchmark (&audioEncoding);
	window->addBenchmark (&bigNumbers);
	window->addBenchmark (&compiler);
	window->addBenchmark (&cpuRayTracing);
	window->addBenchmark (&diskIO);
	window->addBenchmark (&fibonacci);
	window->addBenchmark (&fileCompression);
	window->addBenchmark (&floats);
	window->addBenchmark (&doubles);
	window->addBenchmark (&imageManipulation);
	window->addBenchmark (&memoryBandwidth);
	window->addBenchmark (&objectDetection);
	window->addBenchmark (&primes);
	window->addBenchmark (&sha);
	window->addBenchmark (&tts);
	window->addBenchmark (&videoEncoding);

	window->constructUI();

	window->show();
	return app->exec();
}

