/*=========================================================================
 * This file is part of zBenchmark, which is a system benchmarking tool.
 * (C) 2021, 2024, 2026 Zack T Smith.
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

// (C) Zack T Smith

#include <QThread>
#include <QLabel>
#include <QApplication>
#include <QMessageBox>
#include <string>

extern "C" {
	#include <stdio.h>
};

#include "defs.h"
#include "Utility.h"
#include "MainWindow.h"

bool forceStop = false;

//----------------------------------------------------------------------------
// Name:	addBenchmark
// Purpose:	Injects a benchmark so that it can be selected and run by user.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::addBenchmark (Benchmark *benchmarkObject)
{
	BenchmarkInfo *info = new BenchmarkInfo();
	info->benchmarkObject = benchmarkObject;
	benchmarksArray.push_back(info);
	connectBenchmarkSignals (benchmarkObject);
}

//----------------------------------------------------------------------------
// Name:	clearLog
// Purpose:	Clears the text widget and hides the image.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::clearLog()
{
	resultsTextView->moveCursor (QTextCursor::Start);
	resultsTextView->setPlainText("");
	receiveImage(NULL);
}

//----------------------------------------------------------------------------
// Name:	clearAllCheckmarks
// Purpose:	Clears all of the checkmarks next to benchmark names.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::clearAllCheckmarks()
{
	for (BenchmarkInfo *info : benchmarksArray) {
		Benchmark *benchmark = info->benchmarkObject;
		const char *name = benchmark->name();
		info->label->setText (name);
	}
}

//----------------------------------------------------------------------------
// Name:	setCheckmark
// Purpose:	Adds the check mark next to a benchmark's name.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::setCheckmark (Benchmark *benchmark, bool showCheckmark)
{
	BenchmarkInfo *foundInfo = nullptr;

	for (BenchmarkInfo *info : benchmarksArray) {
		if (info->benchmarkObject == benchmark) {
			foundInfo = info;
			break;
		}
	}

	if (!foundInfo) {
		return;
	}

	QLabel *label = foundInfo->label;
	if (!label) {
		return;
	}
	const char *name = benchmark->name();
	if (!name) {
		return;
	}

	char string[128];
	if (showCheckmark) {
		snprintf (string, sizeof(string), "%s ✓", name);
	} else {
		snprintf (string, sizeof(string), "%s", name);
	}

	label->setText(string);
}

//----------------------------------------------------------------------------
// Name:	receiveResultsText (SLOT)
// Purpose:	Receives text from a worker thread for display in text widget.
// Returns:	Nothing.
// Note:	Frees the text, so it must be strdup'd by the sender.
//----------------------------------------------------------------------------
void MainWindow::receiveResultsText (char *message)
{
	if (!resultsTextView || !message || !*message) {
		return;
	}

	const bool isGuiThread = QThread::currentThread() == QCoreApplication::instance()->thread();
	if (!isGuiThread) {
		fprintf (stderr, "Error (%s): NOT ON MAIN THREAD\n", __FUNCTION__);
		return;
	}

	appendResultsText (message);

	free(message);
}

//----------------------------------------------------------------------------
// Name:	receiveResultsHTML (SLOT)
// Purpose:	Receives HTML from a worker thread for display in text widget.
// Returns:	Nothing.
// Note:	Frees the string, so it must be strdup'd by the sender.
//----------------------------------------------------------------------------
void MainWindow::receiveResultsHTML (char *html)
{
	if (!resultsTextView || !html || !*html) {
		return;
	}

	const bool isGuiThread = QThread::currentThread() == QCoreApplication::instance()->thread();
	if (!isGuiThread) {
		fprintf (stderr, "Error (%s): NOT ON MAIN THREAD\n", __FUNCTION__);
		return;
	}

	appendResultsHTML (html);

	free(html);
}

//----------------------------------------------------------------------------
// Name:	receiveResultsNewline (SLOT)
// Purpose:	Receives newline from worker thread for display in text widget.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::receiveResultsNewline ()
{
	appendResultsText ("\n");
}

//----------------------------------------------------------------------------
// Name:	receiveStatusBarText (SLOT)
// Purpose:	Receives text from worker thread to display in the status bar.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::receiveStatusBarText (const char* text)
{
	statusBar->showMessage (text);
}

//----------------------------------------------------------------------------
// Name:	appendCursorToResultsText
// Purpose:	Adds a fake cursor to the end of the text widget.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::appendCursorToResultsText ()
{
	resultsTextView->moveCursor (QTextCursor::End);
	resultsTextView->insertPlainText("▒");
}

//----------------------------------------------------------------------------
// Name:	removeCursorFromResultsText
// Purpose:	Removes the fake cursor to the end of the text widget.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::removeCursorFromResultsText ()
{
	resultsTextView->moveCursor (QTextCursor::End);
	resultsTextView->textCursor().deletePreviousChar();
}

//----------------------------------------------------------------------------
// Name:	appendResultsText
// Purpose:	Appends text to the text widget.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::appendResultsText (const char *message)
{
	removeCursorFromResultsText ();
	resultsTextView->moveCursor (QTextCursor::End);
	resultsTextView->insertPlainText(message);
	appendCursorToResultsText ();
	resultsTextView->moveCursor (QTextCursor::End);
}

//----------------------------------------------------------------------------
// Name:	appendResultsHTML
// Purpose:	Appends HTML to the text widget.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::appendResultsHTML (const char *html)
{
	removeCursorFromResultsText ();
	resultsTextView->moveCursor (QTextCursor::End);
	resultsTextView->insertHtml(html);
	appendCursorToResultsText ();
	resultsTextView->moveCursor (QTextCursor::End);
}

//----------------------------------------------------------------------------
// Name:	receiveScore (SLOT)
// Purpose:	Receives a test score from a worker thread and displays it.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::receiveScore(float score, const char *name) 
{
	char message[64];
	snprintf (message, sizeof(message), "SCORE for %s is %.2f\n", name, score);
	appendResultsText(message);
	removeCursorFromResultsText();
	resultsTextView->insertPlainText("________________________________\n\n");
}

//----------------------------------------------------------------------------
// Name:	receiveAggregateScore (SLOT)
// Purpose:	Receives aggregate score for all benchmarks from worker thread.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::receiveAggregateScore(float score, float totalRunTime)
{
	displayAggregateScore(score, totalRunTime);
	stopButton->setHidden(true);
	statusBar->showMessage ("");
}

//----------------------------------------------------------------------------
// Name:	receiveImage (SLOT)
// Purpose:	Receives an image (e.g. graph) from worker thread, displays it.
// Returns:	Nothing.
// Note:	If NULL, hides the current image.
//----------------------------------------------------------------------------
void MainWindow::receiveImage(QImage* image) 
{
	if (!testResultsImageView) {
		return;
	}
	if (image == NULL || !image->height() || image->isNull()) {
		if (showingImageView) {
			showingImageView = false;
			layoutGUI();
		}
		return;
	}

	QSize size = image->size();
	imageAspectRatio = (float) size.width() / (float) size.height();
	QPixmap pixmap;
	pixmap.convertFromImage (*image);
	delete image;
	testResultsImageView->setPixmap (pixmap);

	if (!showingImageView) {
		showingImageView = true;
		layoutGUI();
	}
}

//----------------------------------------------------------------------------
// Name:	resizeEvent (override)
// Purpose:	Handles window resize events.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::resizeEvent (QResizeEvent *event) 
{
	if (event) {
		if (verticalList) {
			layoutGUI();
		}
	}
}

//----------------------------------------------------------------------------
// Name:	layoutGUI
// Purpose:	Manually lays out the widgets within the window.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::layoutGUI() 
{
	const int width = size().width();
	const int height = size().height();

	int menuHeight = menuBar->size().height();

	const int verticalListWidth = LEFT_AREA_WIDTH;
	const int buttonWidth = 90;

	verticalList->move(0, menuHeight);
	verticalList->resize(verticalListWidth, height - menuHeight);

	stopButton->move(width - buttonWidth, 0);
	stopButton->resize(buttonWidth, menuHeight-2);

	int rightSideWidth = width - verticalListWidth;

	int y = menuHeight;
	if (showingImageView) {
		float imageViewHeight = height * .75;
#define MAX_GRAPH_HEIGHT 800
		if (imageViewHeight > MAX_GRAPH_HEIGHT) {
			imageViewHeight = MAX_GRAPH_HEIGHT;
		}
		float imageViewWidth = imageAspectRatio * imageViewHeight;

		if (imageAspectRatio >= 1) {
			// wide image
			if (imageViewWidth > rightSideWidth) {
			imageViewWidth = rightSideWidth;
			imageViewHeight = rightSideWidth / imageAspectRatio;
			}
		} else {
			// tall image, already constrained above.
		}

		float imageViewXOffset = (rightSideWidth - imageViewWidth)/2;
		testResultsImageView->move(verticalListWidth + imageViewXOffset, y);
		testResultsImageView->resize(imageViewWidth, imageViewHeight);
		y += imageViewHeight;
	} 
	else {
		testResultsImageView->resize(0, 0);
	}

	resultsTextView->move(verticalListWidth, y);
	const int logHeight = height - y;
	resultsTextView->resize(rightSideWidth, logHeight);
}

//----------------------------------------------------------------------------
// Name:	connectBenchmarkSignals 
// Purpose:	Makes it possible for a benchmark running on a worker thread
//		to send data to the GUI.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::connectBenchmarkSignals (Benchmark *benchmark)
{
	connect (benchmark, SIGNAL(sendTextToUI(char*)), this, SLOT(receiveResultsText(char*)), Qt::QueuedConnection);
	connect (benchmark, SIGNAL(sendHTMLToUI(char*)), this, SLOT(receiveResultsHTML(char*)), Qt::QueuedConnection);
	connect (benchmark, SIGNAL(sendNewlineToUI()), this, SLOT(receiveResultsNewline(void)), Qt::QueuedConnection);
	connect (benchmark, SIGNAL(sendScoreToUI(float,const char*)), this, SLOT(receiveScore(float,const char*)), Qt::QueuedConnection);
	connect (benchmark, SIGNAL(sendImageToUI(QImage*)), this, SLOT(receiveImage(QImage*)), Qt::QueuedConnection);
}

//----------------------------------------------------------------------------
// Name:	constructUI
// Purpose:	Builds the GUI programmatically.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::constructUI () 
{
	if (verticalList != nullptr) {
		return;
	}

	connect (this, SIGNAL(sendNewlineToUI(void)), this, SLOT(receiveResultsNewline(void)), Qt::QueuedConnection);
	connect (this, SIGNAL(sendStatusBarText(const char*)), this, SLOT(receiveStatusBarText(const char*)), Qt::QueuedConnection);
	connect (this, SIGNAL(sendScoreToUI(float,const char*)), this, SLOT(receiveScore(float,const char*)), Qt::QueuedConnection);
	connect (this, SIGNAL(sendAggregateScoreToUI(float, float)), this, SLOT(receiveAggregateScore(float, float)), Qt::QueuedConnection);
	connect (this, SIGNAL(sendImageToUI(QImage*)), this, SLOT(receiveImage(QImage*)), Qt::QueuedConnection);
	connect (this, SIGNAL(sendTextToUI(char*)), this, SLOT(receiveResultsText(char*)), Qt::QueuedConnection);
	connect (this, SIGNAL(sendHTMLToUI(char*)), this, SLOT(receiveResultsHTML(char*)), Qt::QueuedConnection);

	verticalList = new QWidget(this);
	layout = new QVBoxLayout(verticalList);
	layout->setAlignment(Qt::AlignTop);
	verticalList->setLayout(layout);
	verticalList->setAutoFillBackground(true);

	fileMenu = new QMenu("&File");
	QAction *action;
	action = fileMenu->addAction("System &Information");
        action->setShortcut (QKeySequence ("Ctrl+i"));
	connect(action, &QAction::triggered, this, &MainWindow::onSystemInformation);
	action = fileMenu->addAction("&Quit");
        action->setShortcut (QKeySequence ("Ctrl+q"));
	connect(action, &QAction::triggered, this, &MainWindow::onQuit);

	benchmarksMenu = new QMenu("&Run");
	action = benchmarksMenu->addAction("&All benchmarks");
        action->setShortcut (QKeySequence ("Ctrl+r"));
	connect(action, &QAction::triggered, this, &MainWindow::onRunAllBenchmarks);
	action = benchmarksMenu->addAction("&Stop run");
        action->setShortcut (QKeySequence ("Ctrl+."));
	connect(action, &QAction::triggered, this, &MainWindow::onStopRun);

	helpMenu = new QMenu("&Help");
	action = helpMenu->addAction("&About");
	connect(action, &QAction::triggered, this, &MainWindow::onAbout);

	menuBar = new QMenuBar();
	menuBar->addMenu(fileMenu);
	menuBar->addMenu(benchmarksMenu);
	menuBar->addMenu(helpMenu);
	this->setMenuBar (menuBar);

	statusBar = new QStatusBar();
	setStatusBar(statusBar);

	stopButton = new QPushButton("Stop run", menuBar);
	stopButton->setFont(QFont("Helvetica", 11, QFont::Light));
	connect(stopButton, &QPushButton::clicked, this, &MainWindow::onStopRun);

	QLabel *label = new QLabel ("Benchmarks", verticalList); 
	label->setFont(QFont("Helvetica", 16, QFont::Bold));
	label->setMinimumHeight (30);
	layout->addWidget(label);

	for (BenchmarkInfo *info : benchmarksArray) 
	{
		Benchmark *benchmark = info->benchmarkObject;
		const char *name = benchmark->name();
		if (!name) {
			printf ("Nameless benchmark found. Ignoring...\n");
		}
		else {
			// Add label to list of benchmarks.
			QLabel *label = new QLabel (name, verticalList); 
			info->label = label;
			label->setMinimumHeight (24);
			label->setFont(QFont("Helvetica", 12, QFont::Light));
			layout->addWidget(label);

			// Create and add the pulldown menu item.
			QAction *action = benchmarksMenu->addAction(name);
			info->action = action;
			connect(action, &QAction::triggered, this, [this, benchmark]() { 
				userDidRequestSoloBenchmarkRun (benchmark);
			});
		}
	}

	QLabel *spacer = new QLabel (" ", verticalList); 
	spacer->setMinimumHeight (30);
	layout->addWidget(spacer);

	aggregateScoreTitleLabel = new QLabel ("Final Score", verticalList); 
	aggregateScoreTitleLabel->setFont(QFont("Helvetica", 15, QFont::Bold));
	aggregateScoreTitleLabel->setMinimumHeight (17);
	layout->addWidget(aggregateScoreTitleLabel);
	aggregateScoreValueLabel = new QLabel ("N/A", verticalList); 
	aggregateScoreValueLabel->setFont(QFont("Helvetica", 20));
	aggregateScoreValueLabel->setMinimumHeight (21);
	layout->addWidget(aggregateScoreValueLabel);
	aggregateScoreTitleLabel->setHidden (true);
	aggregateScoreValueLabel->setHidden (true);

	resultsTextView = new QTextEdit(this);
	resultsTextView->setFont(QFont("DejaVu Sans", 13, QFont::Thin));
	resultsTextView->setReadOnly(true);

#define GREETING_HTML \
	"<h3>Welcome to zBenchmark</h3>" \
	"This application measures the performance of your computer "\
	"and provides a convenient single-number performance metric as "\
	"its result."\
	"<p>" \
	"This number is relative to a 3.1 GHz Core i5 4278U (Macbook Pro 2014) "\
	"that is running 64-bit Debian OS, which is defined as having a performance "\
	"rating of 1.0."\
	"<p>" \
	"You can either run all benchmarks to get a combined score, or run them individually."\
	"<p>" \
	"To use zBenchmark effectively you will need to install various programs that it invokes."\
	"<p>" \
	"<code>ffmpeg povray gnuplot bc lame clang python3 python3-pip</code>"\
	"<p>" \
	"To install everything, including zBenchmark, run the included script <code>install.sh</code>."

	resultsTextView->setHtml(GREETING_HTML);

	testResultsImageView = new QLabel(this);
	testResultsImageView->setScaledContents(true);
	testResultsImageView->setFrameStyle(QFrame::Panel | QFrame::Sunken);

	layoutGUI();

	stopButton->setHidden(true);
}

//----------------------------------------------------------------------------
// Name:	userDidRequestSoloBenchmarkRun
// Purpose:	Responds to user request via menu to run one benchmark.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::userDidRequestSoloBenchmarkRun(Benchmark *benchmark) 
{
	if (!benchmark) {
		return;
	}
	const char *name = benchmark->name();
	
	clearLog();
	setCheckmark(benchmark, false);

	char message[128];
	snprintf (message, sizeof(message), "Running %s benchmark...\n", name);
	appendResultsText (message);

	QThread *thread = QThread::create([this, benchmark] {
		runningBenchmark = true;

		float score = benchmark->measureAgainstReference();
		if (score < 0) {
			emit sendTextToUI(strdup("Cannot proceed.\n"));
		}
		else {
			emit sendScoreToUI(score, benchmark->name());
			emit setCheckmark(benchmark, true);
		}
		runningBenchmark = false;

		if (forceStop) {
			emit sendTextToUI(strdup("Testing stopped.\n"));
			forceStop = false;
			emit sendStatusBarText("");
		}
	}); 
	thread->start();
}

//----------------------------------------------------------------------------
// Name:	onRunAllBenchmarks
// Purpose:	Run all benchmarks on a worker thread.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::onRunAllBenchmarks() 
{
	statusBar->showMessage ("Running all tests...");

	stopButton->setHidden(false);

	clearAllCheckmarks();
	clearAggregateScore();
	clearLog();

	QThread *thread = QThread::create([this] {

		runAllStartTime = getPreciseTime();

		bool ranAll = runAllBenchmarks();

		if (!forceStop) {
			char message[128];
			if (ranAll) {
				float score = computeAggregateScore();
				double totalRunTime = getPreciseTime() - runAllStartTime;
				emit sendAggregateScoreToUI(score, totalRunTime);
				int seconds = (int) round(totalRunTime);
				int minutes = seconds / 60;
				int hours = seconds / 3600;
				minutes %= 60;
				seconds %= 60;
				if (hours) {
					snprintf (message, sizeof(message), "Completed all benchmarks in %02d:%02d:%02d.", hours, minutes, seconds);
				} else {
					snprintf (message, sizeof(message), "Completed all benchmarks in %02d:%02d.", minutes, seconds);
				}
			}
			else {
				emit sendTextToUI (strdup("Failed to run all benchmarks.\n"));
				forceStop = false;
				stopButton->setHidden(true);
			}
			emit sendStatusBarText (nullptr);
		} else {
			emit sendTextToUI(strdup("Testing stopped.\n"));
			forceStop = false;
			emit sendStatusBarText("");
		}
	}); 

	thread->start();
}

//----------------------------------------------------------------------------
// Name:	computeAggregateScore
// Purpose:	Average all the scores to get the combined test score.
// Returns:	Combined test score.
//----------------------------------------------------------------------------
float MainWindow::computeAggregateScore()
{
	float score = 0;
	int count = scores.size();
	for (int i=0; i < count; i++) {
		// RULE: It's additive for now, not weighted.
		score += scores[i];
	}
	score /= (float)count;
	return score;
}

//----------------------------------------------------------------------------
// Name:	clearAggregateScore
// Purpose:	Hide final test score.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::clearAggregateScore(void)
{
	aggregateScoreValueLabel->setText ("");
	aggregateScoreTitleLabel->setHidden (true);
	aggregateScoreValueLabel->setHidden (true);
}

//----------------------------------------------------------------------------
// Name:	displayAggregateScore
// Purpose:	Display combined test score.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::displayAggregateScore(float score, float totalRunTime)
{
	char message[128];
	snprintf (message, sizeof(message), "%.2f", score);
	
	aggregateScoreTitleLabel->setHidden (false);
	aggregateScoreValueLabel->setHidden (false);
	aggregateScoreValueLabel->setText (message);

	printf("AGGREGATE SCORE %.2f\n", score);

	snprintf(message, sizeof(message), "Time to run all benchmarks: %.1f minutes\n", totalRunTime / 60.0);
	appendResultsText (message);

	statusBar->showMessage ("");
}

//----------------------------------------------------------------------------
// Name:	onStopRun
// Purpose:	Tell test runner to eventually stop.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::onStopRun() 
{
	statusBar->showMessage ("STOPPING");
	forceStop = true;
}

//----------------------------------------------------------------------------
// Name:	onSystemInformation
// Purpose:	Collect and display system information.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::onSystemInformation() 
{
	if (runningBenchmark) {
		QMessageBox::warning (this, PROGRAM_NAME,
    			"You're currently running a benchmark. Please wait until that's done.");
		return;
	}

	clearLog ();

	char message[128];
	bool didRead;

	appendResultsHTML ("<h3>System Information</h3><p>");

	// Get kernel type.
	QString productType = QSysInfo::productType();
	if (productType.length() > 0) {
		std::string string = productType.toStdString();
		snprintf (message, sizeof(message), "<b>PRODUCT TYPE:</b> %s<br>", string.c_str());
		appendResultsHTML (message);
	}

	// Get kernel type.
	QString kernelType = QSysInfo::kernelType();
	if (kernelType.length() > 0) {
		std::string string = kernelType.toStdString();
		snprintf (message, sizeof(message), "<b>KERNEL TYPE:</b> %s<br>", string.c_str());
		appendResultsHTML (message);
	}

#ifndef __APPLE__
	// Get computer type.
	//
	system ("grep Model /proc/cpuinfo | sed 's/Model[ \\t]*://' > /tmp/.model");
	char model[64] = {0};
	if (0 < readTrimmedLineFromFile ("/tmp/.model", model, sizeof(model))) {
		snprintf (message, sizeof(message), "<b>MODEL:</b> %s<br>", model);
		appendResultsHTML (message);
	}
	unlink ("/tmp/.model");

	// Get CPU chip.
	//
	system ("grep Hardware /proc/cpuinfo | sed 's/Hardware[ \\t]*://' > /tmp/.cputype");
	char cputype[64] = {0};
	if (0 < readTrimmedLineFromFile ("/tmp/.cputype", cputype, sizeof(cputype))) {
		snprintf (message, sizeof(message), "<b>CPU CHIP:</b> %s<br>", cputype);
		appendResultsHTML (message);
	}
	unlink ("/tmp/.cputype");
#endif

	// Get CPU class.
	QString cpuClass = QSysInfo::currentCpuArchitecture();
	if (cpuClass.length() > 0) {
		std::string string = cpuClass.toStdString();
		snprintf (message, sizeof(message), "<b>CPU CLASS:</b> %s<br>", string.c_str());
		appendResultsHTML (message);
	}

#ifndef __APPLE__
	// Get instruction set architecture.
	//
	system ("uname -m > /tmp/.isa");
	char isa[64] = {0};
	if (0 < readTrimmedLineFromFile ("/tmp/.isa", isa, sizeof(isa))) {
		snprintf (message, sizeof(message), "<b>CPU INSTRUCTIONS:</b> %s<br>", isa);
		appendResultsHTML (message);
	}
	unlink ("/tmp/.isa");

	// Get total # of cores.
	//
	system ("cat /proc/cpuinfo | grep processor | wc -l > /tmp/.ncores");
	long ncores = 0;
	didRead = readLongFromFile ("/tmp/.ncores", &ncores);
	unlink ("/tmp/.ncores");
	if (didRead) {
		snprintf (message, sizeof(message), "<b>CPU CORES:</b> %ld<br>", ncores);
		appendResultsHTML (message);
	}
	unlink ("/tmp/.ncores");

	// Get all cores' current frequencies.
	//
	for (long i=0; i < ncores; i++) {
		char path[PATH_MAX];
		snprintf (path, sizeof(path), "/sys/devices/system/cpu/cpu%ld/cpufreq/scaling_cur_freq", i);
		long khz = 0;
		didRead = readLongFromFile (path, &khz);
		if (didRead) {
			snprintf (message, sizeof(message), "<b>CORE %ld's FREQUENCY:</b> %.3lf GHz<br>", i, (float)khz / 1000000.f);
			appendResultsHTML (message);
		}
	}

	// Get current CPU temperature.
	long temp0 = 0;
	long temp1 = 0;
	bool didRead0 = readLongFromFile ("/sys/class/thermal/thermal_zone0/temp", &temp0);
	bool didRead1 = readLongFromFile ("/sys/class/thermal/thermal_zone1/temp", &temp1);
	if (didRead0 && !didRead1) {
		snprintf (message, sizeof(message), "<b>CPU TEMPERATURE:</b> %.2f °C<br>", (float)temp0 / 1000.f);
		appendResultsHTML (message);
	}
	else if (didRead0 && didRead1) {
		snprintf (message, sizeof(message), "<b>CPU TEMPERATURES:</b> %.2f °C, %.2f °C<br>", (float)temp0 / 1000.f, (float)temp1 / 1000.f);
		appendResultsHTML (message);
	}

	// Get total main memory.
	//
	system ("grep MemTotal: /proc/meminfo | sed 's/MemTotal://' | sed 's/kB//' > /tmp/.mem");
	float memory;
	didRead = readFloatFromFile ("/tmp/.mem", &memory);
	unlink ("/tmp/.mem");
	if (didRead) {
		memory = ceil(memory / ONE_MEGABYTE);
		snprintf (message, sizeof(message), "<b>MEMORY:</b> %d GB<br>", (int)memory);
		appendResultsHTML (message);
	}
	unlink ("/tmp/.mem");

	// Get current system load.
	//
	float load = 0;
	didRead = readFloatFromFile ("/proc/loadavg", &load);
	if (load > 0) {
		snprintf (message, sizeof(message), "<b>SYSTEM LOAD:</b> %.2f<br>", load);
		appendResultsHTML (message);
	}

	// Get currently attached USB devices other than hubs and controllers.
	//
	system ("lsusb -v 2> /dev/null | grep iProduct | grep -v unavailable | sed 's/^.*iProduct.*[0-9] //' | grep -v Hub | grep -v Host.Con | grep -v '^$' > /tmp/.usb");
	char *usb_devices[16];
	int n_usb_devices = readTrimmedLinesFromFile("/tmp/.usb", usb_devices, sizeof(usb_devices));
	if (n_usb_devices) {
		appendResultsHTML ("<b>USB DEVICES:</b><br>");
		for (int i=0; i < n_usb_devices; i++) {
			snprintf (message, sizeof(message), "&nbsp;&nbsp; • %s<br>", usb_devices[i]);
			appendResultsHTML (message);
		}
	}
	unlink ("/tmp/.usb");
#endif

	removeCursorFromResultsText();
}

//----------------------------------------------------------------------------
// Name:	onAbout
// Purpose:	Display an "About" popup.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::onAbout() 
{
	QMessageBox::information (this, PROGRAM_NAME " " PROGRAM_RELEASE,
		"This is a benchmarking tool for Unix-based computers.");
}

//----------------------------------------------------------------------------
// Name:	onQuit
// Purpose:	Immediately exit the app.
// Returns:	Nothing.
//----------------------------------------------------------------------------
void MainWindow::onQuit() 
{
	// XX Need to kill any program we invoked.
	exit(0);
}

//----------------------------------------------------------------------------
// Name:	runAllBenchmarks
// Purpose:	Run all of the benchmarks.
// Returns:	True if ran all of the benchmarks.
// Note:	This is meant to be called from a worker thread.
//----------------------------------------------------------------------------
bool MainWindow::runAllBenchmarks()
{
	scores.empty();

	bool ranAll = true;
	forceStop = false;
	runningBenchmark = true;

	for (BenchmarkInfo *info : benchmarksArray) {
		emit sendImageToUI(nullptr);

		Benchmark *benchmark = info->benchmarkObject;

		char message[128];
		snprintf (message, sizeof(message), "Running %s benchmark...\n", benchmark->name());
		emit sendTextToUI(strdup(message));

		float score = benchmark->measureAgainstReference();
		if (score >= 0) {
			scores.append(score);

			emit sendScoreToUI(score, benchmark->name());
			emit setCheckmark(benchmark, true);
		}
		else {
			ranAll = false;
			break;
		}

		if (forceStop) {
			break;
		}

		QThread::currentThread()->sleep(INTER_TEST_SLEEP_TIME_SECONDS);
	}

	runningBenchmark = false;

	if (forceStop) {
		statusBar->showMessage ("");
	}

	return ranAll;
}

