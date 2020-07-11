#pragma once
#include <vector>
#include <GodotShaderTranscompiler/Godot/shader_language.h>

namespace gd
{
	struct Uniform
	{
		Uniform()
		{
			UIHidden = false;
			Location = DebugLocation = -1;
			Type = ShaderLanguage::DataType::TYPE_VOID;
			HintType = ShaderLanguage::ShaderNode::Uniform::Hint::HINT_NONE;
			HintRange[0] = HintRange[1] = HintRange[2] = 0.0f;
		}

		unsigned int Location, DebugLocation;
		ShaderLanguage::DataType Type;
		std::vector<ShaderLanguage::ConstantNode::Value> Value;
		bool UIHidden;

		ShaderLanguage::ShaderNode::Uniform::Hint HintType;
		float HintRange[3];
	};
}