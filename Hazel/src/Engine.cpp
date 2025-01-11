#include "Engine.h"

#include "Hazel/Scene/Scene.h"
#include "Resource/Project.h"
#include "Hazel/Physics/PhysicsManager.h"

#include "Hazel/File/FileSystem.h"

namespace Hazel {

    Ref<Project> Engine::currentProject;

    void Engine::Init() {

        PhysicsManager::Get().Init();
    }

    bool Engine::LoadProject(Ref<Project> project) {

        currentProject = project;

        FileSystem::SetRootDirectory(FileSystem::GetParentPath(project->fullPath));

        return true;
    }

    Ref<Project> Engine::GetProject() {
        return currentProject;
    }
}
