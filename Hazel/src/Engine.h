#pragma once

#include "Hazel/Core/Base.h"

namespace Hazel {

	class Project;
	class Scene;

	class Engine {
	public:
		static void Init();
		static void Tick();
		static void Close();

		static bool LoadProject(Ref<Project> project);
		static Ref<Project> GetProject();

	private:
		static Ref<Project> currentProject;
		static Ref<Scene> currentScene;
	};
}