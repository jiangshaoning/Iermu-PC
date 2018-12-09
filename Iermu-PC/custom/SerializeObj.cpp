
#include "stdafx.h"
#include "SerializeObj.h"

// �������� int,double, float,bool...  
template <class T>
void serialize(ofstream& stream, T& x)
{
	stream.write((char*)&x, sizeof(int));
}

template <class T>
void de_serialize(ifstream& stream, T& x)
{
	stream.read((char*)&x, sizeof(int));
}

//std::string    
void serialize(ofstream& stream, string& str)
{
	int len = str.length();
	stream.write((char*)&len, sizeof(int));
	stream.write(str.c_str(), len);
}

void de_serialize(ifstream& stream, string& str)
{
	int len;
	stream.read((char*)&len, sizeof(int));
	if (len > 100 || len < 0) return;
	str.resize(len);
	char x;
	for (int i = 0; i < len; i++)
	{
		stream.read(&x, sizeof(char));
		str[i] = x;
	}
}

//д��¼��Ϣ
void CSerializeData::saveLoginInfo(ofstream& stream)
{
	serialize(stream, this->phonenum);
	serialize(stream, this->password);
	//����ָ��next  
	bool is_not_null_ptr = next != 0;//���ָ���Ƿ�Ϊ�գ�����bool����ֵд���ļ���  
	stream.write((char*)&is_not_null_ptr, sizeof(bool));
	if (next)
		next->saveLoginInfo(stream);;
}

//����¼��Ϣ  
void CSerializeData::loadLoginInfo(ifstream& stream)
{
	de_serialize(stream, this->phonenum);
	de_serialize(stream, this->password);
	//��ָ��next  
	bool is_not_null_ptr;
	stream.read((char*)&is_not_null_ptr, sizeof(bool));
	if (is_not_null_ptr)
	{
		next = new CSerializeData;
		next->loadLoginInfo(stream);
	}
}

void CSerializeData::save(ofstream& stream)
{
	serialize(stream, this->type);
	serialize(stream, this->phonenum);
	serialize(stream, this->password);
	serialize(stream, this->access_token);
	serialize(stream, this->uid);
	serialize(stream, this->refresh_token);
	//����ָ��next  
	//bool is_not_null_ptr = next != 0;//���ָ���Ƿ�Ϊ�գ�����bool����ֵд���ļ���  
	//stream.write((char*)&is_not_null_ptr, sizeof(bool));
	//if (next)
	//	next->save(stream);
}

//���ļ�  
void CSerializeData::load(ifstream& stream)
{
	de_serialize(stream, this->type);
	de_serialize(stream, this->phonenum);
	de_serialize(stream, this->password);
	de_serialize(stream, this->access_token);
	de_serialize(stream, this->uid);
	de_serialize(stream, this->refresh_token);
	//��ָ��next  
	//bool is_not_null_ptr;
	//stream.read((char*)&is_not_null_ptr, sizeof(bool));
	//if (is_not_null_ptr)
	//{
	//	next = new CSerializeData;
	//	next->load(stream);
	//}
}
