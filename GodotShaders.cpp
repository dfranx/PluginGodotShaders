#include "GodotShaders.h"
#include "Plugin/CanvasMaterial.h"
#include "Plugin/Sprite2D.h"

#include <string.h>
#include <glm/gtc/type_ptr.hpp>
#include "imgui/imgui.h"


static const GLenum fboBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7, GL_COLOR_ATTACHMENT8, GL_COLOR_ATTACHMENT9, GL_COLOR_ATTACHMENT10, GL_COLOR_ATTACHMENT11, GL_COLOR_ATTACHMENT12, GL_COLOR_ATTACHMENT13, GL_COLOR_ATTACHMENT14, GL_COLOR_ATTACHMENT15 };

namespace gd
{
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
		m_items.push_back(data);

		data->SetTexture(tex);
	}

	bool GodotShaders::Init()
	{ 
		m_createSpritePopup = false;
		m_clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		m_fbo = 0;
		m_lastSize = glm::vec2(1, 1);

		return true; 
	}
	void GodotShaders::OnEvent(void* e) { }
	void GodotShaders::Update(float delta)
	{ 
		// TODO: check every 500ms if ShaderPass is used -> push an error message if yes
	

		// ##### CREATE SPRITE POPUP #####
		if (m_createSpritePopup) {
			ImGui::OpenPopup("Create Sprite##create_godot_sprite");
			m_createSpriteTexture = "";
			m_createSpritePopup = false;
		}
		ImGui::SetNextWindowSize(ImVec2(330, 100), ImGuiCond_Once);
		if (ImGui::BeginPopupModal("Create Sprite##create_godot_sprite")) {
			ImGui::Text("Texture: "); ImGui::SameLine();
			if (ImGui::BeginCombo("##godot_sprite_texture", m_createSpriteTexture.empty() ? "EMPTY" : m_createSpriteTexture.c_str())) {
				if (ImGui::Selectable("EMPTY"))
					m_createSpriteTexture = "";

				int ocnt = GetObjectCount(ObjectManager);
				for (int i = 0; i < ocnt; i++) {
					const char* oname = GetObjectName(ObjectManager, i);
					if (IsTexture(ObjectManager, oname)) {
						unsigned int texID = GetTexture(ObjectManager, oname);
						if (ImGui::Selectable(oname))
							m_createSpriteTexture = oname;
					}
				}

				ImGui::EndCombo();
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
		glViewport(0, 0, m_rtSize.x, m_rtSize.x);
	}
	void GodotShaders::EndRender()
	{
	}

	void GodotShaders::CopyFilesOnSave(const char* dir) { } // TODO: copy all the .shader files
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
	bool GodotShaders::HasObjectProperties(const char* type) { return false;  }
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
		} else if (strcmp(type, ITEM_NAME_SPRITE2D) == 0) {
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
		printf("Tried to open %s's shader.", type);
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
				delete m_items[i];
				m_items.erase(m_items.begin() + i);

				printf("[GSHADER] Deleting %s\n", itemName);

				break;
			} else {
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
	void* GodotShaders::CopyPipelineItemData(const char* type, void* data) { return nullptr; }
	void GodotShaders::ExecutePipelineItem(void* Owner, ed::plugin::PipelineItemType OwnerType, const char* type, void* data) {}
	void GodotShaders::ExecutePipelineItem(const char* type, void* data, void* children, int count)
	{
		if (strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0)
		{
			pipe::CanvasMaterial* odata = (pipe::CanvasMaterial*)data;
			odata->Bind();
			for (PipelineItem* item : odata->Items) {
				if (item->Type == PipelineItemType::Sprite2D) {
					pipe::Sprite2D* sprite = (pipe::Sprite2D*)item;
					odata->SetModelMatrix(sprite->GetMatrix());
					sprite->Draw();
				}
			}
		}
	}
	void GodotShaders::GetPipelineItemWorldMatrix(const char* name, float (&pMat)[16]) { }
	bool GodotShaders::IntersectPipelineItem(const char* type, void* data, const float* rayOrigin, const float* rayDir, float& hitDist) { return false; }
	void GodotShaders::GetPipelineItemBoundingBox(const char* name, float(&minPos)[3], float(&maxPos)[3]) { }
	bool GodotShaders::HasPipelineItemContext(const char* type) { return false; }
	void GodotShaders::ShowPipelineItemContext(const char* type, void* data) { }
	const char* GodotShaders::ExportPipelineItem(const char* type, void* data) { return nullptr; }
	void* GodotShaders::ImportPipelineItem(const char* ownerName, const char* name, const char* type, const char* argsString) { return nullptr; }

	// options
	bool GodotShaders::HasSectionInOptions() { return false; }
	void GodotShaders::ShowOptions() { }

	// misc
	bool GodotShaders::HandleDropFile(const char* filename) { return false; }
}