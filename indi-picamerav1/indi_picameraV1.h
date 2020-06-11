/*
   INDI Developers Manual
   Tutorial #3

   "Simple CCD Driver"

   We develop a simple CCD driver.

   Refer to README, which contains instruction on how to build this driver, and use it
   with an INDI-compatible client.

*/

#pragma once

#include "indiccd.h"
#include <thread>

class PiCameraCCD : public INDI::CCD
{
  public:
    PiCameraCCD() = default;

  protected:
    // General device functions
    bool Connect();
    bool Disconnect();
    const char *getDefaultName();
    bool initProperties();
    bool updateProperties();

    // CCD specific functions
    bool StartExposure(float duration);
    bool AbortExposure();
    int SetTemperature(double temperature);
    void TimerHit();

  private:
    // Utility functions
    float CalcTimeLeft();
    void setupParams();
    void grabImage();

    // Are we exposing?
    bool InExposure { false };
    // Struct to keep timing
    struct timeval ExpStart { 0, 0 };

    float ExposureRequest { 0 };
    float TemperatureRequest { 0 };
    
    std::thread m_exec;
};
