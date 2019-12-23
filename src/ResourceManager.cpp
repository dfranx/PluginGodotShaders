#include <Core/ResourceManager.h>
#include <memory>

#include <glm/glm.hpp>

#include <GL/glew.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#define EMPTY_TEXTURE_SIZE 128


// vertex shader for copy, hblur and vblur shaders
const char* VS_SHADER_COPY = R"(
#version 330

out vec2 uv_interp;

void main() 
{
    float x = float(((uint(gl_VertexID) + 2u) / 3u)%2u); 
    float y = float(((uint(gl_VertexID) + 1u) / 3u)%2u); 

    gl_Position = vec4(-1.0f + x*2.0f, -1.0f+y*2.0f, 0.0f, 1.0f);
    uv_interp = vec2(x, y);
}
)";

// pixel shader for copy,hblur,vblur shader
const char* PS_SHADER_COPY = R"(
#version 330

in vec2 uv_interp;

uniform sampler2D source; //texunit:0
uniform vec2 pixel_size;

layout(location = 0) out vec4 frag_color;

void main() {
	frag_color = textureLod(source, uv_interp, 0.0);
}
)";
const char* PS_SHADER_HBLUR = R"(
#version 330

in vec2 uv_interp;

uniform sampler2D source; //texunit:0
uniform vec2 pixel_size;

layout(location = 0) out vec4 frag_color;

void main() {
	vec4 color = textureLod(source, uv_interp, 0.0);

	color *= 0.38774;
	color += texture(source, uv_interp + vec2(1.0, 0.0) * pixel_size) * 0.24477;
	color += texture(source, uv_interp + vec2(2.0, 0.0) * pixel_size) * 0.06136;
	color += texture(source, uv_interp + vec2(-1.0, 0.0) * pixel_size) * 0.24477;
	color += texture(source, uv_interp + vec2(-2.0, 0.0) * pixel_size) * 0.06136;

	frag_color = color;
}
)";
const char* PS_SHADER_VBLUR = R"(
#version 330

in vec2 uv_interp;

uniform sampler2D source; //texunit:0
uniform vec2 pixel_size;

layout(location = 0) out vec4 frag_color;

void main() {
	vec4 color = textureLod(source, uv_interp, 0.0);

	color *= 0.38774;
	color += texture(source, uv_interp + vec2(0.0, 1.0) * pixel_size) * 0.24477;
	color += texture(source, uv_interp + vec2(0.0, 2.0) * pixel_size) * 0.06136;
	color += texture(source, uv_interp + vec2(0.0, -1.0) * pixel_size) * 0.24477;
	color += texture(source, uv_interp + vec2(0.0, -2.0) * pixel_size) * 0.06136;

	frag_color = color;
}
)";



namespace gd
{
	GLuint createShader(const char* VS, const char* PS)
	{
		GLuint shader;

		char infoLog[512] = { 0 };
		GLint success = true;

		// create vertex shader
		unsigned int shaderVS = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(shaderVS, 1, &VS, nullptr);
		glCompileShader(shaderVS);
		glGetShaderiv(shaderVS, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shaderVS, 512, NULL, infoLog);
			printf("Failed to compile a vertex shader: %s\n", infoLog);
		}

