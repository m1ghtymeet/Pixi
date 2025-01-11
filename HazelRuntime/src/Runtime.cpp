#include <Hazel.h>

namespace Hazel {

    class HazelRuntime : public Application {
    public:
        HazelRuntime() : Application("Runtime") {
            //PushLayer(new EditorLayer());
        }
    };

    Application* CreateApplication() {
        return new HazelRuntime();
    }
}
