#include "DataServer.h"

DataServer::DataServer() {
	// this->plc = new Plc();
	this->service->InitService();
}

DataServer::DataServer(Service* service) {
	this->service = service;
	this->service->InitService();
}

DataServer::~DataServer() {
	// this->Disconnect();
	this->service->StopService();
}

int DataServer::Connect() {
	// TODO: Connect with local PLC
	this->plc->connectLocalPlc();
	return 0;
}

int DataServer::Disconnect() {
	// TODO: Disconnect with local PLC
	this->plc->disconnectPlc();
	return 0;
}

void DataServer::Serve() {

}

// void DataServer::ListenPort(unsigned short port, std::function<void(SOCKET clientSocket)> listenCallback) {
void DataServer::ListenPort(unsigned short port) {
	// ��ʼ�� WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0) {
		std::cout << "WSAStartup error!" << std::endl;
		return;
	}

	// �����׽���
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) {
		std::cout << "create socket error!" << std::endl;
		return;
	}

	// ��IP�Ͷ˿�
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (::bind(serverSocket, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR) {
		std::cout << "bind error" << std::endl;
		return;
	}

	// ����
	if (listen(serverSocket, 5) == SOCKET_ERROR) {
		std::cout << "listen error" << std::endl;
		return;
	}

	// �ȴ��ͻ�������
	SOCKET clientSocket;
	sockaddr_in remoteAddr;
	int remoteAddrlen = sizeof(remoteAddr);

	while (true) {
		// һ���µĿͻ�������
		clientSocket = accept(serverSocket, (SOCKADDR*)&remoteAddr, &remoteAddrlen);
		char client_ip_str[100];
		inet_ntop(AF_INET, &remoteAddr.sin_addr.S_un.S_addr, client_ip_str, sizeof(client_ip_str));
		// cout << "Connected with client: " << client_ip_str << ":" << remoteAddr.sin_port << endl;
		LOG(INFO) << "Connected with client: " << client_ip_str << ":" << remoteAddr.sin_port << endl;


		// ����һ���µ��߳�ȥ������ͻ��˷�����Ϣ
		std::thread clientConnectThread = std::thread(this->service->GenerateServiceRoutine(), clientSocket);
		clientConnectThread.detach();
	}
}

