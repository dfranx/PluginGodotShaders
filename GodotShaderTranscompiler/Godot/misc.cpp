#include "misc.h"

#include <string.h>

std::string itos(int64_t p_val) {

	return std::to_string(p_val);
}

std::string uitos(uint64_t p_val) {

	return std::to_string(p_val);
}

std::string rtos(double p_val) {

	return std::to_string(p_val);
}

std::string rtoss(double p_val) {
	char buffer[64];
	memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, sizeof(buffer), "%g", p_val);
 
	return std::string(buffer);
}

bool str_is_valid_float(const std::string& str) {

	int len = str.length();

	if (len == 0)
		return false;

	int from = 0;
	if (str[0] == '+' || str[0] == '-') {
		from++;
	}

	bool exponent_found = false;
	bool period_found = false;
	bool sign_found = false;
	bool exponent_values_found = false;
	bool numbers_found = false;

	for (int i = from; i < len; i++) {

		if (str[i] >= '0' && str[i] <= '9') {

			if (exponent_found)
				exponent_values_found = true;
			else
				numbers_found = true;
		} else if (numbers_found && !exponent_found && str[i] == 'e') {
			exponent_found = true;
		} else if (!period_found && !exponent_found && str[i] == '.') {
			period_found = true;
		} else if ((str[i] == '-' || str[i] == '+') && exponent_found && !exponent_values_found && !sign_found) {
			sign_found = true;
		} else
			return false; // no start with number plz
	}

	return numbers_found;
}

bool str_is_valid_integer(const std::string& str) {

	int len = str.length();

	if (len == 0)
		return false;

	int from = 0;
	if (len != 1 && (str[0] == '+' || str[0] == '-'))
		from++;

	for (int i = from; i < len; i++) {

		if (str[i] < '0' || str[i] > '9')
			return false; // no start with number plz
	}

	return true;
}

std::string str_left(const std::string& str, int pos) {

	if (pos <= 0)
		return "";

	if (pos >= str.length())
		return str;

	return str.substr(0, pos);
}

std::string str_replace(const std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return str;
	std::string ret = str;
    ret.replace(start_pos, from.length(), to);
    return str;
}