#define RECV_BUFFER_SIZE (8 * 10000000)
#include "PlcService.h"

// FIXME: Overflow may occur when performing memcpy, where dst is less than src.

PlcService::PlcService() {
	this->plc = new Plc();
	this->temp = new PLC_BUFFER_TYPE[PLC_BUFFER_SIZE];
	this->data_pool = new RingQueue<PLC_BUFFER_TYPE>(DATA_QUEUE_SIZE);
}

PlcService::~PlcService() {
	delete this->plc;
	delete[] this->temp;
	delete this->data_pool;
}

void PlcService::InitService() {
	cout << "Initializing PLC service" << std::endl;
	try {
		this->plc->connectLocalPlc();
		this->plc->stopPlc();
		this->plc->startPlc();
	}
	catch (exception e) {
		cerr << "Failed to connect Local PLC!" << std::endl;
	}
	this->plc_timer.start(PLC_SAMPLE_PERIOD, [this]() { this->UpdatePlcData(); });
}

void PlcService::StopService() {
	try {
		this->plc->stopPlc();
		this->plc->disconnectPlc();
	}
	catch (exception e) {
		cerr << "Failed to disconnect with local PLC!" << std::endl;
	}
}

std::function<void(SOCKET clientSocket)> PlcService::GenerateServiceRoutine() {
	std::function<void(SOCKET clientSocket)> listenCallback = [this](SOCKET clientSocket) {
		char* recv_buff = new char[RECV_BUFFER_SIZE];
		size_t previous_data_index = 0;
		char* buff = new char[RECV_BUFFER_SIZE];
		int len = strlen(buff);

		int exit_flag = 0;

		// FIXME: Clear data_pool buffer, avoid blocking client process.
		this->data_mtx.lock_shared();
		int _init_data_length = this->data_pool->length();
		this->data_pool->dequeue_memcpy(NULL, _init_data_length);
		this->data_mtx.unlock_shared();

		// auto last_connection_time = chrono::system_clock::now();
		while (!exit_flag) {
			memset(recv_buff, 0, RECV_BUFFER_SIZE);
			int recv_len = recv(clientSocket, recv_buff, RECV_BUFFER_SIZE, 0);
			size_t data_length = 0;
			// cout << "Client recv length: " << recv_len << endl;

			// Below timeout reset function is not used.
			// Reset connection if timeout
			/*
			auto current_time = chrono::system_clock::now();
			auto duration_time = chrono::duration_cast<chrono::milliseconds>(current_time - last_connection_time);
			auto duration = (double)(duration_time.count()) * chrono::milliseconds::period::num / chrono::milliseconds::period::den;
			cout << "Duration" << duration << std::endl;
			if (duration > 30.0) {
				exit_flag = true;
				LOG(INFO) << "Connection timeout." << std::endl;
			}
			*/

			// FIXME:
			// this->UpdatePlcData();

			if (recv_len == -1) {
				// recived nothing.
				// break;
				// continue;
				// return;
			}

			if (recv_len > 0) {
				// Reset connection timer
				// auto last_connection_time = chrono::system_clock::now();

				PlcServiceCommand command = PlcServiceCommand(recv_buff[0]);
				// cout << command << std::endl;
				memset(buff, 0, RECV_BUFFER_SIZE);
				switch (command) {
				case PlcServiceCommand::ECHO:
					// cout << "ECHO" << std::endl;
					buff[0] = PlcServiceCommand::ECHO;
					send(clientSocket, buff, 1, 0);
					break;
				case PlcServiceCommand::START:
					// TODO: Start PLC
					this->selected_variable_grp_id = recv_buff[1];
					std::cout << "Monitoring Variable Group: " << selected_variable_grp_id << std::endl;
					this->plc->setPlcVarGrp(static_cast<int>(this->selected_variable_grp_id));

					// FIXME: Clear data_pool buffer, avoid blocking client process.
					this->data_mtx.lock_shared();
					_init_data_length = this->data_pool->length();
					this->data_pool->dequeue_memcpy(NULL, _init_data_length);
					this->data_mtx.unlock_shared();

					// cout << "START" << std::endl;
					buff[0] = PlcServiceCommand::START;
					send(clientSocket, buff, 1, 0);
					break;
				case PlcServiceCommand::STOP:
					// TODO: Stop PLC
					// cout << "STOP" << std::endl;
					buff[0] = PlcServiceCommand::STOP;
					send(clientSocket, buff, 1, 0);
					break;
				case PlcServiceCommand::QUERY:
					// TODO: Query PLC data
					// cout << "QUERY" << std::endl;

					this->data_mtx.lock_shared();
					data_length = this->data_pool->length();
					if (data_length == 0) {
						buff[0] = 0;
						send(clientSocket, buff, 1, 0);
					}
					else {
						this->data_pool->dequeue_memcpy((PLC_BUFFER_TYPE*)buff, data_length);
						// send(clientSocket, (char*)(this->data_pool->get_data_ptr(0)), data_length * sizeof(PLC_BUFFER_TYPE), 0);
						send(clientSocket, (char*)(buff), data_length * sizeof(PLC_BUFFER_TYPE), 0);
						// cout << "data length:" << data_length << std::endl;
					}
					this->data_mtx.unlock_shared();
					break;
				case PlcServiceCommand::QUIT:
					// cout << "QUIT" << std::endl;
					buff[0] = PlcServiceCommand::QUIT;
					send(clientSocket, buff, 1, 0);
					// return;
					exit_flag = 1;
					break;
				default:
					// cout << "NULL COMMAND" << std::endl;
					LOG(WARNING) << "NULL COMMAND" << std::endl;
					buff[0] = 0;
					send(clientSocket, buff, 1, 0);
					break;
				}

			}
			// 当前线程休眠 500ms
			// std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		delete[] recv_buff;
		delete[] buff;
		// cout << "Disconnected" << endl;
		LOG(INFO) << "Disconnected" << endl;

		// 关闭客户端 socket
		closesocket(clientSocket);
		return;
	};

	return listenCallback;
}

void PlcService::UpdatePlcData() {
	size_t data_length = this->plc->fetchDataFromPlc(this->temp);
	// cout << "Update Data: " << data_length << std::endl;
	// LOG(INFO) << "Plc data fetched" << std::endl;
	this->data_mtx.lock();
	/* TODO: */
	// this->data_pool->dequeue_memcpy(NULL, data_length);
	long enqueue_size = this->data_pool->enqueue_memcpy(this->temp, data_length);
	// LOG(INFO) << "Plc data updated." << std::endl;
	// cout << this->data_pool->length() << ":" << enqueue_size << std::endl;
	this->data_mtx.unlock();
	// LOG(INFO) << "Plc data update end." << std::endl;
	return;
}


