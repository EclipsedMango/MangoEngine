
#ifndef MANGORENDERING_RESOURCEMANAGER_H
#define MANGORENDERING_RESOURCEMANAGER_H

#include <memory>
#include <string>
#include <unordered_map>

#include "Renderer/Materials/Texture.h"
#include "Renderer/Materials/Material.h"

class ResourceManager {
public:
    enum class CacheMode {
        Reuse,    // return cached version if it exists (default)
        Replace,  // force a fresh load, replacing the cache entry
    };

    static ResourceManager& Get() {
        static ResourceManager instance;
        return instance;
    }

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    std::shared_ptr<Texture> LoadTexture(const std::string& path, const CacheMode mode = CacheMode::Reuse) {
        if (path.empty()) return nullptr;

        return Load<Texture>(m_textures, path, mode, [&] {
            return std::make_shared<Texture>(path);
        });
    }

    std::shared_ptr<Texture> LoadTexture(const std::string& key, const unsigned char* data, int width, int height, int channels, const CacheMode mode = CacheMode::Reuse) {
        if (key.empty()) return nullptr;

        return Load<Texture>(m_textures, key, mode, [&] {
            return std::make_shared<Texture>(data, width, height, channels, key);
        });
    }

    // TODO: make materials files
    // materials are named, not path-based, since they arent files yet
    std::shared_ptr<Material> LoadMaterial(const std::string& name, const CacheMode mode = CacheMode::Reuse) {
        return Load<Material>(m_materials, name, mode, [&]() {
            return std::make_shared<Material>();
        });
    }

    // duplicate an existing material into a new unique instance
    std::shared_ptr<Material> DuplicateMaterial(const std::string& sourceName) {
        const auto source = LoadMaterial(sourceName);
        return std::make_shared<Material>(*source); // copy construct
    }

    void ReleaseTexture(const std::string& path) { m_textures.erase(path); }
    void ReleaseMaterial(const std::string& name) { m_materials.erase(name); }

    void ReleaseAll() {
        m_textures.clear();
        m_materials.clear();
    }

    // remove any entries whose shared_ptr has expired
    void CollectGarbage() {
        EraseExpired(m_textures);
        EraseExpired(m_materials);
    }

private:
    ResourceManager() = default;

    template<typename T>
    using Cache = std::unordered_map<std::string, std::weak_ptr<T>>;

    // generic loading, handles reuse vs replace and expired weak_ptrs
    template<typename T, typename TFactory>
    std::shared_ptr<T> Load(Cache<T>& cache, const std::string& key, const CacheMode mode, TFactory factory) {
        if (mode == CacheMode::Reuse) {
            if (auto it = cache.find(key); it != cache.end()) {
                if (auto res = it->second.lock()) {
                    return res; // cache hit
                }
            }
        }

        auto res = factory();
        cache[key] = res;
        return res;
    }

    template<typename T>
    void EraseExpired(Cache<T>& cache) {
        for (auto it = cache.begin(); it != cache.end();) {
            it = it->second.expired() ? cache.erase(it) : ++it;
        }
    }

    Cache<Texture> m_textures;
    Cache<Material> m_materials;
};


#endif //MANGORENDERING_RESOURCEMANAGER_H