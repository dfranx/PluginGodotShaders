#pragma once
#include <Core/Uniform.h>
#include <Core/Settings.h>
#include <Core/PipelineItem.h>
#include <GodotShaderTranscompiler/ShaderTranscompiler.h>
#include <PluginAPI/Plugin.h>

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

			void SetRenderTexture(const std::string& rtName, unsigned int rt, unsigned int depth);
			
			void SetViewportSize(float x, float y);
			void Bind();
			void ShowProperties();
			void ShowVariableEditor();
			void Compile();
			void CompileFromSource(const char* filedata, int filesize);

			void SetModelMatrix(glm::mat4 mat);

			void DebugBind();
			void DebugSetID(int id);

			std::vector<unsigned int> VSSPV, PSSPV;

			inline const std::unordered_map<std::string, Uniform>& GetUniforms() { return m_uniforms; }
			inline void SetUniform(const std::string& name, const std::vector<ShaderLanguage::ConstantNode::Value>& val)
			{
				if (m_uniforms.count(name) == 0)
					m_uniforms[name].Type = ShaderLanguage::TYPE_VOID;
				m_uniforms[name].Value = val;
			}
			inline bool IsVertexShaderUsed() { return m_glslData.HasVertexShader; }
			inline bool IsFragmentShaderUsed() { return m_glslData.HasFragmentShader; }
			inline unsigned int GetFBO() { return m_fbo; }
			inline unsigned int GetRenderTexture() { return m_rt; }
			inline const std::string& GetRenderTextureName() { return m_rtName; }
			inline glm::vec2 GetViewportSize() { return glm::vec2(m_vw, m_vh); }

			void UpdateUniforms();

		private:
			void m_bindUniforms();

			gd::GLSLOutput m_glslData;
			std::unordered_map<std::string, Uniform> m_uniforms;

			float m_vw, m_vh;

			std::string m_rtName;
			unsigned int m_rt, m_fbo;

			bool m_isDebug;

			unsigned int m_shader, m_projMatrixLoc, m_modelMatrixLoc, m_timeLoc, m_pixelSizeLoc;
			unsigned int m_debugShader, m_debugProjMatrixLoc, m_debugModelMatrixLoc, m_debugTimeLoc, m_debugPixelSizeLoc;
			glm::mat4 m_projMat;
			glm::mat4 m_modelMat;
		};
	}
}