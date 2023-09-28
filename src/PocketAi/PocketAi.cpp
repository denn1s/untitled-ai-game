#include "PocketAi.h"

#include <print.h>
#include <constants.h>
#include <string>

#include "ECS/Components.h"
#include "ECS/Entity.h"

#include "PocketAi/Ai/AiManager.h"
#include "PocketAi/Systems/AiSystems.h"
#include "PocketAi/Systems/GeneralSystems.h"
#include "PocketAi/Systems/SpriteSystems.h"
#include "PocketAi/Systems/TextSystems.h"
#include "Systems/Systems.h"
#include "Components.h"


PocketAi::PocketAi()
  : Game("Ai", SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE) { }

PocketAi::~PocketAi() {
    // destructor implementation
  // temporarily clean the ai manager here
  AiManager::tearDown();
}

void PocketAi::sceneTransition() {
  delete this->getCurrentScene();
  this->setScene(nullptr);

  if (!this->scenes.empty()) {
    Scene* newScene = this->scenes.front();
    this->setScene(newScene);
    this->scenes.pop();
  }
}

void PocketAi::setup() {
  Scene* logoScene = createLogoScene();
  Scene* creditsScene = createCreditsScene();
  Scene* jamScene = createJamScene();
  Scene* titleScene = createTitleScene();
  Scene* contextScene1 = createContextScene(1);
  Scene* gameplayScene1 = createGameplayScene(1);
  Scene* contextScene2 = createContextScene(2);
  Scene* gameplayScene2 = createGameplayScene(2);
  Scene* contextScene3 = createContextScene(3);
  Scene* gameplayScene3 = createGameplayScene(3);
  Scene* contextScene4 = createContextScene(4);
  Scene* gameplayScene4 = createGameplayScene(4);
  Scene* conclusionRequestScene = createConclusionScene(false);
  Scene* conclusionResponseScene = createConclusionScene(true);
  Scene* endingScene = createEndingScene();


  // this is the scene queue, we push the scenes in order
  setScene(logoScene);
  scenes.push(creditsScene); 
  scenes.push(jamScene); 
  scenes.push(titleScene); 
  scenes.push(contextScene1);
  scenes.push(gameplayScene1);
  scenes.push(contextScene2);
  scenes.push(gameplayScene2);
  scenes.push(contextScene3);
  scenes.push(gameplayScene3);
  scenes.push(contextScene4);
  scenes.push(gameplayScene4);
  scenes.push(conclusionRequestScene);
  scenes.push(conclusionResponseScene);
  scenes.push(endingScene);

}

Scene* PocketAi::createCreditsScene() {
  Scene* scene = new Scene("CREDITS SCENE", r);
  SpriteComponent sprite = {
        "Backgrounds/credits.png",
        160, 144
  };
  addSetupSystem<SlideShowSetupSystem>(scene, sprite, 3, 2000);
  addSetupSystem<SpriteSetupSystem>(scene, renderer);
  addUpdateSystem<SlideShowUpdateSystem>(scene);
  addRenderSystem<SpriteRenderSystem>(scene);

  addUpdateSystem<SceneTransitionOnSlideUpdateSystem>(
    scene,
    std::bind(&PocketAi::sceneTransition, this)
  );

  return scene;
}

Scene* PocketAi::createLogoScene() {
  Scene* scene = new Scene("LOGO SCENE", r);
  
  // we must add the ai setup system from the start since it takes so long
  addSetupSystem<AiSetupSystem>(scene);
  addSetupSystem<AffectionSetupSystem>(scene);
  addSetupSystem<MusicSetupSystem>(scene);
  
  SpriteComponent sprite = {
        "Backgrounds/gamedev.png",
        160, 144,
        0, 0,
        50, 2500,
        PixelShader{ nullptr, "" },
        0,
        true,
        5000
  };
  addSetupSystem<SlideShowSetupSystem>(scene, sprite, 1, 12000);
  addSetupSystem<SpriteSetupSystem>(scene, renderer);
  addUpdateSystem<SpriteUpdateSystem>(scene);
  addUpdateSystem<SlideShowUpdateSystem>(scene);
  addRenderSystem<SpriteRenderSystem>(scene);

  addUpdateSystem<SceneTransitionOnSlideUpdateSystem>(
    scene,
    std::bind(&PocketAi::sceneTransition, this)
  );

  return scene;
}

