#pragma once
#include "Godot/visual_server.h"
#include "Godot/shader_language.h"
#include <string>
#include <unordered_map>

namespace gd
{
	struct GLSLOutput
	{
		std::string Vertex;
		std::string Fragment;

		bool Error;
		int ErrorLine;
		std::string ErrorMessage;

		bool SCREEN_UV, SCREEN_TEXTURE, TIME;
		bool SkipVertexTransform;
		int BlendMode, LightMode;
		std::unordered_map<std::string, ShaderLanguage::ShaderNode::Uniform> Uniforms;
	};

	class ShaderTranscompiler
	{
	public:
		static VisualServer::ShaderMode GetShaderType(const std::string& code);
		static void Transcompile(const std::string& in, GLSLOutput& out);
	};
}