#include "stdafx.h"
#include "WinSocketClient.h"

long long WinSocketClient::Convert(char* data)
{
	long long dVal;
	unsigned char *ci, *co;
	ci = (unsigned char *)data;
	co = (unsigned char *)&dVal;
	co[0] = ci[7];
	co[1] = ci[6];
	co[2] = ci[5];
	co[3] = ci[4];
	co[4] = ci[3];
	co[5] = ci[2];
	co[6] = ci[1];
	co[7] = ci[0];

	return dVal;
}

int WinSocketClient::ll2str(char *s, long long value) 
{
	char *p, aux;
	unsigned long long v;
	size_t l;
	/* Generate the string representation, this method produces
	* an reversed string. */
	v = (value < 0) ? -value : value;
	p = s;
	do {
		*p++ = '0' + (v % 10);
		v /= 10;
	} while (v);
	if (value < 0) *p++ = '-';
	/* Compute length and add null term. */
	l = p - s;
	*p = '\0';
	/* Reverse the string. */
	p--;
	while (s < p) {
		aux = *s;
		*s = *p;
		*p = aux;
		s++;
		p--;
	}
	return l;
}
/* Identical ll2str(), but for unsigned long long type. */
int WinSocketClient::ull2str(char *s, unsigned long long v) 
{
	char *p, aux;
	size_t l;
	/* Generate the string representation, this method produces
	* an reversed string. */
	p = s;
	do {
		*p++ = '0' + (v % 10);
		v /= 10;
	} while (v);
	/* Compute length and add null term. */
	l = p - s;
	*p = '\0';
	/* Reverse the string. */
	p--;
	while (s < p) {
		aux = *s;
		*s = *p;
		*p = aux;
		s++;
		p--;
	}
	return l;
}

int WinSocketClient::cmsSend(SOCKET sockfd, char *buff, int len, int mode)
{
	int sendLen;
	int cmsLen = 0;

	while (cmsLen < len)
	{
		sendLen = send(sockfd, buff + cmsLen, len - cmsLen, mode);
		if (sendLen > 0)
			cmsLen += sendLen;
		else if (sendLen < 0)
			break;
		else
			Sleep(1);
	}
	return cmsLen;
}

int WinSocketClient::cmsRecv(SOCKET sockfd, char *buff, int len, int mode)
{
	int recLen;
	int cmsLen = 0;

	while (cmsLen < len)
	{
		recLen = recv(sockfd, buff + cmsLen, len - cmsLen, mode);

		if (recLen > 0)
			cmsLen += recLen;
		else if (recLen < 0)
			break;
		else
			Sleep(1);
	}
	return cmsLen;
}

