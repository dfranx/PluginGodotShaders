#pragma once
#include <string> 
#include <vector>
#include <GodotShaderTranscompiler/Godot/shader_language.h>
#include <PluginAPI/Plugin.h>
#include <Core/Uniform.h>

#ifdef _WIN32
#include <windows.h>
#endif 

namespace gd
{
	class UIHelper
	{
	public:
		static void TexturePreview(unsigned int tex, float w, float h);
		static void TexturePreview(unsigned int tex);

		static bool ShowValueEditor(ed::IPlugin1* owner, const std::string& name, Uniform& u);
	
		static std::string TrimFilename(const std::string& path);
	};
}