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

#ifndef _BENCHMARK_H
#define _BENCHMARK_H

extern "C" {
	#include <stdio.h>
	#include <stdlib.h>
	#include <stdbool.h>
	#include <math.h>
	#include <sys/stat.h>
}

#include <QObject>
#include <QPixmap>

#include "Utility.h"

#define BENCHMARK_ERROR (-1.f)

class Benchmark : public QObject
{
	Q_OBJECT

signals:
	void sendTextToUI(char *);
	void sendHTMLToUI(char *);
	void sendScoreToUI(float,const char *);
	void sendImageToUI(QImage*);
	void sendNewlineToUI(void);

public:
	virtual float measure () = 0;
	virtual const char *name () = 0;
	virtual float referenceMeasurement () = 0;
	virtual bool smallerMeasurementIsHigherScore () {
		return true;
	}

	float measureAgainstReference () 
	{
		float measurement = measure();
		if (measurement < 0)
			return BENCHMARK_ERROR;

		printf ("RAW MEASUREMENT for %s is %g\n", name(), measurement);

		float reference = referenceMeasurement();
		float score;
		if (smallerMeasurementIsHigherScore()) {
			if (measurement == 0.0)
				score = FP_NAN;
			else
				score = reference / measurement;
		} else {
			score = measurement / reference;
		}
		return score;
	}

	bool ensureThatProgramIsInstalled (const char *name) {
		if (!name)
			return true;

		if (!programIsInstalled(name)) {
			char message[256];
			snprintf (message, sizeof(message), "Problem encountered:\nCannot find a program called %s. Please install it to proceed.\nFor example: sudo apt install %s\n", name, name);
                	emit sendTextToUI(strdup(message));
                	return false;
        	}
		return true;
	}
};

#endif
