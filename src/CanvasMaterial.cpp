#include <Core/CanvasMaterial.h>
#include <Core/ResourceManager.h>
#include <Core/ShaderCompiler.h>
#include <PluginAPI/Plugin.h>
#include <UI/UIHelper.h>
#include "../GodotShaders.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

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

static const char* PixelDebugShaderCode = R"(
#version 330

uniform vec4 _sed_dbg_pixel_color;
out vec4 outColor;

void main()
{
	outColor = _sed_dbg_pixel_color;
}
)";

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
			m_vw = m_vh = 1.0f;
			m_modelMat = m_projMat = glm::mat4(1.0f);
			m_uniforms.clear();
			m_glslData.BlendMode = Shader::CanvasItem::BLEND_MODE_ADD;
			m_rtName = "";
			m_rt = m_fbo = 0;
		}
		CanvasMaterial::~CanvasMaterial()
		{
			if (m_shader != 0)
				glDeleteShader(m_shader);
		}
		void CanvasMaterial::SetRenderTexture(const std::string& rtName, unsigned int rt, unsigned int depth)
		{
			m_rtName = rtName;
			m_rt = rt;

			if (m_fbo != 0)
				glDeleteFramebuffers(1, &m_fbo);
			glGenFramebuffers(1, &m_fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rt, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		void CanvasMaterial::SetViewportSize(float w, float h)
		{
			m_vw = w;
			m_vh = h;
			m_projMat = glm::ortho(0.0f, w, h, 0.0f, 0.1f, 1000.0f);
		}
		void CanvasMaterial::Bind()
		{
			m_isDebug = false;

			// bind shaders
			if (!m_glslData.Error && m_glslData.SCREEN_TEXTURE)
			{
				ResourceManager::Instance().Copy(((gd::GodotShaders*)Owner)->GetColorBuffer(), m_fbo);

				glUseProgram(m_shader);
				glActiveTexture(GL_TEXTURE0 + 1);
				glBindTexture(GL_TEXTURE_2D, ResourceManager::Instance().SCREEN_TEXTURE());
				glUniform1i(glGetUniformLocation(m_shader, "SCREEN_TEXTURE"), 1);

				glUniform2f(m_pixelSizeLoc, 1.0f / m_vw, 1.0f/m_vh);

				
			}

			glUseProgram(m_shader);

			glUniformMatrix4fv(m_projMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));

			if (m_glslData.TIME)
				glUniform1f(m_timeLoc, Owner->GetTime());

			glUniform1i(glGetUniformLocation(m_shader, "TEXTURE"), 0);

			m_bindUniforms();
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
			ImGui::InputText("##pui_spath", ShaderPath, MAX_PATH_LENGTH);
			ImGui::PopItemFlag();
			ImGui::PopItemWidth();
			ImGui::SameLine();
			if (ImGui::Button("...##pui_sbtn", ImVec2(-1, 0)) && ((gd::GodotShaders*)Owner)->GetHostVersion() >= 2) 
				Owner->ImGuiFileDialogOpen("SelectGodotShaderDlg", "Select a shader", "Godot shaders (*.shader){.shader},.*");
			ImGui::NextColumn();



			ImGui::Text("Render texture:");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			if (ImGui::BeginCombo("##godot_canvas_rt", m_rtName.empty() ? "Window" : m_rtName.c_str())) {
				if (ImGui::Selectable("Window")) {
					SetRenderTexture("", Owner->GetWindowColorTexture(Owner->Renderer), Owner->GetWindowDepthTexture(Owner->Renderer));
					Owner->ModifyProject(Owner->Project);
				}
				ImGui::Separator();

				int ocnt = Owner->GetObjectCount(Owner->ObjectManager);
				for (int i = 0; i < ocnt; i++) {
					const char* oname = Owner->GetObjectName(Owner->ObjectManager, i);
					if (Owner->IsRenderTexture(Owner->ObjectManager, oname)) {
						if (ImGui::Selectable(oname)) {
							GLuint rt = Owner->GetTexture(Owner->ObjectManager, oname);
							GLuint depth = Owner->GetDepthTexture(Owner->ObjectManager, oname);
							SetRenderTexture(oname, rt, depth);
							Owner->ModifyProject(Owner->Project);
						}
					}
				}

				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
			ImGui::NextColumn();

			ImGui::Columns(1);


			// file dialogs
			if (((gd::GodotShaders*)Owner)->GetHostVersion() >= 2 && Owner->ImGuiFileDialogIsDone("SelectGodotShaderDlg")) {
				if (Owner->ImGuiFileDialogGetResult()) {
					char filePtr[512] = { 0 };
					Owner->ImGuiFileDialogGetPath(filePtr);

					char tempFile[MAX_PATH_LENGTH] = { 0 };
					Owner->GetRelativePath(Owner->Project, filePtr, tempFile);

					strcpy(ShaderPath, tempFile);
					Owner->ModifyProject(Owner->Project);

					if (Owner->FileExists(Owner->Project, tempFile)) {
						Owner->ClearMessageGroup(Owner->Messages, Name);
						Compile();
					}
					else
						Owner->AddMessage(Owner->Messages, ed::plugin::MessageType::Error, Name, "Shader file doesn't exist", -1);
					((gd::GodotShaders*)Owner)->ShaderPathsUpdated = true;
				}

				Owner->ImGuiFileDialogClose("SelectGodotShaderDlg");
			}
		}
		void CanvasMaterial::ShowVariableEditor()
		{
			ImGui::Columns(3);
			
			static bool firstTime = true;
			if (firstTime) {
				ImGui::SetColumnWidth(0, 150.0f);
				ImGui::SetColumnWidth(1, 90.0f);
				firstTime = false;
			}
			
			ImGui::Separator();
			for (auto& u : m_uniforms) {
				if (u.second.UIHidden)
					continue;

				ImGui::Text("%s", u.first.c_str());
				ImGui::NextColumn();

				ImGui::Text("%s", ShaderLanguage::get_datatype_name(u.second.Type).c_str());
				ImGui::NextColumn();

				if (UIHelper::ShowValueEditor(Owner, u.first, u.second))
					Owner->ModifyProject(Owner->Project);
				ImGui::NextColumn();
				ImGui::Separator();
			}

			ImGui::Columns(1);
		}

		void CanvasMaterial::DebugBind()
		{
			m_isDebug = true;

			// bind shader
			glUseProgram(m_debugShader);
			glUniformMatrix4fv(m_debugProjMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));
			if (m_glslData.TIME) glUniform1f(m_debugTimeLoc, Owner->GetTime());
			glUniform1i(glGetUniformLocation(m_debugShader, "TEXTURE"), 0);

			m_bindUniforms();
		}
		void CanvasMaterial::DebugSetID(int debugID)
		{
			float r = (debugID & 0x000000FF) / 255.0f;
			float g = ((debugID & 0x0000FF00) >> 8) / 255.0f;
			float b = ((debugID & 0x00FF0000) >> 16) / 255.0f;
			float a = 1.0f;
			GLuint dbgPxColor = glGetUniformLocation(m_debugShader, "_sed_dbg_pixel_color");

			glUniform4f(dbgPxColor, r, g, b, a);
		}

		void CanvasMaterial::SetModelMatrix(glm::mat4 mat)
		{
			m_modelMat = mat;

			GLuint loc = m_modelMatrixLoc;
			if (m_isDebug) loc = m_debugModelMatrixLoc;

			if (m_glslData.SkipVertexTransform)
				glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1000.0f))));
			else
				glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m_modelMat));
		}
		void CanvasMaterial::Compile()
		{
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
			Owner->ClearMessageGroup(Owner->Messages, Name);

			std::string vsCodeContent = ResourceManager::Instance().GetDefaultCanvasVertexShader();
			std::string psCodeContent = ResourceManager::Instance().GetDefaultCanvasPixelShader();

			auto unif = m_glslData.Uniforms;

			if (filesize != 0 && filedata != nullptr) {
				gd::ShaderTranscompiler::Transcompile(filedata, m_glslData);
				if (!m_glslData.Error) {
					vsCodeContent = m_glslData.Vertex;
					psCodeContent = m_glslData.Fragment;
				} else {
					Owner->AddMessage(Owner->Messages, ed::plugin::MessageType::Error, Name, m_glslData.ErrorMessage.c_str(), m_glslData.ErrorLine);
					return;
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

			// create a shader program
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

			// create debug pixel shader
			unsigned int debugPS = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(debugPS, 1, &PixelDebugShaderCode, nullptr);
			glCompileShader(debugPS);

			// create debug shader program
			m_debugShader = glCreateProgram();
			glAttachShader(m_debugShader, canvasVS);
			glAttachShader(m_debugShader, debugPS);
			glLinkProgram(m_debugShader);

			glDeleteShader(debugPS);
			glDeleteShader(canvasPS);
			glDeleteShader(canvasVS);

			gd::ShaderCompiler::CompileSourceToSPIRV(this->VSSPV, vsCodeContent, ed::plugin::ShaderStage::Vertex, "main");
			gd::ShaderCompiler::CompileSourceToSPIRV(this->PSSPV, psCodeContent, ed::plugin::ShaderStage::Pixel, "main");

			m_projMatrixLoc = glGetUniformLocation(m_shader, "PROJECTION_MATRIX");
			m_modelMatrixLoc = glGetUniformLocation(m_shader, "WORLD_MATRIX");
			m_timeLoc = glGetUniformLocation(m_shader, "TIME");
			m_pixelSizeLoc = glGetUniformLocation(m_shader, "SCREEN_PIXEL_SIZE");

			m_debugProjMatrixLoc = glGetUniformLocation(m_debugShader, "PROJECTION_MATRIX");
			m_debugModelMatrixLoc = glGetUniformLocation(m_debugShader, "WORLD_MATRIX");
			m_debugTimeLoc = glGetUniformLocation(m_debugShader, "TIME");
			m_debugPixelSizeLoc = glGetUniformLocation(m_debugShader, "SCREEN_PIXEL_SIZE");

			glUniform1i(glGetUniformLocation(m_shader, "TEXTURE"), 0); // color_texture -> texunit: 0
			glUniform1i(glGetUniformLocation(m_debugShader, "TEXTURE"), 0); // color_texture -> texunit: 0


			// user uniforms
			for (const auto& uniform : m_glslData.Uniforms) {
				Uniform* u = &m_uniforms[uniform.first];

				u->Location = glGetUniformLocation(m_shader, (/*"m_" + */uniform.first).c_str());
				u->DebugLocation = glGetUniformLocation(m_debugShader, (/*"m_" + */uniform.first).c_str());

				if (u->Type != uniform.second.type && u->Type != ShaderLanguage::TYPE_VOID)
					u->Value.resize(0);

				u->Type = uniform.second.type;

				bool isSampler = ShaderLanguage::is_sampler_type(u->Type);
				if (isSampler) {
					glUniform1i(u->Location, uniform.second.texture_order + 2);
					u->Location = uniform.second.texture_order + 2;
				}

				if (uniform.second.default_value.size() == 0 && u->Value.size() == 0) {
					u->Value.resize(ShaderLanguage::get_cardinality(uniform.second.type));
					for (auto& val : u->Value)
						val.sint = 0;

					if (isSampler)
						u->Value[0].uint = ResourceManager::Instance().WhiteTexture;
				}

				ShaderLanguage::DataType scalarType = ShaderLanguage::get_scalar_type(u->Type);
				bool equal = u->Value.size() == unif[uniform.first].default_value.size() && !isSampler;
				for (int i = 0; i < u->Value.size() && equal; i++) {
					if (scalarType == ShaderLanguage::DataType::TYPE_BOOL)
						equal = (u->Value[i].boolean == unif[uniform.first].default_value[i].boolean);
					else if (scalarType == ShaderLanguage::DataType::TYPE_FLOAT)
						equal = (u->Value[i].real == unif[uniform.first].default_value[i].real);
					else if (scalarType == ShaderLanguage::DataType::TYPE_INT)
						equal = (u->Value[i].sint == unif[uniform.first].default_value[i].sint);
					else if (scalarType == ShaderLanguage::DataType::TYPE_UINT)
						equal = (u->Value[i].uint == unif[uniform.first].default_value[i].uint);
				}

				if (u->Value.size() == 0 || equal)
					u->Value = uniform.second.default_value;

				memcpy(&u->HintRange[0], (void*)&uniform.second.hint_range[0], 3 * sizeof(float));
				u->HintType = uniform.second.hint;

				if (u->HintType == ShaderLanguage::ShaderNode::Uniform::HINT_NONE) {
					u->HintRange[0] = 0.0f;
					u->HintRange[1] = 0.0f;
					u->HintRange[2] = scalarType == ShaderLanguage::DataType::TYPE_FLOAT ? 0.01f : 1.0f;
				}
				else if (isSampler && u->HintType == ShaderLanguage::ShaderNode::Uniform::HINT_WHITE) {
					if (u->Value[0].uint == ResourceManager::Instance().BlackTexture)
						u->Value[0].uint = ResourceManager::Instance().WhiteTexture;
				}
				else if (isSampler && u->HintType == ShaderLanguage::ShaderNode::Uniform::HINT_BLACK) {
					if (u->Value[0].uint == ResourceManager::Instance().WhiteTexture)
						u->Value[0].uint = ResourceManager::Instance().BlackTexture;
				}
			}

			// erase non used uniforms
			std::vector<std::string> toBeErased;
			for (const auto& uniform : m_uniforms)
			{
				if (m_glslData.Uniforms.count(uniform.first) == 0)
					toBeErased.push_back(uniform.first);
			}

			for (const auto& uniform : toBeErased)
				m_uniforms.erase(uniform);
		}

		void CanvasMaterial::UpdateUniforms()
		{
			std::vector<ShaderLanguage::ConstantNode::Value> scrnPxSize(2), projMat(16), modelMat(16);

			scrnPxSize[0].real = 1.0f / m_vw;
			scrnPxSize[1].real = 1.0f / m_vh;

			for (int c = 0; c < 4; c++)
				for (int r = 0; r < 4; r++) {
					projMat[r * 4 + c].real = m_projMat[r][c];
					modelMat[r * 4 + c].real = m_modelMat[r][c];
				}

			m_uniforms["SCREEN_PIXEL_SIZE"].UIHidden = true;
			m_uniforms["SCREEN_PIXEL_SIZE"].Type = ShaderLanguage::DataType::TYPE_VEC2;
			m_uniforms["SCREEN_PIXEL_SIZE"].Value = scrnPxSize;

			m_uniforms["PROJECTION_MATRIX"].UIHidden = true;
			m_uniforms["PROJECTION_MATRIX"].Type = ShaderLanguage::DataType::TYPE_MAT4;
			m_uniforms["PROJECTION_MATRIX"].Value = projMat;

			m_uniforms["WORLD_MATRIX"].UIHidden = true;
			m_uniforms["WORLD_MATRIX"].Type = ShaderLanguage::DataType::TYPE_MAT4;
			m_uniforms["WORLD_MATRIX"].Value = modelMat;

			m_uniforms["TIME"].UIHidden = true;
			m_uniforms["TIME"].Type = ShaderLanguage::DataType::TYPE_FLOAT;
			m_uniforms["TIME"].Value.resize(1);
			m_uniforms["TIME"].Value[0].real = Owner->GetTime();
		}

		void CanvasMaterial::m_bindUniforms()
		{
			for (const auto& uniform : m_uniforms) {
				const auto& val = uniform.second.Value;
				
				unsigned int loc = uniform.second.Location;
				if (m_isDebug) loc = uniform.second.DebugLocation;

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
					glUniform1i(glGetUniformLocation(m_shader, (/*"m_" + */uniform.first).c_str()), loc);
					break;
				}
			}

			if (m_glslData.BlendMode == Shader::CanvasItem::BLEND_MODE_DISABLED) {
				glDisable(GL_BLEND);
			}
			else {
				glEnable(GL_BLEND);
				switch (m_glslData.BlendMode) {
					//-1 not handled because not blend is enabled anyway
				case Shader::CanvasItem::BLEND_MODE_MIX: {
					glBlendEquation(GL_FUNC_ADD);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				} break;
				case Shader::CanvasItem::BLEND_MODE_ADD: {
					glBlendEquation(GL_FUNC_ADD);
					glBlendFunc(GL_ONE, GL_ONE);
				} break;
				case Shader::CanvasItem::BLEND_MODE_SUB: {
					glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				} break;
				case Shader::CanvasItem::BLEND_MODE_MUL: {
					glBlendEquation(GL_FUNC_ADD);
					glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_ZERO, GL_ONE);
				} break;
				}
			}
		}
	}
}