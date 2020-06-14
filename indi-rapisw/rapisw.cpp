/*
 * RapiSW project.
 *
 * The startingpoint of this project was the INDI Developers Manual Tutorial #5, author Jasem Mutlaq.
 * 
 * This file is part of the RapiSW project.
 * Copyright (c) 2020 Othmar Ehrhardt
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.


*/

#include "rapisw.h"

#include <memory>
#include <cstring>

std::unique_ptr<RapiSW> rapiSW(new RapiSW());

void ISGetProperties(const char *dev)
{
    rapiSW->ISGetProperties(dev);
}

void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    rapiSW->ISNewSwitch(dev, name, states, names, n);
}

void ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    rapiSW->ISNewText(dev, name, texts, names, n);
}

void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    rapiSW->ISNewNumber(dev, name, values, names, n);
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
    rapiSW->ISSnoopDevice(root);
}

/**************************************************************************************
** Client is asking us to establish connection to the device
***************************************************************************************/
bool RapiSW::Connect()
{
    IDMessage(getDeviceName(), "RapiSW box connected successfully!");
    return true;
}

/**************************************************************************************
** Client is asking us to terminate connection to the device
***************************************************************************************/
bool RapiSW::Disconnect()
{
    IDMessage(getDeviceName(), "RapiSW box disconnected successfully!");
    return true;
}

/**************************************************************************************
** INDI is asking us for our default device name
***************************************************************************************/
const char *RapiSW::getDefaultName()
{
    return "RapiSW box";
}

/**************************************************************************************
** INDI is asking us to init our properties.
***************************************************************************************/
bool RapiSW::initProperties()
{
    // Must init parent properties first!
    INDI::DefaultDevice::initProperties();

    IUFillSwitch(&Outlet1S[0], "On", "", ISS_OFF);
    IUFillSwitch(&Outlet1S[1], "Off", "", ISS_OFF);
    IUFillSwitchVector(&Outlet1SP, Outlet1S, 2, getDeviceName(), "PWROUT_1", "Outlet 1", MAIN_CONTROL_TAB, IP_RW, ISR_1OFMANY, 0,
                       IPS_IDLE);
                       
    IUFillSwitch(&Outlet2S[0], "On", "", ISS_OFF);
    IUFillSwitch(&Outlet2S[1], "Off", "", ISS_OFF);
    IUFillSwitchVector(&Outlet2SP, Outlet2S, 2, getDeviceName(), "PWROUT_2", "Outlet 2", MAIN_CONTROL_TAB, IP_RW, ISR_1OFMANY, 0,
                       IPS_IDLE);
                       
    IUFillText(&Outlet1NameT[0], "Name", "", "");
    IUFillTextVector(&Outlet1NameTP, Outlet1NameT, 1, getDeviceName(), "OUTLET1_NAME", "Out1 Name", SITE_TAB, IP_RW, 0, IPS_IDLE);
    
    IUFillText(&Outlet2NameT[0], "Name", "", "");
    IUFillTextVector(&Outlet2NameTP, Outlet2NameT, 1, getDeviceName(), "OUTLET2_NAME", "Out2 Name", SITE_TAB, IP_RW, 0, IPS_IDLE);

    return true;
}

/********************************************************************************************
** INDI is asking us to update the properties because there is a change in CONNECTION status
** This fucntion is called whenever the device is connected or disconnected.
*********************************************************************************************/
bool RapiSW::updateProperties()
{
    // Call parent update properties first
    INDI::DefaultDevice::updateProperties();

    if (isConnected())
    {
        defineSwitch(&Outlet1SP);
        defineSwitch(&Outlet2SP);
        defineText(&Outlet1NameTP);
        defineText(&Outlet2NameTP);
    }
    else
    // We're disconnected
    {
        deleteProperty(Outlet1SP.name);
        deleteProperty(Outlet2SP.name);
        deleteProperty(Outlet1NameTP.name);
        deleteProperty(Outlet2NameTP.name);
    }

    return true;
}

bool RapiSW::saveConfigItems(FILE *fp)
{
    INDI::DefaultDevice::saveConfigItems(fp);

    IUSaveConfigText(fp, &Outlet1NameTP);
    IUSaveConfigText(fp, &Outlet2NameTP);

    return true;
}

/********************************************************************************************
** Client is asking us to update a switch
*********************************************************************************************/
bool RapiSW::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        if (strcmp(name, Outlet1SP.name) == 0)
        {
            strcpy(Outlet1SP.label,Outlet1NameT[0].text);
            IUUpdateSwitch(&Outlet1SP, states, names, n);

            if (Outlet1S[0].s == ISS_ON)
            {
                // Here the Rapi GPIO on code
            }
            else
            {
                // Here the Rapi GPIO off code
            }

            Outlet1SP.s = IPS_OK;
            IDSetSwitch(&Outlet1SP, nullptr);
            return true;
        }
        if (strcmp(name, Outlet2SP.name) == 0)
        {
            strcpy(Outlet2SP.label,Outlet2NameT[0].text);
            IUUpdateSwitch(&Outlet2SP, states, names, n);

            if (Outlet2S[0].s == ISS_ON)
            {
                // Here the Rapi GPIO on code
            }
            else
            {
                // Here the Rapi GPIO off code
            }

            Outlet2SP.s = IPS_OK;
            IDSetSwitch(&Outlet2SP, nullptr);
            return true;
        }
    }

    return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
}

bool RapiSW::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        if (!strcmp(name, Outlet1NameTP.name))
        {
            Outlet1NameTP.s = IPS_OK;
            IText *tp    = IUFindText(&Outlet1NameTP, names[0]);
            IUSaveText(tp, texts[0]);
            IDSetText(&Outlet1NameTP, "Outlet1 name updated");
            return true;
        }
        if (!strcmp(name, Outlet2NameTP.name))
        {
            Outlet2NameTP.s = IPS_OK;
            IText *tp    = IUFindText(&Outlet2NameTP, names[0]);
            IUSaveText(tp, texts[0]);
            IDSetText(&Outlet2NameTP, "Outlet2 name updated");
            return true;
        }
    }

    return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
}

void RapiSW::ISGetProperties(const char *dev)
{
    INDI::DefaultDevice::ISGetProperties(dev);
    defineText(&Outlet1NameTP);
    defineText(&Outlet2NameTP);
    
    loadConfig(true, "OUTLET1_NAME");
    loadConfig(true, "OUTLET2_NAME");
    strcpy(Outlet1SP.label,Outlet1NameT[0].text);
    strcpy(Outlet2SP.label,Outlet2NameT[0].text);
    
}
