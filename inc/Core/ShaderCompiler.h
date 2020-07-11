#pragma once
#include <string>
#include <vector>
#include <PluginAPI/PluginData.h>

namespace gd {
	class ShaderCompiler {
	public:
		static bool CompileSourceToSPIRV(std::vector<unsigned int>& spvOut, const std::string& source, ed::plugin::ShaderStage shaderType, const std::string& entry);
	};
}