#ifndef JSONREADER_H_
#define JSONREADER_H_
#include "json11.hpp"
#include <Log.h>

struct JSONReader : public std::enable_shared_from_this< JSONReader > {
	virtual bool parse(const char* data);
	virtual bool parse(const std::string& data);
	virtual void on_finish();
	virtual bool on_json(const json11::Json& json) = 0;
};


#define CLASS_MEMBER(x,y) x y;
#define JSON(x,y,z) {	auto &e = json[z];   if(!e.is_null()) {convert(y, e);}	}
#define JSON_DEFAULT_INIT(x,y,z) , y()

#define JSON_class_gen(CLASS_NAME, CLASS_FIELDS_LIST, ...)									\
struct CLASS_NAME : public JSONReader	{													\
	virtual bool on_json(const json11::Json& json) override {								\
		CLASS_FIELDS_LIST(JSON)																\
		return true;																		\
	}																						\
public:																						\
	__VA_ARGS__;																			\
	using ptr = std::shared_ptr< CLASS_NAME >;												\
	CLASS_FIELDS_LIST(CLASS_MEMBER)															\
	CLASS_NAME() : JSONReader() CLASS_FIELDS_LIST(JSON_DEFAULT_INIT){}						\
};

template< typename ITEM_TYPE >
struct JSONList : public JSONReader {
	std::string list_name;
	virtual bool on_json(const json11::Json& json) override {
		JSON(_, items, list_name);
		return true;
	}
	using ptr = std::shared_ptr< JSONList >;
	std::vector< ITEM_TYPE > items;
	JSONList(std::string list_name = "") : list_name(std::move(list_name)) {}
};

#define JSONList_gen(ITEM_TYPE, CLASS_NAME_STR)												\
template<>																					\
struct JSONList< ITEM_TYPE > : public JSONReader {											\
	std::string list_name;																	\
	virtual bool on_json(const json11::Json& json) override {								\
		JSON(_, items, list_name);															\
		return true;																		\
	}																						\
	using ptr = std::shared_ptr< JSONList >;												\
	std::vector< ITEM_TYPE > items;															\
	JSONList(std::string list_name = CLASS_NAME_STR) : list_name(std::move(list_name)) {}	\
};


template<class T>
void convert(std::shared_ptr<T> &obj, const json11::Json& value) {
	obj.reset(new T);
	convert(*obj,value);
}

void convert(int & obj, const json11::Json& value);
void convert(int64_t & obj, const json11::Json& value);
void convert(long & obj, const json11::Json& value);
void convert(std::string & obj, const json11::Json& value);
void convert(bool & obj, const json11::Json& value);

template<class T>
void convert(std::vector<T> &obj, const json11::Json& value) {
	auto &arr = value.array_items();
	for (auto& e : arr) {
		obj.emplace_back();
		convert(obj.back(), e);
	}
}

template<class K, class T>
void convert(std::map<K, T> &obj, const json11::Json& value) {
	auto &arr = value.object_items();
	for(auto& a : arr) {
		K key;
		convert(key, a.first.c_str());
		convert(obj[key], a.second);
	}
}

template<class T, typename std::enable_if<std::is_base_of<JSONReader,T>::value>::type* = nullptr>
void convert(T &obj, const json11::Json& value) {
	obj.on_json(value);
}


template<class T, typename  std::enable_if<std::is_enum<T>::value>::type* = nullptr>
void convert(T &obj, const json11::Json& value) {
	obj = EnumSerializer<T>::from_string(value.string_value());
}

#include <map>
#include <string>
#include <stack>
#include <vector>

void convert(uint64_t & obj, const char* value);
void convert(int & obj, const char* value);
void convert(long & obj, const char* value);
void convert(float & obj, const char* value);
void convert(bool & obj, const char* value);
void convert(std::string & obj, const char* value);

template<class T, typename = std::enable_if<std::is_enum<T>::value>>
void convert(T &obj, const char* value) {
	obj = EnumSerializer<T>::from_string(value);
}

template<class T>
struct EnumSerializer {};

/*
	//EnumSerializer sample
	template<>
	struct EnumSerializer<T> {
		static T from_string(const std::string& str) {
			if (str == "VALUE") return T::VALUE;

			return T(0);
		}
	
		static std::string to_string(const T& enm) {
			if (enm == T::VALUE) return "VALUE";

			return "";
		}
	};
*/

template<class T, typename = std::enable_if<std::is_enum<T>::value>>
std::string to_string(const T &obj) {
	return EnumSerializer<T>::to_string(obj);
}

#endif
