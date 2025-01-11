#include <Hazel.h>
#include "imgui.h"

#include "Panel/SceneHierarchyPanel.h"
#include "Panel/ContentBrowserPanel.h"

#include "Hazel/Renderer/RenderCommand.h"

#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/Scene/Components/ParentComponent.h"

#include "Hazel/Utils/PlatformUtil.h"
#include "ImGuizmo/ImGuizmo.h"
#include "Hazel/Math/Math.h"
#include "Engine.h"

namespace Hazel {

    extern const std::filesystem::path s_assetPath;

    class EditorLayer : public Layer {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        void OnUpdate(Time time) override;
        virtual void OnImGuiRender() override;
        void OnEvent(Event& e) override;
    private:
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

        void NewScene();
        void OpenScene();
        void OpenScene(const std::filesystem::path& path);
        void SaveAsScene();

        void OnScenePlay();
        void OnSceneSimulate();
        void OnSceneStop();
        void OnDuplicateEntity();

        // UI Panels
        void UI_Toolbar();
    private:
        Ref<VertexArray> m_vertexArray;
        Ref<VertexBuffer> m_vertexBuffer;
        Ref<IndexBuffer> m_indexBuffer;

        Ref<Framebuffer> m_framebuffer;
        Ref<Scene> m_activeScene;
        Ref<Scene> m_editorScene, m_runtimeScene;
        Entity m_hoveredEntity;

        EditorCamera m_editorCamera;

        bool _viewportFocused = false, _viewportHovered = false;
        glm::vec2 _viewportSize = glm::vec2(0);
        glm::vec2 m_viewportBounds[2];

        int m_gizmoType = -1;

        enum class SceneState {
            Edit = 0, Play = 1, Simulate = 2,
        };
        SceneState m_sceneState = SceneState::Edit;

        // Panels
        SceneHierarchyPanel m_sceneHierarchyPanel;
        ContentBrowserPanel m_contentBrowserPanel;

        // Editor resource
        Ref<Texture2D> m_iconPlay, m_iconStop, m_iconSimulate;
    };

    EditorLayer::EditorLayer() : Layer("EditorLayer") {

    }

    void EditorLayer::OnAttach() {

        HZ_PROFILE_FUNCTION();

        m_iconPlay = Texture2D::Create("res/PlayButton.png");
        m_iconStop = Texture2D::Create("res/StopButton.png");
        m_iconSimulate = Texture2D::Create("res/SimulateButton.png");

        // Framebuffer
        FramebufferSpecification m_fspec;
        m_fspec.Width = 1280; m_fspec.Height = 720;
        m_fspec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
        m_framebuffer = Hazel::Framebuffer::Create(m_fspec);

        float windowWidth = Application::Get().GetWindow().GetWidth();
        float windowHeight = Application::Get().GetWindow().GetHeight();

        m_editorScene = CreateRef<Scene>();
        m_activeScene = m_editorScene;
        Application::Get().GetCurrentScene() = m_editorScene;

        m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;
        m_sceneHierarchyPanel.SetContext(m_activeScene);

        Engine::Init();

        auto project = Project::New();
        try {
            //project->Deserialize(json::parse());
            project->fullPath = "assets/";

            Engine::LoadProject(project);
        }
        catch (std::exception exception) {
            //HZ_CORE_ASSERT("Error Loading project");
        }

        m_editorCamera = EditorCamera(30.0f, 1.7798f, 0.1f, 500.0f);
    }

    void EditorLayer::OnDetach() {

        HZ_PROFILE_FUNCTION();
    }

