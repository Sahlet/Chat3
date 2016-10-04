#pragma once
#pragma comment(lib, "ws2_32.lib")//����������� ���������� win_sock_2.0 (��� � �����, ��� 32 ���)
#include<WinSock2.h>
#include"MySocket.h"
#include"MyData.h"

class MyWinSocketException : public MySocketException{
public:
	MyWinSocketException(int errorCode, string errorCause = "") : MySocketException(errorCode, errorCause){}
};

class MyWinSocketAddress : public MySocketAddress{
private:
	MyData<sockaddr_in> ad;
public:
	MyWinSocketAddress(const char * const &ip = "", const unsigned short &port = 0) throw (MyWinSocketException);
	MyWinSocketAddress(const sockaddr& addr) throw (MyWinSocketException);
	MyWinSocketAddress(const sockaddr_in& addr) throw (MyWinSocketException);
	virtual char* get_ip() const{ return inet_ntoa(((MyData<sockaddr_in>)ad)->sin_addr); }
	virtual unsigned short get_port() const { return ntohs(((MyData<sockaddr_in>)ad)->sin_port); }
	operator sockaddr(){return *((sockaddr*)&ad.getData()); }
};

class MyWinSocket : public MySocket{
protected:
	MyData<SOCKET> socket;
	//������� �������� ����� ������
	static unsigned long n;
public:
	MyWinSocket(const int &type = SOCK_STREAM) throw (MyWinSocketException);
	virtual ~MyWinSocket() throw (MyWinSocketException);
	// �������� ������ � "����� ������"
	virtual void bind (const MySocketAddress * const &address) throw (MyWinSocketException);
	// ��������� ��������� (������������ len ���� �� ������ � data)
	// ���������� ���������� ��������� ����
	virtual int sendto (const MySocketAddress * const &address, const char * const &data, const int &len) throw (MySocketException);
	// ������� ��������� ��������� ����� len
	// ������ ���� ���������� ���������� ����, ���� 0. 0 ����� �� ��������, ��� ���� �������� 0 ����, ��� �������� ��������, ����� ��� ������ �� ������ recvfrom().
	virtual int recvfrom (MySocketAddress * &address, char * const &data, const int &len) throw (MySocketException);
	virtual int recvfrom (char * const &data, const int &len) throw (MySocketException);
	// ��������� len ���� (����� ��� ������ ���� �������� � �������� ������)
	// ���������� ���������� ��������� ����
	virtual int send (const char * const &data, const int &len) throw (MySocketException);
	// ������� ����� (�� ������ len)
	// ������ ���� ���������� ���������� ����, ���� 0. 0 ����� �� ��������, ��� ���� �������� 0 ����, ��� �������� ��������, ����� ��� ������ �� ������ recv().
	virtual int recv (char * const &data, const int &len) throw (MySocketException);
	// �������� ������ � ����������� "����� ������" (� ����)
	// ������� ��� send. ����� ��� len ���� �� data, ����� send ����� �������� �� ���
	virtual int write (const char * const &data, const int &len) throw (MySocketException);
	// ������� ��� recv. �������� ������ ��� len ���� � data (����, ���� �� ������ ��� len ����), ����� recv ����� ������ ������ �� ������, ������� ������ �� ������ ������
	virtual int read (char * const &data, const int &len) throw (MySocketException);
	// �������� ������ � ����������� "����� ������" (� ����)
	virtual void connect (const MySocketAddress * const &address) throw (MySocketException);
	// ����� ������� ������ ��� ������� ����� ������ bind � ����� ������� accept
	// ��������� ����� � ��������� listen
	// �������� backlog ������ ������ ������������ ����� ����������.
	virtual void listen (const int &backlog = SOMAXCONN) throw (MySocketException);
	// ���������� ��������� �� �����, ������� ����������� � ���������� ������� �� ������������� � �������, �� ������� �������� ���� �����.
	// � address ������������ ������, �� ���� �������� ������
	virtual MySocket* accept (MySocketAddress * &address) throw (MySocketException);
	virtual MySocket* accept () throw (MySocketException);
};