bool WinSocketClient::SendBuffToHost(const char *ip, char *sendbuff, int sendlen, char *outbuff, bool save)
{
	//WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 0);
	bool ret = false;
	int n = 0;
	short recvlen = 0;
	short int loginLen = 40;
	char cmdbuf[4] = { 0 };
	char cmdlogin[44] = { 0 };
	char databuf[256] = { 0 };

	//if (WSAStartup(wVersionRequested, &wsaData) == SOCKET_ERROR)
	//{
	//	return ret;
	//}

	//建立通讯socket
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		//WSACleanup();
		return ret;
	}

	//设置登录buff
	cmdlogin[0] = 69;
	cmdlogin[1] = 6;
	memcpy(cmdlogin + 2, &loginLen, 2);
	memcpy(cmdlogin + 4, CAMERA_USERNAME, 8);
	memcpy(cmdlogin + 24, CAMERA_USERPWD, 8);

	SOCKADDR_IN addrSrv;
	memset(&addrSrv, 0, sizeof(addrSrv));
	addrSrv.sin_addr.s_addr = inet_addr(ip);//设定需要连接的服务器的ip地址
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(3357);//设定需要连接的服务器的端口地址

	int tv_out = 3000;
	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv_out, sizeof(tv_out));
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv_out, sizeof(tv_out));

	n = connect(sockfd, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//与服务器进行连接
	int error = GetLastError();
	if (n < 0)
		goto end;
	//发送给服务器
	n = cmsSend(sockfd, cmdlogin, sizeof(cmdlogin), 0);
	if (n < sizeof(cmdlogin))
		goto end;

	//接受服务器消息
	n = cmsRecv(sockfd, cmdbuf, sizeof(cmdbuf), 0);
	//SLOGFMTE("来自摄像机的消息: %d: %d %d %d\n", cmdbuf[0], cmdbuf[1], cmdbuf[2], cmdbuf[3]);
	if (n < sizeof(cmdbuf) || (cmdbuf[2] == -1 && cmdbuf[3] > 0))
		goto end;
	
	recvlen = ntohs(*((unsigned short *)(cmdbuf + 2))); //cmdbuf[2] << 8 | (UINT8)cmdbuf[3];
	n = cmsRecv(sockfd, databuf, recvlen, 0);
	if (n < recvlen)
		goto end;

	
	memset(cmdbuf, 0, sizeof(cmdbuf));
	n = cmsSend(sockfd, sendbuff, sendlen, 0);
	if (n < sendlen)
		goto end;

	n = cmsRecv(sockfd, cmdbuf, sizeof(cmdbuf), 0);
	SLOGFMTE("来自摄像机的消息: %d: %d %d %d\n", cmdbuf[0], cmdbuf[1], cmdbuf[2], cmdbuf[3]);
	if (n < sizeof(cmdbuf) || (cmdbuf[2] == -1 && cmdbuf[3] > 0))
		goto end;

	recvlen = ntohs(*((unsigned short *)(cmdbuf + 2)));//cmdbuf[2] << 8 | (UINT8)cmdbuf[3];

	if (recvlen < 0)
	{
		ret = true;
		goto end;
	}

	n = cmsRecv(sockfd, outbuff, recvlen, 0);
	if (n < recvlen || (cmdbuf[2] == -1 && cmdbuf[3] > 0))
		goto end;

	//是否存菜单
	if (save)
	{
		char cmdsetIP[4] = { 0 };
		short int iplen = 0;

		//设置IP
		cmdsetIP[0] = 65;
		cmdsetIP[1] = 3;
		memcpy(cmdsetIP + 2, &iplen, 2);

		n = cmsSend(sockfd, cmdsetIP, sizeof(cmdsetIP), 0);
		if (n < sizeof(cmdsetIP))
			goto end;

	}
	ret = true;

end:
	closesocket(sockfd);
	//WSACleanup();
	return ret;

}

bool WinSocketClient::GetDeviceID(const char *ip, CameraAddr &cad)
{
	bool ret;
	char cmdgetid[4] = { 0 };
	short int getidLen = 0;
	char databuf[256] = { 0 };
	char mac[8] = { 0 };
	//设置获取id　buff
	cmdgetid[0] = 74;
	cmdgetid[1] = 3;
	memcpy(cmdgetid + 2, &getidLen, 2);

	ret = SendBuffToHost(ip, cmdgetid, sizeof(cmdgetid), databuf, false);
	if (ret)
	{
		char devidstr[16] = { 0 };
		memcpy(mac + 2, databuf + 68, 6);
		long long devid = Convert(mac);
		ll2str(devidstr, devid);
		memcpy(cad.cameraip, ip, strlen(ip));
		memcpy(cad.cameraid, devidstr, sizeof(devidstr));
		sprintf(cad.url, "rtmp://%s:1935/live/%s", cad.cameraip, cad.cameraid);
		SLOGFMTE(" DeviceID: %lld\n", devid);
	}
	return ret;
}

bool WinSocketClient::SetCameraTime(const char *ip, DateTime &dt, bool save)
{
	bool ret;
	char cmdsettime[12] = { 0 };
	short int timelen = 8;
	char databuf[256] = { 0 };

	//设置时间buff
	cmdsettime[0] = 65;
	cmdsettime[1] = 5;
	int date = htonl(dt.date);
	int time = htonl(dt.time);
	memcpy(cmdsettime + 2, &timelen, 2);
	memcpy(cmdsettime + 4, &date, 4);
	memcpy(cmdsettime + 8, &time, 4);

	ret = SendBuffToHost(ip, cmdsettime, sizeof(cmdsettime), databuf, save);

	return ret;

}

