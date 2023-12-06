#pragma once

#define PLC_SAMPLE_PERIOD 250

#include <iostream>
#include <functional>
#include <chrono>

#include <mutex>
#include <shared_mutex>
// #include <Windows.h>
#include "Service.h"
#include "Plc.h"
#include "timer.hpp"
#include "RingQueue.hpp"

enum PlcServiceCommand { ECHO = 1, START, STOP, QUERY, QUIT };

class PlcService: public Service {
private:
	Timer plc_timer;
	Plc* plc;
	shared_mutex data_mtx;
	PLC_BUFFER_TYPE* temp;
	RingQueue<PLC_BUFFER_TYPE>* data_pool;

	int selected_variable_grp_id;

public:
	PlcService();
	~PlcService();

	void InitService();
	void StopService();
	std::function<void(SOCKET)> GenerateServiceRoutine();

	void UpdatePlcData();
};


