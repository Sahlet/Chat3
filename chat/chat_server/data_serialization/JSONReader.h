#ifndef JSONREADER_H_
#define JSONREADER_H_
#include "json11.hpp"
#include <Log.h>

struct JSONReader : public std::enable_shared_from_this< JSONReader > {
	virtual bool parse(const char* data);
	virtual bool parse(const std::string& data);
	virtual void on_finish();
	virtual bool on_json(const json11::Json& json) = 0;
	virtual operator json11::Json() const;
};


#define JSON_CLASS_MEMBER(x,y,z) x y;
#define JSON(x,y,z) {	auto &e = json[z];   if(!e.is_null()) {convert(y, e);}	}
#define JSON_DEFAULT_INIT(x,y,z) y(),
#define JSON_CONSTRUCTOR_PARAMS(x,y,z) x y = x(),
#define JSON_MOVE_INIT(x,y,z) y(std::move(y)),
#define JSON_PARE(x,y,z) {z, to_json(y)},
#define JSON_TMP_STR "JSON_TMP_STRwoxwoudnsindsiniwnooo4iu4nn dontuseme!!!"

#define JSON_class_gen(CLASS_NAME, CLASS_FIELDS_LIST, ...)									\
struct CLASS_NAME : public JSONReader	{													\
	using ptr = std::shared_ptr< CLASS_NAME >;												\
	virtual bool on_json(const json11::Json& json) override {								\
		CLASS_FIELDS_LIST(JSON)																\
		return true;																		\
	}																						\
	virtual operator json11::Json() const override {										\
		json11::Json::object res {															\
			CLASS_FIELDS_LIST(JSON_PARE)													\
			{JSON_TMP_STR, 0}																\
		};																					\
		res.erase(res.find(JSON_TMP_STR));													\
		return json11::Json(std::move(res));												\
	}																						\
	std::string dump() { return json11::Json(*this).dump(); }								\
	CLASS_FIELDS_LIST(JSON_CLASS_MEMBER)													\
	/*CLASS_NAME() : CLASS_FIELDS_LIST(JSON_DEFAULT_INIT) JSONReader() {}*/					\
	CLASS_NAME(CLASS_FIELDS_LIST(JSON_CONSTRUCTOR_PARAMS) void* empty_ptr = nullptr) :		\
		CLASS_FIELDS_LIST(JSON_MOVE_INIT) JSONReader() {}									\
	CLASS_NAME(CLASS_NAME&&) = default;														\
	__VA_ARGS__;																			\
};

template< typename ITEM_TYPE >
struct JSONList : public JSONReader {
	std::string list_name;
	virtual bool on_json(const json11::Json& json) override {
		JSON(_, items, list_name);
		return true;
	}
	virtual operator json11::Json() const override {
		json11::Json::array jsons;
		jsons.reserve(jsons.size());
		for (const auto& item : items) { jsons.push_back(json11::Json(item)); }
		return json11::Json(json11::Json::object{
			{list_name, jsons }
		});
	}
	using ptr = std::shared_ptr< JSONList >;
	std::vector< ITEM_TYPE > items;
	JSONList(std::string list_name = "JSONList") : list_name(std::move(list_name)) {}
	JSONList(JSONList&&) = default;
};

#define JSONList_gen(ITEM_TYPE, CLASS_NAME_STR)												\
template<>																					\
struct JSONList< ITEM_TYPE > : public JSONReader {											\
	std::string list_name;																	\
	virtual bool on_json(const json11::Json& json) override {								\
		JSON(_, items, list_name);															\
		return true;																		\
	}																						\
	virtual operator json11::Json() const override {										\
		json11::Json::array jsons;															\
		jsons.reserve(jsons.size());														\
		for (const auto& item : items) { jsons.push_back(json11::Json(item)); }				\
		return json11::Json(json11::Json::object{											\
			{ list_name, jsons }															\
		});																					\
	}																						\
	using ptr = std::shared_ptr< JSONList >;												\
	std::vector< ITEM_TYPE > items;															\
	JSONList(std::string list_name = CLASS_NAME_STR) : list_name(std::move(list_name)) {}	\
	JSONList(JSONList&&) = default;															\
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

template < class T >
std::enable_if_t< !std::is_enum<T>::value, json11::Json > to_json(const T& obj) {
	return json11::Json(obj);
}

template<class T >
std::enable_if_t< std::is_enum<T>::value, json11::Json > to_json(const T &obj) {
	return json11::Json(to_string(obj));
}

#endif
