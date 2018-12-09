#pragma once

#include <iostream>  
#include <fstream>  
#include <string>  
//#include "irmString.h"

using namespace std;

class CSerializeData
{
public:
	CSerializeData() : next(0) {} // next÷√0 
	CSerializeData(string phonenum, string password) : phonenum(phonenum), password(password), next(0){}
	CSerializeData(const int type, const string& phonenum, const string& password, const string& access_token, const string& refresh_token, const string& uid)
		: type(type), phonenum(phonenum), password(password), access_token(access_token), refresh_token(refresh_token), uid(uid), next(0){}

		//∂¡–¥◊¢≤·”√ªß√˚√‹¬Î
	void saveLoginInfo(ofstream& stream);
	void loadLoginInfo(ifstream& stream);
	int getType() { return type; }
	string getLoginUser() { return phonenum; }
	string getLoginPasswd() { return password; }
	string getToken() { return access_token; }
	string getRefreshToken() { return refresh_token; }
	string getUid() { return uid; } 

	//∂¡–¥  
	void save(ofstream& stream);
	void load(ifstream& stream);

	CSerializeData* next;

private:
	int	   type;
	string phonenum;
	string password;
	string access_token;
	string refresh_token;
	string uid;

};