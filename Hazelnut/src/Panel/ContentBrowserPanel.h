#pragma once

#include "Hazel/Renderer/Texture.h"

#include <filesystem>

namespace Hazel {

	class ContentBrowserPanel {
	public:
		ContentBrowserPanel();

		void OnImGuiRender();
	private:
		std::filesystem::path m_currentDirectory;

		Ref<Texture2D> m_directoryIcon;
		Ref<Texture2D> m_fileIcon;
	};
}