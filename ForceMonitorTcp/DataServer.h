#pragma once
#include <iostream>
#include <cstdio>
#include <cstdlib>

#include "winsock2.h"
#include "Ws2tcpip.h"

#include <mutex>
#include <shared_mutex>
#include "Plc.h"

#include "Service.h"

#pragma comment(lib, "ws2_32.lib") // Importing library file

class DataServer {
private:
	std::mutex mtx;
	std::shared_mutex data_mtx;

	Service* service;

	Plc* plc;
public:
	DataServer();
	DataServer(Service* service);
	~DataServer();

	int Connect();
	int Disconnect();
	void Serve();

	// void ListenPort(unsigned short port, std::function<void(SOCKET clientSocket)> listenCallback);
	void ListenPort(unsigned short port);
};
