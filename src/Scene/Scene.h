#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <entt/entt.hpp>

class Entity;
class SetupSystem;
class EventSystem;
class UpdateSystem;
class RenderSystem;

class Scene {
  public:
    std::vector<std::shared_ptr<SetupSystem>> setupSystems;
    std::vector<std::shared_ptr<EventSystem>> eventSystems;
    std::vector<std::shared_ptr<UpdateSystem>> updateSystems;
    std::vector<std::shared_ptr<RenderSystem>> renderSystems;
    std::string name;

    Scene(const std::string&, entt::registry& r);
    ~Scene();
   
    Scene() = delete;
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    Entity* world;
    Entity* mainCamera;
    Entity* player;

    entt::registry& r;

    Entity* createEntity(
      const std::string& name = "NO NAME",
      int x = 0,
      int y = 0
    );
    
    void setup();
    void update(double dT);
    void render(SDL_Renderer* renderer);
    void processEvents(SDL_Event event);
};
