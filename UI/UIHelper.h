#pragma once
#include <string> 
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
	};
}