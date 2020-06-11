This driver was inspired by the work of https://github.com/jdhill-repo/indi-picamera but it is based on the INDI development example #3 now.

This driver use the program "raspistill" to take up to 6 seconds exposured images. The driver calls raspistill, raspistill store it's image on the shared memory /dev/shm. From this location the driver read the data and convert them from 24bit RGB to 16bit gray scaled image. This could take more than 10 seconds plus the exposure time. This driver is not fast. But it was intended to be used for the polar alignment process at the beginning of a stargazing session.

# Build this driver

This driver does not have any additional requirements beside the INDI requriements. The raspistill programm is part of a standard raspberry pi debian installation.
For build instructions please visit the README.md from the indi-3rdparty project (step one level up in the file tree of this project).

