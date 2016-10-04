#pragma once
//обертка MyData позволяет хранить обект и непариться про засорение памяти при копировании.
//Если нужны данные, чтохранятся внутри - можно воспользоваться getData(),
//но тогда необходим нормальный публичным (public) копирующий конструктор у класса Т;
//в противном случае пользователь класса MyData берет ответственность сохранности данных на себя.
template<class T>
class MyData{
	//напоминалочка: при определении обекта любого класса TYPE (у коорого есть конструктор принимающий параметр типа объекта obj) так (TYPE name = obj), то вызывается конструктор, а не оператор присавивания!!!!!!!!!!!!!!!!
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
	//возвращает количесто копий
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
	
	//возвращает данные, что хранятся тут,
	//в надежде на надежность конструктора копирования класса T
	inline T getData(){
		return node->data;
	}
	inline void setData(const T &data){
		destruct();
		init(data);
	}
};