#!/bin/bash

if [[ `which qmake6` != "" ]]; then
	qmake6
else
	if [[ `which qmake` != "" ]]; then
		qmake
	else
		echo Qmake is not installed.
		exit -1
	fi
fi

if ! sudo apt install ffmpeg povray zip unzip bc gzip bzip2 xz-utils p7zip-full g++ clang lame ; then
	exit 1
fi

SHAREDIR=/usr/share/zBenchmark
if [ ! -d  $SHAREDIR ]; then
	sudo mkdir $SHAREDIR
fi
sudo apt install qmake6 qt6-base-dev qt6-webengine-dev-tools qt6-webengine-dev \
	qt6-multimedia-dev qt6-pdf-dev libqt6pdfwidgets6 \
	gstreamer1.0-plugins-{good,bad,ugly} gstreamer1.0-libav qt6-image-formats-plugins

pip install soundfile bs4 nltk requests
if lspci | grep -i vga | grep -i nvidia ; then
	echo Found Nvidia GPU.
	pip install torch torchvision torchaudio 
else
	echo "There is no Nvidia GPU, so using CPU instead."
	pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cpu
fi
pip install piper-tts pathvalidate

if [ ! -e $SHAREDIR/en_GB-alan-low.onnx ]; then
	wget https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_GB/alan/low/en_GB-alan-low.onnx?download=true
	sudo mv en_GB-alan-low.onnx?download=true $SHAREDIR/en_GB-alan-low.onnx
fi

if [ ! -e $SHAREDIR/en_GB-alan-low.onnx.json ]; then
	wget https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_GB/alan/low/en_GB-alan-low.onnx.json?download=true
	sudo mv en_GB-alan-low.onnx.json?download=true $SHAREDIR/en_GB-alan-low.onnx.json
fi

# Unfortunately the latest gnuplot on Debian has Qt5 as a dependency
# which we should not install, as we are using Qt6.
if [[ `which gnuplot` == "" ]]; then
	sudo apt install libtool m4 pkg-config autoconf automake 
	sudo apt install libcerf-dev libgd-dev
	VER=6.0.rc1
	curl -o gnuplot-$VER.tar.gz -L https://github.com/gnuplot/gnuplot/archive/refs/tags/$VER.tar.gz
	tar xfv gnuplot-$VER.tar.gz
	if pushd gnuplot-$VER; then
		./prepare
		./configure --without-lua --without-latex --with-qt=no --without-tektronix --disable-wxwidgets --without-readline --with-aquaterm 
		if ! sudo make clean install; then
			echo FAILED TO BUILD GNUPLOT.
			exit 1
		fi
		popd
	else
		echo Unable to make and install gnuplot.
		exit 1
	fi
fi

sudo install LakeArrowhead.mp4 $SHAREDIR
sudo install Brisbane64MP.jpg $SHAREDIR
sudo install busyDesktop.jpg $SHAREDIR
sudo install benchmark.pov $SHAREDIR
sudo install hike.wav $SHAREDIR
sudo install zBenchmark.xpm /usr/share/icons
sudo install zBenchmark48x48.png /usr/share/icons/hicolor/48x48/apps/

qmake6
make
sudo rm -f /usr/local/bin/zBenchmark 
sudo install zBenchmark /usr/bin
