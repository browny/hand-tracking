####1. Prepare the OpenCV environment####

#####Linux#####

[Installing Opencv2.2 in Ubuntu 11.04](http://www.samontab.com/web/2011/06/installing-opencv-2-2-in-ubuntu-11-04/)

[Installing OpenCV 2.4.1 in Ubuntu 12.04 LTS](http://www.samontab.com/web/2012/06/installing-opencv-2-4-1-ubuntu-12-04-lts/)

#####Mac#####

[Installing the OpenCV library on Mac OS X Lion using Homebrew](http://craiccomputing.blogspot.tw/2012/08/installing-opencv-library-on-mac-os-x.html)

####2. Prepare the CMake build environment####

	$ sudo apt-get install cmake

####3. Clone the project to local####

	$ git clone git@github.com:browny/hand-tracking.git
	$ cd hand-tracking

####4. Take out-of-source CMake build####

	$ mkdir release
	$ cd release
	$ cmake ..
	$ make

####5. Enjoy####

	$ cd bin
	$ ./main

####6. Exit####

	Press 'Esc' to close window and quit

####7. Demo####

