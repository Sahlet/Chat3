#pragma once
#include<string>
using namespace std;
class MyException{
protected:
	string errorCause;
	int errorCode;
public:
	typedef int j;
	MyException(int errorCode, string errorCause = "") : errorCode(errorCode), errorCause(errorCause) {}
	// получить код ошибки
	int getErrorCode() const { return errorCode; }
	// получить объ€снение
	string getErrorCause() { return string(errorCause.c_str()); }
	// оператор преобразовани€ в стринг
	operator string(){
		return "Error:\nErrorCode is " + to_string(errorCode) + "\nErrorCause is " + errorCause;
	}
	virtual string toString(){
		return this->operator string();
	}
	virtual ~MyException(){}
};