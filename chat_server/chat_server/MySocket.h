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
	// привязка сокета к локальномы "имени сокета" (здесь)
  	virtual void bind (const MySocketAddress * const &address) throw (MySocketException) = 0;
	// отправить сообщение (отправляются len байт по адресу data)
	// возвращает количество отосланых байт
	virtual int sendto (const MySocketAddress * const &address, const char * const &data, const int &len) throw (MySocketException) = 0;
	// принять сообщение ожидаемой длины len
	// вернет либо количество полученных байт, либо 0. 0 вовсе не означает, что было получено 0 байт, это значение означает, сокет был закрыт до вызова recvfrom().
	virtual int recvfrom (MySocketAddress * &address, char * const &data, const int &len) throw (MySocketException) = 0;
	virtual int recvfrom (char * const &data, const int &len) throw (MySocketException) = 0;
	// отправить len байт (сокет уже должен быль привязан к внегнему адресу)
	// возвращает количество отосланых байт
	virtual int send (const char * const &data, const int &len) throw (MySocketException) = 0;
	// принять байты (не больше len)
	// вернет либо количество полученных байт, либо 0. 0 вовсе не означает, что было получено 0 байт, это значение означает, сокет был закрыт до вызова recv().
	virtual int recv (char * const &data, const int &len) throw (MySocketException) = 0;
	// обертка над send. пишет все len байт из data, когда send может записать не все
	virtual int write (const char * const &data, const int &len) throw (MySocketException) = 0;
	// обертка над recv. пытается читать все len байт в data (ждет, пока не придут все len байт), когда recv может читать только те данные, которые пришли на данный момент
	virtual int read (char * const &data, const int &len) throw (MySocketException) = 0;
	// привязка сокета к глобальному "имени сокета" (в сети)
	virtual void connect (const MySocketAddress * const &address) throw (MySocketException) = 0;
	// можно вызвать только для сокетов после вызова bind и перед вызовом accept
	// переводит совет в состояние listen
	// параметр backlog задает задает максимальное число соединений.
	virtual void listen (const int &backlog) throw (MySocketException) = 0;
	// возвращает указатель на сокет, который образовался в результате запроса на подсоединение к серверу, за который отвечает этот сокет.
	// в address записывается адресс, от куда поступил запрос
	virtual MySocket* accept (MySocketAddress * &address) throw (MySocketException) = 0;
	virtual MySocket* accept () throw (MySocketException) = 0;
};