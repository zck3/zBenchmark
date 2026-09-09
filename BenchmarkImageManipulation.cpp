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
#include "BenchmarkImageManipulation.h"

#include <QImageReader>

#define IMAGEFILE_NAME "Brisbane64MP.jpg"

//----------------------------------------------------------------------------
// Method:	measure 
// Purpose:	Perform image conversions using ImageMagick; report live results.
// Returns:	Time in seconds to run.
//----------------------------------------------------------------------------
float BenchmarkImageManipulation::measure () 
{
	QImageReader::setAllocationLimit(0);

	const char *assetPath = findAsset(IMAGEFILE_NAME);
	if (!assetPath) {
		emit sendTextToUI(strdup("Cannot find image file required for this test."));
		return BENCHMARK_ERROR;
	}
		
	double totalTime = 0;
	double t0 = getPreciseTime();

	emit sendTextToUI(strdup ("Load 64MP JPG...\n"));

	QImage image = QImage(assetPath);
	if (image.isNull()) {
		return BENCHMARK_ERROR;
	}

	free((void*)assetPath);

	printf("Image has dimensions %d x %d\n", image.width(),image.height());

	QImage *initialImage = new QImage(image);
	emit sendImageToUI(initialImage);
	emit sendTextToUI(strdup ("Mirror...\n"));

	QImage *mirrored;
	mirrored = new QImage(image.mirrored (true, true));
	emit sendImageToUI(mirrored);
	mirrored = new QImage(image.mirrored (false, true));
	emit sendImageToUI(mirrored);
	mirrored = new QImage(image.mirrored (true, false));
	emit sendImageToUI(mirrored);

	emit sendTextToUI(strdup ("Grayscale...\n"));
	emit sendImageToUI(new QImage(image.convertToFormat(QImage::Format_Grayscale8)));

	emit sendTextToUI(strdup ("Invert...\n"));

	image.invertPixels ();
	emit sendImageToUI(new QImage(image));
	image.invertPixels ();
	emit sendImageToUI(new QImage(image));

	emit sendTextToUI(strdup ("Scale...\n"));

	int width = image.width();
	int height = image.height();
	QImage *scaled;
	for (int factor = 2; factor <= height/8; factor++) {
		scaled = new QImage(image.scaledToHeight (height / factor));
		emit sendImageToUI(scaled);
	}
	for (int factor = height/8; factor >= 1; factor--) {
		scaled = new QImage(image.scaledToHeight (height / factor));
		emit sendImageToUI(scaled);
	}

	emit sendTextToUI(strdup ("Rotate...\n"));

	for (int angle = 61; angle < 360; angle += 57) {
		QTransform transform;
		transform.translate (width/2, height/2);
		transform.rotate (angle);
		transform.translate (-width/2, -height/2);
		QImage *rotated = new QImage(image.transformed (transform));
//printf ("Rotated by %d degrees\n", angle);
		emit sendImageToUI(rotated);
	}

	emit sendImageToUI(NULL);

        totalTime = getPreciseTime() - t0;

	char message[128];
	snprintf (message, sizeof(message), "Total image manipulation time %.1f seconds\n", totalTime);
	emit sendTextToUI(strdup(message));

	return totalTime;
}