Scene* PocketAi::createJamScene() {
  Scene* scene = new Scene("JAM SCENE", r);
  SpriteComponent sprite = {
        "Backgrounds/madewithlove.png",
        160, 144,
        0, 0,
        30, 1000,
        PixelShader{ nullptr, "" },
        0,
        true,
        1000
  };
  addSetupSystem<SlideShowSetupSystem>(scene, sprite, 1, 10000);
  addSetupSystem<SpriteSetupSystem>(scene, renderer);
  addUpdateSystem<SpriteUpdateSystem>(scene);
  addUpdateSystem<SlideShowUpdateSystem>(scene);
  addRenderSystem<SpriteRenderSystem>(scene);

  addUpdateSystem<SceneTransitionOnSlideUpdateSystem>(
    scene,
    std::bind(&PocketAi::sceneTransition, this)
  );

  return scene;
}

Scene* PocketAi::createTitleScene() {
  Scene* scene = new Scene("TITLE SCENE", r);
  SpriteComponent sprite = {
        "Backgrounds/title-screen.png",
        160, 144,
        0, 0,
        2, 1000,
        PixelShader{ nullptr, "" },
        0,
        false,
        3000
  };
  addSetupSystem<MusicPlaySystem>(scene, "Music/daydreamers_intro.wav");
  addSetupSystem<SlideShowSetupSystem>(scene, sprite);
  addSetupSystem<SpriteSetupSystem>(scene, renderer);
  addUpdateSystem<SpriteUpdateSystem>(scene);
  addUpdateSystem<SlideShowUpdateSystem>(scene);
  addRenderSystem<SpriteRenderSystem>(scene);

  addEventSystem<PressStartEventSystem>(
    scene,
    std::bind(&PocketAi::sceneTransition, this)
  );

  return scene;
}


Scene* PocketAi::createContextScene(int day) {
  Scene* scene = new Scene("CONTEXT SCENE " + std::to_string(day), r);

  if (day == 1) {
    /* addSetupSystem<MusicPlaySystem>(scene, "Music/daydreamers_intro.wav"); */
    addSetupSystem<MusicPlaySystem>(scene, "Music/daydreamers_full.wav");
  }
  /* if (day == 3) { */
    /* addSetupSystem<MusicPlaySystem>(scene, "Music/daydreamers_full.wav"); */
  /* } */

  // sprite / ui systems
  addSetupSystem<UiSetupSystem>(scene, renderer, "UI/simple.png", day);
  addRenderSystem<SpriteRenderSystem>(scene);
  addSetupSystem<SpriteSetupSystem>(scene, renderer);

  // text systems
  addRenderSystem<PlayerCursorRenderSystem>(scene);
  addSetupSystem<PlayerTextSetupSystem>(scene);
  addRenderSystem<PlayerTextRenderSystem>(scene);

  std::string context;
  switch (day) {
    case 1:
      context = "You cross paths with your childhood friend once again after what felt like an eternity. As the days pass, you can feel she's starting to warm up to you...";
      break;
    case 2:
      context = "Your time is cut short by the unforgiving bell. Fortunately, you cross paths with her just the next day on your way home...";
      break;
    case 3:
      context = "This time, you meet her first thing in the morning. The stars must be on your side...";
      break;
    case 4:
      context = "In an event that must have used up your luck supply for an entire year, you spot her late at night taking in the sights of the city...";
      break;
    default:
      context = "Error.";
      break;
  }

  addUpdateSystem<TextCrawlUpdateSystem>(scene, context, 20);
  addEventSystem<TextCrawlEventSystem>(
    scene,
    context,
    std::bind(&PocketAi::sceneTransition, this)
  );

  return scene;
}

