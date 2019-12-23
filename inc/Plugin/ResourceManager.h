#pragma once
#include <string>
#include <vector>

namespace gd
{
	class ResourceManager
	{
	public:
		static inline ResourceManager& Instance()
		{
			static ResourceManager res;
			return res;
		}

		ResourceManager();
		~ResourceManager();

		unsigned int EmptyTexture, BlackTexture, WhiteTexture;
		bool CopiedScreenTexture;

		void ResizeResources(int w, int h);
		void Copy(unsigned int colorBuffer, unsigned int currentFBO);

		inline const std::string& GetDefaultCanvasVertexShader() { return m_canvasVS; }
		inline const std::string& GetDefaultCanvasPixelShader() { return m_canvasPS; }

		inline unsigned int SCREEN_TEXTURE() { return m_mipmapData[0].Color; }

		struct MipmapSize
		{
			int Width, Height;
			unsigned int FBO;
		};

	private:
		std::string m_canvasVS, m_canvasPS;

		unsigned int m_mipmapDepth;
		struct MipmapData
		{
			unsigned int Color;
			std::vector<MipmapSize> Sizes;
			unsigned int Levels;
		} m_mipmapData[2];

		unsigned int m_copyShader, m_horizontalBlurShader, m_verticalBlurShader;
		unsigned int m_hblurPixelSizeUniform, m_vblurPixelSizeUniform, m_quadVAO, m_quadVBO;

		void m_createMipmapResources();
		void m_createMipmaps(int rtw, int rth);
		void m_copyScreen();

		int m_rtw, m_rth;
		
		void m_createEmptyTexture();
		void m_createBlackTexture();
		void m_createWhiteTexture();
	};
}