#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

class Texture;

class ResourceManager
{
public:
    explicit ResourceManager(SDL_Renderer *renderer);
    ~ResourceManager();

    std::string getResourcePath(const std::string &relativePath) const;
    std::string getAssetPath(const std::string &relativePath) const;

    Texture *getTexture(const std::string &path);
    bool loadTexture(const std::string &path);

    void clear();

private:
    SDL_Renderer *renderer_ = nullptr;
    std::filesystem::path executableDirectory_;
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures_;
};