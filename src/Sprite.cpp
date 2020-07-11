#include <Core/Sprite.h>
#include <Core/ResourceManager.h>
#include <UI/UIHelper.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/glew.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace gd
{
	namespace pipe
	{
		Sprite::Sprite()
		{
			m_color = glm::vec4(1.0f);
			m_size = glm::vec2(1.0f);
			m_pos = glm::vec2(0.0f);
			m_texID = ResourceManager::Instance().EmptyTexture;
			m_texName = "";
			m_vao = 0;
			m_vbo = 0;
			m_rota = 0.0f;
			m_flipH = false;
			m_flipV = false;
			m_visible = true;
			Type = PipelineItemType::Sprite;
		}
		Sprite::~Sprite()
		{
			if (m_vao != 0)
				glDeleteVertexArrays(1, &m_vao);

			if (m_vbo != 0)
				glDeleteBuffers(1, &m_vbo);
		}

		void Sprite::ShowProperties()
		{
			ImGui::Text("Texture:");
			ImGui::SameLine();
			ImGui::PushItemWidth(-1);
			if (ImGui::BeginCombo("##godot_sprite_texture", m_texName.empty() ? "EMPTY" : UIHelper::TrimFilename(m_texName).c_str())) {
				if (ImGui::Selectable("EMPTY")) {
					SetTexture("");
					Owner->ModifyProject(Owner->Project);
				}

				int ocnt = Owner->GetObjectCount(Owner->ObjectManager);
				for (int i = 0; i < ocnt; i++) {
					const char* oname = Owner->GetObjectName(Owner->ObjectManager, i);
					if (Owner->IsTexture(Owner->ObjectManager, oname)) {
						if (ImGui::Selectable(UIHelper::TrimFilename(oname).c_str())) {
							SetTexture(oname);
							Owner->ModifyProject(Owner->Project);
						}
					}
				}

				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
			UIHelper::TexturePreview(m_texID);
			ImGui::Separator();



			ImGui::Text("Position: "); ImGui::SameLine();
			ImGui::PushItemWidth(-1);
			if (ImGui::DragFloat2("##gsprite_props_pos", glm::value_ptr(m_pos))) {
				m_buildMatrix();
				Owner->ModifyProject(Owner->Project);
			}
			ImGui::PopItemWidth(); ImGui::Separator();



			ImGui::Text("Size: "); ImGui::SameLine();
			ImGui::PushItemWidth(-1);
			if (ImGui::DragFloat2("##gsprite_props_size", glm::value_ptr(m_size))) {
				m_buildVBO();
				Owner->ModifyProject(Owner->Project);
			}
			ImGui::PopItemWidth(); ImGui::Separator();



			ImGui::Text("Rotation: "); ImGui::SameLine();
			ImGui::PushItemWidth(-1);
			if (ImGui::DragFloat("##gsprite_props_rota", &m_rota)) {
				m_buildMatrix();
				Owner->ModifyProject(Owner->Project);
			}
			ImGui::PopItemWidth(); ImGui::Separator();



			ImGui::Text("Color: "); ImGui::SameLine();
			ImGui::PushItemWidth(-1);
			if (ImGui::ColorEdit4("##gsprite_props_color", glm::value_ptr(m_color))) {
				m_buildVBO();
				Owner->ModifyProject(Owner->Project);
			}
			ImGui::PopItemWidth(); ImGui::Separator();



			ImGui::Text("FlipH: "); ImGui::SameLine();
			if (ImGui::Checkbox("##gsprite_props_fliph", &m_flipH)) {
				m_buildVBO();
				Owner->ModifyProject(Owner->Project);
			}
			ImGui::Separator();



			ImGui::Text("FlipV: "); ImGui::SameLine();
			if (ImGui::Checkbox("##gsprite_props_flipv", &m_flipV)) {
				m_buildVBO();
				Owner->ModifyProject(Owner->Project);
			}
			ImGui::Separator();



			ImGui::Text("Visible: "); ImGui::SameLine();
			if (ImGui::Checkbox("##gsprite_props_visible", &m_visible))
				Owner->ModifyProject(Owner->Project);
		}

		void Sprite::SetTexture(const std::string& texObjName)
		{
			if (texObjName.empty()) {
				// empty texture
				m_texName = "";
				m_texID = ResourceManager::Instance().EmptyTexture;
			} else {
				m_texName = texObjName;
				m_texID = Owner->GetFlippedTexture(Owner->ObjectManager, texObjName.c_str());
			}

			printf("[GSHADERS] Setting texture to %s\n", texObjName.c_str());

			// get texture size
			int w, h;
			int miplevel = 0;
			glBindTexture(GL_TEXTURE_2D, m_texID);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);
			glBindTexture(GL_TEXTURE_2D, 0);

			// recreate vbo
			m_size = glm::vec2(w,h);
			m_buildVBO();
		}
		void Sprite::Draw()
		{
			if (!m_visible)
				return;

			glActiveTexture(GL_TEXTURE0 + 0);
			glBindTexture(GL_TEXTURE_2D, m_texID);

			glBindVertexArray(m_vao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		void Sprite::m_buildVBO()
		{
			m_verts[0] = { {-0.5f, -0.5f, 0.0f},		{0.0f, 0.0f},	m_color };
			m_verts[1] = { {0.5f, -0.5f, 0.0f},		{1.0f, 0.0f},	m_color };
			m_verts[2] = { {0.5f, 0.5f, 0.0f},		{1.0f, 1.0f},	m_color };
			m_verts[3] = { {-0.5f, -0.5f, 0.0f},		{0.0f, 0.0f},	m_color };
			m_verts[4] = { {0.5f, 0.5f, 0.0f},		{1.0f, 1.0f},	m_color };
			m_verts[5] = { {-0.5f, 0.5f, 0.0f},		{0.0f, 1.0f},	m_color };

			for (int i = 0; i < 6; i++) {
				m_verts[i].Position.x *= m_size.x;
				m_verts[i].Position.y *= m_size.y;
			}

			if (m_flipH)
				for (int i = 0; i < 6; i++)
					m_verts[i].UV.x = 1.0f - m_verts[i].UV.x;
			if (m_flipV)
				for (int i = 0; i < 6; i++)
					m_verts[i].UV.y = 1.0f - m_verts[i].UV.y;

			// create vao
			if (m_vao == 0)
				glGenVertexArrays(1, &m_vao);
			glBindVertexArray(m_vao);

			// create vbo
			if (m_vbo == 0)
				glGenBuffers(1, &m_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

			// vbo data
			glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(CanvasVertex), m_verts, GL_STATIC_DRAW);

			// position
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CanvasVertex), (void*)0);
			glEnableVertexAttribArray(0);

			// uv
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CanvasVertex), (void*)(3 * sizeof(GLfloat)));
			glEnableVertexAttribArray(1);

			// color
			glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(CanvasVertex), (void*)(5 * sizeof(GLfloat)));
			glEnableVertexAttribArray(2);

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			m_buildMatrix();
		}
		void Sprite::m_buildMatrix()
		{
			glm::vec3 posRect(m_pos.x + m_size.x/2, m_pos.y + m_size.y / 2, -1000.0f);
			m_matrix = glm::translate(glm::mat4(1), posRect) * 
				glm::rotate(glm::mat4(1.0f), glm::radians(m_rota), glm::vec3(0.0f, 0.0f, 1.0f));
		}
	}
}