Scene* PocketAi::createGameplayScene(int day) {
  Scene* scene = new Scene("GAMEPLAY SCENE " + std::to_string(day), r);
  addSetupSystem<CharacterSetupSystem>(scene);
  
  // sprite / ui systems
  addSetupSystem<UiSetupSystem>(scene, renderer, "UI/main.png");
  addSetupSystem<BackgroundSetupSystem>(scene, day - 1);
  addSetupSystem<SpriteSetupSystem>(scene, renderer);
  addUpdateSystem<SpriteUpdateSystem>(scene);
  addRenderSystem<SpriteRenderSystem>(scene);
  addSetupSystem<BlushSetupSystem>(scene, renderer);
  addRenderSystem<BlushRenderSystem>(scene);

  // ai systems
  /* addSetupSystem<AiSetupSystem>(scene); */
  addUpdateSystem<AiPromptProcessingSystem>(scene);
  addUpdateSystem<AiPromptPostProcessingSystem>(scene);
  addUpdateSystem<AiEmotionProcessingSystem>(scene);
  addUpdateSystem<UiUpdateSystem>(scene);

  // text systems
  addEventSystem<PlayerTextInputSystem>(scene);
  addSetupSystem<PlayerTextSetupSystem>(scene);
  addRenderSystem<PlayerTextRenderSystem>(scene);
  addRenderSystem<PlayerCursorRenderSystem>(scene);
  addSetupSystem<ConversationSetupSystem>(scene, 3); // TODO: move to lua

  addUpdateSystem<AiConversationProgressSystem>(
    scene,
    std::bind(&PocketAi::sceneTransition, this),
    day
  );
 
  return scene;
}
 

Scene* PocketAi::createConclusionScene(bool isResponse) {
  Scene* scene = new Scene(isResponse ? "RESPONSE SCENE" : "REQUEST SCENE", r);
  addSetupSystem<CharacterSetupSystem>(scene);
  addSetupSystem<UiSetupSystem>(scene, renderer, "UI/conclusion.png");
  addSetupSystem<SpriteSetupSystem>(scene, renderer);
  addRenderSystem<SpriteRenderSystem>(scene);
  addUpdateSystem<SlideShowUpdateSystem>(scene);

  addRenderSystem<PlayerCursorRenderSystem>(scene);
  addSetupSystem<PlayerTextSetupSystem>(
    scene,
    25, 94,
    22, 6, 
    SDL_Color{ 51, 44, 80 }
  );
  addRenderSystem<PlayerTextRenderSystem>(scene);

  if (!isResponse) {  // this is the first conclusion slide
    std::string context = "After spending time with her the last few days, you finally decide to ask her how she feels about you...";

    addSetupSystem<AiConfessionRequestSetupSystem>(scene);
    addUpdateSystem<TextCrawlUpdateSystem>(scene, context, 20);
    addEventSystem<TextCrawlEventSystem>(
      scene,
      context,
      std::bind(&PocketAi::sceneTransition, this)
    );
  } else {
    addSetupSystem<ConversationSetupSystem>(scene, 1);
    addUpdateSystem<AiPromptProcessingSystem>(scene);
    addUpdateSystem<AiPromptPostProcessingSystem>(scene);
    addUpdateSystem<AiEmotionProcessingSystem>(scene);
    addUpdateSystem<AiConversationProgressSystem>(
      scene,
      std::bind(&PocketAi::sceneTransition, this),
      0
    );
   }

  return scene;
}


Scene* PocketAi::createEndingScene() {
  Scene* scene = new Scene("ENDING SCENE", r);
  SpriteComponent sprite = {
        "Backgrounds/ending.png",
        160, 144,
        0, 0,
        30, 1000,
        PixelShader{ nullptr, "" },
        0,
        true,
        1000
  };
  /* addSetupSystem<AffectionSetupSystem>(scene); */
  addSetupSystem<SlideShowSetupSystem>(scene, sprite, 1, 10000);
  addSetupSystem<SpriteSetupSystem>(scene, renderer);
  addSetupSystem<AiEndingSetupSystem>(scene);
  addUpdateSystem<SlideShowUpdateSystem>(scene);
  addRenderSystem<SpriteRenderSystem>(scene);

  addUpdateSystem<SceneTransitionOnSlideUpdateSystem>(
    scene,
    std::bind(&PocketAi::sceneTransition, this)
  );

  return scene;
}
