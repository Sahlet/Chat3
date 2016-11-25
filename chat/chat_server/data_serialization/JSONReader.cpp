#include "JSONReader.h"

void JSONReader::on_finish() {}
bool JSONReader::parse(const char* data) {
	return parse(std::string(data));
}
bool JSONReader::parse(const std::string& data) {
	std::string err;
	auto obj = json11::Json::parse(data, err);
	if (err.size()) {
		LOGE("json error: %s\n", err.c_str());
		return false;
	}
	on_json(obj);
	on_finish();
	return true;
}
JSONReader::operator json11::Json() const { return json11::Json(); }

//----------------------------------------------------------
void convert(int & obj, const json11::Json& value) {
	if (value.is_number())
	{
		obj = value.int_value();
	}
}

void convert(int64_t & obj, const json11::Json& value) {
	if (value.is_number()) {
		obj = value.int_value();
	}
}

void convert(long & obj, const json11::Json& value)
{
	if (value.is_number())
	{
		obj = value.int_value();
	}
}

void convert(bool & obj, const json11::Json& value)
{
	obj = value.bool_value();
}

void convert(std::string & obj, const json11::Json& value)
{
	obj = value.string_value();

}
//----------------------------------------------------------
//----------------------------------------------------------
void convert(int & obj, const char* value) {
	obj = atoi(value);
}

void convert(long & obj, const char* value) {
	obj = atol(value);
}

void convert(uint64_t & obj, const char* value) {
	obj = atoll(value);
}

void convert(float & obj, const char* value) {
	obj = atof(value);
}

void convert(bool & obj, const char* value) {
	std::string str(value, 4);
	obj = (str == "true") || (str == "TRUE") || (str == "True");
}

void convert(std::string & obj, const char* value) {
	obj = std::string(value);
}
//----------------------------------------------------------
