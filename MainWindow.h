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

#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

#include <QObject>
#include <QMainWindow>
#include <QMenuBar>
#include <vector>
#include <QVector>
#include <QHash>
#include <QPushButton>
#include <QLabel>
#include <QThread>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QTextEdit>

#include "Benchmark.h"

#define LEFT_AREA_WIDTH (215)
#define INTER_TEST_SLEEP_TIME_SECONDS (1)

class BenchmarkInfo : public QObject 
{
public:
	unsigned identifier;
	QLabel *label;
	QAction *action;
	Benchmark *benchmarkObject;
	float score;
};

class MainWindow : public QMainWindow
{
	Q_OBJECT

	// GUI elements
	QMenuBar *menuBar = nullptr;
	QMenu *fileMenu = nullptr;
	QMenu *benchmarksMenu = nullptr;
	QMenu *helpMenu = nullptr;
	QVBoxLayout *layout = nullptr;
	QWidget *verticalList = nullptr;
	QLabel *aggregateScoreTitleLabel = nullptr;
	QLabel *aggregateScoreValueLabel = nullptr;
	QPushButton *stopButton = nullptr;
	QLabel *testResultsImageView = nullptr;
	QTextEdit *resultsTextView = nullptr;
	QStatusBar *statusBar = nullptr;

	void resizeEvent (QResizeEvent *event); // override
	void layoutGUI();

	float imageAspectRatio = 0;
	bool showingImageView = false;

	float computeAggregateScore();
	void displayAggregateScore(float score, float totalRunTime);
	void clearAggregateScore(void);
	void displayIndividualScore(float, const char*);

	void appendResultsText (const char *message);
	void appendResultsHTML (const char *html);
	void appendResultsNewline ();
	void clearLog();
	void clearAllCheckmarks();

	// Artificial cursor to make up for QTextEdit deficiency.
	void appendCursorToResultsText();
	void removeCursorFromResultsText();

	std::vector<BenchmarkInfo*> benchmarksArray;
	void connectBenchmarkSignals (Benchmark *benchmark);

	bool runAllBenchmarks();
	bool runningBenchmark = false;
	double runAllStartTime = 0;

	// Menu callbacks
	void userDidRequestSoloBenchmarkRun(Benchmark*);
	void onStopRun();
	void onRunAllBenchmarks();
	void onAbout();
	void onSystemInformation();
	void onQuit();

	QVector<float> scores;

public:
	void addBenchmark (Benchmark *);
	void constructUI ();

signals:
        void sendTextToUI(char *);
        void sendHTMLToUI(char *);
        void sendImageToUI(QImage*);
        void sendNewlineToUI(void);
        void sendStatusBarText(const char*);
	void sendScoreToUI(float, const char*);
	void sendAggregateScoreToUI(float score, float totalRunTime);

public slots:
	void receiveScore(float, const char *);
	void receiveAggregateScore(float score, float totalRunTime);
	void receiveResultsText(char *message);
	void receiveResultsHTML(char *message);
	void receiveResultsNewline();
	void receiveStatusBarText(const char*);
	void receiveImage(QImage*);
	void setCheckmark (Benchmark *benchmark, bool showCheckmark);
};

#endif
