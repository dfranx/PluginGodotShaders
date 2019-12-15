#include "CanvasMaterial.h"
#include "../PluginAPI/Plugin.h"
#include "../UI/UIHelper.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include <string.h>
#include <string>

#define BUTTON_SPACE_LEFT -40 * Owner->GetDPI()

namespace gd
{
	namespace pipe
	{
		CanvasMaterial::CanvasMaterial()
		{
			memset(ShaderPath, 0, sizeof(char) * MAX_PATH_LENGTH);
		}
		CanvasMaterial::~CanvasMaterial()
		{

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

						// TODO: recompile shader
					}
					else
						Owner->AddMessage(Owner->Messages, ed::plugin::MessageType::Error, Name, "Shader file doesn't exist");
				}
			}
			ImGui::NextColumn();


			ImGui::Columns(1);
		}
	}
}