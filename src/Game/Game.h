#include <SDL2/SDL.h>
#include <memory>
#include "Scene/Scene.h"


class Game {
  public:
    Game(const char* title, int width, int height);
    ~Game();

    virtual void setup() = 0;
    void frameStart();
    void frameEnd();
    void handleEvents();
    void update();
    void render();
    void clean();
    bool running();

    void run();

    void setScene(Scene* newScene);
    Scene* getCurrentScene() const;

    entt::registry r;

  protected:
    bool isRunning;
    SDL_Window *window;
    SDL_Renderer *renderer;

    int screen_width;
    int screen_height;

    // for frame management
    Uint32 frameStartTimestamp;
    Uint32 frameEndTimestamp;
    float frameDuration;
    float dT; // time in miliseconds since last frame
    // for frame count
    int frameCount;
    Uint32 lastFPSUpdateTime;
    float FPS;

    // scene logic
    Scene* currentScene;

    template<typename T>
    void addSetupSystem(Scene* scene, auto&&... args) {
        auto system = std::make_shared<T>(std::forward<decltype(args)>(args)...);
        system->setScene(scene);
        scene->setupSystems.push_back(system);
    }

    template<typename T>
    void addEventSystem(Scene* scene, auto&&... args) {
        auto system = std::make_shared<T>(std::forward<decltype(args)>(args)...);
        system->setScene(scene);
        scene->eventSystems.push_back(system);
    }

    template<typename T>
    void addUpdateSystem(Scene* scene, auto&&... args) {
        auto system = std::make_shared<T>(std::forward<decltype(args)>(args)...);
        system->setScene(scene);
        scene->updateSystems.push_back(system);
    }

    template<typename T>
    void addRenderSystem(Scene* scene, auto&&... args) {
        auto system = std::make_shared<T>(std::forward<decltype(args)>(args)...);
        system->setScene(scene);
        scene->renderSystems.push_back(system);
    }


};
