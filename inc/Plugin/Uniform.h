#pragma once
#include <vector>
#include <GodotShaderTranscompiler/Godot/shader_language.h>

namespace gd
{
	struct Uniform // TODO: move this to a separate file
	{
		unsigned int Location;
		ShaderLanguage::DataType Type;
		std::vector<ShaderLanguage::ConstantNode::Value> Value;


		ShaderLanguage::ShaderNode::Uniform::Hint HintType;
		float HintRange[3];
	};
}