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
		window(window)
	{
		if (window) {
			renderer = SDL_CreateRenderer(window, -1 /* no driver index */, SDL_WINDOW_OPENGL /* no flags */);
			if (renderer == NULL) {
				fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
			}
		}

		world.addSystem(std::make_unique<MovementSystem>());
		world.addSystem(std::make_unique<RenderSystem>(renderer));

		for (int i=0; i<1; i++) {
			auto entity = world.registry.create();
			world.registry.assign<SpatialData>(entity, vec2f(10*i, 10*i));
			world.registry.assign<Renderable>(entity, Renderable::Type::Person);
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
		SDL_Event e;
		bool quit = false;

		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		while (!quit) {
			while (SDL_PollEvent(&e)) {
				if (e.type == SDL_QUIT) quit = true;
			}
			// Static key effects
			if (world.registry.valid(controlled) && world.registry.has<SpatialData>(controlled)) {
				SpatialData& sdata = world.registry.get<SpatialData>(controlled);
				vec2f accel;
				if (keys[SDL_SCANCODE_LEFT]) {
					accel.x -= 1;
					accel.y += 1;
				}
				if (keys[SDL_SCANCODE_RIGHT]) {
					accel.x += 1;
					accel.y -= 1;
				}
				if (keys[SDL_SCANCODE_UP]) {
					accel.x -= 1;
					accel.y -= 1;
				}
				if (keys[SDL_SCANCODE_DOWN]) {
					accel.x += 1;
					accel.y += 1;
				}
				accel.normalize();
				accel *= 20;
				sdata.velocity += accel;
				if (sdata.velocity.x || sdata.velocity.y)
					sdata.orientation = atan2f(sdata.velocity.y, sdata.velocity.x);
				sdata.velocity *= 0.85;
			}
			update();
		}
	}

private:
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

	ClientApplication app(window);
	app.run();
	SDL_Quit();
	return 0;
}