bool WinSocketClient::SetHostIPAddr(const char *ip, IPPort &ipp, bool save)
{
	bool ret;
	char cmdsetIP[244] = { 0 };
	short int iplen = 240;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetIP[0] = 75;
	cmdsetIP[1] = 91;
	UINT16 port = htons(ipp.port);
	UINT16 cp_port = htons(ipp.cp_port);
	memcpy(cmdsetIP + 2, &iplen, 2);
	memcpy(cmdsetIP + 4, &port, 2);
	*(cmdsetIP + 6) = ipp.lib_type;
	memcpy(cmdsetIP + 7, ipp.domain, DOMAIN_LEN);
	memcpy(cmdsetIP + 68, &cp_port, 2);
	memcpy(cmdsetIP + 70, ipp.cp_domain, CPDOMAIN_LEN);
	memcpy(cmdsetIP + 134, ipp.sw_id, LIBNAME_LEN);

	ret = SendBuffToHost(ip, cmdsetIP, sizeof(cmdsetIP), databuf, save);

	return ret;
}

bool WinSocketClient::GetHostIPAddr(const char *ip, IPPort &ipp)
{
	bool ret;
	char cmdsetIP[4] = { 0 };
	short int iplen = 0;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetIP[0] = 74;
	cmdsetIP[1] = 91;
	memcpy(cmdsetIP + 2, &iplen, 2);

	ret = SendBuffToHost(ip, cmdsetIP, sizeof(cmdsetIP), databuf, false);
	ipp.port = ntohs(*((unsigned short *)databuf));
	ipp.lib_type = databuf[2];
	memcpy(ipp.domain, databuf + 3, DOMAIN_LEN);
	ipp.cp_port = ntohs(*((unsigned short *)(databuf + DOMAIN_LEN+3)));
	memcpy(ipp.cp_domain, databuf + 66, CPDOMAIN_LEN);
	memcpy(ipp.sw_id, databuf + 130, LIBNAME_LEN);
	return ret;
}

bool WinSocketClient::SetFaceParameter(const char *ip, FaceParameter &fpar, bool save)
{
	bool ret;
	char cmdsetface[86] = { 0 };
	short int iplen = 82;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetface[0] = 75;
	cmdsetface[1] = 90;
	memcpy(cmdsetface + 2, &iplen, 2);
	memcpy(cmdsetface + 4, &fpar.api_key, 32);
	memcpy(cmdsetface + 36, &fpar.secret_key, 32);
	*(cmdsetface + 68) = fpar.facial;
	*(cmdsetface + 69) = fpar.faces;
	*(cmdsetface + 70) = fpar.face_img;
	*(cmdsetface + 71) = 0;
	UINT16 box = htons(fpar.box);
	UINT16 width = htons(fpar.width);
	UINT16 height = htons(fpar.height);
	UINT16 reliability = htons(fpar.reliability);
	UINT16 res_time = htons(fpar.res_time);
	memcpy(cmdsetface + 72, &box, 2);
	memcpy(cmdsetface + 74, &width, 2);
	memcpy(cmdsetface + 76, &height, 2);
	memcpy(cmdsetface + 78, &reliability, 2);
	memcpy(cmdsetface + 80, &res_time, 2);
	memcpy(cmdsetface + 82, &fpar.groupID, 4);

	ret = SendBuffToHost(ip, cmdsetface, sizeof(cmdsetface), databuf, save);

	return ret;
}

bool WinSocketClient::GetFaceParameter(const char *ip, FaceParameter &fpar)
{
	bool ret;

	char cmdsetIP[4] = { 0 };
	short int iplen = 0;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetIP[0] = 74;
	cmdsetIP[1] = 90;
	memcpy(cmdsetIP + 2, &iplen, 2);

	ret = SendBuffToHost(ip, cmdsetIP, sizeof(cmdsetIP), databuf, false);

	if (ret)
	{
		memcpy(&fpar.api_key, databuf, 32);
		memcpy(&fpar.secret_key, databuf+32, 32);
		fpar.facial = databuf[64];
		fpar.faces = databuf[65];
		fpar.face_img = databuf[66];
		fpar.box = ntohs(*((unsigned short *)(databuf + 68)));
		fpar.width = ntohs(*((unsigned short *)(databuf + 70)));
		fpar.height = ntohs(*((unsigned short*)(databuf + 72)));
		fpar.reliability = ntohs(*((unsigned short*)(databuf + 74)));
		fpar.res_time = ntohs(*((unsigned short*)(databuf + 76)));
		memcpy(&fpar.groupID, databuf+76, 4);
	}
	return ret;
}


