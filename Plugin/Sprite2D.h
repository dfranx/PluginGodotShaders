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

			glm::vec4 m_color;

			void m_buildVBO();
			unsigned int m_vbo, m_vao;
			Vertex m_verts[6];
		};
	}
}