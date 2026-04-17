Kromek Spectrometer Driver Readme

This kromek driver supports interfacing with a range of detectors sold by Kromek Ltd. (http://www.kromek.com)
The driver includes support for both Windows and Linux. 

Example
==========
An example of how to use the driver can be found in the 'SpectrometerExample' folder. 
This example will compile on both Windows and Linux using Qt 5+

Windows
==========
Include .\spectrometerdriver\spectrometerdriver.h header file
Link against the SpectrometerDriver.lib in the libs folder.

Linux
==========

Run cmake in the root directory to build the libraries and examples for your chosen platform or alternatively open the KromekDriver.pro file in Qt Creator and build.
Include .\spectrometerdriver\spectrometerdriver.h header file in your project
Link against the libSpectrometerDriver.so file. You will also need to link against the 'udev' library.