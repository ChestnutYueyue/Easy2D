#include <easy2d/easy2d.h>

using namespace easy2d;

int main() {
    AppConfig config;
    config.title = "Easy2D Script Demo";
    config.width = 800;
    config.height = 600;

    auto& app = Application::instance();
    if (!app.init(config)) return -1;

    // Initialize script engine
    auto& script = ScriptEngine::getInstance();
    if (!script.initialize()) {
        E2D_ERROR("Failed to initialize ScriptEngine");
        return -1;
    }

    // Create scene
    auto scene = Scene::create();
    scene->setBackgroundColor(Color(0.1f, 0.1f, 0.2f, 1.0f));

    // Add a ScriptNode — loads and runs player_controller.nut
    auto ctrl = ScriptNode::create("scripts/player_controller.nut");
    ctrl->setName("player_controller");
    scene->addChild(ctrl);

    app.enterScene(scene);
    app.run();

    script.shutdown();
    app.shutdown();
    return 0;
}
