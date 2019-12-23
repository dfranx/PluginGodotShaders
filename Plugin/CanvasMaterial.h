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

			struct Uniform // TODO: move this to a separate file
			{
				unsigned int Location;
				ShaderLanguage::DataType Type;
				std::vector<ShaderLanguage::ConstantNode::Value> Value;


				ShaderLanguage::ShaderNode::Uniform::Hint HintType;
				float HintRange[3];

			};

			inline const std::unordered_map<std::string, Uniform>& GetUniforms() { return m_uniforms; }
			inline void SetUniform(const std::string& name, const std::vector<ShaderLanguage::ConstantNode::Value>& val)
			{
				if (m_uniforms.count(name) == 0)
					m_uniforms[name].Type = ShaderLanguage::TYPE_VOID;
				m_uniforms[name].Value = val;
			}

		private:

			gd::GLSLOutput m_glslData;
			std::unordered_map<std::string, Uniform> m_uniforms;

			float m_vw, m_vh;

			unsigned int m_shader, m_projMatrixLoc, m_modelMatrixLoc, m_timeLoc, m_pixelSizeLoc;
			glm::mat4 m_projMat;
			glm::mat4 m_modelMat;
		};
	}
}