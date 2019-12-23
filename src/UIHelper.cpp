#include <UI/UIHelper.h>

#include <clocale>
#include <Plugin/ResourceManager.h>
#include <nativefiledialog/nfd.h>
#include <imgui/imgui.h>

#include <GL/glew.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#define TEXTURE_PREVIEW_WIDTH 96

namespace gd
{
	bool UIHelper::GetOpenDirectoryDialog(std::string& outPath)
	{
		nfdchar_t* path = NULL;
		nfdresult_t result = NFD_PickFolder(NULL, &path);
		setlocale(LC_ALL, "C");

		outPath = "";
		if (result == NFD_OKAY) {
			outPath = std::string(path);
			return true;
		}
		else if (result == NFD_ERROR) { /* TODO: log */ }

		return false;
	}
	bool UIHelper::GetOpenFileDialog(std::string& outPath, const std::string& files)
	{
		nfdchar_t *path = NULL;
		nfdresult_t result = NFD_OpenDialog(NULL, NULL, &path );
		setlocale(LC_ALL,"C");

		outPath = "";
		if (result == NFD_OKAY) {
			outPath = std::string(path);
			return true;
		}
		else if (result == NFD_ERROR) { /* TODO: log */ }

		return false;
	}
	bool UIHelper::GetSaveFileDialog(std::string& outPath, const std::string& files)
	{
		nfdchar_t *path = NULL;
		nfdresult_t result = NFD_SaveDialog(files.size() == 0 ? NULL : files.c_str(), NULL, &path );
		setlocale(LC_ALL,"C");

		outPath = "";
		if (result == NFD_OKAY) {
			outPath = std::string(path);
			return true;
		}
		else if (result == NFD_ERROR) { /* TODO: log */ }

		return false;
	}

	void UIHelper::TexturePreview(unsigned int tex, float w, float h)
	{
		ImGui::Image((ImTextureID)tex, ImVec2(TEXTURE_PREVIEW_WIDTH, (h/w) * TEXTURE_PREVIEW_WIDTH));
	}
	void UIHelper::TexturePreview(unsigned int tex)
	{
		// get texture size
		int w, h;
		glBindTexture(GL_TEXTURE_2D, tex);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
		glBindTexture(GL_TEXTURE_2D, 0);

		ImGui::Image((ImTextureID)tex, ImVec2(TEXTURE_PREVIEW_WIDTH, ((float)h / w) * TEXTURE_PREVIEW_WIDTH));
	}