bool WinSocketClient::SetAIFunction(const char *ip, AIFunction &aifunc, bool save)
{
	bool ret;
	char cmdsetAi[12] = { 0 };
	short int iplen = 8;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetAi[0] = 75;
	cmdsetAi[1] = 92;
	char head[4] = { 0x72, 0x2a, 0x3c, 0x6e };
	memcpy(cmdsetAi + 2, &iplen, 2);
	memcpy(cmdsetAi + 4, head, 4);
	cmdsetAi[8] = aifunc.lan;
	cmdsetAi[9] = aifunc.comparison;
	UINT16 jpgmem = htons(aifunc.jpgmem);
	memcpy(cmdsetAi + 10, &jpgmem, 2);

	ret = SendBuffToHost(ip, cmdsetAi, sizeof(cmdsetAi), databuf, save);

	return ret;
}

bool WinSocketClient::GetAIFunction(const char *ip, AIFunction &aifunc)
{
	bool ret;

	char cmdsetIP[4] = { 0 };
	short int iplen = 0;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetIP[0] = 74;
	cmdsetIP[1] = 92;
	memcpy(cmdsetIP + 2, &iplen, 2);

	ret = SendBuffToHost(ip, cmdsetIP, sizeof(cmdsetIP), databuf, false);

	if (ret)
	{
		aifunc.lan = databuf[0];
		aifunc.comparison = databuf[1];
		aifunc.jpgmem = ntohs(*((unsigned short*)(databuf + 4)));
	}
	return ret;
}


bool WinSocketClient::SetFaceFuncSetting(const char *ip, FaceFuncSetting &ffset, bool save)
{
	bool ret;
	char cmdsetFFS[20] = { 0 };
	short int iplen = 16;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetFFS[0] = 75;
	cmdsetFFS[1] = 89;
	memcpy(cmdsetFFS + 2, &iplen, 2);
	
	cmdsetFFS[4] = ffset.face_rec;
	cmdsetFFS[5] = ffset.face_cod;
	cmdsetFFS[6] = ffset.face_cou;
	cmdsetFFS[7] = 0;
	memset(cmdsetFFS+8, 0xff, 12);

	ret = SendBuffToHost(ip, cmdsetFFS, sizeof(cmdsetFFS), databuf, save);

	return ret;
}

bool WinSocketClient::GetFaceFuncSetting(const char *ip, FaceFuncSetting &ffset)
{
	bool ret;

	char cmdsetIP[4] = { 0 };
	short int iplen = 0;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetIP[0] = 74;
	cmdsetIP[1] = 89;
	memcpy(cmdsetIP + 2, &iplen, 2);

	ret = SendBuffToHost(ip, cmdsetIP, sizeof(cmdsetIP), databuf, false);

	if (ret)
	{
		ffset.face_rec = databuf[0];
		ffset.face_cod = databuf[1];
		ffset.face_cou = databuf[2];
	}
	return ret;
}

bool WinSocketClient::SetRec(const char *ip, RECParameter &rec, bool save)
{
	bool ret;
	char cmdsetVmd[12] = { 0 };
	short int iplen = 8;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetVmd[0] = 75;
	cmdsetVmd[1] = 43;
	memcpy(cmdsetVmd + 2, &iplen, 2);

	memset(cmdsetVmd + 4, 0xff, 8);
	cmdsetVmd[9] = rec.rec;

	ret = SendBuffToHost(ip, cmdsetVmd, sizeof(cmdsetVmd), databuf, save);

	return ret;
}

bool WinSocketClient::GetRec(const char *ip, RECParameter &rec)
{
	bool ret;

	char cmdsetIP[4] = { 0 };
	short int iplen = 0;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetIP[0] = 74;
	cmdsetIP[1] = 43;
	memcpy(cmdsetIP + 2, &iplen, 2);

	ret = SendBuffToHost(ip, cmdsetIP, sizeof(cmdsetIP), databuf, false);

	if (ret)
	{
		rec.rec = databuf[5];
	}
	return ret;
}


