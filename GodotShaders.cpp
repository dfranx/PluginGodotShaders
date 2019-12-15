#include "GodotShaders.h"

namespace gd
{
	bool GodotShaders::Init() { return true; }
	void GodotShaders::OnEvent(void* e) { }
	void GodotShaders::Update(float delta) { }
	void GodotShaders::Destroy() { }

	void GodotShaders::CopyFilesOnSave(const char* dir) { } // TODO: copy all the .shader files
	bool GodotShaders::HasCustomMenu() { return false; }

	bool GodotShaders::HasMenuItems(const char* name) { return false; }
	void GodotShaders::ShowMenuItems(const char* name) { }

	bool GodotShaders::HasContextItems(const char* name) { return false; }
	void GodotShaders::ShowContextItems(const char* name, void* owner) { }

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
	bool GodotShaders::HasPipelineItemProperties(const char* type) { return false; }
	void GodotShaders::ShowPipelineItemProperties(const char* type, void* data) { }
	bool GodotShaders::IsPipelineItemPickable(const char* type) { return false; }
	bool GodotShaders::HasPipelineItemShaders(const char* type) { return false; }
	void GodotShaders::OpenPipelineItemInEditor(void* CodeEditor, const char* type, void* data) { }
	bool GodotShaders::CanPipelineItemHaveChild(const char* type, ed::plugin::PipelineItemType itemType) { return false; }
	int GodotShaders::GetPipelineItemInputLayoutSize(const char* itemName) { return 0; }
	void GodotShaders::GetPipelineItemInputLayoutItem(const char* itemName, int index, ed::plugin::InputLayoutItem& out) { }
	void GodotShaders::RemovePipelineItem(const char* itemName, const char* type, void* data) { }
	void GodotShaders::RenamePipelineItem(const char* oldName, const char* newName) { }
	bool GodotShaders::AddPipelineItemChild(const char* owner, const char* name, ed::plugin::PipelineItemType type, void* data) { return false; }
	bool GodotShaders::CanPipelineItemHaveChildren(const char* type) { return false; }
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