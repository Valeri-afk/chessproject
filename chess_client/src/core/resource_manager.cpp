#include "resource_manager.hpp"

#include <iostream>
#include <system_error>

#include "texture.hpp"

ResourceManager::ResourceManager(SDL_Renderer *renderer)
    : renderer_(renderer)
{
    const char *basePath = SDL_GetBasePath();
    if (basePath)
        executableDirectory_ = std::filesystem::path(basePath);
}

ResourceManager::~ResourceManager() = default;

std::string ResourceManager::getResourcePath(const std::string &relativePath) const
{
    if (executableDirectory_.empty())
        return relativePath;

    return (executableDirectory_ / std::filesystem::path(relativePath)).string();
}

std::string ResourceManager::getAssetPath(const std::string &relativePath) const
{
    return getResourcePath((std::filesystem::path("assets") / relativePath).string());
}

Texture *ResourceManager::getTexture(const std::string &path)
{
    auto it = textures_.find(path);
    if (it != textures_.end())
        return it->second.get();

    if (loadTexture(path))
        return textures_[path].get();

    return nullptr;
}

bool ResourceManager::loadTexture(const std::string &path)
{
    const std::string fullPath = getAssetPath(path);
    SDL_Texture *sdlTex = IMG_LoadTexture(renderer_, fullPath.c_str());
    if (!sdlTex)
    {
        std::cerr << "Failed to load texture: " << fullPath
                  << " - " << SDL_GetError() << std::endl;
        return false;
    }

    textures_[path] = std::make_unique<Texture>(renderer_, sdlTex);
    return true;
}

void ResourceManager::clear()
{
    textures_.clear();
}
