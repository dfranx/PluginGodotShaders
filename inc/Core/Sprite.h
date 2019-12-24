#pragma once
#include <Core/Settings.h>
#include <Core/PipelineItem.h>
#include <Core/CanvasVertex.h>

#include <glm/glm.hpp>
#include <string>

namespace gd
{
	namespace pipe
	{
		class Sprite : public PipelineItem
		{
		public:
			Sprite();
			~Sprite();

			void SetTexture(const std::string& texObjName);

			inline glm::mat4 GetMatrix() { return m_matrix; }
			inline const std::string& GetTexture() { return m_texName; }
			inline void SetPosition(glm::vec2 pos) { m_pos = pos; m_buildMatrix(); }
			inline void SetSize(glm::vec2 pos) { m_size = pos; m_buildVBO(); }
			inline void SetFlipHorizontal(bool t) { m_flipH = t; m_buildVBO(); }
			inline void SetFlipVertical(bool t) { m_flipV = t; m_buildVBO(); }
			inline void SetColor(glm::vec4 clr) { m_color = clr; m_buildVBO(); }
			inline void SetRotation(float rota) { m_rota = rota; m_buildMatrix(); }
			inline void SetVisible(bool t) { m_visible = t; }
			inline glm::vec2 GetPosition() { return m_pos; }
			inline glm::vec2 GetSize() { return m_size; }
			inline bool GetFlipHorizontal() { return m_flipH; }
			inline bool GetFlipVertical() { return m_flipV; }
			inline glm::vec4 GetColor() { return m_color; }
			inline bool IsVisible() { return m_visible; }
			inline float GetRotation() { return m_rota; }

			void ShowProperties();

			void Draw();
						
		private:
			std::string m_texName;
			unsigned int m_texID; // GLuint texture ID

			glm::vec2 m_pos, m_size;
			
			void m_buildMatrix();
			glm::mat4 m_matrix;

			float m_rota;
			bool m_flipH, m_flipV, m_visible;
			glm::vec4 m_color;

			void m_buildVBO();
			unsigned int m_vbo, m_vao;
			CanvasVertex m_verts[6];
		};
	}
}