/*
   Based on the INDI Developers Manual Tutorial #3 "Simple CCD Driver"
   
   Raspistill part done by Othmar Ehrhardt.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

//#define RAPICAM_DEBUG 1

#include "indi_picameraV1.h"

#include <stdio.h>
#include <memory>

#define IMG_FILE "/dev/shm/ov5647.frame.bmp"

/* Simple global exposure finished detection 
*/

bool volatile exp_done = false;
int dbg_callCnt = 0;

/* Macro shortcut to CCD temperature value */
#define currentCCDTemperature TemperatureN[0].value

std::unique_ptr<PiCameraCCD> piCameraCCD(new PiCameraCCD());

void ISGetProperties(const char *dev)
{
    piCameraCCD->ISGetProperties(dev);
}

void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    piCameraCCD->ISNewSwitch(dev, name, states, names, n);
}

void ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    piCameraCCD->ISNewText(dev, name, texts, names, n);
}

void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    piCameraCCD->ISNewNumber(dev, name, values, names, n);
}

void ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[],
               char *names[], int n)
{
    INDI_UNUSED(dev);
    INDI_UNUSED(name);
    INDI_UNUSED(sizes);
    INDI_UNUSED(blobsizes);
    INDI_UNUSED(blobs);
    INDI_UNUSED(formats);
    INDI_UNUSED(names);
    INDI_UNUSED(n);
}

void ISSnoopDevice(XMLEle *root)
{
    piCameraCCD->ISSnoopDevice(root);
}

/**************************************************************************************
** Client is asking us to establish connection to the device
***************************************************************************************/
bool PiCameraCCD::Connect()
{
#ifdef RAPICAM_DEBUG
	// It seems LOGF_INFO needs always a argument... stupid.
    LOGF_INFO("Call to Connect %d", 1); 
#endif
    IDMessage(getDeviceName(), "PiCamera V1 connected successfully!");

    // Let's set a timer that checks CCDs status every POLLMS milliseconds.
    SetTimer(POLLMS);
    return true;
}

/**************************************************************************************
** Client is asking us to terminate connection to the device
***************************************************************************************/
bool PiCameraCCD::Disconnect()
{
#ifdef RAPICAM_DEBUG
    LOGF_INFO("Call to Disconnect %d", 1); 
#endif
    IDMessage(getDeviceName(), "PiCamera V1 disconnected successfully!");
    return true;
}

/**************************************************************************************
** INDI is asking us for our default device name
***************************************************************************************/
const char *PiCameraCCD::getDefaultName()
{
#ifdef RAPICAM_DEBUG
    LOGF_INFO("Call to getDefaultName %d", 1); 
#endif
    return "PiCamera V1";
}

/**************************************************************************************
** INDI is asking us to init our properties.
***************************************************************************************/
bool PiCameraCCD::initProperties()
{
#ifdef RAPICAM_DEBUG
    LOGF_INFO("Call to initProperties %d", 1); 
#endif
    // Must init parent properties first!
    INDI::CCD::initProperties();

    // We set the CCD capabilities - NOTHING!
    uint32_t cap = 0; //CCD_CAN_ABORT;
    SetCCDCapability(cap);

    // Add Debug, Simulator, and Configuration controls
    addAuxControls();

    setDefaultPollingPeriod(500);

    return true;
}

/********************************************************************************************
** INDI is asking us to update the properties because there is a change in CONNECTION status
** This fucntion is called whenever the device is connected or disconnected.
*********************************************************************************************/
bool PiCameraCCD::updateProperties()
{
#ifdef RAPICAM_DEBUG
    LOGF_INFO("Call to updateProperties %d", 1); 
#endif
    // Call parent update properties first
    INDI::CCD::updateProperties();

    if (isConnected())
    {
        // Let's get parameters now from CCD
        setupParams();

        // Start the timer
        SetTimer(POLLMS);
    }

    return true;
}

/**************************************************************************************
** Setting up CCD parameters
***************************************************************************************/
void PiCameraCCD::setupParams()
{
#ifdef RAPICAM_DEBUG
    LOGF_INFO("Call to setupParams %d", 1); 
#endif
    int indi_depth = 16;
 
    // Set parameter to match the OV5647 chip in mode 3, where the long exposure is possible:
    SetCCDParams(2592, 1944, indi_depth, 1.44, 1.44);
    // Let's calculate how much memory we need for the primary CCD buffer
    int nbuf;
    nbuf = PrimaryCCD.getXRes() * PrimaryCCD.getYRes() * PrimaryCCD.getBPP() / 8;
    nbuf += 512; //  leave a little extra at the end
    PrimaryCCD.setFrameBufferSize(nbuf);
}

/**************************************************************************************
** Client is asking us to start an exposure
***************************************************************************************/
void raspistill(const std::string cmd)
{
    exp_done = true; // Hae? Wrong meaning of true and false! Could be fixed... but works!
    int ret = system(cmd.c_str());
    exp_done = false;
}

bool PiCameraCCD::StartExposure(float duration)
{
#ifdef RAPICAM_DEBUG
    LOGF_INFO("Call to StartExposure(%f)", duration); 
#endif
    char cmd[512];
    // duration is given in seconds
    ExposureRequest = duration + 6.0; // The raspistill command need aprox. 6 seconds to download the image from the camera.

    int raspistillExpTime = (duration * 1000000) - 116; // raspistill need the time in µs. Max. exp. is 5999884.
    // Since we have only one CCD with one chip, we set the exposure duration of the primary CCD
    PrimaryCCD.setExposureDuration(duration);
    gettimeofday(&ExpStart, nullptr);

    InExposure = true;
    // Start the raspistill command, store the image as bmp (without compression), in RAM.
    sprintf(cmd,"/usr/bin/raspistill -md 3 -e bmp -ss %d -th none -n -o %s", raspistillExpTime, IMG_FILE);
    std::string strCmd(cmd);
    LOGF_INFO("Would execute: %s", strCmd.c_str());

    exp_done = false;
    if(m_exec.joinable())
		m_exec.detach();
    m_exec = std::thread(raspistill,strCmd);
    // We're done
    dbg_callCnt++;
    return true;
}

