#pragma once
#include"MyWinSocket.h"
#include"MyData.h"

#pragma region usingClasses
unsigned int my_strlen(const char * const &s){
	if (s == nullptr) return 0;
	const char *res = s;
	for (; *res != '\0'; res++){}
	return res - s;
}
class charPtr{
	typedef char T;
	MyData<T*> ptr;
	unsigned short len;//�� ������ ���!
	void destr(){
		if(ptr.num_of_copies() == 0) delete ptr.getData();
	}
public:
	charPtr() : ptr(nullptr), len(0){}
	//������������ ������ ����� � ����� ���� ������� ����
	charPtr(const T* const &ptr) : ptr(nullptr){
		init(ptr, my_strlen(ptr));
	}
	charPtr(const T* const &ptr, const unsigned short &len) : ptr(nullptr){
		init(ptr, len);
	}
	charPtr(T* const &ptr, const unsigned short &len) : ptr(nullptr){
		init(ptr, len);
	}
	void init(T* const &ptr, const unsigned short &len){
		destr();
		this->len = len;
		this->ptr.setData((T*)ptr);
	}
	void init(const T* const &ptr, const unsigned short &len){
		char *buf = new char[len];
		for (char* tmp1 = buf, *tmp2 = (char*)ptr, *end = buf + len; tmp1 != end; *tmp1++ = *tmp2++){}
		this->init(buf, len);
	}
	~charPtr(){
		destr();
	}
	unsigned short getLen() const{
		return len;
	}
	//�� ������ ������ �, ��� ������������ ���� ������� �� ��������� �*, ��������� ������ template<class T> class Ptr �� ����� ���������������
	T* getPtr() const{
		return ((MyData<T*>)ptr).getData();
	}
	operator bool(){
		if(this->getPtr()) return true;
		return false;
	}
};
ostream& operator<<(ostream& stream, const charPtr &obj){
	char *ptr = obj.getPtr(), *end = ptr + obj.getLen();
	for (;ptr < end;) stream.put(*ptr++);
	return stream;
}
bool operator<(const charPtr &left, const charPtr &right){
	if (left.getLen() < right.getLen()) return true;
	else if(left.getLen() > right.getLen()) return false;
	for(char *ptr1 = left.getPtr(), *ptr2 = right.getPtr(), * const end = left.getPtr() + left.getLen() - 1; ptr1 <= end; ptr1++, ptr2++){
		if((*ptr1) < (*ptr2)) return true;
		else if((*ptr1) > (*ptr2)) return false;
	}
	return false;
}
bool operator ==(const charPtr &left, const charPtr &right){
	if(left.getLen() != right.getLen()) return false;
	for(char *ptr1 = left.getPtr(), *ptr2 = right.getPtr(), * const end = left.getPtr() + left.getLen() - 1; ptr1 <= end; ptr1++, ptr2++){
		if((*ptr1) != (*ptr2)) return false;
	}
	return true;
}
bool operator >(const charPtr &left, const charPtr &right){
	if (left.getLen() > right.getLen()) return true;
	else if(left.getLen() < right.getLen()) return false;
	for(char *ptr1 = left.getPtr(), *ptr2 = right.getPtr(), * const end = left.getPtr() + left.getLen() - 1; ptr1 <= end; ptr1++, ptr2++){
		if((*ptr1) > (*ptr2)) return true;
		else if((*ptr1) < (*ptr2)) return false;
	}
	return false;
}
bool operator !=(const charPtr &left, const charPtr &right){
	return !(left == right);
}
charPtr operator+(const charPtr &left, const charPtr &right){
	char *buf = new char[left.getLen() + right.getLen()], *ptr1 = left.getPtr(), *ptr2 = right.getPtr();
	for (char* end = buf + left.getLen(); buf < end; *buf++ = *ptr1++){}
	for (char* end = buf + right.getLen(); buf < end; *buf++ = *ptr2++){}
	buf -= left.getLen() + right.getLen();
	return charPtr(buf, left.getLen() + right.getLen());
}
charPtr operator+(const charPtr &left, const char * const &right){
	return left + charPtr(right);
}
charPtr operator+(const char * const &left, const charPtr &right){
	return charPtr(left) + right;
}

class MySocketPtr{
	typedef MySocket* T;
	MyData<T> data;
public:
	MySocketPtr(const T &sock) : data(sock){}
	~MySocketPtr(){
		if(data.num_of_copies() == 0) delete data.getData();
	}
	T operator->(){
		return data.getData();
	}
};

#pragma region for_socket

/**
	������� � ��������:
		-�� ��������� TCP.
		-� �������� ������ ��������� ������� 2 �����, ������� ������������ �� ���� ����� ��������������� ����� [0; 256 * 256 - 1] ��������� � ������� ���� �� �������� � ��������.
	����� ������� ������ ������� ����� �����, ����� ����� ��������� �������.
 **/

 //���������� len ���� �� ������ str, ��������� ����� ���� ���������� ������������ ���� (�� ���� len)
void writestr(MySocketPtr sock, const char* const &str, const unsigned short &len) throw (MySocketException) {
	char len_[] = {len / 256, len % 256};
	sock->write(len_, 2);
	if (len != 0) sock->write(str, len);
}
MySocketPtr& operator<<(MySocketPtr &sock, const charPtr &ptr){
	writestr(sock, ptr.getPtr(), ptr.getLen());
	return sock;
}
MySocketPtr& operator<<(MySocketPtr &sock, const char * const &ptr){
	writestr(sock, ptr, strlen(ptr));
	return sock;
}

//��������� � str, ������������ � ��� ��������� ���������� ����, � ���������� ��� ���������� � len
//str �� ����� ������ ���� �� �����������������
void readstr(MySocketPtr sock, char *&str, unsigned short &len) throw (MySocketException) {
	char len_[2];
	if (sock->read(len_, 2)) len = len_[0] * 256 + len_[1];
	else len = 0;
	try{
		if (len != 0){
			str = new char[len];
			sock->read(str, len);
		} else str = nullptr;
	} catch(...) {
		delete str;
		throw;
	}
}
MySocketPtr& operator>>(MySocketPtr &sock, charPtr &ptr){
	char* buf;
	unsigned short len;
	readstr(sock, buf, len);
	ptr.init(buf, len);
	return sock;
}

#pragma endregion

#pragma endregion