#pragma once
#include <PluginAPI/Plugin.h>
#include <Core/Sprite.h>
#include <Core/PipelineItem.h>
#include <Core/CanvasMaterial.h>

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <GL/glew.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace gd
{
	class GodotShaders : public ed::IPlugin1
	{
	public:
		virtual bool Init(bool isWeb, int sedVersion);
		virtual void InitUI(void* ctx);
		virtual void OnEvent(void* e) { }
		virtual void Update(float delta);
		virtual void Destroy() { }

		virtual bool IsRequired() { return true; }
		virtual bool IsVersionCompatible(int version) { return true; }

		virtual void BeginRender();
		virtual void EndRender() { }

		virtual void Project_BeginLoad();
		virtual void Project_EndLoad();
		virtual void Project_BeginSave();
		virtual void Project_EndSave();
		virtual bool Project_HasAdditionalData() { return false; }
		virtual const char* Project_ExportAdditionalData() { return nullptr; }
		virtual void Project_ImportAdditionalData(const char* xml) {}
		virtual void Project_CopyFilesOnSave(const char* dir);

		/* list: file, newproject, project, createitem, window, custom */
		virtual bool HasCustomMenuItem() { return false; }
		virtual bool HasMenuItems(const char* name) { return false; }
		virtual void ShowMenuItems(const char* name) {}

		/* list: pipeline, shaderpass_add (owner = ShaderPass), pluginitem_add (owner = char* ItemType, extraData = PluginItemData) objects, editcode (owner = char* ItemName) */
		virtual bool HasContextItems(const char* name);
		virtual void ShowContextItems(const char* name, void* owner = nullptr, void* extraData = nullptr);

		// system variable methods
		virtual int SystemVariables_GetNameCount(ed::plugin::VariableType varType) { return 0; }
		virtual const char* SystemVariables_GetName(ed::plugin::VariableType varType, int index) { return 0; }
		virtual bool SystemVariables_HasLastFrame(char* name, ed::plugin::VariableType varType) { return 0; }
		virtual void SystemVariables_UpdateValue(char* data, char* name, ed::plugin::VariableType varType, bool isLastFrame) { }

		// function variables
		virtual int VariableFunctions_GetNameCount(ed::plugin::VariableType vtype) { return 0; }
		virtual const char* VariableFunctions_GetName(ed::plugin::VariableType varType, int index) { return 0; }
		virtual bool VariableFunctions_ShowArgumentEdit(char* fname, char* args, ed::plugin::VariableType vtype) { return 0; }
		virtual void VariableFunctions_UpdateValue(char* data, char* args, char* fname, ed::plugin::VariableType varType) { }
		virtual int VariableFunctions_GetArgsSize(char* fname, ed::plugin::VariableType varType) { return 0; }
		virtual void VariableFunctions_InitArguments(char* args, char* fname, ed::plugin::VariableType vtype) { }
		virtual const char* VariableFunctions_ExportArguments(char* fname, ed::plugin::VariableType vtype, char* args) { return 0; }
		virtual void VariableFunctions_ImportArguments(char* fname, ed::plugin::VariableType vtype, char* args, const char* argsString) { }

		// object manager stuff
		virtual bool Object_HasPreview(const char* type) { return 0; }
		virtual void Object_ShowPreview(const char* type, void* data, unsigned int id) { }
		virtual bool Object_IsBindable(const char* type) { return 0; }
		virtual bool Object_IsBindableUAV(const char* type) { return 0; }
		virtual void Object_Remove(const char* name, const char* type, void* data, unsigned int id) { }
		virtual bool Object_HasExtendedPreview(const char* type) { return 0; }
		virtual void Object_ShowExtendedPreview(const char* type, void* data, unsigned int id) { }
		virtual bool Object_HasProperties(const char* type) { return 0; }
		virtual void Object_ShowProperties(const char* type, void* data, unsigned int id) { }
		virtual void Object_Bind(const char* type, void* data, unsigned int id) { }
		virtual const char* Object_Export(char* type, void* data, unsigned int id) { return 0; }
		virtual void Object_Import(const char* name, const char* type, const char* argsString) { }
		virtual bool Object_HasContext(const char* type) { return 0; }
		virtual void Object_ShowContext(const char* type, void* data) { }


		// pipeline item stuff
		virtual bool PipelineItem_HasProperties(const char* type, void* data);
		virtual void PipelineItem_ShowProperties(const char* type, void* data);
		virtual bool PipelineItem_IsPickable(const char* type, void* data) { return 0; }
		virtual bool PipelineItem_HasShaders(const char* type, void* data);
		virtual void PipelineItem_OpenInEditor(const char* type, void* data);
		virtual bool PipelineItem_CanHaveChild(const char* type, void* data, ed::plugin::PipelineItemType itemType);
		virtual int PipelineItem_GetInputLayoutSize(const char* type, void* data);
		virtual void PipelineItem_GetInputLayoutItem(const char* type, void* data, int index, ed::plugin::InputLayoutItem& out);
		virtual void PipelineItem_Remove(const char* itemName, const char* type, void* data);
		virtual void PipelineItem_Rename(const char* oldName, const char* newName);
		virtual void PipelineItem_AddChild(const char* owner, const char* name, ed::plugin::PipelineItemType type, void* data);
		virtual bool PipelineItem_CanHaveChildren(const char* type, void* data);
		virtual void* PipelineItem_CopyData(const char* type, void* data);
		virtual void PipelineItem_Execute(void* Owner, ed::plugin::PipelineItemType OwnerType, const char* type, void* data) { }
		virtual void PipelineItem_Execute(const char* type, void* data, void* children, int count);
		virtual void PipelineItem_GetWorldMatrix(const char* type, void* data, float(&pMat)[16]) { }
		virtual bool PipelineItem_Intersect(const char* type, void* data, const float* rayOrigin, const float* rayDir, float& hitDist) { return 0; }
		virtual void PipelineItem_GetBoundingBox(const char* type, void* data, float(&minPos)[3], float(&maxPos)[3]) { }
		virtual bool PipelineItem_HasContext(const char* type, void* data);
		virtual void PipelineItem_ShowContext(const char* type, void* data);
		virtual const char* PipelineItem_Export(const char* type, void* data);
		virtual void* PipelineItem_Import(const char* ownerName, const char* name, const char* type, const char* argsString);
		virtual void PipelineItem_MoveDown(void* ownerData, const char* ownerType, const char* itemName);
		virtual void PipelineItem_MoveUp(void* ownerData, const char* ownerType, const char* itemName);
		virtual void PipelineItem_ApplyGizmoTransform(const char* type, void* data, float* transl, float* scale, float* rota) { }
		virtual void PipelineItem_GetTransform(const char* type, void* data, float* transl, float* scale, float* rota) { }
		virtual void PipelineItem_DebugVertexExecute(void* Owner, ed::plugin::PipelineItemType OwnerType, const char* type, void* data, unsigned int colorVarLoc) { }
		virtual int PipelineItem_DebugVertexExecute(const char* type, void* data, const char* childName, float rx, float ry, int vertexGroup);
		virtual void PipelineItem_DebugInstanceExecute(void* Owner, ed::plugin::PipelineItemType OwnerType, const char* type, void* data, unsigned int colorVarLoc) { }
		virtual int PipelineItem_DebugInstanceExecute(const char* type, void* data, const char* childName, float rx, float ry, int vertexGroup);
		virtual unsigned int PipelineItem_GetVBO(const char* type, void* data);
		virtual unsigned int PipelineItem_GetVBOStride(const char* type, void* data);
		virtual bool PipelineItem_CanChangeVariables(const char* type, void* data) { return 0; }
		virtual bool PipelineItem_IsDebuggable(const char* type, void* data);
		virtual bool PipelineItem_IsStageDebuggable(const char* type, void* data, ed::plugin::ShaderStage stage);
		virtual void PipelineItem_DebugExecute(const char* type, void* data, void* children, int count, int* debugID);
		virtual unsigned int PipelineItem_GetTopology(const char* type, void* data);
		virtual unsigned int PipelineItem_GetVariableCount(const char* type, void* data);
		virtual const char* PipelineItem_GetVariableName(const char* type, void* data, unsigned int variable);
		virtual ed::plugin::VariableType PipelineItem_GetVariableType(const char* type, void* data, unsigned int variable);
		virtual float PipelineItem_GetVariableValueFloat(const char* type, void* data, unsigned int variable, int col, int row);
		virtual int PipelineItem_GetVariableValueInteger(const char* type, void* data, unsigned int variable, int col);
		virtual bool PipelineItem_GetVariableValueBoolean(const char* type, void* data, unsigned int variable, int col);
		virtual unsigned int PipelineItem_GetSPIRVSize(const char* type, void* data, ed::plugin::ShaderStage stage);
		virtual unsigned int* PipelineItem_GetSPIRV(const char* type, void* data, ed::plugin::ShaderStage stage);
		virtual void PipelineItem_DebugPrepareVariables(const char* type, void* data, const char* name);
		virtual bool PipelineItem_DebugUsesCustomTextures(const char* type, void* data);
		virtual unsigned int PipelineItem_DebugGetTexture(const char* type, void* data, int loc, const char* variableName);
		virtual void PipelineItem_DebugGetTextureSize(const char* type, void* data, int loc, const char* variableName, int& x, int& y, int& z);

		// options
		virtual bool Options_HasSection() { return false; }
		virtual void Options_RenderSection() { }
		virtual void Options_Parse(const char* key, const char* val) { }
		virtual int Options_GetCount() { return 0; }
		virtual const char* Options_GetKey(int index) { return 0; }
		virtual const char* Options_GetValue(int index) { return 0; }

		// languages
		virtual int CustomLanguage_GetCount() { return 0; }
		virtual const char* CustomLanguage_GetName(int langID) { return 0; }
		virtual const unsigned int* CustomLanguage_CompileToSPIRV(int langID, const char* src, size_t src_len, ed::plugin::ShaderStage stage, const char* entry, ed::plugin::ShaderMacro* macros, size_t macroCount, size_t* spv_length, bool* compiled) { return 0; }
		virtual const char* CustomLanguage_ProcessGeneratedGLSL(int langID, const char* src) { return 0; }
		virtual bool CustomLanguage_SupportsAutoUniforms(int langID) { return 0; }
		virtual bool CustomLanguage_IsDebuggable(int langID) { return 0; }
		virtual const char* CustomLanguage_GetDefaultExtension(int langID) { return 0; }

		// language text editor
		virtual bool ShaderEditor_Supports(int langID) { return 0; }
		virtual void ShaderEditor_Open(int langID, int editorID, const char* data, int dataLen) { }
		virtual void ShaderEditor_Render(int langID, int editorID) { }
		virtual void ShaderEditor_Close(int langID, int editorID) { }
		virtual const char* ShaderEditor_GetContent(int langID, int editorID, size_t* dataLength) { return 0; }
		virtual bool ShaderEditor_IsChanged(int langID, int editorID) { return 0; }
		virtual void ShaderEditor_ResetChangeState(int langID, int editorID) { }
		virtual bool ShaderEditor_CanUndo(int langID, int editorID) { return 0; }
		virtual bool ShaderEditor_CanRedo(int langID, int editorID) { return 0; }
		virtual void ShaderEditor_Undo(int langID, int editorID) { }
		virtual void ShaderEditor_Redo(int langID, int editorID) { }
		virtual void ShaderEditor_Cut(int langID, int editorID) { }
		virtual void ShaderEditor_Paste(int langID, int editorID) { }
		virtual void ShaderEditor_Copy(int langID, int editorID) { }
		virtual void ShaderEditor_SelectAll(int langID, int editorID) { }
		virtual bool ShaderEditor_HasStats(int langID, int editorID) { return 0; }

		// code editor
		virtual void CodeEditor_SaveItem(const char* src, int srcLen, const char* path);
		virtual void CodeEditor_CloseItem(const char* path);
		virtual bool LanguageDefinition_Exists(int id) { return true; }
		virtual int LanguageDefinition_GetKeywordCount(int id);
		virtual const char** LanguageDefinition_GetKeywords(int id);
		virtual int LanguageDefinition_GetTokenRegexCount(int id);
		virtual const char* LanguageDefinition_GetTokenRegex(int index, ed::plugin::TextEditorPaletteIndex& palIndex, int id);
		virtual int LanguageDefinition_GetIdentifierCount(int id);
		virtual const char* LanguageDefinition_GetIdentifier(int index, int id);
		virtual const char* LanguageDefinition_GetIdentifierDesc(int index, int id);
		virtual const char* LanguageDefinition_GetCommentStart(int id);
		virtual const char* LanguageDefinition_GetCommentEnd(int id);
		virtual const char* LanguageDefinition_GetLineComment(int id);
		virtual bool LanguageDefinition_IsCaseSensitive(int id);
		virtual bool LanguageDefinition_GetAutoIndent(int id);
		virtual const char* LanguageDefinition_GetName(int id);
		virtual const char* LanguageDefinition_GetNameAbbreviation(int id);

		// autocomplete
		virtual int Autocomplete_GetCount(ed::plugin::ShaderStage stage) { return 0; }
		virtual const char* Autocomplete_GetDisplayString(ed::plugin::ShaderStage stage, int index) { return 0; }
		virtual const char* Autocomplete_GetSearchString(ed::plugin::ShaderStage stage, int index) { return 0; }
		virtual const char* Autocomplete_GetValue(ed::plugin::ShaderStage stage, int index) { return 0; }

		// file change checks
		virtual int ShaderFilePath_GetCount();
		virtual const char* ShaderFilePath_Get(int index);
		virtual bool ShaderFilePath_HasChanged();
		virtual void ShaderFilePath_Update();

		// misc
		virtual bool HandleDropFile(const char* filename) { return 0; }
		virtual void HandleRecompile(const char* itemName);
		virtual void HandleRecompileFromSource(const char* itemName, int sid, const char* shaderCode, int shaderSize);
		virtual void HandleShortcut(const char* name) { }
		virtual void HandlePluginMessage(const char* sender, char* msg, int msgLen) { }
		virtual void HandleApplicationEvent(ed::plugin::ApplicationEvent event, void* data1, void* data2);
		virtual void HandleNotification(int id) { }

		inline unsigned int GetColorBuffer() { return GetWindowColorTexture(Renderer); }

		bool ShaderPathsUpdated;
	private:
		void m_addCanvasMaterial();
		void m_addSprite(pipe::CanvasMaterial* owner, const std::string& tex);

		void m_bindFBO(pipe::CanvasMaterial* canvas);

		float m_lastErrorCheck;

		void* m_dbgEditor;
		unsigned int m_dbgTextureID;
		std::string m_dbgTexture;

		bool m_varManagerOpened;
				
		bool m_createSpritePopup;
		std::string m_createSpriteTexture;

		std::vector<const char*> m_langDefKeywords;
		std::vector<std::pair<const char*, ed::plugin::TextEditorPaletteIndex>> m_langDefRegex;
		std::vector<std::pair<const char*, const char*>> m_langDefIdentifiers;
		void m_buildLangDefinition();
		std::vector<std::string> m_editorOpened;

		std::unordered_map<GLuint, bool> m_isRTCleared;

		glm::vec2 m_rtSize, m_lastSize;
		glm::vec4 m_clearColor;

		PipelineItem* m_popupItem;
		
		std::string m_tempXML;
		std::unordered_map<pipe::CanvasMaterial*, std::string> m_loadRTs;
		std::unordered_map<pipe::Sprite*, std::string> m_loadTextures;
		std::unordered_map<pipe::Sprite*, glm::vec2> m_loadSizes;
		std::unordered_map<std::string, std::pair<PipelineItem*, std::string>> m_loadUniformTextures;

		bool m_saveRequestedCopy;

		std::vector<gd::PipelineItem*> m_items;
	};
}