#pragma once
#include <string>

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

		inline const std::string& GetDefaultCanvasVertexShader() { return m_canvasVS; }
		inline const std::string& GetDefaultCanvasPixelShader() { return m_canvasPS; }

	private:
		std::string m_canvasVS, m_canvasPS;

		void m_createEmptyTexture();
		void m_createBlackTexture();
		void m_createWhiteTexture();
	};
}