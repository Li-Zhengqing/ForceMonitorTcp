#include "Plc.h"


Plc::Plc() {
    nDeviceState = 0;

    pData = NULL;

    pAddr = &Addr;

}

Plc::~Plc() {
}

long Plc::connectPlc(unsigned char target_address[4]) {
    nPort = AdsPortOpen();
    nErr = AdsGetLocalAddress(pAddr);
    pAddr->netId = { target_address[0], target_address[1], target_address[2], target_address[3], 1, 1 };
    if (nErr) {
        cerr << "Error: AdsGetLocalAddress: " << nErr << '\n';
        LOG(WARNING) << "Error: AdsGetLocalAddress: " << nErr;
        return nErr;
    }

    // TwinCAT3 PLC1 = 851
    pAddr->port = 851;

    nErr = AdsSyncReadDeviceInfoReq(pAddr, pDevName, pVersion);
    if (nErr) {
        cerr << "Error: AdsSyncReadDeviceInfoReq: " << nErr << '\n';
        LOG(WARNING) << "Error: AdsSyncReadDeviceInfoReq: " << nErr;
        return nErr;
    }
    else
    {
        LOG(INFO) << "Name: " << pDevName << '\n';
        LOG(INFO) << "Version: " << (int)pVersion->version << '\n';
        LOG(INFO) << "Revision: " << (int)pVersion->revision << '\n';
        LOG(INFO) << "Build: " << pVersion->build << '\n';
    }
    cout.flush();

    return 0;
}

long Plc::connectLocalPlc() {
    // Open communication port on the ADS router
    nPort = AdsPortOpen();
    nErr = AdsGetLocalAddress(pAddr);
    if (nErr) {
        cerr << "Error: AdsGetLocalAddress: " << nErr << '\n';
        LOG(WARNING) << "Error: AdsGetLocalAddress: " << nErr;
        return nErr;
    }

    // TwinCAT3 PLC1 = 851
    pAddr->port = 851;

    nErr = AdsSyncReadDeviceInfoReq(pAddr, pDevName, pVersion);
    if (nErr) {
        cerr << "Error: AdsSyncReadDeviceInfoReq: " << nErr << '\n';
        LOG(WARNING) << "Error: AdsSyncReadDeviceInfoReq: " << nErr;
        return nErr;
    }
    else
    {
        LOG(INFO) << "Name: " << pDevName << '\n';
        LOG(INFO) << "Version: " << (int)pVersion->version << '\n';
        LOG(INFO) << "Revision: " << (int)pVersion->revision << '\n';
        LOG(INFO) << "Build: " << pVersion->build << '\n';
    }
    cout.flush();

    return 0;
}

long Plc::disconnectPlc() {
    // Close communication port
    nErr = AdsPortClose();
    if (nErr) {
        cerr << "Error: AdsPortClose: " << nErr << '\n';
        return nErr;
    }
    return 0;
}

long Plc::startPlc() {
    nAdsState = ADSSTATE_RUN;
    LOG(INFO) << "Current State: Runing" << endl;
    nErr = AdsSyncWriteControlReq(pAddr, nAdsState, nDeviceState, 0, pData);
    if (nErr) {
        cerr << "Error: AdsSyncWriteControlReq: " << nErr << '\n';
        return nErr;
    }

    this->plc_previous_index = -1;
    this->initPlcVarHdl();
    this->initPlcVarSelectHdl();

    // this->timer.start(200, std::bind(fetchData, this));
    return 0;
}

long Plc::stopPlc() {
    nAdsState = ADSSTATE_STOP;
    LOG(INFO) << "Current State: Stopped" << endl;
    nErr = AdsSyncWriteControlReq(pAddr, nAdsState, nDeviceState, 0, pData);
    if (nErr) {
        cerr << "Error: AdsSyncWriteControlReq: " << nErr << '\n';
        return nErr;
    }
    // this->timer.stop();
    return 0;
}

long Plc::initPlcVarHdl() {
    // char dataVar[] = { "MAIN.data" };
    char dataVar[] = { "MAIN.output" };
    nErr = AdsSyncReadWriteReq(pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(dataVar), dataVar);
    if (nErr) {
        LOG(WARNING) << "Error when initializing variable handler: " << nErr;
        // LOG(FATAL) << "Error when initializing variable handler: " << nErr;
        return nErr;
    }

    LOG(INFO) << "lHdlVar: " << lHdlVar;
    // TODO
    return 0;
}

