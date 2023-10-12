#include "Service.h"

Service::Service() { }

Service::~Service() { }

void Service::InitService() { }

void Service::StopService() { }

std::function<void(SOCKET)> Service::GenerateServiceRoutine() {
	return [](SOCKET) { };
}