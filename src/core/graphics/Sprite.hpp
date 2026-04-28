#ifndef PROJECTCOMPLEXITY_SPRITE_H
#define PROJECTCOMPLEXITY_SPRITE_H

#include <SDL3/SDL.h>

struct Sprite {

    SDL_Texture* texture;
    SDL_FRect srcRect;

};

#endif //PROJECTCOMPLEXITY_SPRITE_H