#include "ResourceManager.h"
#include <memory>

#include <GL/glew.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#define EMPTY_TEXTURE_SIZE 128

namespace gd
{
	ResourceManager::ResourceManager()
	{
		EmptyTexture = 0;
		CopiedScreenTexture = false;
		m_createEmptyTexture();
		m_createBlackTexture();
		m_createWhiteTexture();

		// default canvas item vertex shader
		m_canvasVS = R"(
#version 330

// inputs
layout(location = 0) in vec2 vertex;
layout(location = 1) in vec4 color_attrib;
layout(location = 2) in vec2 uv_attrib;

out vec2 uv_interp;
out vec4 color_interp;

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

void main()
{
	uv_interp = uv_attrib;
	color_interp = color_attrib;

	vec4 outvec = modelview_matrix * vec4(vertex, 0.0, 1.0);
	gl_Position = projection_matrix * outvec;
}
)";

		// default canvas item pixel shader
		m_canvasPS = R"(
#version 330

in vec2 uv_interp;
in vec4 color_interp;

out vec4 frag_color;

uniform sampler2D color_texture;


void main()
{
	frag_color = color_interp * texture(color_texture, uv_interp);
}
)";
	}
	ResourceManager::~ResourceManager()
	{

	}
	void ResourceManager::m_createEmptyTexture()
	{
		unsigned char* pData = (unsigned char*)malloc(EMPTY_TEXTURE_SIZE * EMPTY_TEXTURE_SIZE * 4);

		// checkered texture
		int pxs = EMPTY_TEXTURE_SIZE / 8;
		for (int x = 0; x < EMPTY_TEXTURE_SIZE; x++) {
			for (int y = 0; y < EMPTY_TEXTURE_SIZE; y++) {
				int index = (y * EMPTY_TEXTURE_SIZE + x) * 4;
				unsigned char* group = &pData[index];

				unsigned char clr = (((x / pxs) + (y / pxs)) % 2) * 255;

				group[0] = clr;
				group[1] = clr;
				group[2] = clr;
				group[3] = 255;
			}
		}

		glGenTextures(1, &EmptyTexture);
		glBindTexture(GL_TEXTURE_2D, EmptyTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, EMPTY_TEXTURE_SIZE, EMPTY_TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		free(pData);
	}
	void ResourceManager::m_createBlackTexture()
	{
		unsigned char* pData = (unsigned char*)malloc(1 * 1 * 4);

		// checkered texture
		pData[0] = 0;
		pData[1] = 0;
		pData[2] = 0;
		pData[3] = 255;

		glGenTextures(1, &BlackTexture);
		glBindTexture(GL_TEXTURE_2D, BlackTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		free(pData);
	}
	void ResourceManager::m_createWhiteTexture()
	{
		unsigned char* pData = (unsigned char*)malloc(1 * 1 * 4);

		// checkered texture
		pData[0] = 255;
		pData[1] = 255;
		pData[2] = 255;
		pData[3] = 255;

		glGenTextures(1, &WhiteTexture);
		glBindTexture(GL_TEXTURE_2D, WhiteTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		free(pData);
	}
}