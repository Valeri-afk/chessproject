#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

class Texture;

class ResourceManager
{
public:
    ResourceManager(SDL_Renderer *renderer);
    ~ResourceManager();

    std::string getAssetPath(const std::string &relativePath) const;

    Texture *getTexture(const std::string &path);
    bool loadTexture(const std::string &path);

    void clear();

private:
    SDL_Renderer *renderer;
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
};