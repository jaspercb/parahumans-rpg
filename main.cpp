// SDL2 Hello, World!
// This should display a white screen for 2 seconds
// compile with: clang++ main.cpp -o hello_sdl2 -lSDL2
// run with: ./hello_sdl2
#include <SDL2/SDL.h>
#include <stdio.h>
#include "systems.hpp"
#include "components.hpp"
#include "World.hpp"

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 700
#define FPS 60

class ClientApplication {
public:
	explicit ClientApplication(SDL_Window* window) :
		window(window),
		running(true)
	{
		if (window) {
			renderer = SDL_CreateRenderer(window, -1 /* no driver index */, SDL_WINDOW_OPENGL /* no flags */);
			if (renderer == NULL) {
				fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
			}
		}

		world.addSystem(std::make_shared<MovementSystem>());
		world.addSystem(std::make_shared<RenderSystem>(renderer));
		world.addSystem(std::make_shared<CollisionSystem>(100 /* gridwidth */));
		world.addSystem(std::make_shared<DestructibleSystem>());
		world.addSystem(std::make_shared<InputSystem>());

		for (int i=0; i<1; i++) {
			auto entity = world.registry.create();
			world.registry.assign<SpatialData>(entity, vec2f(10*i, 10*i));
			world.registry.assign<Renderable>(entity, Renderable::Type::Person);
			world.registry.assign<Destructible>(entity, 50);
			world.registry.assign<Controllable>(entity);
			controlled = entity;
		}
		for (int i=0; i<10; i++) {
			auto entity = world.registry.create();
			world.registry.assign<SpatialData>(entity, vec2f(60*i, 0));
			world.registry.assign<Renderable>(entity, Renderable::Type::Cube);      
		}
		for (int x=0; x<20; x++) {
		for (int y=0; y<20; y++) {
			auto entity = world.registry.create();
			world.registry.assign<SpatialData>(entity, vec2f(50*x, 50*y), vec2f(0, 0), -50.0);
			world.registry.assign<Renderable>(entity, Renderable::Type::Cube);
			}
		}
		lastFrameTimeMilliseconds = SDL_GetTicks();
	}

	void init(std::shared_ptr<ClientApplication> sharedptr) {
		// To register a __, we need a shared pointer. We obviously can't have
		// a shared pointer during initalization, and I'd rather not make this
		// static. So here we are, passing a shared pointer to an instance back
		// to the instance immediately after initialization. -_-
		world.bus.reg(sharedptr);
	}

	void update() {
		int currentFrameTimeMilliseconds = SDL_GetTicks();
		TimeDelta timePassedSeconds = (currentFrameTimeMilliseconds - lastFrameTimeMilliseconds) / 1000.0;

		world.update_all(timePassedSeconds);

		lastFrameTimeMilliseconds = currentFrameTimeMilliseconds;
		int postFrameTimeMilliseconds = SDL_GetTicks();
		// If updating was slow, we should sleep less to maintain a consistent framerate
		Uint32 delay = std::max(0, 1000/FPS - (postFrameTimeMilliseconds - currentFrameTimeMilliseconds));
		printf("%d\n", postFrameTimeMilliseconds-currentFrameTimeMilliseconds);
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
	Entity controlled;
	int lastFrameTimeMilliseconds;
};

int main(int argc, char* args[]) {
	SDL_Window* window = NULL;
	SDL_Surface* screenSurface = NULL;
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
	SDL_Quit();
	return 0;
}
