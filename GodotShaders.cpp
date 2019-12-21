#include "GodotShaders.h"
#include "Plugin/CanvasMaterial.h"
#include "Plugin/Sprite2D.h"
#include "UI/UIHelper.h"

#include "Plugin/ResourceManager.h"

#include <utility>
#include <sstream>
#include <fstream>
#include <string.h>
#include <glm/gtc/type_ptr.hpp>
#include "imgui/imgui.h"
#include "pugixml/src/pugixml.hpp"
#include "ghc/filesystem.hpp"


static const GLenum fboBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7, GL_COLOR_ATTACHMENT8, GL_COLOR_ATTACHMENT9, GL_COLOR_ATTACHMENT10, GL_COLOR_ATTACHMENT11, GL_COLOR_ATTACHMENT12, GL_COLOR_ATTACHMENT13, GL_COLOR_ATTACHMENT14, GL_COLOR_ATTACHMENT15 };

static const char* SLang_Keywords[] = {
	"shader_type", "render_mode", "hint_color", "hint_color", "hint_albedo", "hint_black_albedo", "hint_normal", "hint_white", "hint_black", "hint_aniso"
	
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

	void GodotShaders::m_addCanvasMaterial()
	{
		// initialize the data
		pipe::CanvasMaterial* data = new pipe::CanvasMaterial();

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
		pipe::Sprite2D* data = new pipe::Sprite2D();

		// generate name
		std::string name = "Sprite";
		for (size_t i = 0; /*BREAK WHEN WE FIND A NUMBER*/; i++) {
			name = "Sprite" + std::to_string(i);
			if (!ExistsPipelineItem(PipelineManager, name.c_str()))
				break;
		}

		// add the item
		void* ownerData = GetPipelineItem(PipelineManager, owner->Name);

		AddCustomPipelineItem(PipelineManager, ownerData, name.c_str(), ITEM_NAME_SPRITE2D, data, this);

		// add it to our local list
		strcpy(data->Name, name.c_str());
		data->Items.clear();
		data->Owner = this;

		data->SetTexture(tex);
	}

	bool GodotShaders::Init()
	{
		m_createSpritePopup = false;
		m_clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		m_fbo = 0;
		m_lastSize = glm::vec2(1, 1);
		ShaderPathsUpdated = false;
		m_varManagerOpened = false;
		m_editorCurrentID = 0;
		m_buildLangDefIdentifiers();

		return true;
	}
	void GodotShaders::OnEvent(void* e) { }
	void GodotShaders::Update(float delta)
	{
		// TODO: check every 500ms if ShaderPass is used -> push an error message if yes


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
				ImGui::Image((ImTextureID)(ResourceManager::Instance().EmptyTexture), ImVec2(64,64));
			else
				ImGui::Image((ImTextureID)GetFlippedTexture(ObjectManager, m_createSpriteTexture.c_str()), ImVec2(64, 64));

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
	void GodotShaders::Destroy() { }

	void GodotShaders::BeginRender()
	{
		GetViewportSize(m_rtSize.x, m_rtSize.y);
		if (m_lastSize != m_rtSize) {
			m_lastSize = m_rtSize;

			// create a FBO
			if (m_fbo == 0) {
				glGenFramebuffers(1, &m_fbo);
				glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GetWindowColorTexture(Renderer), 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, GetWindowDepthTexture(Renderer), 0);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

			// update canvas materials
			for (int i = 0; i < m_items.size(); i++) {
				if (m_items[i]->Type == PipelineItemType::CanvasMaterial) {
					pipe::CanvasMaterial* data = (pipe::CanvasMaterial*)m_items[i];
					data->SetViewportSize(m_rtSize.x, m_rtSize.y);
				}
			}
		}

		// bind fbo and buffers
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glDrawBuffers(1, fboBuffers);

		glStencilMask(0xFFFFFFFF);
		glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

		// bind RTs
		glClearBufferfv(GL_COLOR, 0, glm::value_ptr(m_clearColor));

		// update viewport value
		glViewport(0, 0, m_rtSize.x, m_rtSize.y);
	}
	void GodotShaders::EndRender()
	{
	}

	void GodotShaders::BeginProjectLoading()
	{
		m_loadTextures.clear();
		m_loadSizes.clear();
	}
	void GodotShaders::EndProjectLoading()
	{
		for (auto& k : m_loadTextures)
			k.first->SetTexture(k.second);
		for (auto& k : m_loadSizes)
			k.first->SetSize(k.second);


		for (auto& owner : m_items) {
			if (owner->Type == PipelineItemType::CanvasMaterial) {
				pipe::CanvasMaterial* canv = (pipe::CanvasMaterial*)owner;
				canv->SetViewportSize(m_rtSize.x, m_rtSize.y);
				canv->Compile();
			}
		}
	}
	void GodotShaders::BeginProjectSaving()
	{
		m_saveRequestedCopy = false;
	}
	void GodotShaders::EndProjectSaving()
	{

	}
	void GodotShaders::CopyFilesOnSave(const char* dir)
	{
		m_saveRequestedCopy = true;

		std::string ppath = std::string(dir) + "/shaders/";
		printf("[GSHADERS] Copying to %s\n", ppath.c_str());


		if (!ghc::filesystem::exists(ppath))
			ghc::filesystem::create_directories(ppath);

		char sPath[MAX_PATH_LENGTH];
		std::error_code errc;

		for (auto& item : m_items) {
			if (item->Type == PipelineItemType::CanvasMaterial) {
				pipe::CanvasMaterial* data = (pipe::CanvasMaterial*)item;
				GetProjectPath(Project, data->ShaderPath, sPath);
				ghc::filesystem::copy_file(sPath, ppath + std::string(item->Name) + ".shader", ghc::filesystem::copy_options::overwrite_existing, errc);
			}
		}
	}
	bool GodotShaders::HasCustomMenu() { return false; }

	bool GodotShaders::HasMenuItems(const char* name) { return false; }
	void GodotShaders::ShowMenuItems(const char* name) { }

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
		}
		// plugin item add
		else if (strcmp(name, "pluginitem_add") == 0) {
			const char* ownerType = (const char*)owner;
			if (strcmp(ownerType, ITEM_NAME_CANVAS_MATERIAL) == 0) {
				if (ImGui::Selectable("Create " ITEM_NAME_SPRITE2D)) {
					m_createSpritePopup = true;
					m_popupItem = (PipelineItem*)extraData;
				}
			}
		}
		// edit shader code
		else if (strcmp(name, "editcode") == 0) {
			if (ImGui::Selectable("Shader")) {
				pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)owner;
				OpenInCodeEditor(CodeEditor, GetPipelineItem(PipelineManager, odata->Name), odata->ShaderPath, m_editorCurrentID);
				m_editorID.push_back(m_editorCurrentID);
				m_editorOpened.push_back(odata->ShaderPath);
				m_editorCurrentID++;
			}
		}
	}

	// system variables (not needed for this plugin)
	bool GodotShaders::HasSystemVariables(ed::plugin::VariableType varType) { return false; }
	int GodotShaders::GetSystemVariableNameCount(ed::plugin::VariableType varType) { return 0; }
	const char* GodotShaders::GetSystemVariableName(ed::plugin::VariableType varType, int index) { return nullptr; }
	bool GodotShaders::HasLastFrame(char* name, ed::plugin::VariableType varType) { return false; }
	void GodotShaders::UpdateSystemVariableValue(char* data, char* name, ed::plugin::VariableType varType, bool isLastFrame) { }

	// functions (not needed for this plugin)
	bool GodotShaders::HasVariableFunctions(ed::plugin::VariableType vtype) { return false; }
	int GodotShaders::GetVariableFunctionNameCount(ed::plugin::VariableType vtype) { return 0; }
	const char* GodotShaders::GetVariableFunctionName(ed::plugin::VariableType varType, int index) { return nullptr; }
	bool GodotShaders::ShowFunctionArgumentEdit(char* fname, char* args, ed::plugin::VariableType vtype) { return false; }
	void GodotShaders::UpdateVariableFunctionValue(char* data, char* args, char* fname, ed::plugin::VariableType varType) { }
	int GodotShaders::GetVariableFunctionArgSpaceSize(char* fname, ed::plugin::VariableType varType) { return 0; }
	void GodotShaders::InitVariableFunctionArguments(char* args, char* fname, ed::plugin::VariableType vtype) { }
	const char* GodotShaders::ExportFunctionArguments(char* fname, ed::plugin::VariableType vtype, char* args) { return nullptr; }
	void GodotShaders::ImportFunctionArguments(char* fname, ed::plugin::VariableType vtype, char* args, const char* argsString) { }

	// object manager stuff
	bool GodotShaders::HasObjectPreview(const char* type) { return false; }
	void GodotShaders::ShowObjectPreview(const char* type, void* data, unsigned int id) { }
	bool GodotShaders::IsObjectBindable(const char* type) { return false; }
	bool GodotShaders::IsObjectBindableUAV(const char* type) { return false; }
	void GodotShaders::RemoveObject(const char* name, const char* type, void* data, unsigned int id) { }
	bool GodotShaders::HasObjectExtendedPreview(const char* type) { return false; }
	void GodotShaders::ShowObjectExtendedPreview(const char* type, void* data, unsigned int id) { }
	bool GodotShaders::HasObjectProperties(const char* type) { return false; }
	void GodotShaders::ShowObjectProperties(const char* type, void* data, unsigned int id) { }
	void GodotShaders::BindObject(const char* type, void* data, unsigned int id) { }
	const char* GodotShaders::ExportObject(char* type, void* data, unsigned int id) { return nullptr; }
	void GodotShaders::ImportObject(const char* name, const char* type, const char* argsString) { }
	bool GodotShaders::HasObjectContext(const char* type) { return false; }
	void GodotShaders::ShowObjectContext(const char* type, void* data) { }

	// pipeline item stuff
	bool GodotShaders::HasPipelineItemProperties(const char* type)
	{
		return strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0 ||
			strcmp(type, ITEM_NAME_SPRITE2D) == 0;
	}
	void GodotShaders::ShowPipelineItemProperties(const char* type, void* data)
	{
		if (strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0) {
			pipe::CanvasMaterial* item = (pipe::CanvasMaterial*)data;
			item->ShowProperties();
		}
		else if (strcmp(type, ITEM_NAME_SPRITE2D) == 0) {
			pipe::Sprite2D* item = (pipe::Sprite2D*)data;
			item->ShowProperties();
		}
	}
	bool GodotShaders::IsPipelineItemPickable(const char* type) { return false; }
	bool GodotShaders::HasPipelineItemShaders(const char* type)
	{
		return strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0;
	}
	void GodotShaders::OpenPipelineItemInEditor(void* CodeEditor, const char* type, void* data)
	{
		if (strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0) {
			pipe::CanvasMaterial* mat = (pipe::CanvasMaterial*)data;
			OpenInCodeEditor(CodeEditor, GetPipelineItem(PipelineManager, mat->Name), mat->ShaderPath, m_editorCurrentID);
			m_editorID.push_back(m_editorCurrentID);
			m_editorOpened.push_back(mat->ShaderPath);
			m_editorCurrentID++;

			printf("[GSHADERS] Opened %s's shader.\n", mat->Name);
		}
	}
	bool GodotShaders::CanPipelineItemHaveChild(const char* type, ed::plugin::PipelineItemType itemType)
	{
		// only allow GItems
		return strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0 && itemType == ed::plugin::PipelineItemType::PluginItem;
	}
	int GodotShaders::GetPipelineItemInputLayoutSize(const char* itemName) { return 0; }
	void GodotShaders::GetPipelineItemInputLayoutItem(const char* itemName, int index, ed::plugin::InputLayoutItem& out) { }
	void GodotShaders::RemovePipelineItem(const char* itemName, const char* type, void* data)
	{
		// delete allocated data
		for (size_t i = 0; i < m_items.size(); i++) {
			// check for main items
			if (strcmp(m_items[i]->Name, itemName) == 0) {
				for (size_t j = 0; j < m_items[i]->Items.size(); j++) {
					printf("[GSHADER] Deleting %s\n", m_items[i]->Items[j]->Name);
					delete m_items[i]->Items[j];
				}

				delete m_items[i];
				m_items.erase(m_items.begin() + i);

				printf("[GSHADER] Deleting %s\n", itemName);

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

						printf("[GSHADER] Deleting %s\n", itemName);

						break;
					}
				}
				if (found)
					break;
			}
		}
	}
	void GodotShaders::RenamePipelineItem(const char* oldName, const char* newName)
	{
		// update our local copy of pipeline items
		for (size_t i = 0; i < m_items.size(); i++) {
			// check for main items
			if (strcmp(m_items[i]->Name, oldName) == 0) {
				strcpy(m_items[i]->Name, newName);

				printf("[GSHADER] Renaming %s to %s\n", oldName, newName);

				break;
			}
			else {
				// check for children items
				bool found = false;
				for (size_t j = 0; j < m_items[i]->Items.size(); j++) {
					if (strcmp(m_items[i]->Items[j]->Name, oldName) == 0) {
						strcpy(m_items[i]->Items[j]->Name, newName);
						found = true;

						printf("[GSHADER] Renaming %s to %s\n", oldName, newName);

						break;
					}
				}
				if (found)
					break;
			}
		}
	}
	void GodotShaders::AddPipelineItemChild(const char* owner, const char* name, ed::plugin::PipelineItemType type, void* data)
	{
		for (int i = 0; i < m_items.size(); i++)
			if (strcmp(m_items[i]->Name, owner) == 0) {
				printf("[GSHADER] Added %s to %s\n", name, owner);
				m_items[i]->Items.push_back((PipelineItem*)data);
				break;
			}
	}
	bool GodotShaders::CanPipelineItemHaveChildren(const char* type)
	{
		return strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0;
	}
	void* GodotShaders::CopyPipelineItemData(const char* type, void* data)
	{
		if (strcmp(type, ITEM_NAME_SPRITE2D) == 0) {
			gd::pipe::Sprite2D* idata = (gd::pipe::Sprite2D*)data;
			gd::pipe::Sprite2D* newData = new gd::pipe::Sprite2D();

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
	void GodotShaders::ExecutePipelineItem(void* Owner, ed::plugin::PipelineItemType OwnerType, const char* type, void* data) {}
	void GodotShaders::ExecutePipelineItem(const char* type, void* data, void* children, int count)
	{
		if (strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0)
		{
			glDisable(GL_CULL_FACE);

			pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;
			odata->Bind();
			for (PipelineItem* item : odata->Items) {
				if (item->Type == PipelineItemType::Sprite2D) {
					pipe::Sprite2D* sprite = (pipe::Sprite2D*)item;
					odata->SetModelMatrix(sprite->GetMatrix());
					sprite->Draw();
				}
			}

			glEnable(GL_CULL_FACE);
		}
	}
	void GodotShaders::GetPipelineItemWorldMatrix(const char* name, float(&pMat)[16]) { }
	bool GodotShaders::IntersectPipelineItem(const char* type, void* data, const float* rayOrigin, const float* rayDir, float& hitDist) { return false; }
	void GodotShaders::GetPipelineItemBoundingBox(const char* name, float(&minPos)[3], float(&maxPos)[3]) { }
	bool GodotShaders::HasPipelineItemContext(const char* type)
	{
		return strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0;
	}
	void GodotShaders::ShowPipelineItemContext(const char* type, void* data)
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
	const char* GodotShaders::ExportPipelineItem(const char* type, void* data)
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

			pugi::xml_node uniformsNode = doc.append_child("uniforms");

			const auto& uniforms = mat->GetUniforms();
			for (const auto& u : uniforms) {
				pugi::xml_node uniformNode = uniformsNode.append_child("uniform");
				uniformNode.append_attribute("name").set_value(u.first.c_str());
				uniformNode.append_attribute("type").set_value(ShaderLanguage::get_datatype_name(u.second.Type).c_str());
				
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
			
			std::ostringstream oss;
			doc.print(oss);
			m_tempXML = oss.str();

			return m_tempXML.c_str();
		}
		else if (strcmp(type, ITEM_NAME_SPRITE2D) == 0) {
			pipe::Sprite2D* spr = (pipe::Sprite2D*)data;

			pugi::xml_document doc;
			doc.append_child("texture").text().set(spr->GetTexture().c_str());
			doc.append_child("width").text().set(spr->GetSize().x);
			doc.append_child("height").text().set(spr->GetSize().y);
			doc.append_child("x").text().set(spr->GetPosition().x);
			doc.append_child("y").text().set(spr->GetPosition().y);

			std::ostringstream oss;
			doc.print(oss);
			m_tempXML = oss.str();

			return m_tempXML.c_str();
		}

		return nullptr;
	}
	void* GodotShaders::ImportPipelineItem(const char* ownerName, const char* name, const char* type, const char* argsString)
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

			
			printf("[GSHADERS] Loading CanvasMaterial\n");
		}
		else if (strcmp(type, ITEM_NAME_SPRITE2D) == 0) {
			item = new pipe::Sprite2D();
			pipe::Sprite2D* spr = (pipe::Sprite2D*)item;

			float w = doc.child("width").text().as_float();
			float h = doc.child("height").text().as_float();
			float x = doc.child("x").text().as_float();
			float y = doc.child("y").text().as_float();

			spr->SetPosition(glm::vec2(x, y));

			m_loadSizes[spr] = glm::vec2(w, h);
			m_loadTextures[spr] = toGenericPath(doc.child("texture").text().as_string());
		}

		strcpy(item->Name, name);
		item->Items.clear();
		item->Owner = this;

		if (ownerName == nullptr)
			m_items.push_back(item);

		return (void*)item;
	}

	// options
	bool GodotShaders::HasSectionInOptions() { return false; }
	void GodotShaders::ShowOptions() { }

	// code editor
	void GodotShaders::m_buildLangDefIdentifiers()
	{
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
	void GodotShaders::SaveCodeEditorItem(const char* src, int srcLen, int sid)
	{
		for (int i = 0; i < m_editorID.size(); i++) {
			if (m_editorID[i] == sid) {
				char outPath[MAX_PATH_LENGTH] = { 0 };
				GetProjectPath(Project, m_editorOpened[i].c_str(), outPath);
				std::ofstream out(outPath);
				out.write(src, srcLen);
				out.close();
				break;
			}
		}
	}
	void GodotShaders::CloseCodeEditorItem(int sid)
	{
		for (int i = 0; i < m_editorID.size(); i++)
			if (m_editorID[i] == sid) {
				m_editorID.erase(m_editorID.begin() + i);
				m_editorOpened.erase(m_editorOpened.begin() + i);
				break;
			}
	}
	int GodotShaders::GetLanguageDefinitionKeywordCount(int sid)
	{
		return 155; // TODO: ew
	}
	const char** GodotShaders::GetLanguageDefinitionKeywords(int sid)
	{
		return SLang_Keywords;
	}
	int GodotShaders::GetLanguageDefinitionTokenRegexCount(int sid)
	{
		return 9; // TODO: ew
	}
	const char* GodotShaders::GetLanguageDefinitionTokenRegex(int index, ed::plugin::TextEditorPaletteIndex& palIndex, int sid)
	{
		// TODO: ew
		switch (index) {
		case 0:
			palIndex = ed::plugin::TextEditorPaletteIndex::Preprocessor;
			return "[ \\t]*#[ \\t]*[a-zA-Z_]+";
			break;
		case 1:
			palIndex = ed::plugin::TextEditorPaletteIndex::String;
			return "L?\\\"(\\\\.|[^\\\"])*\\\"";
			break;
		case 2:
			palIndex = ed::plugin::TextEditorPaletteIndex::CharLiteral;
			return "\\'\\\\?[^\\']\\'";
			break;
		case 3:
			palIndex = ed::plugin::TextEditorPaletteIndex::Number;
			return "[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?";
			break;
		case 4:
			palIndex = ed::plugin::TextEditorPaletteIndex::Number;
			return "[+-]?[0-9]+[Uu]?[lL]?[lL]?";
			break;
		case 5:
			palIndex = ed::plugin::TextEditorPaletteIndex::Number;
			return "0[0-7]+[Uu]?[lL]?[lL]?";
			break;
		case 6:
			palIndex = ed::plugin::TextEditorPaletteIndex::Number;
			return "0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?";
			break;
		case 7:
			palIndex = ed::plugin::TextEditorPaletteIndex::Identifier;
			return "[a-zA-Z_][a-zA-Z0-9_]*";
			break;
		case 8:
			palIndex = ed::plugin::TextEditorPaletteIndex::Punctuation;
			return "[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]";
			break;
		}

		return "";
	}
	int GodotShaders::GetLanguageDefinitionIdentifierCount(int sid)
	{
		return m_langDefIdentifiers.size();
	}
	const char* GodotShaders::GetLanguageDefinitionIdentifier(int index, int sid)
	{
		return m_langDefIdentifiers[index].first;
	}
	const char* GodotShaders::GetLanguageDefinitionIdentifierDesc(int index, int sid)
	{
		return m_langDefIdentifiers[index].second;
	}
	const char* GodotShaders::GetLanguageDefinitionCommentStart(int sid)
	{
		return "/*";
	}
	const char* GodotShaders::GetLanguageDefinitionCommentEnd(int sid)
	{
		return "*/";
	}
	const char* GodotShaders::GetLanguageDefinitionLineComment(int sid)
	{
		return "//";
	}
	bool GodotShaders::IsLanguageDefinitionCaseSensitive(int sid) { return true; }
	bool GodotShaders::GetLanguageDefinitionAutoIndent(int sid) { return true; }
	const char* GodotShaders::GetLanguageDefinitionName(int sid) { return "Godot"; }

	// misc
	bool GodotShaders::HandleDropFile(const char* filename) { return false; }
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
	int GodotShaders::GetShaderFilePathCount()
	{
		return m_items.size();
	}
	const char* GodotShaders::GetShaderFilePath(int index)
	{
		return ((pipe::CanvasMaterial*)m_items[index])->ShaderPath;
	}
	bool GodotShaders::HasShaderFilePathChanged()
	{
		return ShaderPathsUpdated;
	}
	void GodotShaders::UpdateShaderFilePath()
	{
		ShaderPathsUpdated = false;
	}
}