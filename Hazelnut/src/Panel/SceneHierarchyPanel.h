#pragma once

#include "Hazel/Core/Base.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"

namespace Hazel {

	class SceneHierarchyPanel {
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);

		void OnImGuiRender();

		Entity GetSelectedEntity() const { return _selectionContext; }
		void SetSelectedEntity(Entity entity);
	private:
		void DrawGameObjectNode(Entity entity);
		void DrawComponents(Entity entity);
	private:
		Ref<Scene> _context;
		Entity _selectionContext;
		std::string searchQuery = "";
		friend class Scene;
	};
}