    void EditorLayer::OnUpdate(Time time) {

        HZ_PROFILE_FUNCTION();

        m_activeScene->OnViewportResize((uint32_t)_viewportSize.x, (uint32_t)_viewportSize.y);

        // Resize
        if (FramebufferSpecification spec = m_framebuffer->GetSpecification();
            _viewportSize.x > 0.0f && _viewportSize.y > 0.0f &&
            (spec.Width != _viewportSize.x || spec.Height != _viewportSize.y)) {

            m_framebuffer->Resize((uint32_t)_viewportSize.x, (uint32_t)_viewportSize.y);
            m_editorCamera.SetViewportSize(_viewportSize.x, _viewportSize.y);
        }

        // Render
        m_framebuffer->Bind();
        RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        RenderCommand::Clear();
        RenderCommand::Enable(RendererEnum::DEPTH_TEST);

        // Clear our entity ID attachment to -1
        m_framebuffer->ClearAttachment(1, -1);

        switch (m_sceneState) {
        case SceneState::Edit:
            if (_viewportFocused)
                m_editorCamera.OnUpdate(time);
            m_activeScene->OnUpdateEditor(time, m_editorCamera);
            break;
        case SceneState::Simulate:
            m_editorCamera.OnUpdate(time);
            m_activeScene->OnUpdateSimulation(time, m_editorCamera);
            break;
        case SceneState::Play:
            m_activeScene->OnUpdateRuntime(time);
            break;
        }

        // Update scene
        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_viewportBounds[0].x;
        my -= m_viewportBounds[0].y;
        glm::vec2 viewportSize = m_viewportBounds[1] - m_viewportBounds[0];
        my = viewportSize.y - my;
        int mouseX = (int)mx;
        int mouseY = (int)my;

        if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y) {
            int pixelData = m_framebuffer->ReadPixel(1, mouseX, mouseY);
            m_hoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_activeScene.get());
        }

        m_framebuffer->UnBind();
    }

    void EditorLayer::OnImGuiRender() {

        HZ_PROFILE_FUNCTION();

        static bool dockSpaceOpen = true;
        static bool opt_fullscreen = true;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockSpaceOpen, window_flags);
        ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // DockSpace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSize = style.WindowMinSize.x;
        style.WindowMinSize.x = 370.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        style.WindowMinSize.x = minWinSize;

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New")) {
                    NewScene();
                }
                if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                    OpenScene();
                }
                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
                    SaveAsScene();
                }

                if (ImGui::MenuItem("Exit")) Hazel::Application::Get().Close();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings"))
            {
                if (ImGui::MenuItem("Camera Editor")) {
                    
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        m_sceneHierarchyPanel.OnImGuiRender();
        m_contentBrowserPanel.OnImGuiRender();

        /*ImGui::Begin("Stats");

        //std::string name = "None";
        //if (m_hoveredEntity)
        //    name = m_hoveredEntity.GetComponent<NameComponent>().name;
        //ImGui::Text("Hovered Entity: %s", name.c_str());

        ImGui::End();*/

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Viewport");
        auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
        auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
        auto viewportOffset = ImGui::GetWindowPos();
        m_viewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
        m_viewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

        _viewportFocused = ImGui::IsWindowFocused();
        _viewportHovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiLayer()->SetBlockEvents(_viewportFocused || _viewportHovered);

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        _viewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        if (_viewportSize != *((glm::vec2*)&viewportPanelSize)) {
            m_framebuffer->Resize(viewportPanelSize.x, viewportPanelSize.y);
            _viewportSize = { viewportPanelSize.x, viewportPanelSize.y };

            m_activeScene->OnViewportResize(_viewportSize.x, _viewportSize.y);
        }
        uint32_t textureID = m_framebuffer->GetColorAttachmentRendererID();
        ImGui::Image((void*)textureID, ImVec2{ _viewportSize.x, _viewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Scene")) {
                const wchar_t* path = (const wchar_t*)payload->Data;
                OpenScene(std::filesystem::path(s_assetPath) / path);
            }
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Model")) {
                const wchar_t* path = (const wchar_t*)payload->Data;
                std::string fullPath = (std::filesystem::path(s_assetPath) / path).string();
                if (FileSystem::EndsWith(fullPath, ".obj")) {
                    Entity newEntity = m_editorScene->CreateEntity("New Entity");
                    newEntity.AddComponent<MeshRendererComponent>(fullPath);
                    newEntity.GetComponent<MeshRendererComponent>().LoadModel();
                }
            }
            ImGui::EndDragDropTarget();
        }

        // Gizmos
        Entity selectedEntity = m_sceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity && m_gizmoType != -1) {

            ImGuizmo::SetDrawlist();
            float windowWidth = (float)ImGui::GetWindowWidth();
            float windowHeight = (float)ImGui::GetWindowHeight();

            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

            // Camera

            // Runtime camera from entity
            // auto cameraEntity = m_activeScene->GetPrimaryCameraEntity();
            // const glm::mat4& cameraProjection = cameraEntity.GetComponent<CameraComponent>().camera.GetProjection();
            // glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<Transform>().to_mat4());

            // Editor camera
            const glm::mat4& cameraProjection = m_editorCamera.GetProjection();
            glm::mat4 cameraView = m_editorCamera.GetViewMatrix();

            // Entity transform
            auto& tc = selectedEntity.GetComponent<TransformComponent>();
            glm::mat4 transform = tc.GetGlobalTransform();
            glm::vec3 originalRotation = Vector3(tc.GetGlobalRotation().x, tc.GetGlobalRotation().y, tc.GetGlobalRotation().z);

            // Snapping
            bool snap = HInput::KeyPressed(Key::LeftControl);
            float snapValue = 0.5f;
            // Snap to 45 degrees for rotation
            if (m_gizmoType == ImGuizmo::OPERATION::ROTATE)
                snapValue = 45.0f;

            float snapValues[3] = { snapValue, snapValue, snapValue };

            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                (ImGuizmo::OPERATION)m_gizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
                nullptr, snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing()) {

                glm::vec3 translation, rotation, scale;

                ParentComponent& parent = selectedEntity.GetComponent<ParentComponent>();
                if (parent.HasParent) {
                    const auto& parentTransformComponent = parent.Parent.GetComponent<TransformComponent>();
                    const Matrix4& parentTransform = parentTransformComponent.GetGlobalTransform();
                    transform = glm::inverse(parentTransform) * transform;
                }

                Math::DecomposeTransform(transform, translation, rotation, scale);
                glm::vec3 deltaRotation = rotation - Vector3(tc.GetGlobalRotation().x, tc.GetGlobalRotation().y, tc.GetGlobalRotation().z);
                tc.SetGlobalPosition(translation);
                tc.GetGlobalRotation() += Quat(deltaRotation);
                tc.SetGlobalScale(scale);
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();

        UI_Toolbar();

        ImGui::End();
    }

    void EditorLayer::UI_Toolbar() {

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        auto& colors = ImGui::GetStyle().Colors;
        const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
        const auto& buttonActive = colors[ImGuiCol_ButtonActive];
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

        ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        float size = ImGui::GetWindowHeight() - 4.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

        bool hasPlayButton = m_sceneState == SceneState::Edit || m_sceneState == SceneState::Play;
        bool hasSimulateButton = m_sceneState == SceneState::Edit || m_sceneState == SceneState::Simulate;
        bool hasPauseButton = m_sceneState != SceneState::Edit;

        if (hasPlayButton) {
            Ref<Texture2D> icon = (m_sceneState == SceneState::Edit || m_sceneState == SceneState::Simulate) ? m_iconPlay : m_iconStop;
            //ImGui::SameLine((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
            if (ImGui::ImageButton((ImTextureID)icon->GetID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0)) {
                if (m_sceneState == SceneState::Edit)
                    OnScenePlay();
                else if (m_sceneState == SceneState::Play)
                    OnSceneStop();
            }
        }

        if (hasSimulateButton) {

            if (hasPlayButton)
                ImGui::SameLine();

            Ref<Texture2D> icon = (m_sceneState == SceneState::Edit || m_sceneState == SceneState::Play) ? m_iconSimulate : m_iconStop;
            if (ImGui::ImageButton((ImTextureID)icon->GetID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0))) {
                if (m_sceneState == SceneState::Edit || m_sceneState == SceneState::Play)
                    OnSceneSimulate();
                else if (m_sceneState == SceneState::Simulate)
                    OnSceneStop();
            }
        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
        ImGui::End();
    }

    void EditorLayer::OnEvent(Event& e) {

        m_editorCamera.OnEvent(e);

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(HZ_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(HZ_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& e) {
        if (e.GetRepeatCount() > 0)
            return false;

        bool control = HInput::KeyPressed(Key::LeftControl) || HInput::KeyPressed(Key::RightControl);
        bool shift = HInput::KeyPressed(Key::LeftShift) || HInput::KeyPressed(Key::RightShift);
        switch (e.GetKeyCode()) {
        case Key::N: if (control) { NewScene(); } break;
        case Key::O: if (control) { OpenScene(); } break;
        case Key::S: if (control && shift) { SaveAsScene(); } break;
        case Key::D: if (control) OnDuplicateEntity(); break;
        case Key::Delete:
            if (Application::Get().GetImGuiLayer()->GetActiveWidgetID() == 0) {
                Entity selectedEntity = m_sceneHierarchyPanel.GetSelectedEntity();
                if (selectedEntity) {
                    m_sceneHierarchyPanel.SetSelectedEntity({});
                    m_activeScene->DestroyEntity(selectedEntity);
                }
            }
            break;
        case Key::Q: m_gizmoType = ImGuizmo::OPERATION::TRANSLATE; break;
        case Key::W: m_gizmoType = ImGuizmo::OPERATION::ROTATE; break;
        case Key::E: m_gizmoType = ImGuizmo::OPERATION::SCALE; break;
        default: break;
        }
    }

    bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e) {

        if (e.GetMouseButton() == Mouse::ButtonLeft)
            if (_viewportHovered && !ImGuizmo::IsOver() && !HInput::KeyPressed(Key::LeftAlt))
                m_sceneHierarchyPanel.SetSelectedEntity(m_hoveredEntity);

        return false;
    }

    void EditorLayer::NewScene() {
        m_activeScene = CreateRef<Scene>();
        m_sceneHierarchyPanel.SetContext(m_activeScene);
    }

    void EditorLayer::OpenScene() {

        std::string filepath = FileDialogs::OpenFile("Hazel Scene (*.hazel)\0*.hazel\0");
        if (!filepath.empty()) {
            OpenScene(filepath);
        }
    }

    void EditorLayer::OpenScene(const std::filesystem::path& path) {

        if (m_sceneState != SceneState::Edit)
            OnSceneStop();

        if (path.extension().string() != ".hazel") {
            HZ_WARN("Could not load {0} - not a scene file", path.filename().string());
            return;
        }

        Ref<Scene> newScene = CreateRef<Scene>();
        SceneSerializer serializer(newScene);
        if (serializer.Deserialize(path.string())) {
            
            m_editorScene = newScene;
            m_sceneHierarchyPanel.SetContext(m_editorScene);
            
            m_activeScene = m_editorScene;
        }
    }

    void EditorLayer::SaveAsScene() {
        std::string filepath = FileDialogs::SaveFile("Hazel Scene (*.hazel)\0*.hazel\0");
        if (!filepath.empty()) {
            SceneSerializer serializer(m_activeScene);
            serializer.Serialize(filepath);
        }
    }

    void EditorLayer::OnScenePlay() {

        m_sceneState = SceneState::Play;

        m_activeScene = Scene::Copy(m_editorScene);
        m_activeScene->OnRuntimeStart();

        m_sceneHierarchyPanel.SetContext(m_activeScene);
        Application::Get().GetCurrentScene() = m_activeScene;
    }

    void EditorLayer::OnSceneSimulate() {

        if (m_sceneState == SceneState::Play)
            OnSceneStop();

        m_sceneState = SceneState::Simulate;

        m_activeScene = Scene::Copy(m_editorScene);
        m_activeScene->OnSimulationStart();

        m_sceneHierarchyPanel.SetContext(m_activeScene);
    }

    void EditorLayer::OnSceneStop() {

        HZ_CORE_ASSERT(m_sceneState == SceneState::Play || m_sceneState == SceneState::Simulate);
        if (m_sceneState == SceneState::Play)
            m_activeScene->OnRuntimeStop();

        m_sceneState = SceneState::Edit;
        m_activeScene->OnRuntimeStop();
        m_activeScene = m_editorScene;

        m_sceneHierarchyPanel.SetContext(m_activeScene);
        Application::Get().GetCurrentScene() = m_activeScene;
    }

    void EditorLayer::OnDuplicateEntity() {

        if (m_sceneState != SceneState::Edit)
            return;

        Entity selectedEntity = m_sceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity)
            m_editorScene->DuplicateEntity(selectedEntity);
    }
}