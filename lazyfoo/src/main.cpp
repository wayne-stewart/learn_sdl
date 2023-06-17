
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
	TEXTURE_FONT_MATRIX_TITLE,
	TEXTURE_FONT_MATRIX_CODE,

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
TTF_Font* font_matrix_title = NULL;
TTF_Font* font_matrix_code = NULL;
Texture textures[TEXTURE_COUNT];
SDL_Rect sprite_clips[4];
uint8_t alpha_fade = 1;
uint8_t alpha = 0;

bool init_sdl();
SDL_Surface* load_optimized_image(const char* path);
bool load_texture(const char* path, Texture* texture);
void mod_color(Texture* texture, uint8_t red, uint8_t green, uint8_t blue);
void set_blend_mode(Texture* texture, SDL_BlendMode blend_mode);
void mod_alpha(Texture* texture, uint8_t alpha);
bool load_font_texture(const char* text, SDL_Color color, Texture* texture);

bool load_media();
void cleanup();
void update_window_surface();
void render_sprite(Texture*, int, int, SDL_Rect*, SDL_Rect*);
void render_sprite_fullscreen(Texture* texture);
void render_circles();

int main(int argc, char **argv) {
   
	//char file_path[260];
	//getcwd(file_path, 260);
	//printf("CWD: %s\n", file_path);

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
	
		//SDL_SetRenderDrawColor(window_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		//SDL_RenderClear(window_renderer);

    	//SDL_FillRect(window_surface, NULL, SDL_MapRGB(window_surface->format, 0xFF, 0xFF, 0x00));
	    //SDL_UpdateWindowSurface(window);

		render_sprite_fullscreen(&textures[TEXTURE_KITTEN1]);

		alpha += alpha_fade;
		if (alpha >= 255) { alpha_fade = -1; alpha = 255; }
		if (alpha <= 0) { alpha_fade = 1; alpha = 0; }
		mod_alpha(&textures[TEXTURE_KITTEN2], alpha);
		render_sprite_fullscreen(&textures[TEXTURE_KITTEN2]);

		render_circles();

		Texture* texture = &textures[TEXTURE_FONT_MATRIX_TITLE];
		SDL_Rect size_rect { 0, 0, texture->width, texture->height };
		SDL_Rect clip_rect { 0, 0, texture->width, texture->height };
		int x = (SCREEN_WIDTH - size_rect.w)/2;
		int y = (SCREEN_HEIGHT - size_rect.h)/2;
		render_sprite(texture, x, y, &clip_rect, &size_rect);
		
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

void render_circles() { 
	
	Texture* texture = &textures[TEXTURE_CIRCLES];

	SDL_Rect size_rect = { 0, 0, texture->width / 2, texture->height / 2 };

	render_sprite(texture, 0,0, &sprite_clips[0], &size_rect);

	render_sprite(texture, SCREEN_WIDTH - sprite_clips[1].w, 0, &sprite_clips[1], &size_rect);

	render_sprite(texture, 0, SCREEN_HEIGHT - sprite_clips[2].h, &sprite_clips[2], &size_rect);

	render_sprite(texture, SCREEN_WIDTH - sprite_clips[3].w, SCREEN_HEIGHT - sprite_clips[3].h, &sprite_clips[3], &size_rect);
}

void render_sprite(Texture* texture, int x, int y, SDL_Rect* clip_rect, SDL_Rect* size_rect) {
	size_rect->x = x;
	size_rect->y = y;
	SDL_RenderCopy(window_renderer, texture->sdl_texture, clip_rect, size_rect);
}

void render_sprite_fullscreen(Texture* texture) { 
	SDL_Rect render_rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	SDL_Rect clip_rect = { 0, 0, texture->width, texture->height };
	SDL_RenderCopy(window_renderer, texture->sdl_texture, &clip_rect, &render_rect);
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

bool load_font_texture(const char* text, SDL_Color color, Texture* texture)
{
	texture->sdl_texture = NULL;
	texture->width = 0;
	texture->height = 0;

	SDL_Surface* text_surface = TTF_RenderText_Solid(font_matrix_title, text, color);
	if (text_surface == NULL) {
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}

	texture->sdl_texture = SDL_CreateTextureFromSurface(window_renderer, text_surface);
	if (texture->sdl_texture == NULL) { 
		printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	texture->width = text_surface->w;
	texture->height = text_surface->h;

	SDL_FreeSurface(text_surface);

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

	const char* matrix_code_title = "assets/fonts/Miltown/Miltown_.ttf";
	font_matrix_title = TTF_OpenFont(matrix_code_title, 72);
	if (font_matrix_title == NULL) { 
		printf("Failed to load font %s\n SDL_ttf Error: %s\n", matrix_code_title, TTF_GetError());
		return false;
	}
	SDL_Color text_color = { 75, 255, 125 };
	if (!load_font_texture(" THE MATRIX ", text_color, &textures[TEXTURE_FONT_MATRIX_TITLE])) return false;

	const char* matrix_code_path = "assets/fonts/matrix_code_font.ttf";
	font_matrix_code = TTF_OpenFont(matrix_code_path, 64);
	if (font_matrix_code == NULL) { 
		printf("Failed to load font %s\n SDL_ttf Error: %s\n", matrix_code_path, TTF_GetError());
		return false;
	}

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

	if (TTF_Init() == -1) { 
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
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

	TTF_CloseFont(font_matrix_title);
	TTF_CloseFont(font_matrix_code);
	SDL_DestroyRenderer(window_renderer);
	SDL_DestroyWindow(window);
    
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}
