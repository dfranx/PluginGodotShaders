#include "CanvasMaterial.h"
#include "ResourceManager.h"
#include "../PluginAPI/Plugin.h"
#include "../UI/UIHelper.h"
#include "../GodotShaders.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include <string.h>
#include <fstream>
#include <string>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/glew.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#define BUTTON_SPACE_LEFT -40 * Owner->GetDPI()

std::string LoadFile(const std::string& file)
{
	std::ifstream in(file);
	if (in.is_open()) {
		in.seekg(0, std::ios::beg);

		std::string content((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
		in.close();
		return content;
	}
	return "";
}

namespace gd
{
	namespace pipe
	{
		CanvasMaterial::CanvasMaterial()
		{
			Type = PipelineItemType::CanvasMaterial;
			memset(ShaderPath, 0, sizeof(char) * MAX_PATH_LENGTH);
			m_shader = 0;
			m_modelMat = m_projMat = glm::mat4(1.0f);
		}
		CanvasMaterial::~CanvasMaterial()
		{
			if (m_shader != 0)
				glDeleteShader(m_shader);
		}
		void CanvasMaterial::SetViewportSize(float w, float h)
		{
			m_projMat = glm::ortho(0.0f, w, h, 0.0f, 0.1f, 1000.0f);
		}
		void CanvasMaterial::Bind()
		{
			// bind shaders
			glUseProgram(m_shader);

			glUniformMatrix4fv(m_projMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));

			if (m_glslData.TIME)
				glUniform1f(m_timeLoc, Owner->GetTime());

			for (const auto& uniform : m_uniforms) {
				const auto& val = uniform.second.Value;
				const auto& loc = uniform.second.Location;
				switch (uniform.second.Type) {
				case ShaderLanguage::TYPE_BOOL: glUniform1i(loc, val[0].sint); break;
				case ShaderLanguage::TYPE_BVEC2: glUniform2i(loc, val[0].sint, val[1].sint); break;
				case ShaderLanguage::TYPE_BVEC3: glUniform3i(loc, val[0].sint, val[1].sint, val[2].sint); break;
				case ShaderLanguage::TYPE_BVEC4: glUniform4i(loc, val[0].sint, val[1].sint, val[2].sint, val[3].sint); break;
				case ShaderLanguage::TYPE_INT: glUniform1i(loc, val[0].sint); break;
				case ShaderLanguage::TYPE_IVEC2: glUniform2i(loc, val[0].sint, val[1].sint); break;
				case ShaderLanguage::TYPE_IVEC3: glUniform3i(loc, val[0].sint, val[1].sint, val[2].sint); break;
				case ShaderLanguage::TYPE_IVEC4: glUniform4i(loc, val[0].sint, val[1].sint, val[2].sint, val[3].sint); break;
				case ShaderLanguage::TYPE_UINT: glUniform1ui(loc, val[0].uint); break;
				case ShaderLanguage::TYPE_UVEC2: glUniform2ui(loc, val[0].uint, val[1].uint); break;
				case ShaderLanguage::TYPE_UVEC3: glUniform3ui(loc, val[0].uint, val[1].uint, val[2].uint); break;
				case ShaderLanguage::TYPE_UVEC4: glUniform4ui(loc, val[0].uint, val[1].uint, val[2].uint, val[3].uint); break;
				case ShaderLanguage::TYPE_FLOAT: glUniform1f(loc, val[0].real); break;
				case ShaderLanguage::TYPE_VEC2: glUniform2f(loc, val[0].real, val[1].real); break;
				case ShaderLanguage::TYPE_VEC3: glUniform3f(loc, val[0].real, val[1].real, val[2].real); break;
				case ShaderLanguage::TYPE_VEC4: glUniform4f(loc, val[0].real, val[1].real, val[2].real, val[3].real); break;
				case ShaderLanguage::TYPE_MAT2: glUniformMatrix2fv(loc, 1, GL_FALSE, (float*)&val[0]); break;
				case ShaderLanguage::TYPE_MAT3: glUniformMatrix3fv(loc, 1, GL_FALSE, (float*)&val[0]); break;
				case ShaderLanguage::TYPE_MAT4: glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&val[0]); break;
				case ShaderLanguage::TYPE_SAMPLER2D: 
					glActiveTexture(GL_TEXTURE0 + loc);
					glBindTexture(GL_TEXTURE_2D, val[0].uint);
					break;
				}
			}
		}
		void CanvasMaterial::ShowProperties()
		{
			ImGui::Columns(2, "##plugin_columns");

			// TODO: this is only a temprorary fix for non-resizable columns
			static bool isColumnWidthSet = false;
			if (!isColumnWidthSet) {
				ImGui::SetColumnWidth(0, ImGui::GetWindowSize().x * 0.3f);
				isColumnWidthSet = true;
			}

			/* shader path */
			ImGui::Text("Shader:");
			ImGui::NextColumn();

			ImGui::PushItemWidth(BUTTON_SPACE_LEFT);
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::InputText("##pui_vspath", ShaderPath, MAX_PATH_LENGTH);
			ImGui::PopItemFlag();
			ImGui::PopItemWidth();
			ImGui::SameLine();
			if (ImGui::Button("...##pui_vsbtn", ImVec2(-1, 0))) {
				std::string file = "";
				bool success = UIHelper::GetOpenFileDialog(file);
				if (success) {
					char tempFile[MAX_PATH_LENGTH];
					Owner->GetRelativePath(Owner->Project, file.c_str(), tempFile);
					file = std::string(tempFile);

					strcpy(ShaderPath, file.c_str());

					Owner->ModifyProject(Owner->Project);

					if (Owner->FileExists(Owner->Project, file.c_str())) {
						Owner->ClearMessageGroup(Owner->Messages, Name);
						Compile();
					}
					else
						Owner->AddMessage(Owner->Messages, ed::plugin::MessageType::Error, Name, "Shader file doesn't exist", -1);
					((gd::GodotShaders*)Owner)->ShaderPathsUpdated = true;
				}
			}
			ImGui::NextColumn();


			ImGui::Columns(1);
		}
		void CanvasMaterial::ShowVariableEditor()
		{

		}

		void CanvasMaterial::SetModelMatrix(glm::mat4 mat)
		{
			m_modelMat = mat;
			glUniformMatrix4fv(m_modelMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));
		}
		void CanvasMaterial::Compile()
		{
			std::string vsCodeContent = ResourceManager::Instance().GetDefaultCanvasVertexShader();
			std::string psCodeContent = ResourceManager::Instance().GetDefaultCanvasPixelShader();

			std::string godotShaderContents = "";

			if (strlen(ShaderPath) != 0) {
				char outPath[MAX_PATH_LENGTH];
				Owner->GetProjectPath(Owner->Project, ShaderPath, outPath);

				godotShaderContents = LoadFile(outPath);
			}

			const char* filedata = godotShaderContents.c_str();
			int filesize = godotShaderContents.size();

			CompileFromSource(filedata, filesize);
		}
		void CanvasMaterial::CompileFromSource(const char* filedata, int filesize)
		{
			m_uniforms.clear();
			Owner->ClearMessageGroup(Owner->Messages, Name);

			std::string vsCodeContent = ResourceManager::Instance().GetDefaultCanvasVertexShader();
			std::string psCodeContent = ResourceManager::Instance().GetDefaultCanvasPixelShader();

			if (filesize != 0 && filedata != nullptr) {
				gd::ShaderTranscompiler::Transcompile(filedata, m_glslData);

				if (!m_glslData.Error) {
					vsCodeContent = m_glslData.Vertex;
					psCodeContent = m_glslData.Fragment;
				} else {
					Owner->AddMessage(Owner->Messages, ed::plugin::MessageType::Error, Name, m_glslData.ErrorMessage.c_str(), m_glslData.ErrorLine);
				}
			}

			const char* vsCode = vsCodeContent.c_str();
			const char* psCode = psCodeContent.c_str();

			GLint success = 0;
			char infoLog[512];

			if (m_shader != 0)
				glDeleteShader(m_shader);

			// create vertex shader
			unsigned int canvasVS = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(canvasVS, 1, &vsCode, nullptr);
			glCompileShader(canvasVS);
			glGetShaderiv(canvasVS, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(canvasVS, 512, NULL, infoLog);
				Owner->Log("Failed to compile a GCanvasMaterial vertex shader", true, nullptr, -1);
				Owner->Log(infoLog, true, nullptr, -1);
			}

			// create pixel shader
			unsigned int canvasPS = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(canvasPS, 1, &psCode, nullptr);
			glCompileShader(canvasPS);
			glGetShaderiv(canvasPS, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(canvasPS, 512, NULL, infoLog);
				Owner->Log("Failed to compile a GCanvasMaterial pixel shader", true, nullptr, -1);
				Owner->Log(infoLog, true, nullptr, -1);
			}

			// create a shader program for gizmo
			m_shader = glCreateProgram();
			glAttachShader(m_shader, canvasVS);
			glAttachShader(m_shader, canvasPS);
			glLinkProgram(m_shader);
			glGetProgramiv(m_shader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(m_shader, 512, NULL, infoLog);
				Owner->Log("Failed to create a GCanvasMaterial shader program", true, nullptr, -1);
				Owner->Log(infoLog, true, nullptr, -1);
			}

			//glDeleteShader(canvasPS);
			//glDeleteShader(canvasVS);

			m_projMatrixLoc = glGetUniformLocation(m_shader, "projection_matrix");
			m_modelMatrixLoc = glGetUniformLocation(m_shader, "modelview_matrix");
			m_timeLoc = glGetUniformLocation(m_shader, "time");

			glUniform1i(glGetUniformLocation(m_shader, "color_texture"), 0); // color_texture -> texunit: 0
			
			// user uniforms
			for (const auto& uniform : m_glslData.Uniforms) {
				m_uniforms[uniform.first].Location = glGetUniformLocation(m_shader, ("m_" + uniform.first).c_str());

				m_uniforms[uniform.first].Type = uniform.second.type;
				m_uniforms[uniform.first].Value = uniform.second.default_value;

				if (uniform.second.default_value.size() == 0) {
					m_uniforms[uniform.first].Value.resize(ShaderLanguage::get_cardinality(uniform.second.type));
					for (auto& val : m_uniforms[uniform.first].Value)
						val.sint = 0;
				}
			}
		}
	}
}