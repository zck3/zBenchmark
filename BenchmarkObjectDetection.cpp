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
#include "BenchmarkObjectDetection.h"

//----------------------------------------------------------------------------
// Method:	measure 
// Purpose:	Perform text to speech conversion, report live results.
// Returns:	Time in seconds to run.
// Note:	Using PiperObjectDetection, which is faster than most.
//----------------------------------------------------------------------------
float BenchmarkObjectDetection::measure ()
{
	if (!programIsInstalled("python3")) {
		emit sendTextToUI(strdup ("Can't locate python3. Please run install.sh.\n"));
		return BENCHMARK_ERROR;
	}

	// Verify that the input photo is installed.
#define PHOTO_FILEPATH "/usr/share/zBenchmark/busyDesktop.jpg"
	FILE *file = fopen (PHOTO_FILEPATH, "r");
	if (!file) {
		emit sendTextToUI(strdup ("Can't locate the input photo. Please run install.sh.\n"));
		return BENCHMARK_ERROR;
	}
	fclose (file);

#define PYTHON_SCRIPT "\n\
import torch\n\
from PIL import Image\n\
from torchvision.models.detection import (\n\
    fasterrcnn_resnet50_fpn_v2,\n\
    FasterRCNN_ResNet50_FPN_V2_Weights,\n\
)\n\
weights = FasterRCNN_ResNet50_FPN_V2_Weights.DEFAULT\n\
model = fasterrcnn_resnet50_fpn_v2(weights=weights)\n\
model.eval()\n\
preprocess = weights.transforms()\n\
categories = weights.meta[\"categories\"]\n\
image = Image.open(\"%s\").convert(\"RGB\")\n\
image_tensor = preprocess(image)\n\
with torch.inference_mode():\n\
    prediction = model([image_tensor])[0]\n\
for box, label, score in zip(\n\
    prediction[\"boxes\"],\n\
    prediction[\"labels\"],\n\
    prediction[\"scores\"]\n\
):\n\
    if score >= 0.5:\n\
        print(\n\
            categories[label],\n\
            float(score),\n\
            box.tolist()\n\
        )\n\
from torchvision.utils import draw_bounding_boxes\n\
from torchvision.transforms.functional import pil_to_tensor, to_pil_image\n\
pil_tensor = pil_to_tensor(image)\n\
threshold = 0.30\n\
keep = prediction[\"scores\"] >= threshold\n\
boxes = prediction[\"boxes\"][keep]\n\
labels = prediction[\"labels\"][keep]\n\
scores = prediction[\"scores\"][keep]\n\
label_text = [\n\
    f\"{categories[label]} {score:.2f}\"\n\
    for label, score in zip(labels, scores)\n\
]\n\
result = draw_bounding_boxes(\n\
    pil_tensor,\n\
    boxes=boxes,\n\
    labels=label_text,\n\
    width=3,\n\
)\n\
result_image = to_pil_image(result)\n\
result_image.save(\"/tmp/object_detection.jpg\")\n\
print(f\"Found {len(boxes)} objects\")\n\
"

#define OUTPUT_FILE "/tmp/object_detection.out"
#define OUTPUT_JPG "/tmp/object_detection.jpg"
	unlink(OUTPUT_JPG);
	unlink(OUTPUT_FILE);

#define PYTHON_SCRIPT_PATH "/tmp/object_detection.py"
	file = fopen (PYTHON_SCRIPT_PATH, "wb");
	if (!file) {
		return BENCHMARK_ERROR;
	}
	fprintf (file, PYTHON_SCRIPT, PHOTO_FILEPATH);
	fclose (file);

	char command[4*PATH_MAX];
	snprintf (command, sizeof(command), "python3 %s > %s", PYTHON_SCRIPT_PATH, OUTPUT_FILE);
		
	emit sendTextToUI(strdup ("Using TorchVision and FasterRCNN_ResNet50_FPN_V2 to identify objects in a photo...\n"));

	double t0 = getPreciseTime();
	int retval = system(command);
        double dt = getPreciseTime() - t0;

	char message[256];
	if (retval) {
		snprintf (message, sizeof(message), "Python error occurred, return value = %d.\n", retval);
		emit sendTextToUI (strdup (message));
		unlink (PYTHON_SCRIPT_PATH);
		return BENCHMARK_ERROR;
	}

	snprintf (message, sizeof(message), "Processing time = %ld seconds.\n", (long) round(dt));
	emit sendTextToUI(strdup (message));

	char *output = contentsOfFileAtPath(OUTPUT_FILE);
	if (output) {
		emit sendTextToUI(strdup ("\nObjects:\n"));
		emit sendTextToUI(strdup (output));
		free (output);
	}

	QImage *annotated_image = new QImage(OUTPUT_JPG);
	if (annotated_image) { 
		emit sendImageToUI(annotated_image);
	}

	unlink(PYTHON_SCRIPT_PATH);
	unlink(OUTPUT_FILE);
	unlink(OUTPUT_JPG);

	return (float)dt;
}