	bool UIHelper::ShowValueEditor(ed::IPlugin* owner, const std::string& name, Uniform& u)
	{
		bool ret = false;
		switch (u.Type)
		{
		case ShaderLanguage::TYPE_BOOL:
			ret = ImGui::Checkbox(("##gsh_bool0_" + name).c_str(), &u.Value[0].boolean);
			break;

		case ShaderLanguage::TYPE_BVEC2:
			ret = ImGui::Checkbox(("##gsh_bool0_" + name).c_str(), &u.Value[0].boolean) || ret; ImGui::SameLine();
			ret = ImGui::Checkbox(("##gsh_bool1_" + name).c_str(), &u.Value[1].boolean) || ret;
			break;
		case ShaderLanguage::TYPE_BVEC3:
			ret = ImGui::Checkbox(("##gsh_bool0_" + name).c_str(), &u.Value[0].boolean) || ret; ImGui::SameLine();
			ret = ImGui::Checkbox(("##gsh_bool1_" + name).c_str(), &u.Value[1].boolean) || ret; ImGui::SameLine();
			ret = ImGui::Checkbox(("##gsh_bool2_" + name).c_str(), &u.Value[2].boolean) || ret;
			break;
		case ShaderLanguage::TYPE_BVEC4:
			ret = ImGui::Checkbox(("##gsh_bool0_" + name).c_str(), &u.Value[0].boolean) || ret; ImGui::SameLine();
			ret = ImGui::Checkbox(("##gsh_bool1_" + name).c_str(), &u.Value[1].boolean) || ret; ImGui::SameLine();
			ret = ImGui::Checkbox(("##gsh_bool2_" + name).c_str(), &u.Value[2].boolean) || ret; ImGui::SameLine();
			ret = ImGui::Checkbox(("##gsh_bool3_" + name).c_str(), &u.Value[3].boolean) || ret;
			break;

		case ShaderLanguage::TYPE_INT:
			ret = ImGui::DragInt(("##gsh_int0_" + name).c_str(), &u.Value[0].sint, u.HintRange[2], u.HintRange[0], u.HintRange[1]) || ret;
			break;
		case ShaderLanguage::TYPE_IVEC2:
			ret = ImGui::DragInt2(("##gsh_int2_" + name).c_str(), &u.Value[0].sint) || ret;
			break;
		case ShaderLanguage::TYPE_IVEC3:
			ret = ImGui::DragInt3(("##gsh_int3_" + name).c_str(), &u.Value[0].sint) || ret;
			break;
		case ShaderLanguage::TYPE_IVEC4:
			ret = ImGui::DragInt4(("##gsh_int4_" + name).c_str(), &u.Value[0].sint) || ret;
			break; break;
		case ShaderLanguage::TYPE_UINT:
			ret = ImGui::DragScalarN(("##gsh_uint1_" + name).c_str(), ImGuiDataType_U32, (void*)&u.Value[0].uint, 1, 1.0f) || ret;
			break;
		case ShaderLanguage::TYPE_UVEC2:
			ret = ImGui::DragScalarN(("##gsh_uint2_" + name).c_str(), ImGuiDataType_U32, (void*)&u.Value[0].uint, 2, 1.0f) || ret;
			break;
		case ShaderLanguage::TYPE_UVEC3:
			ret = ImGui::DragScalarN(("##gsh_uint3_" + name).c_str(), ImGuiDataType_U32, (void*)&u.Value[0].uint, 3, 1.0f) || ret;
			break;
		case ShaderLanguage::TYPE_UVEC4:
			ret = ImGui::DragScalarN(("##gsh_uint4_" + name).c_str(), ImGuiDataType_U32, (void*)&u.Value[0].uint, 4, 1.0f) || ret;
			break;
		case ShaderLanguage::TYPE_FLOAT:
			ret = ImGui::DragFloat(("##gsh_float1_" + name).c_str(), &u.Value[0].real, u.HintRange[2], u.HintRange[0], u.HintRange[1]) || ret;
			break;
		case ShaderLanguage::TYPE_VEC2:
			ret = ImGui::DragFloat2(("##gsh_float2_" + name).c_str(), &u.Value[0].real, 0.01f) || ret;
			break;
		case ShaderLanguage::TYPE_VEC3:
			ret = ImGui::DragFloat3(("##gsh_float3_" + name).c_str(), &u.Value[0].real, 0.01f) || ret;
			break;
		case ShaderLanguage::TYPE_VEC4:
			if (u.HintType == ShaderLanguage::ShaderNode::Uniform::HINT_COLOR)
				ret = ImGui::ColorEdit4("##gsh_color", &u.Value[0].real) || ret;
			else ret = ImGui::DragFloat4(("##gsh_float4_" + name).c_str(), &u.Value[0].real, 0.01f) || ret;
			break;
		case ShaderLanguage::TYPE_MAT2:
			ret = ImGui::DragFloat2(("##gsh_mat2_0" + name).c_str(), &u.Value[0].real, 0.01f) || ret;
			ret = ImGui::DragFloat2(("##gsh_mat2_1" + name).c_str(), &u.Value[2].real, 0.01f) || ret;
			break;
		case ShaderLanguage::TYPE_MAT3:
			ret = ImGui::DragFloat3(("##gsh_mat3_0" + name).c_str(), &u.Value[0].real, 0.01f) || ret;
			ret = ImGui::DragFloat3(("##gsh_mat3_1" + name).c_str(), &u.Value[3].real, 0.01f) || ret;
			ret = ImGui::DragFloat3(("##gsh_mat3_2" + name).c_str(), &u.Value[6].real, 0.01f) || ret;
			break;
		case ShaderLanguage::TYPE_MAT4:
			ret = ImGui::DragFloat4(("##gsh_mat4_0" + name).c_str(), &u.Value[0].real, 0.01f) || ret;
			ret = ImGui::DragFloat4(("##gsh_mat4_1" + name).c_str(), &u.Value[4].real, 0.01f) || ret;
			ret = ImGui::DragFloat4(("##gsh_mat4_2" + name).c_str(), &u.Value[8].real, 0.01f) || ret;
			ret = ImGui::DragFloat4(("##gsh_mat4_3" + name).c_str(), &u.Value[12].real, 0.01f) || ret;
			break;
		case ShaderLanguage::TYPE_ISAMPLER2D:
		case ShaderLanguage::TYPE_USAMPLER2D:
		case ShaderLanguage::TYPE_SAMPLER2D:
			bool isHint = u.HintType != ShaderLanguage::ShaderNode::Uniform::HINT_NONE;
			bool isHintValue = false;
			if (u.Value[0].uint == ResourceManager::Instance().BlackTexture ||
				u.Value[0].uint == ResourceManager::Instance().WhiteTexture)
				isHintValue = true;

			std::string filename = "";
			int ocnt = owner->GetObjectCount(owner->ObjectManager);
			if (!isHintValue) {
				for (int i = 0; i < ocnt; i++) {
					const char* oname = owner->GetObjectName(owner->ObjectManager, i);
					if (owner->IsTexture(owner->ObjectManager, oname)) {
						if (u.Value[0].uint == owner->GetFlippedTexture(owner->ObjectManager, oname)) {
							filename = oname;
							break;
						}
					}
				}
			}

			if (ImGui::BeginCombo(("##gsh_sampler_" + name).c_str(), isHintValue ? "-- NONE --" : UIHelper::TrimFilename(filename).c_str())) {
				if (ImGui::Selectable("-- NONE --")) {
					ret = true;
					if (u.HintType == ShaderLanguage::ShaderNode::Uniform::HINT_BLACK)
						u.Value[0].uint = ResourceManager::Instance().BlackTexture;
					else
						u.Value[0].uint = ResourceManager::Instance().WhiteTexture;
				}

				for (int i = 0; i < ocnt; i++) {
					const char* oname = owner->GetObjectName(owner->ObjectManager, i);
					if (owner->IsTexture(owner->ObjectManager, oname)) {
						if (ImGui::Selectable(UIHelper::TrimFilename(oname).c_str())) {
							ret = true;
							u.Value[0].uint = owner->GetFlippedTexture(owner->ObjectManager, oname);
						}
					}
				}

				ImGui::EndCombo();
			}

			UIHelper::TexturePreview(u.Value[0].uint);
			
			break;
		}

		return ret;
	}

	std::string UIHelper::TrimFilename(const std::string& path)
	{
		size_t lastSlash = path.find_last_of("/\\");
		if (lastSlash != std::string::npos)
			return path.substr(lastSlash + 1);
		return path;
	}
}