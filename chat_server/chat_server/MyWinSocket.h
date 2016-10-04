#pragma once
#pragma comment(lib, "ws2_32.lib")//подключение библиотеки win_sock_2.0 (как я думаю, для 32 бит)
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
	//счетчик объектов этого класса
	static unsigned long n;
public:
	MyWinSocket(const int &type = SOCK_STREAM) throw (MyWinSocketException);
	virtual ~MyWinSocket() throw (MyWinSocketException);
	// привязка сокета к "имени сокета"
	virtual void bind (const MySocketAddress * const &address) throw (MyWinSocketException);
	// отправить сообщение (отправляются len байт по адресу в data)
	// возвращает количество отосланых байт
	virtual int sendto (const MySocketAddress * const &address, const char * const &data, const int &len) throw (MySocketException);
	// принять сообщение ожидаемой длины len
	// вернет либо количество полученных байт, либо 0. 0 вовсе не означает, что было получено 0 байт, это значение означает, сокет был закрыт до вызова recvfrom().
	virtual int recvfrom (MySocketAddress * &address, char * const &data, const int &len) throw (MySocketException);
	virtual int recvfrom (char * const &data, const int &len) throw (MySocketException);
	// отправить len байт (сокет уже должен быль привязан к внегнему адресу)
	// возвращает количество отосланых байт
	virtual int send (const char * const &data, const int &len) throw (MySocketException);
	// принять байты (не больше len)
	// вернет либо количество полученных байт, либо 0. 0 вовсе не означает, что было получено 0 байт, это значение означает, сокет был закрыт до вызова recv().
	virtual int recv (char * const &data, const int &len) throw (MySocketException);
	// привязка сокета к глобальному "имени сокета" (в сети)
	// обертка над send. пишет все len байт из data, когда send может записать не все
	virtual int write (const char * const &data, const int &len) throw (MySocketException);
	// обертка над recv. пытается читать все len байт в data (ждет, пока не придут все len байт), когда recv может читать только те данные, которые пришли на данный момент
	virtual int read (char * const &data, const int &len) throw (MySocketException);
	// привязка сокета к глобальному "имени сокета" (в сети)
	virtual void connect (const MySocketAddress * const &address) throw (MySocketException);
	// можно вызвать только для сокетов после вызова bind и перед вызовом accept
	// переводит совет в состояние listen
	// параметр backlog задает задает максимальное число соединений.
	virtual void listen (const int &backlog = SOMAXCONN) throw (MySocketException);
	// возвращает указатель на сокет, который образовался в результате запроса на подсоединение к серверу, за который отвечает этот сокет.
	// в address записывается адресс, от куда поступил запрос
	virtual MySocket* accept (MySocketAddress * &address) throw (MySocketException);
	virtual MySocket* accept () throw (MySocketException);
};