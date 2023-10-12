#pragma once
#include <WinSock2.h>
#include <functional>

class Service {
public:
	Service();
	~Service();

	virtual void InitService();
	virtual void StopService();
	virtual std::function<void(SOCKET)> GenerateServiceRoutine();
};
