#ifndef __RESOURCE_LOAD_H__
#define __RESOURCE_LOAD_H__

#include <SDL2/SDL.h>
#include <string>

bool openResourceFile(const char *filename);

SDL_RWops *getResourceRW(const char *RES_ID);

std::string loadStringFromResource(const char *RES_ID);

#define BINARY_START(name) _binary_##name##_start
#define BINARY_END(name) _binary_##name##_end
#define BINARY_SIZE(name) (BINARY_END(name) - BINARY_START(name))

#define BINARY_DECLARE(name) extern char BINARY_START(name)[]; extern char BINARY_END(name)[];
#define BINARY_STRING(name) std::string(BINARY_START(name), BINARY_SIZE(name))

#endif // __RESOURCES_H__
