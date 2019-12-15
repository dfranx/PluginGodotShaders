#include "GodotShaders.h"
#include "Plugin/CanvasMaterial.h"
#include "Plugin/Sprite2D.h"

#include <string.h>
#include "imgui/imgui.h"

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

		data->Compile();
	}
	void GodotShaders::m_addSprite(pipe::CanvasMaterial* owner)
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
	}

	bool GodotShaders::Init() { return true; }
	void GodotShaders::OnEvent(void* e) { }
	void GodotShaders::Update(float delta)
	{ 
		// TODO: check every 500ms if ShaderPass is used -> push an error message if yes
	}
	void GodotShaders::Destroy() { }

	void GodotShaders::CopyFilesOnSave(const char* dir) { } // TODO: copy all the .shader files
	bool GodotShaders::HasCustomMenu() { return false; }

	bool GodotShaders::HasMenuItems(const char* name) { return false; }
	void GodotShaders::ShowMenuItems(const char* name) { }

	bool GodotShaders::HasContextItems(const char* name)
	{ 
		return strcmp(name, "pipeline") == 0 || strcmp(name, "pluginitem_add") == 0;
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
					pipe::CanvasMaterial* ownerData = (pipe::CanvasMaterial*)extraData;
					m_addSprite(ownerData);
				}
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
		return strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0;
	}
	void GodotShaders::ShowPipelineItemProperties(const char* type, void* data)
	{ 
		if (strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0) {
			pipe::CanvasMaterial* item = (pipe::CanvasMaterial*)data;
			item->ShowProperties();
		}
	}
	bool GodotShaders::IsPipelineItemPickable(const char* type) { return false; }
	bool GodotShaders::HasPipelineItemShaders(const char* type) { return false; }
	void GodotShaders::OpenPipelineItemInEditor(void* CodeEditor, const char* type, void* data) { }
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
			if (strcmp(m_items[i]->Name, name) == 0) {
				m_items[i]->Items.push_back((PipelineItem*)data);
				break;
			}

		printf("[GSHADER] Added %s to %s\n", name, owner);
	}
	bool GodotShaders::CanPipelineItemHaveChildren(const char* type)
	{
		return strcmp(type, ITEM_NAME_CANVAS_MATERIAL) == 0;
	}
	void* GodotShaders::CopyPipelineItemData(const char* type, void* data) { return nullptr; }
	void GodotShaders::ExecutePipelineItem(void* Owner, ed::plugin::PipelineItemType OwnerType, const char* type, void* data) { }
	void GodotShaders::ExecutePipelineItem(const char* type, void* data, void* children, int count) { }
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