long Plc::queryPlcData() {
    nErr = AdsSyncReadReq(pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(data), data);

    if (nErr) {
        // Comment for debug
        // LOG(WARNING) << "Error when querying PLC Data: " << nErr;
        return nErr;
    }

    // Debug
    /*
    cout << "sizeof(data) = " << sizeof(data) << endl;
    for (int i = 0; i < PLC_BUFFER_SIZE; i++) {
        cout << "data[" << i << "] = " << data[i] << endl;
    }
    */
    return 0;
}

size_t Plc::fetchDataFromPlc(PLC_BUFFER_TYPE* temp) {
    // TODO:
    clock_t start = clock();

    // LOG(INFO) << "Fetching Data From PLC" << endl;
    
    this->queryPlcData();
    unsigned int index = static_cast<unsigned int>(this->data[PLC_BUFFER_SIZE - 1]);

    unsigned int data_length = 0;
    if (this->plc_previous_index >= 0) {
        // Copying from plc data buffer to temp buffer
        // FIXME: seems only dealing single channel data
		unsigned int _data_length = 0;
		PLC_BUFFER_TYPE* _start = NULL;
		if (this->plc_previous_index <= index) {
			// TODO: data copy
			_data_length = (index - this->plc_previous_index) * 30;
			_start = this->data + (30 * this->plc_previous_index);
			memcpy(temp, _start, _data_length * sizeof(PLC_BUFFER_TYPE));
			// memcmp(_start, _temp_area, _data_length * sizeof(PLC_BUFFER_TYPE));
		}
		else {
			// TODO: data copy
			_data_length = (PLC_CHANNEL_SIZE * 3) - (this->plc_previous_index) * 30;
			_start = this->data + (30 * this->plc_previous_index);
			memcpy(temp, _start, _data_length * sizeof(PLC_BUFFER_TYPE));
			_start = this->data;
			memcpy(temp + _data_length, _start, index * 30 * sizeof(PLC_BUFFER_TYPE));
			_data_length += index * 30;
		}
        // Comment for debug
		// LOG(INFO) << "data_length = " << _data_length;
		data_length = _data_length;

        // TODO: Copyging from temp buffer to data queue
		// int _res = this->data_queue_array[i]->enqueue_memcpy(temp_array[i], data_length);
		// int _res = this->data_queue_array->enqueue_from_array(temp_array, data_length);
		// _res = this->data_queue_array[i]->dequeue_memcpy(temp_array[i], data_length);
		
		// For test only
		// _res = this->data_queue_array[i]->dequeue_to_array(temp_array[i], data_length);
		// LOG(WARNING) << "enqueue result = " << _res;
    }

    // TODO: Put forward, after operation on temp is done
    this->plc_previous_index = index;

    clock_t end = clock();

	// Comment for debug
    /*
    LOG(INFO) << "PLC previous index: " << this->plc_previous_index << endl;

    LOG(INFO) << "ADS Transfer Time Consumption: " << end - start << endl;
    */

    return data_length;
}

long Plc::initPlcVarSelectHdl() {
    char grpIdVar[] = { "MAIN.var_grp_id" };
    nErr = AdsSyncReadWriteReq(pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVarGrpId), &lHdlVarGrpId, sizeof(grpIdVar), grpIdVar);
    if (nErr) {
        LOG(WARNING) << "Error when initializing variable handler: " << nErr;
        // LOG(FATAL) << "Error when initializing variable handler: " << nErr;
        return nErr;
    }

    LOG(INFO) << "lHdlVarGrpId: " << lHdlVarGrpId;
    // TODO
    return 0;
}

long Plc::setPlcVarGrp(int selected_variable_grp_id) {
    int var_grp_id = selected_variable_grp_id;
    nErr = AdsSyncWriteReq(pAddr, ADSIGRP_SYM_VALBYHND, lHdlVarGrpId, sizeof(var_grp_id), &var_grp_id);
    if (nErr) {
        // Comment for debug
        LOG(WARNING) << "Error when setting PLC var_grp_id: " << nErr;
        return nErr;
    }
    return 0;
}

/*
void Plc::copyDataToClient(PLC_BUFFER_TYPE* dst, unsigned int* ret) {
    // TODO:
    LOG(INFO) << "Copying Data From Client" << endl;
    unsigned int _size = 0;
    // unsigned int _ret = 0;
    for (int i = 0; i < 3; i++) {
        _size = this->data_queue_array[i]->get_length();
        // _size = this->data_queue_array[i]->length();
        if (CLIENT_BUFFER_SIZE < _size) {
            _size = CLIENT_BUFFER_SIZE;
        }
        ret[i] = this->data_queue_array[i]->dequeue_to_array(dst[i], _size);
        LOG(INFO) << "_size = " << _size << "; ret[" << i << "] = " << ret[i];
    }
}

void fetchData(Plc *plc) {
    // cout << "test" << endl;
    plc->fetchDataFromPlc();
}
*/