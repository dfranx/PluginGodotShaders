#include "Sprite2D.h"
#include "ResourceManager.h"
#include "../UI/UIHelper.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

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
		Sprite2D::Sprite2D()
		{
			m_color = glm::vec4(1.0f);
			m_size = glm::vec2(1.0f);
			m_pos = glm::vec2(0.0f);
			m_texID = ResourceManager::Instance().EmptyTexture;
			m_texName = "";
			m_vao = 0;
			m_vbo = 0;
			Type = PipelineItemType::Sprite2D;
		}
		Sprite2D::~Sprite2D()
		{
			if (m_vao != 0)
				glDeleteVertexArrays(1, &m_vao);

			if (m_vbo != 0)
				glDeleteBuffers(1, &m_vbo);
		}

		void Sprite2D::ShowProperties()
		{
			ImGui::Text("Texture:");
			ImGui::SameLine();
			ImGui::PushItemWidth(-1);
			if (ImGui::BeginCombo("##godot_sprite_texture", m_texName.empty() ? "EMPTY" : UIHelper::TrimFilename(m_texName).c_str())) {
				if (ImGui::Selectable("EMPTY"))
					SetTexture("");

				int ocnt = Owner->GetObjectCount(Owner->ObjectManager);
				for (int i = 0; i < ocnt; i++) {
					const char* oname = Owner->GetObjectName(Owner->ObjectManager, i);
					if (Owner->IsTexture(Owner->ObjectManager, oname)) {
						unsigned int texID = Owner->GetTexture(Owner->ObjectManager, oname);
						if (ImGui::Selectable(UIHelper::TrimFilename(oname).c_str()))
							SetTexture(oname);
					}
				}

				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
			ImGui::Image((ImTextureID)m_texID, ImVec2(64, 64), ImVec2(1,1), ImVec2(0,0));
			ImGui::Separator();

			ImGui::Text("Position: "); ImGui::SameLine();
			ImGui::PushItemWidth(-1);
			if (ImGui::DragFloat2("##gsprite_props_pos", glm::value_ptr(m_pos)))
				m_buildMatrix();
			ImGui::PopItemWidth(); ImGui::Separator();

			ImGui::Text("Size: "); ImGui::SameLine();
			ImGui::PushItemWidth(-1);
			if (ImGui::DragFloat2("##gsprite_props_size", glm::value_ptr(m_size)))
				m_buildMatrix();
			ImGui::PopItemWidth(); ImGui::Separator();
		}

		void Sprite2D::SetTexture(const std::string& texObjName)
		{
			if (texObjName.empty()) {
				// empty texture
				m_texName = "";
				m_texID = ResourceManager::Instance().EmptyTexture;
			} else {
				m_texName = texObjName;
				m_texID = Owner->GetTexture(Owner->ObjectManager, texObjName.c_str());
			}

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
		void Sprite2D::SetColor(glm::vec4 clr)
		{
			m_color = clr;
			m_buildVBO();
		}
		void Sprite2D::Draw()
		{
			glActiveTexture(GL_TEXTURE0 + 0);
			glBindTexture(GL_TEXTURE_2D, m_texID);

			glBindVertexArray(m_vao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		void Sprite2D::m_buildVBO()
		{
			m_verts[0] = { {-0.5f, -0.5f},		{0.0f, 0.0f},	m_color };
			m_verts[1] = { {0.5f, -0.5f},		{1.0f, 0.0f},	m_color };
			m_verts[2] = { {0.5f, 0.5f},		{1.0f, 1.0f},	m_color };
			m_verts[3] = { {-0.5f, -0.5f},		{0.0f, 0.0f},	m_color };
			m_verts[4] = { {0.5f, 0.5f},		{1.0f, 1.0f},	m_color };
			m_verts[5] = { {-0.5f, 0.5f},		{0.0f, 1.0f},	m_color };

			// create vao
			if (m_vao == 0)
				glGenVertexArrays(1, &m_vao);
			glBindVertexArray(m_vao);

			// create vbo
			if (m_vbo == 0)
				glGenBuffers(1, &m_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

			// vbo data
			glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Sprite2D::Vertex), m_verts, GL_STATIC_DRAW);

			// position
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Sprite2D::Vertex), (void*)0);
			glEnableVertexAttribArray(0);

			// color
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Sprite2D::Vertex), (void*)(4 * sizeof(GLfloat)));
			glEnableVertexAttribArray(1);

			// uv
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Sprite2D::Vertex), (void*)(2 * sizeof(GLfloat)));
			glEnableVertexAttribArray(2);

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			m_buildMatrix();
		}
		void Sprite2D::m_buildMatrix()
		{
			glm::vec3 scaleRect(m_size.x, m_size.y, 1.0f);
			glm::vec3 posRect(m_pos.x + m_size.x/2, m_pos.y + m_size.y / 2, -1000.0f);
			m_matrix = glm::translate(glm::mat4(1), posRect) *
				glm::scale(glm::mat4(1.0f), scaleRect);
		}
	}
}