#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Hazel {

    class Hazelnut : public Application {
    public:
        Hazelnut() : Application("Vivanut") {
            PushLayer(new EditorLayer());
        }
    };

    Application* CreateApplication() {
        return new Hazelnut();
    }
}