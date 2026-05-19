#include <type_traits>
#include <SFML/Graphics/Font.hpp>

template <typename Resource, typename Identifier>
bool ResourceHolder<Resource, Identifier>::loadFromFile(Identifier id, const std::string& path)
{
    if (path == "Default")
        return false;
        
    std::unique_ptr<Resource> resource(new Resource());

    // SFML 3: Fonts stream, so they use openFromFile instead of loadFromFile
    if constexpr (std::is_same_v<Resource, sf::Font>)
    {
        if (!resource->openFromFile(path))
        {
            std::cerr << "Failed to load resource - \"" + path + "\"\n";
            return false;
        }
    }
    else
    {
        if (!resource->loadFromFile(path))
        {
            std::cerr << "Failed to load resource - \"" + path + "\"\n";
            return false;
        }
    }
    
    insertResource(id, std::move(resource));
    return true;
}

template <typename Resource, typename Identifier>
void ResourceHolder<Resource, Identifier>::loadFromMemory(Identifier id
                                                , const void* data
                                                , std::size_t sizeInBytes)
{
    std::unique_ptr<Resource> resource(new Resource());
    
    // SFML 3: Fonts stream, so they use openFromMemory
    if constexpr (std::is_same_v<Resource, sf::Font>)
    {
        if (!resource->openFromMemory(data, sizeInBytes))
            throw std::runtime_error("ResourceHolder::loadFromMemory - Failed to load default resource");
    }
    else
    {
        if (!resource->loadFromMemory(data, sizeInBytes))
            throw std::runtime_error("ResourceHolder::loadFromMemory - Failed to load default resource");
    }
    
    insertResource(id, std::move(resource));
}

template <typename Resource, typename Identifier>
Resource& ResourceHolder<Resource, Identifier>::get(Identifier id) const
{
    auto found = mResourceMap.find(id);
    assert(found != mResourceMap.end());

    return *found->second;
}

template <typename Resource, typename Identifier>
void ResourceHolder<Resource, Identifier>::clear()
{
    mResourceMap.clear();
}

template <typename Resource, typename Identifier>
void ResourceHolder<Resource, Identifier>::insertResource(Identifier id, std::unique_ptr<Resource> resource)
{
    auto inserted = mResourceMap.insert(std::make_pair(id, std::move(resource)));
    assert(inserted.second);
}