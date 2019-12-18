#include "UIHelper.h"

#include <clocale>
#include "../nativefiledialog/nfd.h"

namespace gd
{
	bool UIHelper::GetOpenDirectoryDialog(std::string& outPath)
	{
		nfdchar_t* path = NULL;
		nfdresult_t result = NFD_PickFolder(NULL, &path);
		setlocale(LC_ALL, "C");

		outPath = "";
		if (result == NFD_OKAY) {
			outPath = std::string(path);
			return true;
		}
		else if (result == NFD_ERROR) { /* TODO: log */ }

		return false;
	}
	bool UIHelper::GetOpenFileDialog(std::string& outPath, const std::string& files)
	{
		nfdchar_t *path = NULL;
		nfdresult_t result = NFD_OpenDialog(NULL, NULL, &path );
		setlocale(LC_ALL,"C");

		outPath = "";
		if (result == NFD_OKAY) {
			outPath = std::string(path);
			return true;
		}
		else if (result == NFD_ERROR) { /* TODO: log */ }

		return false;
	}
	bool UIHelper::GetSaveFileDialog(std::string& outPath, const std::string& files)
	{
		nfdchar_t *path = NULL;
		nfdresult_t result = NFD_SaveDialog(files.size() == 0 ? NULL : files.c_str(), NULL, &path );
		setlocale(LC_ALL,"C");

		outPath = "";
		if (result == NFD_OKAY) {
			outPath = std::string(path);
			return true;
		}
		else if (result == NFD_ERROR) { /* TODO: log */ }

		return false;
	}

	std::string UIHelper::TrimFilename(const std::string& path)
	{
		size_t lastSlash = path.find_last_of("/\\");
		if (lastSlash != std::string::npos)
			return path.substr(lastSlash + 1);
		return path;
	}
}