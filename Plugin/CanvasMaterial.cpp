#include "CanvasMaterial.h"
#include "ResourceManager.h"
#include "../PluginAPI/Plugin.h"
#include "../UI/UIHelper.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include <string.h>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#define BUTTON_SPACE_LEFT -40 * Owner->GetDPI()

namespace gd
{
	namespace pipe
	{
		CanvasMaterial::CanvasMaterial()
		{
			Type = PipelineItemType::CanvasMaterial;
			memset(ShaderPath, 0, sizeof(char) * MAX_PATH_LENGTH);
			m_shader = 0;
		}
		CanvasMaterial::~CanvasMaterial()
		{
			if (m_shader != 0)
				glDeleteShader(m_shader);
		}

		void CanvasMaterial::Bind()
		{
			// bind shaders
			glUseProgram(m_shader);

			glUniformMatrix4fv(m_projMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
			glUniformMatrix4fv(m_modelMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
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
						Owner->AddMessage(Owner->Messages, ed::plugin::MessageType::Error, Name, "Shader file doesn't exist");
				}
			}
			ImGui::NextColumn();


			ImGui::Columns(1);
		}

		void CanvasMaterial::Compile()
		{
			const char* vsCode = ResourceManager::Instance().GetDefaultCanvasVertexShader().c_str();
			const char* psCode = ResourceManager::Instance().GetDefaultCanvasPixelShader().c_str();

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

			glDeleteShader(canvasPS);
			glDeleteShader(canvasVS);

			m_projMatrixLoc = glGetUniformLocation(m_shader, "projection_matrix");
			m_modelMatrixLoc = glGetUniformLocation(m_shader, "modelview_matrix");
		}
	}
}