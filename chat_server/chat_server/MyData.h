#pragma once
//������� MyData ��������� ������� ����� � ���������� ��� ��������� ������ ��� �����������.
//���� ����� ������, ����������� ������ - ����� ��������������� getData(),
//�� ����� ��������� ���������� ��������� (public) ���������� ����������� � ������ �;
//� ��������� ������ ������������ ������ MyData ����� ��������������� ����������� ������ �� ����.
template<class T>
class MyData{
	//�������������: ��� ����������� ������ ������ ������ TYPE (� ������� ���� ����������� ����������� �������� ���� ������� obj) ��� (TYPE name = obj), �� ���������� �����������, � �� �������� ������������!!!!!!!!!!!!!!!!
private:
	struct Node{ T data; unsigned long n; } *node;
	void init(const T &data){
		Node tmp = {data, unsigned long(1)};
		node = new Node(tmp);
	}
	void init(const MyData<T> &obj){
		node = obj.node;
		(node->n)++;
	}
	void destruct(){
		if (--(node->n) == 0)
			delete node;
	}
public:
	//���������� ��������� �����
	inline unsigned long num_of_copies() const { return (node->n) - 1; }
	inline MyData(const T &data){ init(data); }
	inline MyData(const MyData<T> &obj){ init(obj); }
	virtual ~MyData(){ destruct(); }
	inline MyData& operator=(const MyData<T>& obj){
		destruct();
		init(obj);
		return *this;
	}
	inline MyData& operator=(const T &data){
		destruct();
		init(data);
		return *this;
	}

	(const T)* operator->(){ return (T*)node; }
	
	//���������� ������, ��� �������� ���,
	//� ������� �� ���������� ������������ ����������� ������ T
	inline T getData(){
		return node->data;
	}
	inline void setData(const T &data){
		destruct();
		init(data);
	}
};