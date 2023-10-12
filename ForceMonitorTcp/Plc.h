#pragma once

#include <iostream>
#include <windows.h>
#include <conio.h>
#include <ctime>
// #include <mutex>

// ADS headers for TwinCAT 3
#include "C:\TwinCAT\AdsApi\TcAdsDll\Include\TcAdsDef.h"
#include "C:\TwinCAT\AdsApi\TcAdsDll\Include\TcAdsAPI.h"

#ifndef GLOG_NO_ABBREVIATED_SEVERITIES
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#endif

#include "timer.hpp"
#include "RingQueue.hpp"

// #define PLC_BUFFER_SIZE 1000
#define MAXINDEX 300
// #define PLC_BUFFER_SIZE 9001
#define PLC_CHANNEL_SIZE ( MAXINDEX * 10 )
#define PLC_BUFFER_SIZE ( PLC_CHANNEL_SIZE * 3 + 1 )
#define PLC_BUFFER_TYPE double
// #define DATA_QUEUE_SIZE 1000000
#define DATA_QUEUE_SIZE 1000000
#define CLIENT_BUFFER_SIZE 50000

#define SAMPLE_RATE 10000

using namespace std;

class Plc {
private:
    USHORT nAdsState;
    USHORT nDeviceState;
    long    nErr, nPort;
    // int     ch;
    void* pData;
    AmsAddr Addr;
    PAmsAddr pAddr;

    char pDevName[50];
    AdsVersion Version;
    AdsVersion* pVersion = &Version;

    unsigned long lHdlVar;

    PLC_BUFFER_TYPE data[PLC_BUFFER_SIZE];
    PLC_BUFFER_TYPE temp[PLC_BUFFER_SIZE];
    PLC_BUFFER_TYPE data_queue[DATA_QUEUE_SIZE];

    int plc_previous_index;
    // TODO: Data Queue
    // int data_queue_front;
    Timer timer;


public:
    Plc();
    ~Plc();

    long connectPlc(unsigned char target_address[4]);

    long connectLocalPlc();

    long disconnectPlc();

    // long getPlcInfo();

    long startPlc();

    long stopPlc();

    long initPlcVarHdl();

    long queryPlcData();

    size_t fetchDataFromPlc(PLC_BUFFER_TYPE* dst);

    // void copyDataToClient(PLC_BUFFER_TYPE* dst, unsigned int* ret);
};
