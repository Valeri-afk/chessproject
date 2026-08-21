#include "resource_manager.hpp"

#include <fstream>
#include <iostream>

#include "texture.hpp"

ResourceManager::ResourceManager(SDL_Renderer *renderer) : renderer(renderer) {}

ResourceManager::~ResourceManager() {}

Texture *ResourceManager::getTexture(const std::string &path)
{
    auto it = textures.find(path);
    if (it != textures.end())
        return it->second.get();

    if (loadTexture(getAssetPath(path)))
    {
        return textures[path].get();
    }
    return nullptr;
}

std::string ResourceManager::getAssetPath(
    const std::string &relativePath) const
{
    return std::string(SDL_GetBasePath()) +
           "assets/" +
           relativePath;
}

bool ResourceManager::loadTexture(const std::string &path)
{
    SDL_Texture *sdlTex = IMG_LoadTexture(renderer, getAssetPath(path).c_str());
    if (!sdlTex)
    {
        std::cerr << "Failed to load texture: " << path << " - " << SDL_GetError() << std::endl;
        return false;
    }
    textures[path] = std::make_unique<Texture>(renderer, sdlTex);
    return true;
}

void ResourceManager::clear()
{
    textures.clear();
}