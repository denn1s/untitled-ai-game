#include "GeneralSystems.h"

#include <print.h>
#include <constants.h>

#include "ECS/Entity.h"
#include "ECS/Components.h"
#include "PocketAi/Components.h"


void CharacterSetupSystem::run() {
    scene->player = new Entity(scene->r.create(), scene);
    scene->player->addComponent<TransformComponent>(0, 24 * SCALE);  // ui offset
    scene->player->addComponent<SpriteComponent>("Characters/main.png");
    scene->player->addComponent<PlayerTextComponent>();
    scene->player->addComponent<PlayerPromptComponent>("Pocket: ", "Rob: ");
}