/**************************************************************************************
** Client is asking us to abort an exposure
***************************************************************************************/
bool PiCameraCCD::AbortExposure()
{
#ifdef RAPICAM_DEBUG
    LOGF_INFO("Call to AbortExposure %d", 1); 
#endif
    InExposure = false;
    return true;
}

/**************************************************************************************
** Client is asking us to set a new temperature
***************************************************************************************/
int PiCameraCCD::SetTemperature(double temperature)
{
#ifdef RAPICAM_DEBUG
    LOGF_INFO("Call to SetTemperature (%lf)", temperature);
#endif
    TemperatureRequest = temperature;

    // 0 means it will take a while to change the temperature
    return 0;
}

/**************************************************************************************
** How much longer until exposure is done?
***************************************************************************************/
float PiCameraCCD::CalcTimeLeft()
{
    double timesince;
    double timeleft;
    struct timeval now { 0, 0 };
    gettimeofday(&now, nullptr);

    timesince = (double)(now.tv_sec * 1000.0 + now.tv_usec / 1000) -
                (double)(ExpStart.tv_sec * 1000.0 + ExpStart.tv_usec / 1000);
    timesince = timesince / 1000;

    timeleft = ExposureRequest - timesince;
    return timeleft;
}

/**************************************************************************************
** Main device loop. We check for exposure and temperature progress here
***************************************************************************************/
void PiCameraCCD::TimerHit()
{
    long timeleft;

    if (!isConnected())
        return; //  No need to reset timer if we are not connected anymore

    if (InExposure)
    {
        timeleft = CalcTimeLeft();

        // Less than a 0.1 second away from exposure completion
        // This is an over simplified timing method, check CCDSimulator and simpleCCD for better timing checks
#ifdef RAPICAM_DEBUG
        LOGF_INFO("running: %d", exp_done); 
#endif
        if (timeleft < 0.1 && !exp_done)
        {
            /* We're done exposing */
            IDMessage(getDeviceName(), "Exposure done, downloading image...");

            // Set exposure left to zero
            PrimaryCCD.setExposureLeft(0);

            // We're no longer exposing...
            InExposure = false;

            /* grab and save image */
            grabImage();
        }
        else
            // Just update time left in client
            PrimaryCCD.setExposureLeft(timeleft);
    }

    // TemperatureNP is defined in INDI::CCD
    switch (TemperatureNP.s)
    {
        case IPS_IDLE:
        case IPS_OK:
            break;

        case IPS_BUSY:
            /* If target temperature is higher, then increase current CCD temperature */
            if (currentCCDTemperature < TemperatureRequest)
                currentCCDTemperature++;
            /* If target temperature is lower, then decrese current CCD temperature */
            else if (currentCCDTemperature > TemperatureRequest)
                currentCCDTemperature--;
            /* If they're equal, stop updating */
            else
            {
                TemperatureNP.s = IPS_OK;
                IDSetNumber(&TemperatureNP, "Target temperature reached.");

                break;
            }

            IDSetNumber(&TemperatureNP, nullptr);

            break;

        case IPS_ALERT:
            break;
    }

    SetTimer(POLLMS);
}

/**************************************************************************************
** Create a random image and return it to client
***************************************************************************************/
void PiCameraCCD::grabImage()
{
#ifdef RAPICAM_DEBUG
    LOGF_INFO("Call to grabImage %d", 1); 
#endif
    // Let's get a pointer to the frame buffer
    uint8_t *image = PrimaryCCD.getFrameBuffer();

    // Get width and height
#ifdef RAPICAM_DEBUG
    int width  = PrimaryCCD.getSubW() / PrimaryCCD.getBinX() * PrimaryCCD.getBPP() / 8;
    int height = PrimaryCCD.getSubH() / PrimaryCCD.getBinY();
    LOGF_INFO("Try grab image of format: %d x %d", width, height); 
#endif
    FILE* f = fopen(IMG_FILE, "rb");
    unsigned char info[54];

    // read the 54-byte header
    size_t sizerd = fread(info, sizeof(unsigned char), 54, f); 

    // extract image height and width from header
    int bmpwidth = *(int*)&info[18];
    int bmpheight = *(int*)&info[22];
#ifdef RAPICAM_DEBUG
    LOGF_INFO("Found image data in header: %d x %d", bmpwidth, bmpheight); 
#endif
    // allocate 3 bytes per pixel
    unsigned long size = 3 * bmpwidth * bmpheight;
    unsigned char* data = new unsigned char[size];

    // read the rest of the data at once
    sizerd = fread(data, sizeof(unsigned char), size, f); 
    fclose(f);
    
    unsigned long resPtr = 0;
    unsigned short pixel = 0;
    unsigned char *pixelPtr = (unsigned char*)&pixel;
    unsigned long idx = 0;
    for(idx = 0; idx < size; idx += 3)
    {
	unsigned char b = data[idx];
	unsigned char g = data[idx+1];
	unsigned char r = data[idx+2];
	// Put max. value of RGB to the gray scale image:
	pixel = ((g*r*b)/255);
        image[resPtr++] = pixelPtr[0];
        image[resPtr++] = pixelPtr[1];
    }
    
    IDMessage(getDeviceName(), "Download complete.");

    // Let INDI::CCD know we're done filling the image buffer
    ExposureComplete(&PrimaryCCD);
    delete data;
}
