#pragma once
#include <iostream>
#include <ws2tcpip.h> 

#pragma comment(lib, "ws2_32.lib")  
using namespace std;

#define PORT	1900
#define UPNP_MCAST_ADDR "239.255.255.250"

typedef struct
{
	char cameraid[20];
	char cameraip[20];
	char url[60];
}CameraAddr;

class UpnpTool
{
public:
	bool upnpDiscover(int delay, string &localip, SArray<CameraAddr> &iplist);
private:
	void GetHostAddress(string &strIPAddr);
	int ReceiveData(int socket, char * data, int length, int timeout, struct sockaddr* addr, socklen_t* len);
	bool UpnpParse(char *bufr, int n, char *id, char *ip, char* buf);
};