#pragma once
#include <string> 
#include <vector>
#include "../GodotShaderTranscompiler/Godot/shader_language.h"

#ifdef _WIN32
#include <windows.h>
#endif 

namespace gd
{
	class UIHelper
	{
	public:
		static bool GetOpenDirectoryDialog(std::string& outPath);
		static bool GetOpenFileDialog(std::string& outPath, const std::string& files = "");
		static bool GetSaveFileDialog(std::string& outPath, const std::string& files = "");

		static bool ShowValueEditor(const std::string& name, ShaderLanguage::DataType type, std::vector<ShaderLanguage::ConstantNode::Value>& value, ShaderLanguage::ShaderNode::Uniform::Hint hint, float hint_range[3]);
	
		static std::string TrimFilename(const std::string& path);
	};
}