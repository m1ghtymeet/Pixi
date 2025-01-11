#include "hzpch.h"
#include "ContentBrowserPanel.h"

#include <imgui.h>

namespace Hazel {

	constexpr char* s_assetsDirectory = "assets";
	extern const std::filesystem::path s_assetPath = "assets";
	bool s_isTexturesLoaded = false;
	
	ContentBrowserPanel::ContentBrowserPanel() {
		m_currentDirectory = s_assetPath;
		m_directoryIcon = Texture2D::Create("res/ContentBrowser/DirectoryIcon.png");
		m_fileIcon = Texture2D::Create("res/ContentBrowser/FileIcon.png");
	}

	void ContentBrowserPanel::OnImGuiRender() {

		ImGui::Begin("Content Browser");

		if (m_currentDirectory != std::filesystem::path(s_assetsDirectory)) {
			if (ImGui::Button("<-")) {
				m_currentDirectory = m_currentDirectory.parent_path();
			}
		}

		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		for (auto& directory : std::filesystem::directory_iterator(m_currentDirectory)) {

			const auto& dirPath = directory.path();

			auto relativePath = std::filesystem::relative(directory.path(), s_assetPath);
			std::string path = directory.path().string();
			std::string filename = relativePath.filename().string();
			std::string extension = relativePath.extension().string();

			ImGui::PushID(relativePath.filename().string().c_str());
			Ref<Texture> icon;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

			if (extension == ".png" || extension == ".jpg")
			{
				if (!s_isTexturesLoaded) {
					for (std::filesystem::path a : directory.path().filename()) {
						std::cout << a << "\n";
					}
					s_isTexturesLoaded = true;
				}
				
				ImGui::ImageButton("dirp", (ImTextureID)icon->GetID(), {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});
			}
			else
			{
				icon = directory.is_directory() ? m_directoryIcon : m_fileIcon;
				ImGui::ImageButton("dir", (ImTextureID)icon->GetID(), {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});
			}

			if (ImGui::BeginDragDropSource()) {
				const wchar_t* itemPath = relativePath.c_str();
				std::string dragType;
				if (extension == ".hazel") {
					dragType = "_Scene";
				}
				if (extension == ".cs") {
					dragType = "_CSharp";
				}
				if (extension == ".obj") {
					dragType = "_Model";
				}
				if (extension == ".png" || extension == ".jpg") {
					dragType = "_Texture";
				}

				ImGui::SetDragDropPayload(dragType.c_str(), itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
				ImGui::Text(dragType.c_str());
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				if (directory.is_directory())
					m_currentDirectory /= directory.path().filename();
			}
			ImGui::TextWrapped(relativePath.filename().string().c_str());
			ImGui::NextColumn();

			ImGui::PopID();
		}

		ImGui::Columns(1);

		ImGui::End();
	}
}