bool WinSocketClient::SetNAS(const char *ip, NASParameter &nas, bool save)
{
	bool ret;
	char cmdsetNas[146] = { 0 };
	short int iplen = 142;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetNas[0] = 75;
	cmdsetNas[1] = 62;

	memcpy(cmdsetNas + 2, &iplen, 2);

	int space = htonl(nas.space);
	cmdsetNas[4] = nas.status;
	memcpy(cmdsetNas + 5, nas.user, NASUSER_LEN);
	memcpy(cmdsetNas + 38, nas.passwd, NASPASSWD_LEN);
	memcpy(cmdsetNas + 71, nas.path, NASPATH_LEN);
	memcpy(cmdsetNas+ 122, nas.name, NASNAME_LEN);
	memcpy(cmdsetNas + 138, nas.ip, sizeof(nas.ip));
	memcpy(cmdsetNas + 142, &space, 4);

	ret = SendBuffToHost(ip, cmdsetNas, sizeof(cmdsetNas), databuf, save);

	return ret;
}


bool WinSocketClient::GetNAS(const char *ip, NASParameter &nas)
{
	bool ret;;
	char cmdsetIP[4] = { 0 };
	short int iplen = 0;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetIP[0] = 74;
	cmdsetIP[1] = 62;
	memcpy(cmdsetIP + 2, &iplen, 2);

	ret = SendBuffToHost(ip, cmdsetIP, sizeof(cmdsetIP), databuf, false);

	if (ret)
	{
		nas.status = databuf[0];
		memcpy(nas.user, databuf + 1, NASUSER_LEN);
		memcpy(nas.passwd, databuf + 34, NASPASSWD_LEN);
		memcpy(nas.path, databuf + 67, NASPATH_LEN);
		memcpy(nas.name, databuf + 118, NASNAME_LEN);
		memcpy(nas.ip, databuf + 134, sizeof(nas.ip));
		nas.space = ntohl(*((unsigned int *)(databuf + 138)));
	}
	return ret;
}

bool WinSocketClient::RestartDevice(const char *ip)
{
	bool ret;

	char cmdsetIP[4] = { 0 };
	short int iplen = 0;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetIP[0] = 65;
	cmdsetIP[1] = 1;
	memcpy(cmdsetIP + 2, &iplen, 2);

	ret = SendBuffToHost(ip, cmdsetIP, sizeof(cmdsetIP), databuf, false);
	
	return ret;
}

bool WinSocketClient::SetFormateSDCard(const char *ip)
{
	bool ret;

	char cmdsetIP[4] = { 0 };
	short int iplen = 0;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetIP[0] = 75;
	cmdsetIP[1] = 83;
	memcpy(cmdsetIP + 2, &iplen, 2);

	ret = SendBuffToHost(ip, cmdsetIP, sizeof(cmdsetIP), databuf, false);

	return ret;
}

bool WinSocketClient::GetFormateSDCard(const char *ip, int &progress)
{
	bool ret;

	char cmdsetIP[4] = { 0 };
	short int iplen = 0;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetIP[0] = 74;
	cmdsetIP[1] = 83;
	memcpy(cmdsetIP + 2, &iplen, 2);

	ret = SendBuffToHost(ip, cmdsetIP, sizeof(cmdsetIP), databuf, false);
	if (ret)
		progress = ntohl(*((unsigned int *)databuf));
	else
		progress = 1000;
	return ret;
}

