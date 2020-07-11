#ifndef TRANSCOMPILER_MISC_H
#define TRANSCOMPILER_MISC_H

#include <string>

typedef char CharType;

typedef int Error;
#define ERR_BUG 3
#define PARSER_ERROR 2
#define FAILED 1
#define OK 0

std::string itos(int64_t p_val);
std::string uitos(uint64_t p_val);
std::string rtos(double p_val);
std::string rtoss(double p_val);
std::string str_left(const std::string& str, int pos);
bool str_is_valid_float(const std::string& str);
bool str_is_valid_integer(const std::string& str);
std::string str_replace(const std::string& str, const std::string& from, const std::string& to);


#define likely(x) x
#define unlikely(x) x
#define ERR_FAIL_COND_V(m_cond, m_retval)                                                                                            \
	{                                                                                                                                \
		if (unlikely(m_cond)) {                                                                                                      \
			printf("Condition ... is true. returned: ..."); 											 \
			return m_retval;                                                                                                         \
		}                                                                                                                            \
	}

#define ERR_FAIL_COND_V_MSG(m_cond, m_retval, m_msg)                                                                                                   \
	{                                                                                                                                                  \
		if (unlikely(m_cond)) {                                                                                                                        \
			printf("Condition is true. returned: ..");                                                                          \
			return m_retval;                                                                                                                           \
		}                                                                                                                                              \
	}

#define ERR_FAIL_V(m_value)                                                                                       \
	{                                                                                                             \
		printf("Method/Function Failed."); \
		return m_value;                                                                                           \
	}
#define ERR_FAIL_COND(m_cond)                                                                              \
	{                                                                                                      \
		if (unlikely(m_cond)) {                                                                            \
			printf("COndition is true...");																   \
			return;                                                                                        \
		}                                                                                                  \
	}

class Shader
{
public:
	class CanvasItem {
	public:
		enum BlendMode {

			BLEND_MODE_MIX, //default
			BLEND_MODE_ADD,
			BLEND_MODE_SUB,
			BLEND_MODE_MUL,
			BLEND_MODE_PREMULT_ALPHA,
			BLEND_MODE_DISABLED
		};
		enum LightMode {
			LIGHT_MODE_NORMAL,
			LIGHT_MODE_UNSHADED,
			LIGHT_MODE_LIGHT_ONLY
		};
	};
};

#endif // TRANSCOMPILER_MISC_H