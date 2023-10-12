#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "winsock2.h"
#include "Ws2tcpip.h"
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <functional>

#include "Plc.h"

#pragma comment(lib, "ws2_32.lib") // Importing library file

#define RECV_BUFFER_SIZE (65536 * 8 * 8)
#define SEND_BUFFER_SIZE (65536 * 8 * 8)
#define DATA_BUFFER_SIZE (65536 * 8 * 8)

using namespace std;

char data_buf[DATA_BUFFER_SIZE];
mutex mtx;
shared_mutex data_mtx;
SOCKET sock_conn;

Plc plc;
char recv_buf[RECV_BUFFER_SIZE];

DWORD WINAPI ServeClient(LPVOID lpParameter) {
	memset(recv_buf, 0, sizeof(recv_buf));
	while (true) {
		memset(recv_buf, 0, sizeof(recv_buf));
		recv(sock_conn, recv_buf, sizeof(recv_buf), 0);
		cout << recv_buf << endl;

		char send_buf[] = "test";
		int i_send = send(sock_conn, send_buf, sizeof(send_buf), 0);
		if (i_send == SOCKET_ERROR) {
			cout << "Failed to transmit: " << WSAGetLastError() << endl;
			// break;
			// exit(1);
			// continue;
			closesocket(sock_conn);
		}
	}
	closesocket(sock_conn);
}

int main(int argc, char* argv[]) {
	WSADATA wsa_data;
	int port = 41720;	// TODO:
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		cout << "Failed to initialize socket!" << endl;
		exit(1);
	}

	SOCKET sock_srv = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN addr_srv;
	addr_srv.sin_family = AF_INET;
	addr_srv.sin_port = htons(port);

	addr_srv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	int ret_val = ::bind(sock_srv, (LPSOCKADDR)&addr_srv, sizeof(SOCKADDR_IN));
	if (ret_val == SOCKET_ERROR) {
		cout << "Failed to connect: " << WSAGetLastError() << endl;
		exit(1);
	}

	if (listen(sock_srv, 10) == SOCKET_ERROR) {
		cout << "Failed to listen: " << WSAGetLastError() << endl;
		exit(1);
	}

	SOCKADDR_IN addr_client;
	int len = sizeof(SOCKADDR);

	while (true) {
		// Waiting for client request
		sock_conn = accept(sock_srv, (SOCKADDR*)&addr_client, &len);
		if (sock_conn == SOCKET_ERROR) {
			cout << "Failed to wait: " << WSAGetLastError() << endl;
			exit(1);
		}

		char client_ip_str[100];
		inet_ntop(AF_INET, &addr_client.sin_addr.S_un.S_addr, client_ip_str, sizeof(client_ip_str));
		cout << "Connected with client: " << client_ip_str << ":" << addr_client.sin_port << endl;
		cout << "Connected" << endl;

		// std::thread client_connect_thread = std::thread(ServeClient, sock_conn);
		// client_connect_thread.detach();

		// char send_buf[SEND_BUFFER_SIZE] = "test";
		/*
		char send_buf[] = "test";
		int i_send = send(sock_conn, send_buf, sizeof(send_buf), 0);
		if (i_send == SOCKET_ERROR) {
			cout << "Failed to transmit: " << WSAGetLastError() << endl;
			break;
		}
		*/

		HANDLE hThread = CreateThread(NULL, 0, ServeClient, NULL, 0, NULL);
		CloseHandle(hThread);

	}

	closesocket(sock_srv);
	WSACleanup();
	system("pause");

	return 0;
}