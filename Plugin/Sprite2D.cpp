#include "Sprite2D.h"
#include "ResourceManager.h"
#include "../GodotShaders.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

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
			m_texID = ResourceManager::Instance().EmptyTexture;
			m_texName = "";
			m_vao = 0;
			m_vbo = 0;
			Type = PipelineItemType::Sprite2D;
			m_buildVBO();
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
			ImGui::Image((ImTextureID)((gd::GodotShaders*)Owner)->GetMyTexture(), ImVec2(64, 64));
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
		}
		void Sprite2D::SetColor(glm::vec4 clr)
		{
			m_color = clr;
			m_buildVBO();
		}
		void Sprite2D::Draw()
		{
			glBindVertexArray(m_vao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		void Sprite2D::m_buildVBO()
		{
			m_verts[0] = { {-1, -1},	{0.0f, 0.0f},	m_color };
			m_verts[1] = { {1, -1},		{1.0f, 0.0f},	m_color };
			m_verts[2] = { {1, 1},		{1.0f, 1.0f},	m_color };
			m_verts[3] = { {-1, -1},	{0.0f, 0.0f},	m_color };
			m_verts[4] = { {1, 1},		{1.0f, 1.0f},	m_color };
			m_verts[5] = { {-1, 1},		{0.0f, 1.0f},	m_color };

			// create vao
			glGenVertexArrays(1, &m_vao);
			glBindVertexArray(m_vao);

			// create vbo
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
		}
	}
}