		// create pixel shader
		unsigned int shaderPS = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(shaderPS, 1, &PS, nullptr);
		glCompileShader(shaderPS);
		glGetShaderiv(shaderPS, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shaderPS, 512, NULL, infoLog);
			printf("Failed to compile a pixel shader: %s\n", infoLog);
		}

		// create a shader program for gizmo
		shader = glCreateProgram();
		glAttachShader(shader, shaderVS);
		glAttachShader(shader, shaderPS);
		glLinkProgram(shader);
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader, 512, NULL, infoLog);
			printf("Failed to compile a shader program: %s\n", infoLog);
		}

		glDeleteShader(shaderPS);
		glDeleteShader(shaderVS);

		return shader;
	}
	struct QuadVertex
	{
		glm::vec4 Position;
		glm::vec2 UV;
	};

	ResourceManager::ResourceManager()
	{
		EmptyTexture = 0;
		CopiedScreenTexture = false;
		m_createEmptyTexture();
		m_createBlackTexture();
		m_createWhiteTexture();
		m_createMipmapResources();

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

	void ResourceManager::ResizeResources(int w, int h)
	{
		m_createMipmaps(w, h);

		// TODO: other resources
	}
	void ResourceManager::Copy(unsigned int colorBuffer, unsigned int curFBO)
	{
		if (m_mipmapData[0].Sizes.size() == 0 || CopiedScreenTexture)
			return;

		CopiedScreenTexture = true;

		glDisable(GL_BLEND);

		glm::vec2 wh(m_rtw, m_rth);

		glBindFramebuffer(GL_FRAMEBUFFER, m_mipmapData[0].Sizes[0].FBO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffer);

		glUseProgram(m_copyShader);

		m_copyScreen();

		for (int i = 0; i < m_mipmapData[1].Sizes.size(); i++) {
			int vp_w = m_mipmapData[1].Sizes[i].Width;
			int vp_h = m_mipmapData[1].Sizes[i].Height;
			glViewport(0, 0, vp_w, vp_h);

			//horizontal pass
			glUseProgram(m_horizontalBlurShader);

			glUniform2f(m_hblurPixelSizeUniform, 1.0f / vp_w, 1.0 / vp_h);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_mipmapData[0].Color); //previous level, since mipmaps[0] starts one level bigger
			glBindFramebuffer(GL_FRAMEBUFFER, m_mipmapData[1].Sizes[i].FBO);

			m_copyScreen();


			//vertical pass
			glUseProgram(m_verticalBlurShader);
			glUniform2f(m_vblurPixelSizeUniform, 1.0f / vp_w, 1.0 / vp_h);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_mipmapData[1].Color);
			glBindFramebuffer(GL_FRAMEBUFFER, m_mipmapData[0].Sizes[i + 1].FBO); //next level, since mipmaps[0] starts one level bigger

			m_copyScreen();
		}

		glBindFramebuffer(GL_FRAMEBUFFER, curFBO); //back to front
		glViewport(0, 0, m_rtw, m_rth);

		glEnable(GL_BLEND);
	}

	void ResourceManager::m_createMipmapResources()
	{
		m_copyShader = createShader(VS_SHADER_COPY, PS_SHADER_COPY);
		m_horizontalBlurShader = createShader(VS_SHADER_COPY, PS_SHADER_HBLUR);
		m_verticalBlurShader = createShader(VS_SHADER_COPY, PS_SHADER_VBLUR);

		m_hblurPixelSizeUniform = glGetUniformLocation(m_horizontalBlurShader, "pixel_size");
		m_vblurPixelSizeUniform = glGetUniformLocation(m_verticalBlurShader, "pixel_size");

		QuadVertex verts[6];
		verts[0] = { {-1.0f, -1.0f, 0.0f, 1.0f},		{0.0f, 0.0f} };
		verts[1] = { {1.0f, -1.0f, 0.0f, 1.0f},			{1.0f, 0.0f} };
		verts[2] = { {1.0f, 1.0f, 0.0f, 1.0f},			{1.0f, 1.0f} };
		verts[3] = { {-1.0f, -1.0f, 0.0f, 1.0f},		{0.0f, 0.0f} };
		verts[4] = { {1.0f, 1.0f, 0.0f, 1.0f},			{1.0f, 1.0f} };
		verts[5] = { {-1.0f, 1.0f, 0.0f, 1.0f},			{0.0f, 1.0f} };

		// create vao
		glGenVertexArrays(1, &m_quadVAO);
		glBindVertexArray(m_quadVAO);

		// create vbo
		glGenBuffers(1, &m_quadVBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);

		// vbo data
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(QuadVertex), verts, GL_STATIC_DRAW);

		// position
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)0);
		glEnableVertexAttribArray(0);

		// uv
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)(4 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	void ResourceManager::m_createMipmaps(int rtw, int rth)
	{
		printf("[GSHADERS] Resizing mipmap data\n");

		glDeleteTextures(1, &m_mipmapData[0].Color);
		for (int i = 0; i < m_mipmapData[0].Sizes.size(); i++)
			glDeleteFramebuffers(1, &m_mipmapData[0].Sizes[i].FBO);
		glDeleteTextures(1, &m_mipmapData[1].Color);
		for (int i = 0; i < m_mipmapData[1].Sizes.size(); i++)
			glDeleteFramebuffers(1, &m_mipmapData[1].Sizes[i].FBO);

		m_mipmapData[0].Sizes.clear();
		m_mipmapData[1].Sizes.clear();

		m_rtw = rtw;
		m_rth = rth;

		if (m_mipmapDepth == 0)
			glGenTextures(1, &m_mipmapDepth);
		glBindTexture(GL_TEXTURE_2D, m_mipmapDepth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, rtw, rth, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		GLuint color_internal_format = GL_RGBA8;
		if (rtw >= 2 && rth >= 2) {
			for (int i = 0; i < 2; i++) {
				int w = rtw;
				int h = rth;

				if (i > 0) {
					w >>= 1;
					h >>= 1;
				}

				glGenTextures(1, &m_mipmapData[i].Color);
				glBindTexture(GL_TEXTURE_2D, m_mipmapData[i].Color);

				int level = 0;
				int fb_w = w;
				int fb_h = h;

				while (true) {
					MipmapSize mm;
					mm.Width = w;
					mm.Height = h;
					m_mipmapData[i].Sizes.push_back(mm);

					w >>= 1;
					h >>= 1;

					if (w < 2 || h < 2)
						break;

					level++;
				}

				glTexStorage2D(GL_TEXTURE_2D, level+1, color_internal_format, fb_w, fb_h);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level);
				glDisable(GL_SCISSOR_TEST);
				glColorMask(1, 1, 1, 1);
				
				for (int j = 0; j < m_mipmapData[i].Sizes.size(); j++) {

					MipmapSize& mm = m_mipmapData[i].Sizes[j];

					glGenFramebuffers(1, &mm.FBO);
					glBindFramebuffer(GL_FRAMEBUFFER, mm.FBO);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_mipmapData[i].Color, j);
					bool used_depth = false;
					if (j == 0 && i == 0) { //use always
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_mipmapDepth, 0);
						used_depth = true;
					}

					GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
					if (status != GL_FRAMEBUFFER_COMPLETE) {
						printf("Failed to create SCREEN_TEXTURE FBO!\n");
					}

					float zero[4] = { 1, 0, 1, 0 };
					glViewport(0, 0, m_mipmapData[i].Sizes[j].Width, m_mipmapData[i].Sizes[j].Height);
					glClearBufferfv(GL_COLOR, 0, zero);
					if (used_depth) {
						glClearDepth(1.0);
						glClear(GL_DEPTH_BUFFER_BIT);
					}
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				m_mipmapData[i].Levels = level;

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		}
	}
	void ResourceManager::m_copyScreen()
	{
		glBindVertexArray(m_quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
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