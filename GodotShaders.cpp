﻿#include "GodotShaders.h"
#include <Core/ResourceManager.h>
#include <Core/CanvasMaterial.h>
#include <Core/BackBufferCopy.h>
#include <Core/Sprite.h>
#include <UI/UIHelper.h>


#include <utility>
#include <sstream>
#include <fstream>
#include <string.h>
#include <algorithm>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <pugixml/src/pugixml.hpp>
#include <glslang/Public/ShaderLang.h>

static const GLenum fboBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7, GL_COLOR_ATTACHMENT8, GL_COLOR_ATTACHMENT9, GL_COLOR_ATTACHMENT10, GL_COLOR_ATTACHMENT11, GL_COLOR_ATTACHMENT12, GL_COLOR_ATTACHMENT13, GL_COLOR_ATTACHMENT14, GL_COLOR_ATTACHMENT15 };

namespace gd
{
	std::string toGenericPath(const std::string& p)
	{
		std::string ret = p;
		std::replace(ret.begin(), ret.end(), '\\', '/');
		return ret;
	}
	ShaderLanguage::DataType toDataType(const std::string& name)
	{
		static std::unordered_map<std::string, ShaderLanguage::DataType> ret = {
			{ "void", ShaderLanguage::TYPE_VOID },
			{ "bool", ShaderLanguage::TYPE_BOOL },
			{ "bvec2", ShaderLanguage::TYPE_BVEC2 },
			{ "bvec3", ShaderLanguage::TYPE_BVEC3 },
			{ "bvec4", ShaderLanguage::TYPE_BVEC4 },
			{ "int", ShaderLanguage::TYPE_INT },
			{ "ivec2", ShaderLanguage::TYPE_IVEC2 },
			{ "ivec3", ShaderLanguage::TYPE_IVEC3 },
			{ "ivec4", ShaderLanguage::TYPE_IVEC4 },
			{ "uint", ShaderLanguage::TYPE_UINT },
			{ "uvec2", ShaderLanguage::TYPE_UVEC2 },
			{ "uvec3", ShaderLanguage::TYPE_UVEC3 },
			{ "uvec4", ShaderLanguage::TYPE_UVEC4 },
			{ "float", ShaderLanguage::TYPE_FLOAT },
			{ "vec2", ShaderLanguage::TYPE_VEC2 },
			{ "vec3", ShaderLanguage::TYPE_VEC3 },
			{ "vec4", ShaderLanguage::TYPE_VEC4 },
			{ "mat2", ShaderLanguage::TYPE_MAT2 },
			{ "mat3", ShaderLanguage::TYPE_MAT3 },
			{ "mat4", ShaderLanguage::TYPE_MAT4 },
			{ "sampler2D", ShaderLanguage::TYPE_SAMPLER2D },
			{ "isampler2D", ShaderLanguage::TYPE_ISAMPLER2D },
			{ "usampler2D", ShaderLanguage::TYPE_USAMPLER2D },
			{ "sampler2DArray", ShaderLanguage::TYPE_SAMPLER2DARRAY },
			{ "isampler2DArray", ShaderLanguage::TYPE_ISAMPLER2DARRAY },
			{ "usampler2DArray", ShaderLanguage::TYPE_USAMPLER2DARRAY },
			{ "sampler3D", ShaderLanguage::TYPE_SAMPLER3D },
			{ "isampler3D", ShaderLanguage::TYPE_ISAMPLER3D },
			{ "usampler3D", ShaderLanguage::TYPE_USAMPLER3D },
			{ "samplerCube", ShaderLanguage::TYPE_SAMPLERCUBE },
		};

		return ret.count(name)>0 ? ret[name] : ShaderLanguage::TYPE_VOID;
	}
	ed::plugin::VariableType convertGodotType(ShaderLanguage::DataType dtype)
	{
		switch (dtype) {
		case ShaderLanguage::DataType::TYPE_BOOL: return ed::plugin::VariableType::Boolean1; break;
		case ShaderLanguage::DataType::TYPE_BVEC2: return ed::plugin::VariableType::Boolean2; break;
		case ShaderLanguage::DataType::TYPE_BVEC3: return ed::plugin::VariableType::Boolean3; break;
		case ShaderLanguage::DataType::TYPE_BVEC4: return ed::plugin::VariableType::Boolean4; break;
		case ShaderLanguage::DataType::TYPE_INT: return ed::plugin::VariableType::Integer1; break;
		case ShaderLanguage::DataType::TYPE_IVEC2: return ed::plugin::VariableType::Integer2; break;
		case ShaderLanguage::DataType::TYPE_IVEC3: return ed::plugin::VariableType::Integer3; break;
		case ShaderLanguage::DataType::TYPE_IVEC4: return ed::plugin::VariableType::Integer4; break;
		case ShaderLanguage::DataType::TYPE_FLOAT: return ed::plugin::VariableType::Float1; break;
		case ShaderLanguage::DataType::TYPE_VEC2: return ed::plugin::VariableType::Float2; break;
		case ShaderLanguage::DataType::TYPE_VEC3: return ed::plugin::VariableType::Float3; break;
		case ShaderLanguage::DataType::TYPE_VEC4: return ed::plugin::VariableType::Float4; break;
		case ShaderLanguage::DataType::TYPE_MAT2: return ed::plugin::VariableType::Float2x2; break;
		case ShaderLanguage::DataType::TYPE_MAT3: return ed::plugin::VariableType::Float3x3; break;
		case ShaderLanguage::DataType::TYPE_MAT4: return ed::plugin::VariableType::Float4x4; break;
		}
		return ed::plugin::VariableType::Integer1;
	}
	void DebugDrawPrimitives(int vertexStart, int vertexCount, int maxVertexCount, int vertexStrip, GLuint topology, pipe::CanvasMaterial* canvas)
	{
		int actualVertexCount = vertexCount;
		while (vertexStart < maxVertexCount) {
			canvas->DebugSetID(vertexStart);

			actualVertexCount = std::min<int>(vertexCount, maxVertexCount - vertexStart);
			if (actualVertexCount <= 0) break;

			glDrawArrays(topology, vertexStart, actualVertexCount);
			vertexStart += vertexCount - vertexStrip;
		}
	}
	uint8_t* GetRawPixel(GLuint rt, uint8_t* data, int x, int y, int width)
	{
		glBindTexture(GL_TEXTURE_2D, rt);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glBindTexture(GL_TEXTURE_2D, 0);
		return &data[(x + y * width) * 4];
	}
	uint32_t GetPixelID(GLuint rt, uint8_t* data, int x, int y, int width)
	{
		uint8_t* pxData = GetRawPixel(rt, data, x, y, width);
		return ((uint32_t)pxData[0] << 0) | ((uint32_t)pxData[1] << 8) | ((uint32_t)pxData[2] << 16) | ((uint32_t)pxData[3] << 24);
	}

	void GodotShaders::m_addCanvasMaterial()
	{
		// initialize the data
		pipe::CanvasMaterial* data = new pipe::CanvasMaterial();

		data->SetRenderTexture("", GetWindowColorTexture(Renderer), GetWindowDepthTexture(Renderer));

		// generate name
		std::string name = "Material";
		for (size_t i = 0; /*BREAK WHEN WE FIND A NUMBER*/; i++) {
			name = "Material" + std::to_string(i);
			if (!ExistsPipelineItem(PipelineManager, name.c_str()))
				break;
		}

		// add the item
		AddCustomPipelineItem(PipelineManager, nullptr, name.c_str(), ITEM_NAME_CANVAS_MATERIAL, data, this);

		// add it to our local list
		strcpy(data->Name, name.c_str());
		data->Items.clear();
		data->Owner = this;
		m_items.push_back(data);

		data->SetViewportSize(m_rtSize.x, m_rtSize.y);
		data->Compile();
	}
	void GodotShaders::m_addSprite(pipe::CanvasMaterial* owner, const std::string& tex)
	{
		// initialize the data
		pipe::Sprite* data = new pipe::Sprite();

		// generate name
		std::string name = "Sprite";
		for (size_t i = 0; /*BREAK WHEN WE FIND A NUMBER*/; i++) {
			name = "Sprite" + std::to_string(i);
			if (!ExistsPipelineItem(PipelineManager, name.c_str()))
				break;
		}

		// add the item
		void* ownerData = GetPipelineItem(PipelineManager, owner->Name);

		AddCustomPipelineItem(PipelineManager, ownerData, name.c_str(), ITEM_NAME_SPRITE, data, this);

		// add it to our local list
		strcpy(data->Name, name.c_str());
		data->Items.clear();
		data->Owner = this;

		data->SetTexture(tex);
	}

	void GodotShaders::m_bindFBO(pipe::CanvasMaterial* canvas)
	{
		// bind fbo and buffers
		glBindFramebuffer(GL_FRAMEBUFFER, canvas->GetFBO());
		glDrawBuffers(1, fboBuffers);

		// update viewport value
		glViewport(0, 0, canvas->GetViewportSize().x, canvas->GetViewportSize().y);
	
		if (m_isRTCleared.count(canvas->GetRenderTexture()) == 0 || !m_isRTCleared[canvas->GetRenderTexture()]) {
			glStencilMask(0xFFFFFFFF);
			glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

			// bind RTs
			glClearBufferfv(GL_COLOR, 0, glm::value_ptr(m_clearColor));

			m_isRTCleared[canvas->GetRenderTexture()] = true;
		}
	}

