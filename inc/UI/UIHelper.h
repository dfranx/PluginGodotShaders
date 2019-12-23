#pragma once
#include <string> 
#include <vector>
#include <GodotShaderTranscompiler/Godot/shader_language.h>
#include <PluginAPI/Plugin.h>
#include <Plugin/Uniform.h>

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

		static void TexturePreview(unsigned int tex, float w, float h);
		static void TexturePreview(unsigned int tex);

		static bool ShowValueEditor(ed::IPlugin* owner, const std::string& name, Uniform& u);
	
		static std::string TrimFilename(const std::string& path);
	};
}