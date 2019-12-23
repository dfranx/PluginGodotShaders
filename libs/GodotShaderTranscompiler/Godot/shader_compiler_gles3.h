/*************************************************************************/
/*  shader_compiler_gles3.h                                              */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef SHADERCOMPILERGLES3_H
#define SHADERCOMPILERGLES3_H

#include <unordered_map>
#include "shader_language.h"
#include "shader_types.h"
#include "visual_server.h"

class ShaderCompilerGLES3 {
public:
	struct IdentifierActions {

		std::unordered_map<std::string, std::pair<int *, int> > render_mode_values;
		std::unordered_map<std::string, bool *> render_mode_flags;
		std::unordered_map<std::string, bool *> usage_flag_pointers;
		std::unordered_map<std::string, bool *> write_flag_pointers;

		std::unordered_map<std::string, ShaderLanguage::ShaderNode::Uniform> *uniforms;
	};

	struct GeneratedCode {

		std::vector<std::string> defines;
		std::vector<std::string> texture_uniforms;
		std::vector<ShaderLanguage::DataType> texture_types;
		std::vector<ShaderLanguage::ShaderNode::Uniform::Hint> texture_hints;

		std::vector<uint32_t> uniform_offsets;
		uint32_t uniform_total_size;
		std::string uniforms;
		std::string vertex_global;
		std::string vertex;
		std::string fragment_global;
		std::string fragment;
		std::string light;

		bool uses_fragment_time;
		bool uses_vertex_time;
	};

private:
	ShaderLanguage parser;

	struct DefaultIdentifierActions {

		std::unordered_map<std::string, std::string> renames;
		std::unordered_map<std::string, std::string> render_mode_defines;
		std::unordered_map<std::string, std::string> usage_defines;
	};

	void _dump_function_deps(ShaderLanguage::ShaderNode *p_node, const std::string &p_for_func, const std::unordered_map<std::string, std::string> &p_func_code, std::string &r_to_add, std::set<std::string> &added);
	std::string _dump_node_code(ShaderLanguage::Node *p_node, int p_level, GeneratedCode &r_gen_code, IdentifierActions &p_actions, const DefaultIdentifierActions &p_default_actions, bool p_assigning);

	std::string current_func_name;
	std::string vertex_name;
	std::string fragment_name;
	std::string light_name;
	std::string time_name;

	std::set<std::string> used_name_defines;
	std::set<std::string> used_flag_pointers;
	std::set<std::string> used_rmode_defines;
	std::set<std::string> internal_functions;

	DefaultIdentifierActions actions[VS::SHADER_MAX];

public:
	Error compile(VS::ShaderMode p_mode, const std::string &p_code, IdentifierActions *p_actions, const std::string &p_path, GeneratedCode &r_gen_code, std::string& errorMsg, int& errorLine);

	ShaderCompilerGLES3();
};

#endif // SHADERCOMPILERGLES3_H