	bool GodotShaders::Init(bool isWeb, int sedVersion)
	{
		m_createSpritePopup = false;
		m_clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		m_lastSize = glm::vec2(1, 1);
		ShaderPathsUpdated = false;
		m_varManagerOpened = false;
		m_lastErrorCheck = 0.0f;
		m_buildLangDefinition();

		glslang::InitializeProcess();

		if (sedVersion == 1003005)
			m_hostVersion = 1;
		else
			m_hostVersion = GetHostIPluginMaxVersion();

		return true;
	}
	void GodotShaders::InitUI(void* ctx)
	{
		ImGui::SetCurrentContext((ImGuiContext*)ctx);
	}
	void GodotShaders::Update(float delta)
	{
		if (GetTime() - m_lastErrorCheck > 0.75f) {
			ClearMessageGroup(Messages, "[GodotShaders]");
			if (m_items.size() > 0) {
				int pipeCount = GetPipelineItemCount(PipelineManager);
				for (int i = 0; i < pipeCount; i++) {
					void* pItem = GetPipelineItemByIndex(PipelineManager, i);
					ed::plugin::PipelineItemType type = GetPipelineItemType(pItem);

					if (type != ed::plugin::PipelineItemType::PluginItem)
						AddMessage(Messages, ed::plugin::MessageType::Error, "[GodotShaders]", "Not allowed to use ShaderPass alongside Godot's materials.", 0);
				}
			}

			m_lastErrorCheck = GetTime();
		}


		// ##### UNIFORM MANAGER POPUP #####
		if (m_varManagerOpened) {
			ImGui::OpenPopup("Uniforms##gshader_uniforms");
			m_varManagerOpened = false;
		}
		ImGui::SetNextWindowSize(ImVec2(700, 200), ImGuiCond_Once);
		if (ImGui::BeginPopupModal("Uniforms##gshader_uniforms")) {
			ImGui::Text("List of all uniforms:");
			((pipe::CanvasMaterial*)m_popupItem)->ShowVariableEditor();
			if (ImGui::Button("Ok"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}


		// ##### CREATE SPRITE POPUP #####
		if (m_createSpritePopup) {
			ImGui::OpenPopup("Create Sprite##create_godot_sprite");
			m_createSpriteTexture = "";
			m_createSpritePopup = false;
		}
		ImGui::SetNextWindowSize(ImVec2(430, 270), ImGuiCond_Once);
		if (ImGui::BeginPopupModal("Create Sprite##create_godot_sprite")) {
			ImGui::Text("Texture: "); ImGui::SameLine();
			if (ImGui::BeginCombo("##godot_sprite_texture", m_createSpriteTexture.empty() ? "EMPTY" : UIHelper::TrimFilename(m_createSpriteTexture).c_str())) {
				if (ImGui::Selectable("EMPTY"))
					m_createSpriteTexture = "";

				int ocnt = GetObjectCount(ObjectManager);
				for (int i = 0; i < ocnt; i++) {
					const char* oname = GetObjectName(ObjectManager, i);
					if (IsTexture(ObjectManager, oname)) {
						unsigned int texID = GetTexture(ObjectManager, oname);
						if (ImGui::Selectable(UIHelper::TrimFilename(oname).c_str()))
							m_createSpriteTexture = oname;
					}
				}

				ImGui::EndCombo();
			}

			if (m_createSpriteTexture.empty())
				UIHelper::TexturePreview(ResourceManager::Instance().EmptyTexture, 128, 128);
			else {
				int texpw, texph;
				GetTextureSize(ObjectManager, m_createSpriteTexture.c_str(), texpw, texph);
				UIHelper::TexturePreview(GetFlippedTexture(ObjectManager, m_createSpriteTexture.c_str()),texpw,texph);
			}

			if (ImGui::Button("Ok")) {
				m_addSprite((pipe::CanvasMaterial*)m_popupItem, m_createSpriteTexture);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}
	
	void GodotShaders::BeginRender()
	{
		ResourceManager::Instance().CopiedScreenTexture = false;

		int w = 1, h = 1;
		GetLastRenderSize(Renderer, w, h);
		m_rtSize.x = w;
		m_rtSize.y = h;
		if (m_lastSize != m_rtSize) {
			m_lastSize = m_rtSize;

			// update SCREEN_TEXTURE
			ResourceManager::Instance().ResizeResources(m_lastSize.x, m_lastSize.y);

			// update canvas materials
			for (int i = 0; i < m_items.size(); i++) {
				if (m_items[i]->Type == PipelineItemType::CanvasMaterial) {
					pipe::CanvasMaterial* data = (pipe::CanvasMaterial*)m_items[i];
					
					if (data->GetRenderTextureName().empty())
						data->SetViewportSize(m_rtSize.x, m_rtSize.y);
					else {
						int w = 1, h = 1;
						GetRenderTextureSize(ObjectManager, data->GetRenderTextureName().c_str(), w, h);
						data->SetViewportSize(w, h);
					}
				}
			}
		}

		for (auto& state : m_isRTCleared)
			state.second = false;
	}
	
	void GodotShaders::Project_BeginLoad()
	{
		m_items.clear();
		m_loadTextures.clear();
		m_loadSizes.clear();
		m_loadUniformTextures.clear();
		m_loadRTs.clear();
	}
	void GodotShaders::Project_EndLoad()
	{
		for (auto& k : m_loadTextures)
			k.first->SetTexture(k.second);
		for (auto& k : m_loadSizes)
			k.first->SetSize(k.second);

		for (auto& rt : m_loadRTs) {
			if (rt.second.empty())
				rt.first->SetRenderTexture("", GetWindowColorTexture(Renderer), GetWindowDepthTexture(Renderer));
			else {
				unsigned int rtID = GetTexture(ObjectManager, rt.second.c_str());
				unsigned int depthID = GetDepthTexture(ObjectManager, rt.second.c_str());
				rt.first->SetRenderTexture(rt.second, rtID, depthID);
			}
		}

		for (auto& k : m_loadUniformTextures) {
			std::vector<ShaderLanguage::ConstantNode::Value> value;
			ShaderLanguage::ConstantNode::Value aval;
			if (k.second.second.empty())
				aval.uint = ResourceManager::Instance().WhiteTexture;
			else {
				if (IsRenderTexture(ObjectManager, k.second.second.c_str())) {
					std::string txt = k.second.second;
					aval.uint = GetTexture(ObjectManager, txt.c_str());
				}
				else {
					std::string txt = toGenericPath(k.second.second);
					aval.uint = GetFlippedTexture(ObjectManager, txt.c_str());
				}
			}
			value.push_back(aval);
			((pipe::CanvasMaterial*)k.second.first)->SetUniform(k.first, value);
		}


		for (auto& owner : m_items) {
			if (owner->Type == PipelineItemType::CanvasMaterial) {
				pipe::CanvasMaterial* canv = (pipe::CanvasMaterial*)owner;
				canv->SetViewportSize(m_rtSize.x, m_rtSize.y);
				canv->Compile();
			}
		}
	}
	void GodotShaders::Project_BeginSave()
	{
		m_saveRequestedCopy = false;
	}
	void GodotShaders::Project_EndSave()
	{

	}
	void GodotShaders::Project_CopyFilesOnSave(const char* dir)
	{
		m_saveRequestedCopy = true;

		std::string ppath = std::string(dir) + "/shaders/";
		printf("[GSHADERS] Copying to %s\n", ppath.c_str());


		if (!std::filesystem::exists(ppath))
			std::filesystem::create_directories(ppath);

		char sPath[MAX_PATH_LENGTH];
		std::error_code errc;

		for (auto& item : m_items) {
			if (item->Type == PipelineItemType::CanvasMaterial) {
				pipe::CanvasMaterial* data = (pipe::CanvasMaterial*)item;
				GetProjectPath(Project, data->ShaderPath, sPath);
				std::filesystem::copy_file(sPath, ppath + std::string(item->Name) + ".shader", std::filesystem::copy_options::overwrite_existing, errc);
			}
		}
	}
	
	bool GodotShaders::HasContextItems(const char* name)
	{
		return strcmp(name, "pipeline") == 0 || strcmp(name, "pluginitem_add") == 0 ||
			strcmp(name, "editcode") == 0;
	}
	void GodotShaders::ShowContextItems(const char* name, void* owner, void* extraData)
	{
		// create pipeline item
		if (strcmp(name, "pipeline") == 0) {
			if (ImGui::Selectable("Create " ITEM_NAME_CANVAS_MATERIAL))
				m_addCanvasMaterial();
			if (ImGui::Selectable("Create " ITEM_NAME_BACKBUFFERCOPY)) {
				pipe::BackBufferCopy* data = new pipe::BackBufferCopy();
				std::string name = "";
				int i = 0;
				while (true) {
					name = "BackBufferCopy" + std::to_string(i);
					if (!ExistsPipelineItem(PipelineManager, name.c_str()))
						break;
					i++;
				}
				strcpy(data->Name, name.c_str());
				data->Items.clear();
				data->Owner = this;

				// add the item
				AddCustomPipelineItem(PipelineManager, nullptr, name.c_str(), ITEM_NAME_BACKBUFFERCOPY, data, this);

				m_items.push_back(data);
			}
		}
		// plugin item add
		else if (strcmp(name, "pluginitem_add") == 0) {
			const char* ownerType = (const char*)owner;
			if (strcmp(ownerType, ITEM_NAME_CANVAS_MATERIAL) == 0) {
				if (ImGui::Selectable("Create " ITEM_NAME_SPRITE)) {
					m_createSpritePopup = true;
					m_popupItem = (PipelineItem*)extraData;
				}
			}
		}
		// edit shader code
		else if (strcmp(name, "editcode") == 0) {
			if (ImGui::Selectable("Shader")) {
				pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)owner;
				OpenInCodeEditor(UI, GetPipelineItem(PipelineManager, odata->Name), odata->ShaderPath, (int)ed::plugin::ShaderStage::Vertex);
				m_editorOpened.push_back(odata->ShaderPath);
			}
		}
	}

	// pipeline item stuff
	bool GodotShaders::PipelineItem_HasProperties(const char* type, void* data)
	{
		return strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0 ||
			strcmp(type, ITEM_NAME_SPRITE) == 0 ||
			strcmp(type, ITEM_NAME_BACKBUFFERCOPY) == 0;
	}
	void GodotShaders::PipelineItem_ShowProperties(const char* type, void* data)
	{
		PipelineItem* item = (PipelineItem*)data;
		if (item->Type == PipelineItemType::CanvasMaterial)
			((pipe::CanvasMaterial*)item)->ShowProperties();
		else if (item->Type == PipelineItemType::Sprite)
			((pipe::Sprite*)item)->ShowProperties();
	}
	bool GodotShaders::PipelineItem_HasShaders(const char* type, void* data)
	{
		return strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0;
	}
	void GodotShaders::PipelineItem_OpenInEditor(const char* type, void* data)
	{
		if (strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0) {
			pipe::CanvasMaterial* mat = (pipe::CanvasMaterial*)data;
			OpenInCodeEditor(UI, GetPipelineItem(PipelineManager, mat->Name), mat->ShaderPath, (int)ed::plugin::ShaderStage::Vertex);
			m_editorOpened.push_back(mat->ShaderPath);

			printf("[GSHADERS] Opened %s's shader.\n", mat->Name);
		}
	}
	bool GodotShaders::PipelineItem_CanHaveChild(const char* type, void* data, ed::plugin::PipelineItemType itemType)
	{
		// only allow GItems
		return strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0 && itemType == ed::plugin::PipelineItemType::PluginItem;
	}
	int GodotShaders::PipelineItem_GetInputLayoutSize(const char* type, void* data)
	{
		return 3;
	}
	void GodotShaders::PipelineItem_GetInputLayoutItem(const char* type, void* data, int index, ed::plugin::InputLayoutItem& out)
	{
		if (index == 0)
			out.Value = ed::plugin::InputLayoutValue::Position;
		else if (index == 1)
			out.Value = ed::plugin::InputLayoutValue::Texcoord;
		else
			out.Value = ed::plugin::InputLayoutValue::Color;
	}
	void GodotShaders::PipelineItem_Remove(const char* itemName, const char* type, void* data)
	{
		// delete allocated data
		for (size_t i = 0; i < m_items.size(); i++) {
			// check for main items
			if (strcmp(m_items[i]->Name, itemName) == 0) {
				for (size_t j = 0; j < m_items[i]->Items.size(); j++) {
					printf("[GSHADERS] Deleting item %s\n", m_items[i]->Items[j]->Name);
					delete m_items[i]->Items[j];
				}

				delete m_items[i];
				m_items.erase(m_items.begin() + i);

				printf("[GSHADERS] Deleting %s\n", itemName);

				break;
			}
			else {
				// check for children items
				bool found = false;
				for (size_t j = 0; j < m_items[i]->Items.size(); j++) {
					if (strcmp(m_items[i]->Items[j]->Name, itemName) == 0) {
						delete m_items[i]->Items[j];
						m_items[i]->Items.erase(m_items[i]->Items.begin() + j);
						found = true;

						printf("[GSHADERS] Deleting item %s\n", itemName);

						break;
					}
				}
				if (found)
					break;
			}
		}
	}
	void GodotShaders::PipelineItem_Rename(const char* oldName, const char* newName)
	{
		// update our local copy of pipeline items
		for (size_t i = 0; i < m_items.size(); i++) {
			// check for main items
			if (strcmp(m_items[i]->Name, oldName) == 0) {
				strcpy(m_items[i]->Name, newName);

				printf("[GSHADERS] Renaming %s to %s\n", oldName, newName);

				break;
			}
			else {
				// check for children items
				bool found = false;
				for (size_t j = 0; j < m_items[i]->Items.size(); j++) {
					if (strcmp(m_items[i]->Items[j]->Name, oldName) == 0) {
						strcpy(m_items[i]->Items[j]->Name, newName);
						found = true;

						printf("[GSHADERS] Renaming %s to %s\n", oldName, newName);

						break;
					}
				}
				if (found)
					break;
			}
		}
	}
	void GodotShaders::PipelineItem_AddChild(const char* owner, const char* name, ed::plugin::PipelineItemType type, void* data)
	{
		for (int i = 0; i < m_items.size(); i++)
			if (strcmp(m_items[i]->Name, owner) == 0) {
				printf("[GSHADERS] Added %s to %s\n", name, owner);
				m_items[i]->Items.push_back((PipelineItem*)data);
				break;
			}
	}
	bool GodotShaders::PipelineItem_CanHaveChildren(const char* type, void* data)
	{
		return strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0;
	}
	void* GodotShaders::PipelineItem_CopyData(const char* type, void* data)
	{
		if (strcmp(type, ITEM_NAME_SPRITE) == 0) {
			gd::pipe::Sprite* idata = (gd::pipe::Sprite*)data;
			gd::pipe::Sprite* newData = new gd::pipe::Sprite();

			strcpy(newData->Name, idata->Name);
			newData->Owner = idata->Owner;
			newData->Items = idata->Items;
			newData->SetColor(idata->GetColor());
			newData->SetTexture(idata->GetTexture());
			newData->SetPosition(idata->GetPosition());
			newData->SetSize(idata->GetSize());

			return (void*)newData;
		}
		return nullptr;
	}
	void GodotShaders::PipelineItem_Execute(const char* type, void* data, void* children, int count)
	{
		PipelineItem* idata = (PipelineItem*)data;
		if (idata->Type == PipelineItemType::CanvasMaterial)
		{
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);

			pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;
			m_bindFBO(odata);
			odata->Bind();
			for (PipelineItem* item : odata->Items) {
				if (item->Type == PipelineItemType::Sprite) {
					pipe::Sprite* sprite = (pipe::Sprite*)item;
					odata->SetModelMatrix(sprite->GetMatrix());
					sprite->Draw();
				}
			}

			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
		}
		else if (idata->Type == PipelineItemType::BackBufferCopy)
		{
			ResourceManager::Instance().CopiedScreenTexture = false; // just reset the flag -> next shader (if any) that uses SCREEN_TEXTURE will copy the contents
		}
	}
	int GodotShaders::PipelineItem_DebugVertexExecute(const char* type, void* data, const char* childName, float rx, float ry, int vertexGroup)
	{
		if (vertexGroup == -1) {
			PipelineItem* idata = (PipelineItem*)data;
			if (idata->Type == PipelineItemType::CanvasMaterial)
			{
				glDisable(GL_CULL_FACE);
				glDisable(GL_DEPTH_TEST);

				pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;
				m_bindFBO(odata);
				odata->DebugBind();
				for (PipelineItem* item : odata->Items) {
					if (strcmp(item->Name, childName) != 0)
						continue;

					if (item->Type == PipelineItemType::Sprite) {
						pipe::Sprite* sprite = (pipe::Sprite*)item;
						odata->SetModelMatrix(sprite->GetMatrix());

						glBindVertexArray(sprite->GetVAO());
						DebugDrawPrimitives(0, 3, 6, 0, GL_TRIANGLES, odata);
					}
				}

				glEnable(GL_DEPTH_TEST);
				glEnable(GL_CULL_FACE);


				// window pixel color
				glm::vec2 rtSize = odata->GetViewportSize();
				int x = rx * rtSize.x;
				int y = ry * rtSize.y;
				uint8_t* mainPixelData = new uint8_t[(int)(rtSize.x * rtSize.y) * 4];
				int id = 0x00ffffff & GetPixelID(odata->GetRenderTexture(), mainPixelData, x, y, rtSize.x);
				delete[] mainPixelData;

				return id;
			}
		}
		return vertexGroup; // since there won't be many vertices, we can fetch vertex id in the first pass
	}
	int GodotShaders::PipelineItem_DebugInstanceExecute(const char* type, void* data, const char* childName, float rx, float ry, int vertexGroup)
	{
		return 0;
	}
	unsigned int GodotShaders::PipelineItem_GetVBO(const char* type, void* data)
	{
		PipelineItem* idata = (PipelineItem*)data;
		if (idata->Type == PipelineItemType::Sprite) {
			pipe::Sprite* odata = (pipe::Sprite*)data;
			return odata->GetVBO();
		}
		return 0;
	}
	unsigned int GodotShaders::PipelineItem_GetVBOStride(const char* type, void* data)
	{
		return 9;
	}
	bool GodotShaders::PipelineItem_HasContext(const char* type, void* data)
	{
		return strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0;
	}
	void GodotShaders::PipelineItem_ShowContext(const char* type, void* data)
	{
		if (strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0)
		{
			if (ImGui::Selectable("Compile"))
			{
				pipe::CanvasMaterial* idata = (pipe::CanvasMaterial*)data;
				idata->Compile();
			}
			if (ImGui::Selectable("Uniforms"))
			{
				m_varManagerOpened = true;
				m_popupItem = (PipelineItem*)data;
			}
		}
	}
	const char* GodotShaders::PipelineItem_Export(const char* type, void* data)
	{
		if (strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0) {
			pipe::CanvasMaterial* mat = (pipe::CanvasMaterial*)data;
			
			pugi::xml_document doc;

			std::string actualPath = mat->ShaderPath;
			if (m_saveRequestedCopy) {
				actualPath = std::string(GetProjectDirectory(Project)) + "/shaders/" + std::string(mat->Name) + ".shader";
				char outPath[MAX_PATH_LENGTH] = { 0 };
				GetRelativePath(Project, actualPath.c_str(), outPath);
				actualPath = outPath;
			}

			doc.append_child("path").text().set(actualPath.c_str());

			if (!mat->GetRenderTextureName().empty())
				doc.append_child("render_texture").text().set(mat->GetRenderTextureName().c_str());

			pugi::xml_node uniformsNode = doc.append_child("uniforms");

			const auto& uniforms = mat->GetUniforms();
			for (const auto& u : uniforms) {
				pugi::xml_node uniformNode = uniformsNode.append_child("uniform");
				uniformNode.append_attribute("name").set_value(u.first.c_str());
				uniformNode.append_attribute("type").set_value(ShaderLanguage::get_datatype_name(u.second.Type).c_str());

				if (ShaderLanguage::is_sampler_type(u.second.Type)) {
					int ocnt = GetObjectCount(ObjectManager);
					for (int i = 0; i < ocnt; i++) {
						const char* oname = GetObjectName(ObjectManager, i);
						if (IsTexture(ObjectManager, oname)) {
							if (u.second.Value[0].uint == GetFlippedTexture(ObjectManager, oname)) {
								uniformNode.append_child("value").text().set(oname);
								break;
							}
						}
						else if (IsRenderTexture(ObjectManager, oname)) {
							if (u.second.Value[0].uint == GetTexture(ObjectManager, oname)) {
								uniformNode.append_child("value").text().set(oname);
								break;
							}
						}
					}
				} else {
					ShaderLanguage::DataType scalarType = ShaderLanguage::get_scalar_type(u.second.Type);
					for (const auto& val : u.second.Value)
					{
						if (scalarType == ShaderLanguage::DataType::TYPE_BOOL)
							uniformNode.append_child("value").text().set(val.boolean);
						else if (scalarType == ShaderLanguage::DataType::TYPE_INT)
							uniformNode.append_child("value").text().set(val.sint);
						else if (scalarType == ShaderLanguage::DataType::TYPE_UINT)
							uniformNode.append_child("value").text().set(val.uint);
						else if (scalarType == ShaderLanguage::DataType::TYPE_FLOAT)
							uniformNode.append_child("value").text().set(val.real);
					}
				}
			}
			
			std::ostringstream oss;
			doc.print(oss);
			m_tempXML = oss.str();

			return m_tempXML.c_str();
		}
		else if (strcmp(type, ITEM_NAME_SPRITE) == 0) {
			pipe::Sprite* spr = (pipe::Sprite*)data;

			pugi::xml_document doc;
			doc.append_child("texture").text().set(spr->GetTexture().c_str());
			doc.append_child("width").text().set(spr->GetSize().x);
			doc.append_child("height").text().set(spr->GetSize().y);
			doc.append_child("x").text().set(spr->GetPosition().x);
			doc.append_child("y").text().set(spr->GetPosition().y);
			doc.append_child("rotation").text().set(spr->GetRotation());
			doc.append_child("fliph").text().set(spr->GetFlipHorizontal());
			doc.append_child("flipv").text().set(spr->GetFlipVertical());
			doc.append_child("visible").text().set(spr->IsVisible());
			doc.append_child("color_r").text().set(spr->GetColor().r);
			doc.append_child("color_g").text().set(spr->GetColor().g);
			doc.append_child("color_b").text().set(spr->GetColor().b);
			doc.append_child("color_a").text().set(spr->GetColor().a);

			std::ostringstream oss;
			doc.print(oss);
			m_tempXML = oss.str();

			return m_tempXML.c_str();
		}
		else if (strcmp(type, ITEM_NAME_BACKBUFFERCOPY) == 0) {
			pipe::BackBufferCopy* mat = (pipe::BackBufferCopy*)data;
			return "";
		}

		return nullptr;
	}
	void* GodotShaders::PipelineItem_Import(const char* ownerName, const char* name, const char* type, const char* argsString)
	{
		pugi::xml_document doc;
		doc.load_string(argsString);

		PipelineItem* item = nullptr;

		if (strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0) {
			item = new pipe::CanvasMaterial();
			pipe::CanvasMaterial* mat = (pipe::CanvasMaterial*)item;

			strcpy(mat->ShaderPath, doc.child("path").text().as_string());

			for (const auto& unode : doc.child("uniforms").children("uniform")) {
				std::string uname(unode.attribute("name").as_string());
				ShaderLanguage::DataType utype = toDataType(unode.attribute("type").as_string());
				ShaderLanguage::DataType scalarType = ShaderLanguage::get_scalar_type(utype);

				if (utype == ShaderLanguage::DataType::TYPE_VOID)
					continue;

				if (ShaderLanguage::is_sampler_type(utype)) {
					std::string txt = unode.child("value").text().as_string();
					m_loadUniformTextures[uname] = std::make_pair((PipelineItem*)mat, txt);
				} 
				else {
					std::vector<ShaderLanguage::ConstantNode::Value> value;
					for (const auto& vnode : unode.children("value"))
					{
						ShaderLanguage::ConstantNode::Value aval;
						if (scalarType == ShaderLanguage::DataType::TYPE_BOOL)
							aval.boolean = vnode.text().as_bool();
						else if (scalarType == ShaderLanguage::DataType::TYPE_INT)
							aval.sint = vnode.text().as_int();
						else if (scalarType == ShaderLanguage::DataType::TYPE_UINT)
							aval.uint = vnode.text().as_uint();
						else if (scalarType == ShaderLanguage::DataType::TYPE_FLOAT)
							aval.real = vnode.text().as_float();
						value.push_back(aval);
					}
					mat->SetUniform(uname, value);
				}
			}

			if (doc.child("render_texture")) {
				std::string rtName(doc.child("render_texture").text().get());
				m_loadRTs[mat] = rtName;
			}
			else
				m_loadRTs[mat] = "";

			printf("[GSHADERS] Loading CanvasMaterial\n");
		}
		else if (strcmp(type, ITEM_NAME_SPRITE) == 0) {
			item = new pipe::Sprite();
			pipe::Sprite* spr = (pipe::Sprite*)item;

			float w = doc.child("width").text().as_float();
			float h = doc.child("height").text().as_float();
			float x = doc.child("x").text().as_float();
			float y = doc.child("y").text().as_float();
			float rota = doc.child("rotation").text().as_float();
			bool fliph = doc.child("fliph").text().as_bool();
			bool flipv = doc.child("flipv").text().as_bool();
			bool visible = doc.child("visible").text().as_bool();
			float clr_r = doc.child("color_r").text().as_float();
			float clr_g = doc.child("color_g").text().as_float();
			float clr_b = doc.child("color_b").text().as_float();
			float clr_a = doc.child("color_a").text().as_float();

			spr->SetPosition(glm::vec2(x, y));
			spr->SetRotation(rota);
			spr->SetFlipHorizontal(fliph);
			spr->SetFlipVertical(flipv);
			spr->SetVisible(visible);
			spr->SetColor(glm::vec4(clr_r, clr_g, clr_b, clr_a));

			m_loadSizes[spr] = glm::vec2(w, h);
			m_loadTextures[spr] = toGenericPath(doc.child("texture").text().as_string());
		}
		else if (strcmp(type, ITEM_NAME_BACKBUFFERCOPY) == 0)
		{
			item = new pipe::BackBufferCopy();
		}

		strcpy(item->Name, name);
		item->Items.clear();
		item->Owner = this;

		if (ownerName == nullptr)
			m_items.push_back(item);

		return (void*)item;
	}
	void GodotShaders::PipelineItem_MoveDown(void* ownerData, const char* ownerType, const char* itemName)
	{
		PipelineItem* mat = (PipelineItem*)ownerData;
		if (strcmp(mat->Name, itemName) == 0) {
			// move the container up
			for (int i = 0; i < m_items.size(); i++) {
				if (strcmp(m_items[i]->Name, itemName) == 0) {
					PipelineItem* temp = m_items[i];
					m_items[i] = m_items[i + 1];
					m_items[i + 1] = temp;
					break;
				}
			}
		}
		else {
			for (int i = 0; i < mat->Items.size(); i++) {
				if (strcmp(mat->Items[i]->Name, itemName) == 0) {
					PipelineItem* temp = mat->Items[i];
					mat->Items[i] = mat->Items[i + 1];
					mat->Items[i + 1] = temp;
					break;
				}
			}
		}
	}
	void GodotShaders::PipelineItem_MoveUp(void* ownerData, const char* ownerType, const char* itemName)
	{
		PipelineItem* mat = (PipelineItem*)ownerData;
		if (strcmp(mat->Name, itemName) == 0) {
			// move the container up
			for (int i = 0; i < m_items.size(); i++) {
				if (strcmp(m_items[i]->Name, itemName) == 0) {
					PipelineItem* temp = m_items[i];
					m_items[i] = m_items[i - 1];
					m_items[i - 1] = temp;
					break;
				}
			}
		}
		else {
			for (int i = 0; i < mat->Items.size(); i++) {
				if (strcmp(mat->Items[i]->Name, itemName) == 0) {
					PipelineItem* temp = mat->Items[i];
					mat->Items[i] = mat->Items[i - 1];
					mat->Items[i - 1] = temp;
					break;
				}
			}
		}
	}
	bool GodotShaders::PipelineItem_IsDebuggable(const char* type, void* data)
	{
		return strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0;
	}
	bool GodotShaders::PipelineItem_IsStageDebuggable(const char* type, void* data, ed::plugin::ShaderStage stage)
	{
		PipelineItem* idata = (PipelineItem*)data;
		if (idata->Type == PipelineItemType::CanvasMaterial)
		{
			pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;
			if (stage == ed::plugin::ShaderStage::Pixel)
				return odata->IsFragmentShaderUsed();
			else if (stage == ed::plugin::ShaderStage::Vertex)
				return odata->IsVertexShaderUsed();
		}
		return false;
	}
	void GodotShaders::PipelineItem_DebugExecute(const char* type, void* data, void* children, int count, int* debugID)
	{
		PipelineItem* idata = (PipelineItem*)data;
		if (idata->Type == PipelineItemType::CanvasMaterial)
		{
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);

			pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;
			m_bindFBO(odata);
			odata->DebugBind();
			for (PipelineItem* item : odata->Items) {
				if (item->Type == PipelineItemType::Sprite) {
					odata->DebugSetID(*debugID);

					pipe::Sprite* sprite = (pipe::Sprite*)item;
					odata->SetModelMatrix(sprite->GetMatrix());
					sprite->Draw();
					(*debugID)++;
				}
			}

			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
		}
	}
	unsigned int GodotShaders::PipelineItem_GetTopology(const char* type, void* data)
	{
		return GL_TRIANGLES;
	}
	unsigned int GodotShaders::PipelineItem_GetVariableCount(const char* type, void* data)
	{
		PipelineItem* idata = (PipelineItem*)data;
		if (idata->Type == PipelineItemType::CanvasMaterial) {
			pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;
			return odata->GetUniforms().size();
		}
		return 0;
	}
	const char* GodotShaders::PipelineItem_GetVariableName(const char* type, void* data, unsigned int variable)
	{
		PipelineItem* idata = (PipelineItem*)data;
		if (idata->Type == PipelineItemType::CanvasMaterial) {
			pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;

			int i = 0;
			for (const auto& pair : odata->GetUniforms()) {
				if (i == variable)
					return pair.first.c_str();
				i++;
			}
		}
		return nullptr;
	}
	ed::plugin::VariableType GodotShaders::PipelineItem_GetVariableType(const char* type, void* data, unsigned int variable)
	{
		PipelineItem* idata = (PipelineItem*)data;
		if (idata->Type == PipelineItemType::CanvasMaterial) {
			pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;

			int i = 0;
			for (const auto& pair : odata->GetUniforms()) {
				if (i == variable)
					return convertGodotType(pair.second.Type);
				i++;
			}
		}
		return ed::plugin::VariableType::Integer1;
	}
	float GodotShaders::PipelineItem_GetVariableValueFloat(const char* type, void* data, unsigned int variable, int col, int row)
	{
		PipelineItem* idata = (PipelineItem*)data;
		if (idata->Type == PipelineItemType::CanvasMaterial) {
			pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;

			int i = 0;
			for (const auto& pair : odata->GetUniforms()) {
				if (i == variable) {
					int stride = 4;
					if (pair.second.Type == ShaderLanguage::DataType::TYPE_MAT2)
						stride = 2;
					if (pair.second.Type == ShaderLanguage::DataType::TYPE_MAT3)
						stride = 3;
					return pair.second.Value[row * stride + col].real;
				}
				i++;
			}
		}
		return 0.0f;
	}
	int GodotShaders::PipelineItem_GetVariableValueInteger(const char* type, void* data, unsigned int variable, int col)
	{
		PipelineItem* idata = (PipelineItem*)data;
		if (idata->Type == PipelineItemType::CanvasMaterial) {
			pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;

			int i = 0;
			for (const auto& pair : odata->GetUniforms()) {
				if (i == variable)
					return pair.second.Value[col].sint;
				i++;
			}
		}
		return 0;
	}
	bool GodotShaders::PipelineItem_GetVariableValueBoolean(const char* type, void* data, unsigned int variable, int col)
	{
		PipelineItem* idata = (PipelineItem*)data;
		if (idata->Type == PipelineItemType::CanvasMaterial) {
			pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;

			int i = 0;
			for (const auto& pair : odata->GetUniforms()) {
				if (i == variable)
					return pair.second.Value[col].boolean;
				i++;
			}
		}
		return 0;
	}
	unsigned int GodotShaders::PipelineItem_GetSPIRVSize(const char* type, void* data, ed::plugin::ShaderStage stage)
	{
		PipelineItem* idata = (PipelineItem*)data;
		if (idata->Type == PipelineItemType::CanvasMaterial) {
			pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;
			if (stage == ed::plugin::ShaderStage::Pixel)
				return odata->PSSPV.size();
			else
				return odata->VSSPV.size();
		}
		return 0;
	}
	unsigned int* GodotShaders::PipelineItem_GetSPIRV(const char* type, void* data, ed::plugin::ShaderStage stage)
	{
		PipelineItem* idata = (PipelineItem*)data;
		if (idata->Type == PipelineItemType::CanvasMaterial) {
			pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;
			if (stage == ed::plugin::ShaderStage::Pixel)
				return odata->PSSPV.data();
			else
				return odata->VSSPV.data();
		}
		return 0;
	}
	void GodotShaders::PipelineItem_DebugPrepareVariables(const char* type, void* data, const char* name)
	{
		PipelineItem* idata = (PipelineItem*)data;
		if (idata->Type == PipelineItemType::CanvasMaterial)
		{
			pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;
			for (PipelineItem* item : odata->Items) {
				if (strcmp(item->Name, name) != 0)
					continue;

				if (item->Type == PipelineItemType::Sprite) {
					pipe::Sprite* sprite = (pipe::Sprite*)item;
					odata->SetModelMatrix(sprite->GetMatrix());
					m_dbgTextureID = sprite->GetTextureID();
					m_dbgTexture = sprite->GetTexture();
					break;
				}
			}

			odata->UpdateUniforms();
		}
	}
	bool GodotShaders::PipelineItem_DebugUsesCustomTextures(const char* type, void* data)
	{
		return true;
	}
	unsigned int GodotShaders::PipelineItem_DebugGetTexture(const char* type, void* data, int loc, const char* variableName)
	{
		if (variableName != nullptr) {
			if (strcmp(variableName, "TEXTURE") == 0)
				return m_dbgTextureID;
			else if (strcmp(variableName, "SCREEN_TEXTURE") == 0)
				return ResourceManager::Instance().SCREEN_TEXTURE();
			else {
				PipelineItem* idata = (PipelineItem*)data;
				if (idata->Type == PipelineItemType::CanvasMaterial)
				{
					pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;
					const auto& unifs = odata->GetUniforms();
					for (const auto& entry : unifs)
						if (strcmp(entry.first.c_str(), variableName) == 0)
							return entry.second.Value[0].uint;
				}
			}
		}
		return 0;
	}
	void GodotShaders::PipelineItem_DebugGetTextureSize(const char* type, void* data, int loc, const char* variableName, int& x, int& y, int& z)
	{
		if (variableName != nullptr) {
			if (strcmp(variableName, "TEXTURE") == 0) {
				if (m_dbgTexture.empty()) {
					x = 128;
					y = 128;
				}
				else
					GetTextureSize(ObjectManager, m_dbgTexture.c_str(), x, y);
				z = 1;
			}
			else if (strcmp(variableName, "SCREEN_TEXTURE") == 0) {
				x = m_rtSize.x;
				y = m_rtSize.y;
				z = 1;
			}
			else {
				GLuint tex = 0;
				PipelineItem* idata = (PipelineItem*)data;
				if (idata->Type == PipelineItemType::CanvasMaterial)
				{
					pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;
					const auto& unifs = odata->GetUniforms();
					for (const auto& entry : unifs)
						if (strcmp(entry.first.c_str(), variableName) == 0) {
							tex = entry.second.Value[0].uint;
							break;
						}
				}

				int ocnt = GetObjectCount(ObjectManager);
				for (int i = 0; i < ocnt; i++) {
					const char* oname = GetObjectName(ObjectManager, i);
					if (IsTexture(ObjectManager, oname)) {
						if (tex == GetFlippedTexture(ObjectManager, oname)) {
							GetTextureSize(ObjectManager, oname, x, y);
							z = 1;
							break;
						}
					}
				}
			}
		}
	}

	// code editor
	void GodotShaders::m_buildLangDefinition()
	{
		// keywords
		m_langDefKeywords.clear();
		m_langDefKeywords = {
			"shader_type", "render_mode", "hint_color", "hint_range", "hint_albedo", "hint_black_albedo", "hint_normal", "hint_white", "hint_black", "hint_aniso", "discard",
			"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long", "register", "restrict", "return", "short",
			"signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary",
			"_Noreturn", "_Static_assert", "_Thread_local", "attribute", "uniform", "varying", "layout", "centroid", "flat", "smooth", "noperspective", "patch", "sample", "subroutine", "in", "out", "inout",
			"bool", "true", "false", "invariant", "mat2", "mat3", "mat4", "dmat2", "dmat3", "dmat4", "mat2x2", "mat2x3", "mat2x4", "dmat2x2", "dmat2x3", "dmat2x4", "mat3x2", "mat3x3", "mat3x4", "dmat3x2", "dmat3x3", "dmat3x4",
			"mat4x2", "mat4x3", "mat4x4", "dmat4x2", "dmat4x3", "dmat4x4", "vec2", "vec3", "vec4", "ivec2", "ivec3", "ivec4", "bvec2", "bvec3", "bvec4", "dvec2", "dvec3", "dvec4", "uint", "uvec2", "uvec3", "uvec4",
			"lowp", "mediump", "highp", "precision", "sampler1D", "sampler2D", "sampler3D", "samplerCube", "sampler1DShadow", "sampler2DShadow", "samplerCubeShadow", "sampler1DArray", "sampler2DArray", "sampler1DArrayShadow",
			"sampler2DArrayShadow", "isampler1D", "isampler2D", "isampler3D", "isamplerCube", "isampler1DArray", "isampler2DArray", "usampler1D", "usampler2D", "usampler3D", "usamplerCube", "usampler1DArray", "usampler2DArray",
			"sampler2DRect", "sampler2DRectShadow", "isampler2DRect", "usampler2DRect", "samplerBuffer", "isamplerBuffer", "usamplerBuffer", "sampler2DMS", "isampler2DMS", "usampler2DMS", "sampler2DMSArray", "isampler2DMSArray",
			"usampler2DMSArray", "samplerCubeArray", "samplerCubeArrayShadow", "isamplerCubeArray", "usamplerCubeArray"
		};

		// regex
		m_langDefRegex.clear();
		m_langDefRegex.push_back(std::make_pair("[ \\t]*#[ \\t]*[a-zA-Z_]+", ed::plugin::TextEditorPaletteIndex::Preprocessor));
		m_langDefRegex.push_back(std::make_pair("L?\\\"(\\\\.|[^\\\"])*\\\"", ed::plugin::TextEditorPaletteIndex::String));
		m_langDefRegex.push_back(std::make_pair("\\'\\\\?[^\\']\\'", ed::plugin::TextEditorPaletteIndex::CharLiteral));
		m_langDefRegex.push_back(std::make_pair("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", ed::plugin::TextEditorPaletteIndex::Number));
		m_langDefRegex.push_back(std::make_pair("[+-]?[0-9]+[Uu]?[lL]?[lL]?", ed::plugin::TextEditorPaletteIndex::Number));
		m_langDefRegex.push_back(std::make_pair("0[0-7]+[Uu]?[lL]?[lL]?", ed::plugin::TextEditorPaletteIndex::Number));
		m_langDefRegex.push_back(std::make_pair("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", ed::plugin::TextEditorPaletteIndex::Number));
		m_langDefRegex.push_back(std::make_pair("[a-zA-Z_][a-zA-Z0-9_]*", ed::plugin::TextEditorPaletteIndex::Identifier));
		m_langDefRegex.push_back(std::make_pair("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", ed::plugin::TextEditorPaletteIndex::Punctuation));

		// identifiers
		m_langDefIdentifiers.clear();
		m_langDefIdentifiers.push_back(std::make_pair("radians", "Converts x from degrees to radians."));
		m_langDefIdentifiers.push_back(std::make_pair("degrees", "Converts x from radians to degrees."));
		m_langDefIdentifiers.push_back(std::make_pair("sin", "Returns the sine of x"));
		m_langDefIdentifiers.push_back(std::make_pair("cos", "Returns the cosine of x."));
		m_langDefIdentifiers.push_back(std::make_pair("tan", "Returns the tangent of x"));
		m_langDefIdentifiers.push_back(std::make_pair("asin", "Returns the arcsine of each component of x."));
		m_langDefIdentifiers.push_back(std::make_pair("acos", "Returns the arccosine of each component of x."));
		m_langDefIdentifiers.push_back(std::make_pair("atan", "Returns the arctangent of x."));
		m_langDefIdentifiers.push_back(std::make_pair("sinh", "Returns the hyperbolic sine of x"));
		m_langDefIdentifiers.push_back(std::make_pair("cosh", "Returns the hyperbolic cosine of x."));
		m_langDefIdentifiers.push_back(std::make_pair("tanh", "Returns the hyperbolic tangent of x"));
		m_langDefIdentifiers.push_back(std::make_pair("asinh", "Returns the arc hyperbolic sine of x"));
		m_langDefIdentifiers.push_back(std::make_pair("acosh", "Returns the arc hyperbolic cosine of x."));
		m_langDefIdentifiers.push_back(std::make_pair("atanh", "Returns the arc hyperbolic tangent of x"));
		m_langDefIdentifiers.push_back(std::make_pair("pow", "Returns x^n."));
		m_langDefIdentifiers.push_back(std::make_pair("exp", "Returns the base-e exponent."));
		m_langDefIdentifiers.push_back(std::make_pair("exp2", "Base 2 exponent(per component)."));
		m_langDefIdentifiers.push_back(std::make_pair("log", "Returns the base-e logarithm of x."));
		m_langDefIdentifiers.push_back(std::make_pair("log2", "Returns the base - 2 logarithm of x."));
		m_langDefIdentifiers.push_back(std::make_pair("sqrt", "Square root (per component)."));
		m_langDefIdentifiers.push_back(std::make_pair("inversesqrt", "Returns rcp(sqrt(x))."));
		m_langDefIdentifiers.push_back(std::make_pair("abs", "Absolute value (per component)."));
		m_langDefIdentifiers.push_back(std::make_pair("sign", "Computes the sign of x."));
		m_langDefIdentifiers.push_back(std::make_pair("floor", "Returns the greatest integer which is less than or equal to x."));
		m_langDefIdentifiers.push_back(std::make_pair("trunc", "Truncates floating-point value(s) to integer value(s)"));
		m_langDefIdentifiers.push_back(std::make_pair("round", "Rounds x to the nearest integer"));
		m_langDefIdentifiers.push_back(std::make_pair("roundEven", "Returns a value equal to the nearest integer to x. A fractional part of 0.5 will round toward the nearest even integer."));
		m_langDefIdentifiers.push_back(std::make_pair("ceil", "Returns the smallest integer which is greater than or equal to x."));
		m_langDefIdentifiers.push_back(std::make_pair("fract", "Returns the fractional part of x."));
		m_langDefIdentifiers.push_back(std::make_pair("mod", "Modulus. Returns x – y ∗ floor (x/y)."));
		m_langDefIdentifiers.push_back(std::make_pair("modf", "Splits the value x into fractional and integer parts."));
		m_langDefIdentifiers.push_back(std::make_pair("max", "Selects the greater of x and y."));
		m_langDefIdentifiers.push_back(std::make_pair("min", "Selects the lesser of x and y."));
		m_langDefIdentifiers.push_back(std::make_pair("clamp", "Clamps x to the range [min, max]."));
		m_langDefIdentifiers.push_back(std::make_pair("mix", "Returns x*(1-a)+y*a."));
		m_langDefIdentifiers.push_back(std::make_pair("isinf", "Returns true if x is +INF or -INF, false otherwise."));
		m_langDefIdentifiers.push_back(std::make_pair("isnan", "Returns true if x is NAN or QNAN, false otherwise."));
		m_langDefIdentifiers.push_back(std::make_pair("smoothstep", "Returns a smooth Hermite interpolation between 0 and 1."));
		m_langDefIdentifiers.push_back(std::make_pair("step", "Returns (x >= a) ? 1 : 0"));
		m_langDefIdentifiers.push_back(std::make_pair("floatBitsToInt", "Returns a signed or unsigned integer value representing the encoding of a floating-point value. The floatingpoint value's bit-level representation is preserved."));
		m_langDefIdentifiers.push_back(std::make_pair("floatBitsToUint", "Returns a signed or unsigned integer value representing the encoding of a floating-point value. The floatingpoint value's bit-level representation is preserved."));
		m_langDefIdentifiers.push_back(std::make_pair("intBitsToFloat", "Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value."));
		m_langDefIdentifiers.push_back(std::make_pair("uintBitsToFloat", "Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value."));
		m_langDefIdentifiers.push_back(std::make_pair("fmod", "Returns the floating point remainder of x/y."));
		m_langDefIdentifiers.push_back(std::make_pair("fma", "Returns the double-precision fused multiply-addition of a * b + c."));
		m_langDefIdentifiers.push_back(std::make_pair("ldexp", "Returns x * 2exp"));
		m_langDefIdentifiers.push_back(std::make_pair("packUnorm2x16", "First, converts each component of the normalized floating - point value v into 8 or 16bit integer values. Then, the results are packed into the returned 32bit unsigned integer."));
		m_langDefIdentifiers.push_back(std::make_pair("packUnorm4x8", "First, converts each component of the normalized floating - point value v into 8 or 16bit integer values. Then, the results are packed into the returned 32bit unsigned integer."));
		m_langDefIdentifiers.push_back(std::make_pair("packSnorm4x8", "First, converts each component of the normalized floating - point value v into 8 or 16bit integer values. Then, the results are packed into the returned 32bit unsigned integer."));
		m_langDefIdentifiers.push_back(std::make_pair("unpackUnorm2x16", "First, unpacks a single 32bit unsigned integer p into a pair of 16bit unsigned integers, four 8bit unsigned integers, or four 8bit signed integers.Then, each component is converted to a normalized floating point value to generate the returned two or four component vector."));
		m_langDefIdentifiers.push_back(std::make_pair("unpackUnorm4x8", "First, unpacks a single 32bit unsigned integer p into a pair of 16bit unsigned integers, four 8bit unsigned integers, or four 8bit signed integers.Then, each component is converted to a normalized floating point value to generate the returned two or four component vector."));
		m_langDefIdentifiers.push_back(std::make_pair("unpackSnorm4x8", "First, unpacks a single 32bit unsigned integer p into a pair of 16bit unsigned integers, four 8bit unsigned integers, or four 8bit signed integers.Then, each component is converted to a normalized floating point value to generate the returned two or four component vector."));
		m_langDefIdentifiers.push_back(std::make_pair("packDouble2x32", "Returns a double-precision value obtained by packing the components of v into a 64-bit value."));
		m_langDefIdentifiers.push_back(std::make_pair("unpackDouble2x32", "Returns a two-component unsigned integer vector representation of v."));
		m_langDefIdentifiers.push_back(std::make_pair("length", "Returns the length of the vector v."));
		m_langDefIdentifiers.push_back(std::make_pair("distance", "Returns the distance between two points."));
		m_langDefIdentifiers.push_back(std::make_pair("dot", "Returns the dot product of two vectors."));
		m_langDefIdentifiers.push_back(std::make_pair("cross", "Returns the cross product of two 3D vectors."));
		m_langDefIdentifiers.push_back(std::make_pair("normalize", "Returns a normalized vector."));
		m_langDefIdentifiers.push_back(std::make_pair("faceforward", "Returns -n * sign(dot(i, ng))."));
		m_langDefIdentifiers.push_back(std::make_pair("reflect", "Returns a reflection vector."));
		m_langDefIdentifiers.push_back(std::make_pair("refract", "Returns the refraction vector."));
		m_langDefIdentifiers.push_back(std::make_pair("matrixCompMult", "Multiply matrix x by matrix y component-wise."));
		m_langDefIdentifiers.push_back(std::make_pair("outerProduct", "Linear algebraic matrix multiply c * r."));
		m_langDefIdentifiers.push_back(std::make_pair("transpose", "Returns the transpose of the matrix m."));
		m_langDefIdentifiers.push_back(std::make_pair("determinant", "Returns the determinant of the square matrix m."));
		m_langDefIdentifiers.push_back(std::make_pair("inverse", "Returns a matrix that is the inverse of m."));
		m_langDefIdentifiers.push_back(std::make_pair("lessThan", "Returns the component-wise compare of x < y"));
		m_langDefIdentifiers.push_back(std::make_pair("lessThanEqual", "Returns the component-wise compare of x <= y"));
		m_langDefIdentifiers.push_back(std::make_pair("greaterThan", "Returns the component-wise compare of x > y"));
		m_langDefIdentifiers.push_back(std::make_pair("greaterThanEqual", "Returns the component-wise compare of x >= y"));
		m_langDefIdentifiers.push_back(std::make_pair("equal", "Returns the component-wise compare of x == y"));
		m_langDefIdentifiers.push_back(std::make_pair("notEqual", "Returns the component-wise compare of x != y"));
		m_langDefIdentifiers.push_back(std::make_pair("any", "Test if any component of x is nonzero."));
		m_langDefIdentifiers.push_back(std::make_pair("all", "Test if all components of x are nonzero."));
		m_langDefIdentifiers.push_back(std::make_pair("not", "Returns the component-wise logical complement of x."));
		m_langDefIdentifiers.push_back(std::make_pair("uaddCarry", "Adds 32bit unsigned integer x and y, returning the sum modulo 2^32."));
		m_langDefIdentifiers.push_back(std::make_pair("usubBorrow", "Subtracts the 32bit unsigned integer y from x, returning the difference if non-negatice, or 2^32 plus the difference otherwise."));
		m_langDefIdentifiers.push_back(std::make_pair("umulExtended", "Multiplies 32bit integers x and y, producing a 64bit result."));
		m_langDefIdentifiers.push_back(std::make_pair("imulExtended", "Multiplies 32bit integers x and y, producing a 64bit result."));
		m_langDefIdentifiers.push_back(std::make_pair("bitfieldExtract", "Extracts bits [offset, offset + bits - 1] from value, returning them in the least significant bits of the result."));
		m_langDefIdentifiers.push_back(std::make_pair("bitfieldpush_back", "Returns the push_backion the bits leas-significant bits of push_back into base"));
		m_langDefIdentifiers.push_back(std::make_pair("bitfieldReverse", "Returns the reversal of the bits of value."));
		m_langDefIdentifiers.push_back(std::make_pair("bitCount", "Returns the number of bits set to 1 in the binary representation of value."));
		m_langDefIdentifiers.push_back(std::make_pair("findLSB", "Returns the bit number of the least significant bit set to 1 in the binary representation of value."));
		m_langDefIdentifiers.push_back(std::make_pair("findMSB", "Returns the bit number of the most significant bit in the binary representation of value."));
		m_langDefIdentifiers.push_back(std::make_pair("textureSize", "Returns the dimensions of level lod  (if present) for the texture bound to sample."));
		m_langDefIdentifiers.push_back(std::make_pair("textureQueryLod", "Returns the mipmap array(s) that would be accessed in the x component of the return value."));
		m_langDefIdentifiers.push_back(std::make_pair("texture", "Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler."));
		m_langDefIdentifiers.push_back(std::make_pair("textureProj", "Do a texture lookup with projection."));
		m_langDefIdentifiers.push_back(std::make_pair("textureLod", "Do a texture lookup as in texture but with explicit LOD."));
		m_langDefIdentifiers.push_back(std::make_pair("textureOffset", "Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel."));
		m_langDefIdentifiers.push_back(std::make_pair("texelFetch", "Use integer texture coordinate P to lookup a single texel from sampler."));
		m_langDefIdentifiers.push_back(std::make_pair("texelFetchOffset", "Fetch a single texel as in texelFetch offset by offset."));
		m_langDefIdentifiers.push_back(std::make_pair("texetureProjOffset", "Do a projective texture lookup as described in textureProj offset by offset as descrived in textureOffset."));
		m_langDefIdentifiers.push_back(std::make_pair("texetureLodOffset", "Do an offset texture lookup with explicit LOD."));
		m_langDefIdentifiers.push_back(std::make_pair("textureProjLod", "Do a projective texture lookup with explicit LOD."));
		m_langDefIdentifiers.push_back(std::make_pair("textureLodOffset", "Do an offset texture lookup with explicit LOD."));
		m_langDefIdentifiers.push_back(std::make_pair("textureProjLodOffset", "Do an offset projective texture lookup with explicit LOD."));
		m_langDefIdentifiers.push_back(std::make_pair("textureGrad", "Do a texture lookup as in texture but with explicit gradients."));
		m_langDefIdentifiers.push_back(std::make_pair("textureGradOffset", "Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset."));
		m_langDefIdentifiers.push_back(std::make_pair("textureProjGrad", "Do a texture lookup both projectively and with explicit gradient."));
		m_langDefIdentifiers.push_back(std::make_pair("textureProjGradOffset", "Do a texture lookup both projectively and with explicit gradient as well as with offset."));
		m_langDefIdentifiers.push_back(std::make_pair("textureGather", "Built-in function."));
		m_langDefIdentifiers.push_back(std::make_pair("textureGatherOffset", "Built-in function."));
		m_langDefIdentifiers.push_back(std::make_pair("textureGatherOffsets", "Built-in function."));
		m_langDefIdentifiers.push_back(std::make_pair("texture1D", "1D texture lookup."));
		m_langDefIdentifiers.push_back(std::make_pair("texture1DLod", "1D texture lookup with LOD."));
		m_langDefIdentifiers.push_back(std::make_pair("texture1DProj", "1D texture lookup with projective divide."));
		m_langDefIdentifiers.push_back(std::make_pair("texture1DProjLod", "1D texture lookup with projective divide and with LOD."));
		m_langDefIdentifiers.push_back(std::make_pair("texture2D", "2D texture lookup."));
		m_langDefIdentifiers.push_back(std::make_pair("texture2DLod", "2D texture lookup with LOD."));
		m_langDefIdentifiers.push_back(std::make_pair("texture2DProj", "2D texture lookup with projective divide."));
		m_langDefIdentifiers.push_back(std::make_pair("texture2DProjLod", "2D texture lookup with projective divide and with LOD."));
		m_langDefIdentifiers.push_back(std::make_pair("texture3D", "3D texture lookup."));
		m_langDefIdentifiers.push_back(std::make_pair("texture3DLod", "3D texture lookup with LOD."));
		m_langDefIdentifiers.push_back(std::make_pair("texture3DProj", "3D texture lookup with projective divide."));
		m_langDefIdentifiers.push_back(std::make_pair("texture3DProjLod", "3D texture lookup with projective divide and with LOD."));
		m_langDefIdentifiers.push_back(std::make_pair("textureCube", "Cube texture lookup."));
		m_langDefIdentifiers.push_back(std::make_pair("textureCubeLod", "Cube texture lookup with LOD."));
		m_langDefIdentifiers.push_back(std::make_pair("shadow1D", "1D texture lookup."));
		m_langDefIdentifiers.push_back(std::make_pair("shadow1DLod", "1D texture lookup with LOD."));
		m_langDefIdentifiers.push_back(std::make_pair("shadow1DProj", "1D texture lookup with projective divide."));
		m_langDefIdentifiers.push_back(std::make_pair("shadow1DProjLod", "1D texture lookup with projective divide and with LOD."));
		m_langDefIdentifiers.push_back(std::make_pair("shadow2D", "2D texture lookup."));
		m_langDefIdentifiers.push_back(std::make_pair("shadow2DLod", "2D texture lookup with LOD."));
		m_langDefIdentifiers.push_back(std::make_pair("shadow2DProj", "2D texture lookup with projective divide."));
		m_langDefIdentifiers.push_back(std::make_pair("shadow2DProjLod", "2D texture lookup with projective divide and with LOD."));
		m_langDefIdentifiers.push_back(std::make_pair("dFdx", "Returns the partial derivative of x with respect to the screen-space x-coordinate."));
		m_langDefIdentifiers.push_back(std::make_pair("dFdy", "Returns the partial derivative of x with respect to the screen-space y-coordinate."));
		m_langDefIdentifiers.push_back(std::make_pair("fwidth", "Returns abs(ddx(x)) + abs(ddy(x))"));
		m_langDefIdentifiers.push_back(std::make_pair("interpolateAtCentroid", "Return the value of the input varying interpolant sampled at a location inside the both the pixel and the primitive being processed."));
		m_langDefIdentifiers.push_back(std::make_pair("interpolateAtSample", "Return the value of the input varying interpolant at the location of sample number sample."));
		m_langDefIdentifiers.push_back(std::make_pair("interpolateAtOffset", "Return the value of the input varying interpolant sampled at an offset from the center of the pixel specified by offset."));
		m_langDefIdentifiers.push_back(std::make_pair("noise1", "Generates a random value"));
		m_langDefIdentifiers.push_back(std::make_pair("noise2", "Generates a random value"));
		m_langDefIdentifiers.push_back(std::make_pair("noise3", "Generates a random value"));
		m_langDefIdentifiers.push_back(std::make_pair("noise4", "Generates a random value"));
		m_langDefIdentifiers.push_back(std::make_pair("EmitStreamVertex", "Emit the current values of output variables to the current output primitive on stream stream."));
		m_langDefIdentifiers.push_back(std::make_pair("EndStreamPrimitive", "Completes the current output primitive on stream stream and starts a new one."));
		m_langDefIdentifiers.push_back(std::make_pair("EmitVertex", "Emit the current values to the current output primitive."));
		m_langDefIdentifiers.push_back(std::make_pair("EndPrimitive", "Completes the current output primitive and starts a new one."));
		m_langDefIdentifiers.push_back(std::make_pair("barrier", "For any given static instance of barrier(), all tessellation control shader invocations for a single input patch must enter it before any will be allowed to continue beyond it."));
	}
	void GodotShaders::CodeEditor_SaveItem(const char* src, int srcLen, const char* sid)
	{
		for (int i = 0; i < m_editorOpened.size(); i++) {
			if (m_editorOpened[i] == sid) {
				char outPath[MAX_PATH_LENGTH] = { 0 };
				GetProjectPath(Project, m_editorOpened[i].c_str(), outPath);
				std::ofstream out(outPath);
				out.write(src, srcLen);
				out.close();
				break;
			}
		}
	}
	void GodotShaders::CodeEditor_CloseItem(const char* sid)
	{
		for (int i = 0; i < m_editorOpened.size(); i++)
			if (m_editorOpened[i] == sid) {
				m_editorOpened.erase(m_editorOpened.begin() + i);
				break;
			}
	}
	int GodotShaders::LanguageDefinition_GetKeywordCount(int sid)
	{
		return m_langDefKeywords.size();
	}
	const char** GodotShaders::LanguageDefinition_GetKeywords(int sid)
	{
		return m_langDefKeywords.data();
	}
	int GodotShaders::LanguageDefinition_GetTokenRegexCount(int sid)
	{
		return m_langDefRegex.size();
	}
	const char* GodotShaders::LanguageDefinition_GetTokenRegex(int index, ed::plugin::TextEditorPaletteIndex& palIndex, int sid)
	{
		palIndex = m_langDefRegex[index].second;
		return m_langDefRegex[index].first;
	}
	int GodotShaders::LanguageDefinition_GetIdentifierCount(int sid)
	{
		return m_langDefIdentifiers.size();
	}
	const char* GodotShaders::LanguageDefinition_GetIdentifier(int index, int sid)
	{
		return m_langDefIdentifiers[index].first;
	}
	const char* GodotShaders::LanguageDefinition_GetIdentifierDesc(int index, int sid)
	{
		return m_langDefIdentifiers[index].second;
	}
	const char* GodotShaders::LanguageDefinition_GetCommentStart(int sid)
	{
		return "/*";
	}
	const char* GodotShaders::LanguageDefinition_GetCommentEnd(int sid)
	{
		return "*/";
	}
	const char* GodotShaders::LanguageDefinition_GetLineComment(int sid)
	{
		return "//";
	}
	bool GodotShaders::LanguageDefinition_IsCaseSensitive(int sid) { return true; }
	bool GodotShaders::LanguageDefinition_GetAutoIndent(int sid) { return true; }
	const char* GodotShaders::LanguageDefinition_GetName(int sid) { return "Godot"; }
	const char* GodotShaders::LanguageDefinition_GetNameAbbreviation(int id) { return "CM"; } // CM as in CanvasMaterial

	// misc
	void GodotShaders::HandleRecompile(const char* itemName)
	{
		for (auto& item : m_items)
		{
			if (item->Type == PipelineItemType::CanvasMaterial &&
				strcmp(item->Name, itemName) == 0)
			{
				gd::pipe::CanvasMaterial* data = (gd::pipe::CanvasMaterial*)item;
				data->Compile();
			}
		}
	}
	void GodotShaders::HandleRecompileFromSource(const char* itemName, int sid, const char* shaderCode, int shaderSize)
	{
		for (auto& item : m_items)
		{
			if (item->Type == PipelineItemType::CanvasMaterial &&
				strcmp(item->Name, itemName) == 0)
			{
				gd::pipe::CanvasMaterial* data = (gd::pipe::CanvasMaterial*)item;
				data->CompileFromSource(shaderCode, shaderSize);
			}
		}
	}
	void GodotShaders::HandleApplicationEvent(ed::plugin::ApplicationEvent event, void* data1, void* data2)
	{
		if (event == ed::plugin::ApplicationEvent::DebuggerStarted) {
			m_dbgEditor = nullptr;
			for (auto& item : m_items)
			{
				if (item->Type == PipelineItemType::CanvasMaterial &&
					strcmp(item->Name, (char*)data1) == 0)
				{
					m_dbgEditor = data2;
					DebuggerJump(Debugger, data2, 650001);
					DebuggerStep(Debugger, data2);
					break;
				}
			}
		} else if (event == ed::plugin::ApplicationEvent::DebuggerStepped) {
			if (m_dbgEditor) {
				if (DebuggerGetCurrentLine(Debugger) > 700000)
					DebuggerContinue(Debugger, m_dbgEditor);
			}
		}
	}
	int GodotShaders::ShaderFilePath_GetCount()
	{
		return m_items.size();
	}
	const char* GodotShaders::ShaderFilePath_Get(int index)
	{
		return ((pipe::CanvasMaterial*)m_items[index])->ShaderPath;
	}
	bool GodotShaders::ShaderFilePath_HasChanged()
	{
		return ShaderPathsUpdated;
	}
	void GodotShaders::ShaderFilePath_Update()
	{
		ShaderPathsUpdated = false;
	}
}