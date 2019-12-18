#pragma once
#include "Settings.h"
#include "PipelineItem.h"

#include <glm/glm.hpp>
#include <string>

namespace gd
{
	namespace pipe
	{
		class Sprite2D : public PipelineItem
		{
		public:
			Sprite2D();
			~Sprite2D();

			void SetTexture(const std::string& texObjName);
			void SetColor(glm::vec4 clr);

			inline glm::mat4 GetMatrix() { return m_matrix; }
			inline const std::string& GetTexture() { return m_texName; }
			inline void SetPosition(glm::vec2 pos) { m_pos = pos; m_buildMatrix(); }
			inline void SetSize(glm::vec2 pos) { m_size = pos; m_buildMatrix(); }
			inline glm::vec2 GetPosition() { return m_pos; }
			inline glm::vec2 GetSize() { return m_size; }

			void ShowProperties();

			void Draw();

			inline glm::vec4 GetColor() { return m_color; }

			struct Vertex
			{
				glm::vec2 Position;
				glm::vec2 UV;
				glm::vec4 Color;
			};

		private:
			std::string m_texName;
			unsigned int m_texID; // GLuint texture ID

			glm::vec2 m_pos, m_size;
			
			void m_buildMatrix();
			glm::mat4 m_matrix;

			glm::vec4 m_color;

			void m_buildVBO();
			unsigned int m_vbo, m_vao;
			Vertex m_verts[6];
		};
	}
}