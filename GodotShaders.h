#pragma once
#include "PluginAPI/Plugin.h"
#include "Plugin/PipelineItem.h"
#include "Plugin/CanvasMaterial.h"

#include <vector>
#include <string>


namespace gd
{
	class GodotShaders : public ed::IPlugin
	{
	public:
		virtual bool Init();
		virtual void OnEvent(void* e);
		virtual void Update(float delta);
		virtual void Destroy();

		virtual void CopyFilesOnSave(const char* dir);
		virtual bool HasCustomMenu();

		virtual bool HasMenuItems(const char* name);
		virtual void ShowMenuItems(const char* name);

		virtual bool HasContextItems(const char* name);
		virtual void ShowContextItems(const char* name, void* owner = nullptr, void* extraData = nullptr);

		// system variables
		virtual bool HasSystemVariables(ed::plugin::VariableType varType);
		virtual int GetSystemVariableNameCount(ed::plugin::VariableType varType);
		virtual const char* GetSystemVariableName(ed::plugin::VariableType varType, int index);
		virtual bool HasLastFrame(char* name, ed::plugin::VariableType varType);
		virtual void UpdateSystemVariableValue(char* data, char* name, ed::plugin::VariableType varType, bool isLastFrame);

		// functions
		virtual bool HasVariableFunctions(ed::plugin::VariableType vtype);
		virtual int GetVariableFunctionNameCount(ed::plugin::VariableType vtype);
		virtual const char* GetVariableFunctionName(ed::plugin::VariableType varType, int index);
		virtual bool ShowFunctionArgumentEdit(char* fname, char* args, ed::plugin::VariableType vtype);
		virtual void UpdateVariableFunctionValue(char* data, char* args, char* fname, ed::plugin::VariableType varType);
		virtual int GetVariableFunctionArgSpaceSize(char* fname, ed::plugin::VariableType varType);
		virtual void InitVariableFunctionArguments(char* args, char* fname, ed::plugin::VariableType vtype);
		virtual const char* ExportFunctionArguments(char* fname, ed::plugin::VariableType vtype, char* args);
		virtual void ImportFunctionArguments(char* fname, ed::plugin::VariableType vtype, char* args, const char* argsString);

		// object manager stuff
		virtual bool HasObjectPreview(const char* type);
		virtual void ShowObjectPreview(const char* type, void* data, unsigned int id);
		virtual bool IsObjectBindable(const char* type);
		virtual bool IsObjectBindableUAV(const char* type);
		virtual void RemoveObject(const char* name, const char* type, void* data, unsigned int id);
		virtual bool HasObjectExtendedPreview(const char* type);
		virtual void ShowObjectExtendedPreview(const char* type, void* data, unsigned int id);
		virtual bool HasObjectProperties(const char* type);
		virtual void ShowObjectProperties(const char* type, void* data, unsigned int id);
		virtual void BindObject(const char* type, void* data, unsigned int id);
		virtual const char* ExportObject(char* type, void* data, unsigned int id);
		virtual void ImportObject(const char* name, const char* type, const char* argsString);
		virtual bool HasObjectContext(const char* type);
		virtual void ShowObjectContext(const char* type, void* data);

		// pipeline item stuff
		virtual bool HasPipelineItemProperties(const char* type);
		virtual void ShowPipelineItemProperties(const char* type, void* data);
		virtual bool IsPipelineItemPickable(const char* type);
		virtual bool HasPipelineItemShaders(const char* type);
		virtual void OpenPipelineItemInEditor(void* CodeEditor, const char* type, void* data);
		virtual bool CanPipelineItemHaveChild(const char* type, ed::plugin::PipelineItemType itemType);
		virtual int GetPipelineItemInputLayoutSize(const char* itemName);
		virtual void GetPipelineItemInputLayoutItem(const char* itemName, int index, ed::plugin::InputLayoutItem& out);
		virtual void RemovePipelineItem(const char* itemName, const char* type, void* data);
		virtual void RenamePipelineItem(const char* oldName, const char* newName);
		virtual void AddPipelineItemChild(const char* owner, const char* name, ed::plugin::PipelineItemType type, void* data);
		virtual bool CanPipelineItemHaveChildren(const char* type);
		virtual void* CopyPipelineItemData(const char* type, void* data);
		virtual void ExecutePipelineItem(void* Owner, ed::plugin::PipelineItemType OwnerType, const char* type, void* data);
		virtual void ExecutePipelineItem(const char* type, void* data, void* children, int count);
		virtual void GetPipelineItemWorldMatrix(const char* name, float (&pMat)[16]);
		virtual bool IntersectPipelineItem(const char* type, void* data, const float* rayOrigin, const float* rayDir, float& hitDist);
		virtual void GetPipelineItemBoundingBox(const char* name, float(&minPos)[3], float(&maxPos)[3]);
		virtual bool HasPipelineItemContext(const char* type);
		virtual void ShowPipelineItemContext(const char* type, void* data);
		virtual const char* ExportPipelineItem(const char* type, void* data);
		virtual void* ImportPipelineItem(const char* ownerName, const char* name, const char* type, const char* argsString);

		// options
		virtual bool HasSectionInOptions();
		virtual void ShowOptions();

		// misc
		virtual bool HandleDropFile(const char* filename);
	
	private:
		void m_addCanvasMaterial();
		void m_addSprite(pipe::CanvasMaterial* owner);
		
		std::vector<gd::PipelineItem*> m_items;
	};
}