bool WinSocketClient::GetCameraNET(const char *ip, CameraNet &net)
{
	bool ret;

	char cmdsetIP[4] = { 0 };
	short int iplen = 0;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetIP[0] = 74;
	cmdsetIP[1] = 8;
	memcpy(cmdsetIP + 2, &iplen, 2);
	ret = SendBuffToHost(ip, cmdsetIP, sizeof(cmdsetIP), databuf, false);
	if (ret)
	{
		net.dhcp = ntohl(*((unsigned int *)databuf));
		memcpy(net.hostIP, databuf + 4, 4);
		memcpy(net.ipmask, databuf + 8, 4);
		memcpy(net.gateway, databuf + 12, 4);
		memcpy(net.dnsip1, databuf + 16, 4);
		memcpy(net.dnsip2, databuf + 20, 4);

		net.cmdPort = (*((unsigned short *)(databuf + 24)));
		net.dataPort = (*((unsigned short *)(databuf + 26)));
		net.httpPort = (*((unsigned short *)(databuf + 28)));
		net.talkPort = (*((unsigned short *)(databuf + 30)));
		net.autoConnect = (*((unsigned short *)(databuf + 32)));
		net.reserved = (*((unsigned short *)(databuf + 34)));
		net.centerIp0 = (*((unsigned int *)(databuf + 36)));
		net.centerIp1 = (*((unsigned int *)(databuf + 40)));
	}
	return ret;
}

bool WinSocketClient::SetCameraNET(const char *ip, CameraNet &net, bool save)
{

	bool ret;

	char cmdsetNet[48] = { 0 };
	short int iplen = 44;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetNet[0] = 75;
	cmdsetNet[1] = 8;
	memcpy(cmdsetNet + 2, &iplen, 2);
	
	int dhcp = htonl(net.dhcp);
	memcpy(cmdsetNet + 4, &dhcp, 4);
	memcpy(cmdsetNet + 8, net.hostIP, 4);
	memcpy(cmdsetNet + 12, net.ipmask, 4);
	memcpy(cmdsetNet + 16, net.gateway, 4);
	memcpy(cmdsetNet + 20, net.dnsip1, 4);
	memcpy(cmdsetNet + 24, net.dnsip2, 4);
	memcpy(cmdsetNet + 28, &net.cmdPort, 2);
	memcpy(cmdsetNet + 30, &net.dataPort, 2);
	memcpy(cmdsetNet + 32, &net.httpPort, 2);
	memcpy(cmdsetNet + 34, &net.talkPort, 2);
	memcpy(cmdsetNet + 36, &net.autoConnect, 2);
	memcpy(cmdsetNet + 38, &net.reserved, 2);
	memcpy(cmdsetNet + 40, &net.centerIp0, 4);
	memcpy(cmdsetNet + 44, &net.centerIp1, 4);

	ret = SendBuffToHost(ip, cmdsetNet, sizeof(cmdsetNet), databuf, true);
	return ret;
}


bool WinSocketClient::GetCameraStore(const char *ip, int totalSpace)
{
	bool ret;

	char cmdsetIP[4] = { 0 };
	short int iplen = 0;
	char databuf[256] = { 0 };

	//设置IP
	cmdsetIP[0] = 74;
	cmdsetIP[1] = 1;
	memcpy(cmdsetIP + 2, &iplen, 2);
	ret = SendBuffToHost(ip, cmdsetIP, sizeof(cmdsetIP), databuf, false);
	if (ret)
	{
		int total = ntohl(*((unsigned int *)(databuf + 20)))/100;
		if (total<4)
			totalSpace = 0;
		else if (total<6)
			totalSpace = 4;
		else if (total >= 6 && total<12)
			totalSpace = 8;
		else if (total >= 12 && total<24)
			totalSpace = 16;
		else if (total >= 24 && total<48)
			totalSpace = 32;
		else if (total >= 48 && total<96)
			totalSpace = 64;
		else if (total >= 96 && total<192)
			totalSpace = 128;
		else if (total >= 192 && total<384)
			totalSpace = 256;
		else if (total >= 384 && total<768)
			totalSpace = 512;
		else
			totalSpace = 0;

	}
	return ret;
}

//bool WinSocketClient::GetPanorama(const char *ip, string &panoTemplate)
//{
//	bool ret;
//
//	char cmdgetPano[8] = { 0 };
//	short int iplen = 0;
//	char databuf[256] = { 0 };
//
//	//设置IP
//	cmdgetPano[0] = 74;
//	cmdgetPano[1] = 94;
//	memcpy(cmdgetPano + 2, &iplen, 2);
//
//	if (ip[12] == '7')
//		ret = true;
//	ret = SendBuffToHost(ip, cmdgetPano, sizeof(cmdgetPano), databuf);
//
//	if (ret)
//	{
//		panoTemplate = databuf[5];
//	}
//	return ret;
//}