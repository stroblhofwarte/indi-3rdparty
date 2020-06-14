This driver is for the small Raspberry Pi project providing a regid setup for a Astroberry environment with three switchable power outlets, one of them could be controled via PWM for a dew cap or something else. Furthermore this project provides the RS232 interface of the Raspberry Pi useable to control a mount with serial port withou having this chunky USB to RS232 converter hanging around.
For more information please see the page: https://astro.stroblhof-oberrohrbach.de


# Build this driver

This driver have additional requirements beside the INDI requriements. This driver requires the WiringPi library. This library is normaly installed on the astroberry image.
For build instructions please visit the README.md from the indi-3rdparty project (step one level up in the file tree of this project).

Change to your build directory (_develop as example):
```
cd _develop
```
Clone the indi-3rdparty fork:
```
git clone https://github.com/stroblhofwarte/indi-3rdparty.git
```
Create INDI build directory and change to it:
```
mkdir -p build/indi-raspisw
cd build/indi-raspisw
```
Create the makefiles:
```
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ~/_develop/indi-3rdparty/indi-raspisw
```
Build the driver:
```
make -j4
```
Install the driver:
```
sudo make install
```






