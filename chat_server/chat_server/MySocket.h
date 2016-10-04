#pragma once;
#include<string>
#include"MyException.h"
#pragma warning(disable : 4290)
using namespace std;

class MySocketException : public MyException{
public:
	MySocketException(int errorCode, string errorCause = "") : MyException(errorCode, errorCause){}
};

struct MySocketAddress{
	virtual char* get_ip () const = 0;
	virtual unsigned short get_port () const = 0;
	virtual ~MySocketAddress(){};
};

class MySocket{
public:	
	virtual ~MySocket () throw (MySocketException){}
	// �������� ������ � ���������� "����� ������" (�����)
  	virtual void bind (const MySocketAddress * const &address) throw (MySocketException) = 0;
	// ��������� ��������� (������������ len ���� �� ������ data)
	// ���������� ���������� ��������� ����
	virtual int sendto (const MySocketAddress * const &address, const char * const &data, const int &len) throw (MySocketException) = 0;
	// ������� ��������� ��������� ����� len
	// ������ ���� ���������� ���������� ����, ���� 0. 0 ����� �� ��������, ��� ���� �������� 0 ����, ��� �������� ��������, ����� ��� ������ �� ������ recvfrom().
	virtual int recvfrom (MySocketAddress * &address, char * const &data, const int &len) throw (MySocketException) = 0;
	virtual int recvfrom (char * const &data, const int &len) throw (MySocketException) = 0;
	// ��������� len ���� (����� ��� ������ ���� �������� � �������� ������)
	// ���������� ���������� ��������� ����
	virtual int send (const char * const &data, const int &len) throw (MySocketException) = 0;
	// ������� ����� (�� ������ len)
	// ������ ���� ���������� ���������� ����, ���� 0. 0 ����� �� ��������, ��� ���� �������� 0 ����, ��� �������� ��������, ����� ��� ������ �� ������ recv().
	virtual int recv (char * const &data, const int &len) throw (MySocketException) = 0;
	// ������� ��� send. ����� ��� len ���� �� data, ����� send ����� �������� �� ���
	virtual int write (const char * const &data, const int &len) throw (MySocketException) = 0;
	// ������� ��� recv. �������� ������ ��� len ���� � data (����, ���� �� ������ ��� len ����), ����� recv ����� ������ ������ �� ������, ������� ������ �� ������ ������
	virtual int read (char * const &data, const int &len) throw (MySocketException) = 0;
	// �������� ������ � ����������� "����� ������" (� ����)
	virtual void connect (const MySocketAddress * const &address) throw (MySocketException) = 0;
	// ����� ������� ������ ��� ������� ����� ������ bind � ����� ������� accept
	// ��������� ����� � ��������� listen
	// �������� backlog ������ ������ ������������ ����� ����������.
	virtual void listen (const int &backlog) throw (MySocketException) = 0;
	// ���������� ��������� �� �����, ������� ����������� � ���������� ������� �� ������������� � �������, �� ������� �������� ���� �����.
	// � address ������������ ������, �� ���� �������� ������
	virtual MySocket* accept (MySocketAddress * &address) throw (MySocketException) = 0;
	virtual MySocket* accept () throw (MySocketException) = 0;
};