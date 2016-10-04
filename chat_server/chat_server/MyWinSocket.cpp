#include"MyWinSocket.h"

inline static void init_winSocket() throw (MyWinSocketException) {
	WORD winSock_version = MAKEWORD(2, 0); // запрашиваем winsock версии 2.0
	WSADATA winSock_data;	// сюда будут помещены данные об инициализированном интерфейсе winsock

	int error = WSAStartup (winSock_version, &winSock_data);	// непосредственно инициализция
	if (error != 0)
		throw MyWinSocketException(GetLastError(), "initSock: could not initialize winSock.");
}
inline static void cleanup_winSocket() throw (MyWinSocketException){
	int error = WSACleanup();
	if (error != 0)
		throw MyWinSocketException(GetLastError(), "cleanSock: could not perform cleaning up.");
}

inline static sockaddr_in make_sockaddr_in(const char * const &ip, const unsigned short &port){
	sockaddr_in ad;
	ad.sin_family = AF_INET;
	if (strcmp(ip, "") == 0) ad.sin_addr.s_addr = INADDR_ANY;
	else ad.sin_addr.s_addr = inet_addr (ip);
	if (ad.sin_addr.s_addr == INADDR_NONE) throw MyWinSocketException(GetLastError(), "WinSocketAddress: the provided IP address seems to be invalid.");
	ad.sin_port = htons (port);
	return ad;
}
MyWinSocketAddress::MyWinSocketAddress(const char * const &ip, const unsigned short &port) throw (MyWinSocketException) : ad(make_sockaddr_in(ip, port)) {}
inline static sockaddr_in make_sockaddr_in(const sockaddr_in &addr){
	if (addr.sin_addr.s_addr == INADDR_NONE) throw MyWinSocketException(GetLastError(), "WinSocketAddress: the provided IP address seems to be invalid.");
	return addr;
}
MyWinSocketAddress::MyWinSocketAddress(const sockaddr_in &addr) throw (MyWinSocketException) : ad(make_sockaddr_in(addr)) {}
MyWinSocketAddress::MyWinSocketAddress(const sockaddr& addr) throw (MyWinSocketException) : ad(make_sockaddr_in(*((sockaddr_in*)&addr))) {}


