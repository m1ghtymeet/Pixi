#include "SceneHierarchyPanel.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Core/AssetManager.h"
#include "Hazel/Core/Object/Object.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <filesystem>

#include "Hazel/Scene/Components/ParentComponent.h"
#include "Hazel/Scene/Components/RigidBodyComponent.h"

#ifdef _MSVC_LANG
	#define _CRT_SECURE_NO_WARNINGS
#endif

namespace Hazel {

	extern const std::filesystem::path s_assetPath;

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context) {
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context) {
		_context = context;
		_selectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender() {

		ImGui::Begin("Scene Hierarchy");
		//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 8));
		//ImGui::InputTextWithHint("##search", "Search entity", (char*)searchQuery.c_str(), 0, 0, 0);
		//ImGui::PopStyleVar();

		if (_context) {
			auto view = _context->Reg().view<NameComponent>();
			for (auto e : view) {
				Entity entity{ e, _context.get() };
				DrawGameObjectNode(entity);
			}

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				_selectionContext = {};

			// Right-click on black space
			if (ImGui::BeginPopupContextWindow(0, 1)) {
				if (ImGui::MenuItem("Create Empty Entity"))
					_context->CreateEntity("Empty Entity");

				ImGui::EndPopup();
			}

			//auto view = _context->Reg().view<NameComponent>();
			//for (auto e : view) {
			//	Entity entity{ e, _context.get() };
			//
			//	for (auto&& [componentTypeId, storage] : _context->Reg().storage()) {
			//
			//		entt::type_info componentType = storage.type();
			//
			//		entt::entity entityId = static_cast<entt::entity>(entity.GetHandle());
			//		if (storage.contains(entityId)) {
			//			entt::meta_type type = entt::resolve(componentType);
			//			entt::meta_any component = type.from_void(storage.value(entityId));
			//
			//			ComponentTypeTrait typeTraits = type.traits<ComponentTypeTrait>();
			//			// Component not exposed as an inspector panel
			//			if ((typeTraits & ComponentTypeTrait::InspectorExposed) == ComponentTypeTrait::None) {
			//				continue;
			//			}
			//		}
			//	}
			//}

			if (ImGui::BeginTable("entity_table", 4, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchSame)) {

			}

			//if (ImGui::BeginDragDropTarget()) {
			//	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY")) {
			//		Entity payload_entity = *(const Entity*)payload->Data;
			//
			//		// Check if entity is already parent.
			//		ParentComponent& parentPayload = payload_entity.GetComponent<ParentComponent>();
			//		//if (!payload_entity.)
			//	}
			//}

			ImGui::End();
		}

		ImGui::Begin("Properties");
		if (_selectionContext) {
			DrawComponents(_selectionContext);
		}

		//if (ImGui::BeginChild("Scene tree", ImGui::GetContentRegionAvail(), false)) {
		//	//if (ImGui::BeginTable("entity_table", 4, ImGuiTable))
		//	ImGui::EndChild();
		//}

		ImGui::End();
	}
	void SceneHierarchyPanel::SetSelectedEntity(Entity entity) {
		_selectionContext = entity;
	}
	void SceneHierarchyPanel::DrawGameObjectNode(Entity entity) {

		std::string tagName = entity.GetComponent<NameComponent>().name;

		ImGuiTreeNodeFlags flags = ((_selectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tagName.c_str());
		if (ImGui::IsItemClicked()) {
			_selectionContext = entity;
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Delete GameObject"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		//if (opened) {
		//	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth;
		//	opened = ImGui::TreeNodeEx((void*)9817239, flags, tagName.c_str());
		//	if (opened)
		//		ImGui::TreePop();
		//	ImGui::TreePop();
		//}

		if (entityDeleted) {
			_context->DestroyEntity(entity);
			if (_selectionContext == entity)
				_selectionContext = {};
		}
	}
	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f) {

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight + 1.0f };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.05f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();
		
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.05f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.24f, 0.7f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.07f, 0.24f, 0.7f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.05f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::Columns(1);

		ImGui::PopID();
	}

	static void DrawQuatControl(const std::string& label, Quat& values, float resetValue = 0.0f, float columnWidth = 100.0f) {

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight + 1.0f };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.05f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.05f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.24f, 0.7f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.07f, 0.24f, 0.7f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.05f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.24f, 0.7f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.07f, 0.24f, 0.7f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("W", buttonSize))
			values.w = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##W", &values.z, 0.05f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::Columns(1);

		ImGui::PopID();
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction ui) {
		
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<T>()) {

			ImGui::PushID(name.c_str());

			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight })) {
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings")) {
				if (ImGui::MenuItem("Remove Component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open) {
				ui(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();

			ImGui::PopID();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity) {

		if (entity.HasComponent<NameComponent>()) {
			auto& tagName = entity.GetComponent<NameComponent>().name;
			
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tagName.c_str());
			if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
				tagName = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent")) {

			if (!_selectionContext.HasComponent<CameraComponent>()) {
				if (ImGui::MenuItem("Camera")) {
					_selectionContext.AddComponent<CameraComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!_selectionContext.HasComponent<CameraComponent>() || !entity.HasComponent<MeshRendererComponent>()) {
				if (ImGui::MenuItem("Mesh Renderer")) {
					_selectionContext.AddComponent<MeshRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!_selectionContext.HasComponent<RigidBodyComponent>()) {
				if (ImGui::MenuItem("RigidBody")) {
					_selectionContext.AddComponent<RigidBodyComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!_selectionContext.HasComponent<BoxColliderComponent>()) {
				if (ImGui::MenuItem("Box Collider")) {
					_selectionContext.AddComponent<BoxColliderComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component) {
			DrawVec3Control("Position", component.GetGlobalPosition());
			//DrawQuatControl("Rotation", component.GetGlobalRotation());
			DrawVec3Control("Scale", component.GetGlobalScale());
		});

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component) {
			auto& camera = component.camera;

			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];

			if (ImGui::BeginCombo("Projection", currentProjectionTypeString)) {

				for (int i = 0; i < 2; i++) {
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
					if (ImGui::Selectable(projectionTypeStrings[i], isSelected)) {
						currentProjectionTypeString = projectionTypeStrings[i];
						camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective) {
				float fov = camera.GetFieldOfView();
				if (ImGui::DragFloat("Field Of View", &fov))
					camera.SetFieldOfView(fov);

				float nearPlane = camera.GetNearPlane();
				if (ImGui::DragFloat("Near Plane", &nearPlane))
					camera.SetNearPlane(nearPlane);

				float farPlane = camera.GetFarPlane();
				if (ImGui::DragFloat("Far Plane", &farPlane))
					camera.SetFarPlane(farPlane);

				camera.SetPerspective(fov, nearPlane, farPlane);
			}
			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic) {
				ImGui::Text("Coming Soon");
			}
		});

		DrawComponent<MeshRendererComponent>("Mesh Renderer Component", entity, [](auto& component) {
			if (component.texture) ImGui::ImageButton((ImTextureID)component.texture->GetID(), ImVec2(80.0f, 80.0f));
			else ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Texture")) {
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texturePath = std::filesystem::path(s_assetPath) / path;
					component.texture = Texture2D::Create(texturePath.string());
				}
				ImGui::EndDragDropTarget();
			}
		});

		DrawComponent<BoxColliderComponent>("Box Collider Component", entity, [](auto& component) {
			DrawVec3Control("Size", component.Size);
		});

		DrawComponent<RigidBodyComponent>("RigidBody Component", entity, [](auto& component) {
			
		});
	}
}