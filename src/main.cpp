// SDL2 Hello, World!
// This should display a white screen for 2 seconds
// compile with: clang++ main.cpp -o hello_sdl2 -lSDL2
// run with: ./hello_sdl2
#include <SDL2/SDL.h>
#include <stdio.h>
#include "systems.hpp"
#include "components.hpp"
#include "World.hpp"
#include "abilities.hpp"

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 700
#define FPS 60

class ClientApplication {
public:
	explicit ClientApplication(SDL_Window* window) :
		running(true),
		window(window)
	{
		if (window) {
			renderer = SDL_CreateRenderer(window, -1 /* no driver index */, SDL_WINDOW_OPENGL /* no flags */);
			if (renderer == NULL) {
				fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
			}
		}
		viewxform.viewcenter = {0, 0};
		viewxform.scale = 1;
		viewxform.screensize = {SCREEN_WIDTH, SCREEN_HEIGHT};
		world.addSystem(std::make_shared<MovementSystem>());
		world.addSystem(std::make_shared<CollisionSystem>(500 /* gridwidth */));
		world.addSystem(std::make_shared<DestructibleSystem>());
		world.addSystem(std::make_shared<InputSystem>(&viewxform));
		world.addSystem(std::make_shared<ControlSystem>());
		world.addSystem(std::make_shared<ConditionSystem>());
		world.addSystem(std::make_shared<CollisionHandlerSystem>());
		world.addSystem(std::make_shared<AbilitySystem>());
		world.addSystem(std::make_shared<TimeOutSystem>());
		world.addSystem(std::make_shared<CameraTrackingSystem>(&viewxform));
		world.addSystem(std::make_shared<RenderSystem>(renderer, &viewxform));

		for (int i=0; i<1; i++) {
			auto entity = world.registry.create();
			world.registry.assign<SpatialData>(entity, vec2f{0, 0});
			world.registry.assign<Renderable>(entity, Renderable::Type::Person);
			world.registry.assign<Destructible>(entity, 50);
			world.registry.assign<Controllable>(entity);
			world.registry.assign<Collidable>(entity, Collidable::Circle, 20);
			world.registry.assign<Conditions>(entity);
			world.registry.assign<Stats>(entity, 133 /*movespeed*/, 40 /*accel*/);
			auto &abilitydata = world.registry.assign<AbilityData>(entity);
			abilitydata.abilities.push_back(TestProjectileAbility(&world, entity));
			//abilitydata.abilities.push_back(TestBuffAbility(&world, entity, Condition{Condition::Priority::Multiplier, Condition::Type::MOD_SPEED, 2, 1 /* seconds */}));
			//abilitydata.abilities.push_back(TestAreaEffectTargetAbility(&world, entity));
			//abilitydata.abilities.push_back(TestPuckAbility(&world, entity));
			abilitydata.abilities.push_back(TestHeartseekerAbility(&world, entity));
			world.registry.attach<CameraFocus>(entity);
		}
		for (int i=0; i<8; i++) {
			for (int j=0; j<8; j++) {
				auto entity = world.registry.create();
				world.registry.assign<SpatialData>(entity, 100 * vec2f{i-j+1, j+i+1});
				world.registry.assign<Renderable>(entity, Renderable::Type::Person, SDL_Colors::GREY);
				world.registry.assign<Destructible>(entity, 50);
				world.registry.assign<Collidable>(entity, Collidable::Circle, 20);
				world.registry.assign<Conditions>(entity);
				world.registry.assign<Stats>(entity, 0, 0);
			}
		}
		// floor
		/*
		for (int x=0; x<20; x++) {
		for (int y=0; y<20; y++) {
			auto entity = world.registry.create();
			world.registry.assign<SpatialData>(entity, vec2f{50.0*x, 50.0*y}, vec2f{0, 0}, -50.0);
			world.registry.assign<Renderable>(entity, Renderable::Type::Cube);
			}
		}
		*/
		lastFrameTimeMilliseconds = SDL_GetTicks();
	}

	void init(std::shared_ptr<ClientApplication> sharedptr) {
		// To register an event handler, we need a shared pointer. We obviously
		// can't have a shared pointer to an object during the initalization of
		// that object, and I'd rather not make this static. So here we are,
		// passing a shared pointer to an instance back to the instance
		// immediately after initialization. -_-
		world.bus.reg(sharedptr);
	}

	void close() {
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
	}

	void update() {
		int currentFrameTimeMilliseconds = SDL_GetTicks();
		TimeDelta timePassedSeconds = (currentFrameTimeMilliseconds - lastFrameTimeMilliseconds) / 1000.0;
		timePassedSeconds = 0.03;
		world.update_all(timePassedSeconds);

		lastFrameTimeMilliseconds = currentFrameTimeMilliseconds;
		int postFrameTimeMilliseconds = SDL_GetTicks();
		// If updating was slow, we should sleep less to maintain a consistent framerate
		Uint32 delay = std::max(0, 1000/FPS - (postFrameTimeMilliseconds - currentFrameTimeMilliseconds));
		// printf("%d\n", postFrameTimeMilliseconds-currentFrameTimeMilliseconds);
		if (delay) SDL_Delay(delay);
	}

	void handleEvent(const SDL_Event& e) {}

	void run() {
		while (running)
			update();
	}

	void receive(const WindowExitEvent&) {
		running = false;
	}

private:
	bool running;
	SDL_Window* window;
	SDL_Renderer* renderer;
	World world;
	ViewTransform viewxform;
	int lastFrameTimeMilliseconds;
};

int main(int argc, char* args[]) {
	SDL_Window* window = NULL;
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		fprintf(stderr, "Could not initialize sdl2: %s\n", SDL_GetError());
		return 1;
	}

	window = SDL_CreateWindow(
					"parahumans",
					SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH, SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN // SDL_WINDOW_FULLSCREEN_DESKTOP
					);
	if (window == NULL) {
		fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
		return 1;
	}

	auto app = std::make_shared<ClientApplication>(window);
	app->init(app);
	app->run();
	app->close();
	SDL_Quit();
	return 0;
}
