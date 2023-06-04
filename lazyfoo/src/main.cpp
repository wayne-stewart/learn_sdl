
// using SDL and Standard IO
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <unistd.h>

// screen dimensions
const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;

enum TEXTURES {
	TEXTURE_CIRCLES,
	TEXTURE_KITTEN1,
	TEXTURE_KITTEN2,

	TEXTURE_COUNT
};

typedef struct { 
	SDL_Texture* sdl_texture;
	int width;
	int height;
} Texture;


SDL_Window* window = NULL;
SDL_Surface* window_surface = NULL;
SDL_Surface* current_image_surface = NULL;
SDL_Renderer* window_renderer = NULL;
Texture textures[TEXTURE_COUNT];
SDL_Rect sprite_clips[4];

bool init_sdl();
SDL_Surface* load_optimized_image(const char* path);
bool load_texture(const char* path, Texture* texture);
void mod_color(Texture* texture, uint8_t red, uint8_t green, uint8_t blue);
void set_blend_mode(Texture* texture, SDL_BlendMode blend_mode);
void mod_alpha(Texture* texture, uint8_t alpha);

bool load_media();
void cleanup();
void update_window_surface();
void render_sprite(int, int, SDL_Rect*);
void render_sprites();

int main(int argc, char **argv) {
   
	char file_path[260];
	getcwd(file_path, 260);
	printf("CWD: %s\n", file_path);

	if (!init_sdl()) return 1;

	load_media();

    SDL_Event e; 
	bool quit = false; 
    while(!quit) { 
		while(SDL_PollEvent(&e) != 0) {
	    	if (e.type == SDL_QUIT) {
				quit = true;
			}
		}
		
    	SDL_FillRect(window_surface, NULL, SDL_MapRGB(window_surface->format, 0xFF, 0xFF, 0x00));
	    SDL_UpdateWindowSurface(window);

		render_sprites();
		
		// update screen
		SDL_RenderPresent(window_renderer);
    }

	cleanup();

	return 0;
}

void update_window_surface() {
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = SCREEN_WIDTH;
	rect.h = SCREEN_HEIGHT;
	SDL_BlitScaled(current_image_surface, NULL, window_surface, &rect);
	SDL_UpdateWindowSurface(window);
}

void render_sprites() { 
	SDL_SetRenderDrawColor(window_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(window_renderer);

	render_sprite(0,0, &sprite_clips[0]);

	render_sprite(SCREEN_WIDTH - sprite_clips[1].w, 0, &sprite_clips[1]);

	render_sprite(0, SCREEN_HEIGHT - sprite_clips[2].h, &sprite_clips[2]);

	render_sprite(SCREEN_WIDTH - sprite_clips[3].w, SCREEN_HEIGHT - sprite_clips[3].h, &sprite_clips[3]);
}

void render_sprite(int x, int y, SDL_Rect* clip_rect) {
	Texture texture = textures[TEXTURE_CIRCLES];
	SDL_Rect rect = { x, y, texture.width, texture.height };
	rect.w = clip_rect->w;
	rect.h = clip_rect->h;
	SDL_RenderCopy(window_renderer, texture.sdl_texture, clip_rect, &rect);
}

SDL_Surface* load_optimized_image(const char* path) { 
	SDL_Surface* image = IMG_Load(path);
	SDL_Surface* optimized_image = NULL;
	if (image == NULL) { 
		printf("Unable to load image %s! SDL Error: %s\n", path, IMG_GetError());
		return 0;
	}

	optimized_image = SDL_ConvertSurface(image, window_surface->format, 0);
	if (optimized_image == NULL) { 
		printf("Unable to optimize image %s! SDL Error: %s\n", path, SDL_GetError());
		return 0;
	}
	
	SDL_FreeSurface(image);

	return optimized_image;
}

bool load_texture(const char* path, Texture* texture) {

	texture->sdl_texture = NULL;
	texture->width = 0;
	texture->height = 0;

	SDL_Surface* image = IMG_Load(path);

	if (image == NULL) { 
		printf("Unable to load image %s! SDL Error: %s\n", path, IMG_GetError());
		return false;
	}
	
	SDL_SetColorKey(image, SDL_TRUE, SDL_MapRGB(image->format, 0, 0xFF, 0xFF));

	SDL_Texture* sdl_texture = SDL_CreateTextureFromSurface(window_renderer, image);
	if (sdl_texture == NULL) {
		printf("Unable to load texture %s! SDL Error: %s\n", path, SDL_GetError());
		return false;
	}

	texture->sdl_texture = sdl_texture;
	texture->width = image->w;
	texture->height = image->h;

	SDL_FreeSurface(image);

	return true;
}

void mod_color(Texture* texture, uint8_t red, uint8_t green, uint8_t blue)
{
	SDL_SetTextureColorMod(texture->sdl_texture, red, green, blue);
}

void set_blend_mode(Texture* texture, SDL_BlendMode blend_mode) 
{
	SDL_SetTextureBlendMode(texture->sdl_texture, blend_mode);
}

void mod_alpha(Texture* texture, uint8_t alpha)
{
	SDL_SetTextureAlphaMod(texture->sdl_texture, alpha);
}

bool load_media() {

	if (!load_texture("assets/images/kitten1.jpg", &textures[TEXTURE_KITTEN1])) return false;

	if (!load_texture("assets/images/kitten2.jpg", &textures[TEXTURE_KITTEN2])) return false;
	set_blend_mode(&textures[TEXTURE_KITTEN2], SDL_BLENDMODE_BLEND);
	
	if (!load_texture("assets/images/circles.png", &textures[TEXTURE_CIRCLES])) return false;
	mod_color(&textures[TEXTURE_CIRCLES], 100, 100, 100);

	// top left
	sprite_clips[0].x = 0;
	sprite_clips[0].y = 0;
	sprite_clips[0].w = 100;
	sprite_clips[0].h = 100;

	// top right
	sprite_clips[1].x = 100;
	sprite_clips[1].y = 0;
	sprite_clips[1].w = 100;
	sprite_clips[1].h = 100;

	// bottom left
	sprite_clips[2].x = 0;
	sprite_clips[2].y = 100;
	sprite_clips[2].w = 100;
	sprite_clips[2].h = 100;

	// bottom right
	sprite_clips[3].x = 100;
	sprite_clips[3].y = 100;
	sprite_clips[3].w = 100;
	sprite_clips[3].h = 100;

	return true;
}

bool init_sdl() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not be initialized! SDL_Error: %s\n", SDL_GetError());
		return false;
    }

	window = SDL_CreateWindow("SDL Tutorial", 
	  	SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
	   	SDL_WINDOW_SHOWN);

	if (window == NULL) {
    	printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	window_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (window_renderer == NULL) { 
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	SDL_SetRenderDrawColor(window_renderer, 0xFF, 0xFF, 0xFF, 0xFF);

	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) { 
		printf("SDL_image could not be initialized! SDL_image Error: %s\n", IMG_GetError());
		return false;
	}

	window_surface = SDL_GetWindowSurface(window);

	return true;
}


void cleanup() {

	for(int i = 0; i < TEXTURE_COUNT; i++) { 
		if (textures[i].sdl_texture != NULL) { 
			SDL_DestroyTexture(textures[i].sdl_texture);
		}
	}

	SDL_DestroyRenderer(window_renderer);
	SDL_DestroyWindow(window);
    
	IMG_Quit();
	SDL_Quit();
}