unsigned long MyWinSocket::n = 0;
MyWinSocket::MyWinSocket(const int &type) throw (MyWinSocketException) : socket(0){
	if (n++ == 0) init_winSocket();
	SOCKET sock = ::socket(PF_INET, type, 0);
	if (sock == SOCKET_ERROR) throw MyWinSocketException(GetLastError(), "WinSocket: could not create a WinSocket.");
	socket.setData(sock);
}
MyWinSocket::~MyWinSocket() throw (MyWinSocketException){
	if (this->socket.num_of_copies() == 0){
		int error = closesocket (socket.getData()); // закрываем сокет
		if (error == SOCKET_ERROR) throw MyWinSocketException(GetLastError(), "~WinSocket: could not close WinSocket properly");
		if (--n == 0) cleanup_winSocket();
	}
}
void MyWinSocket::bind(const MySocketAddress * const &address) throw (MyWinSocketException){
	MyWinSocketAddress addr(address->get_ip(), address->get_port());
	int error = ::bind(socket.getData(), &((sockaddr)addr), sizeof(sockaddr_in));
	if (error == SOCKET_ERROR) throw MyWinSocketException(GetLastError(), "WinSocket::bind: could not bind socket to a provided address.");
}
int MyWinSocket::sendto (const MySocketAddress * const &address, const char * const &data, const int &len) throw (MySocketException){
	MyWinSocketAddress addr(address->get_ip(), address->get_port());
	int error = ::sendto (socket.getData(), data, len, 0, &((sockaddr)addr), sizeof(sockaddr_in));
	if (error == SOCKET_ERROR) throw MyWinSocketException(GetLastError(), "WinSocket::sendto: could not send data.");
	return error;
}
int MyWinSocket::recvfrom (MySocketAddress * &address, char * const &data, const int &len) throw (MySocketException){
	int address_len = sizeof(sockaddr_in);
	sockaddr *address_ = new sockaddr;
	int error = ::recvfrom (socket.getData(), data, len, 0, address_, &address_len);
	if (error == SOCKET_ERROR || address_len != sizeof(sockaddr_in)) throw MyWinSocketException(GetLastError(), "WinSocket::recvfrom: could not receive data.");
	address = (MySocketAddress*)(new MyWinSocketAddress(*address_));
	delete address_;
	return error;
}
int MyWinSocket::recvfrom (char * const &data, const int &len) throw (MySocketException){
	int error = ::recvfrom (socket.getData(), data, len, 0, nullptr, nullptr);
	if (error == SOCKET_ERROR) throw MyWinSocketException(GetLastError(), "WinSocket::recvfrom: could not receive data.");
	return error;
}
int MyWinSocket::send (const char * const &data, const int &len) throw (MySocketException){
	int error = ::send (socket.getData(), data, len, 0);
	if (error == SOCKET_ERROR) throw MyWinSocketException(GetLastError(), "WinSocket::send: could not send data.");
	return error;
}
int MyWinSocket::recv (char * const &data, const int &len) throw (MySocketException){
	int error = ::recv (socket.getData(), data, len, 0);
	if (error == SOCKET_ERROR) throw MyWinSocketException(GetLastError(), "WinSocket::recv: could not receive data.");
	return error;
}
int MyWinSocket::write (const char * const &data, const int &len) throw (MySocketException){
	int _len = len, error = 0;
	char *ptr = (char*)data;
	do{ error = ::send (socket.getData(), (ptr = ptr + error), _len, 0); }while(error > 0 && (_len = _len - error) > 0 || error == SOCKET_ERROR && GetLastError() == WSAEINTR);
	if (error == SOCKET_ERROR) throw MyWinSocketException(GetLastError(), "WinSocket::write: could not write data.");
	return len - _len;
}
int MyWinSocket::read (char * const &data, const int &len) throw (MySocketException){
	int _len = len, error = 0;
	char *ptr = data;
	do{ error = ::recv (socket.getData(), (ptr = ptr + error), _len, 0); }while(error > 0 && (_len = _len - error) > 0 || error == SOCKET_ERROR && GetLastError() == WSAEINTR);
	if (error == SOCKET_ERROR) throw MyWinSocketException(GetLastError(), "WinSocket::read: could not read data.");
	return len - _len;
}
void MyWinSocket::connect (const MySocketAddress * const &address) throw (MySocketException){
	MyWinSocketAddress addr(address->get_ip(), address->get_port());
	int error = ::connect(socket.getData(), &((sockaddr)addr), sizeof(sockaddr_in));
	if (error == SOCKET_ERROR) throw MyWinSocketException(GetLastError(), "WinSocket::connect: could not connect socket to a provided address.");
}
void MyWinSocket::listen (const int &backlog) throw (MySocketException){
	int error = ::listen(socket.getData(), backlog);
	if (error == SOCKET_ERROR) throw MyWinSocketException(GetLastError(), "WinSocket::listen: could not make listen socket.");
}
MySocket* MyWinSocket::accept (MySocketAddress * &address) throw (MySocketException){
	int address_len = sizeof(sockaddr_in);
	sockaddr *address_ = new sockaddr;
	SOCKET sock = ::accept(socket.getData(), address_, &address_len);
	if (sock == INVALID_SOCKET || address_len != sizeof(sockaddr_in)) throw MyWinSocketException(GetLastError(), "WinSocket::accept: could not accsept connection.");
	address = (MySocketAddress*)(new MyWinSocketAddress(*address_));
	delete address_;
	MyWinSocket * res = new MyWinSocket(*this);
	res->socket.setData(sock);
	n++;
	return (MySocket*)res;
}
MySocket* MyWinSocket::accept () throw (MySocketException){
	SOCKET sock = ::accept(socket.getData(), nullptr, nullptr);
	if (sock == INVALID_SOCKET) throw MyWinSocketException(GetLastError(), "WinSocket::accept: could not accsept connection.");
	MyWinSocket * res = new MyWinSocket(*this);
	res->socket.setData(sock);
	n++;
	return (MySocket*)res;
}