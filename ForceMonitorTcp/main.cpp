#include <functional>
#include "timer.hpp"

// #include "Plc.h"
#include "Service.h"
#include "DataServer.h"
#include "PlcService.h"

#ifndef GLOG_NO_ABBREVIATED_SEVERITIES
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#endif

#define RECV_BUFFER_SIZE 100000

int main(int argc, char* argv[]) {
	/*
	Plc* plc = new Plc();
	try {
		plc->connectLocalPlc();
	}
	catch (exception plc_error) {
		cerr << "Failed to connect local PLC!" << std::endl;
	}
	*/
	// PlcService service;
	// service.InitService();

	// 每 500ms 向客户端发送一次 "hello" 消息（如果客户端一秒接受一次的话，可能会接收到连续的字符串，如客户端接收一次可能是："hellohello"，由两次send的发送结果）
	std::function<void(SOCKET clientSocket)> listenCallback = [](SOCKET clientSocket) {
		char* recv_buff = new char[RECV_BUFFER_SIZE];
		while (true) {
			memset(recv_buff, 0, RECV_BUFFER_SIZE);
			int recv_len = recv(clientSocket, recv_buff, RECV_BUFFER_SIZE, 0);
			// cout << recv_buff << endl;

			if (recv_len == 0) {
				break;
			}
			const char* buff = "hello";
			int len = strlen(buff);
			send(clientSocket, buff, len, 0);

			// 当前线程休眠 500ms
			// std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		delete[] recv_buff;
		cout << "Disconnected" << endl;

		return;

		// 关闭客户端 socket
		// closesocket(clientSocket);
	};

	google::InitGoogleLogging(argv[0]);
	google::SetStderrLogging(google::GLOG_INFO);

	Service* service = new PlcService();
	DataServer* server = new DataServer(service);
	// server.ListenPort(41720, listenCallback);
	server->ListenPort(41720);

	return 0;
}