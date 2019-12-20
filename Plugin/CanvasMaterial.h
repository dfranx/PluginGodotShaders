#pragma once
#include "Settings.h"
#include "PipelineItem.h"
#include "../GodotShaderTranscompiler/ShaderTranscompiler.h"

#include <glm/glm.hpp>

namespace gd
{
	namespace pipe
	{
		class CanvasMaterial : public PipelineItem
		{
		public:
			char ShaderPath[MAX_PATH_LENGTH];

			CanvasMaterial();
			~CanvasMaterial();

			void SetViewportSize(float x, float y);
			void Bind();
			void ShowProperties();
			void ShowVariableEditor();
			void Compile();
			void CompileFromSource(const char* filedata, int filesize);

			void SetModelMatrix(glm::mat4 mat);

		private:
			struct Uniform
			{
				unsigned int Location;
				ShaderLanguage::DataType Type;
				std::vector<ShaderLanguage::ConstantNode::Value> Value;
			};

			gd::GLSLOutput m_glslData;
			std::unordered_map<std::string, Uniform> m_uniforms;

			unsigned int m_shader, m_projMatrixLoc, m_modelMatrixLoc, m_timeLoc;
			glm::mat4 m_projMat;
			glm::mat4 m_modelMat;
		};
	}
}