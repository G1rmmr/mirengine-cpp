#pragma once

#include <string>
#include <cstring>
#include <memory>
#include <vector>
#include <fstream>
#include <algorithm>
#include <utility>

#include "Entity.hpp"
#include "Components.hpp"

#include "../handle/Event.hpp"
#include "../util/Debugger.hpp"
#include "../util/Types.hpp"
#include "../util/Math.hpp"

#ifndef ASSET_DIR
    #define ASSET_DIR ""
#endif

#include <SFML/Audio/SoundSource.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>

#define REGISTER_COMPONENT(type, array) \
    { #type, reinterpret_cast<void*>(array.data()), sizeof(array[0]), array.size() }

namespace mir {
    static constexpr Uint MAX_RESOURCES = 0xFF;

    using Tag = Uint;

    namespace record{
        namespace{
            struct ComponentInfo{
                const char* Name;
                void* Data;
                Uint Size;
                Uint Count;
            };

            static inline const List<ComponentInfo> GetAllComponents(){
                return {
                    REGISTER_COMPONENT("Position", transform::Positions),
                    REGISTER_COMPONENT("Velocity", transform::Velocities),
                    REGISTER_COMPONENT("Scale", transform::Scales),
                    REGISTER_COMPONENT("Rotation", transform::Rotations),
                    REGISTER_COMPONENT("Mass", physics::Masses),
                    REGISTER_COMPONENT("Health", stats::Healths),
                };
            }
        }

        static inline void Save(const String& filename){
            std::ofstream file(filename, std::ios::binary);

            const List<ComponentInfo>& components = GetAllComponents();
            Size componentCount = components.size();

            file.write(reinterpret_cast<const char*>(&componentCount), sizeof(Size));

            for(const ComponentInfo& comp : components){
                std::streamsize nameLen = TypeCast<std::streamsize>(std::strlen(comp.Name));
                file.write(reinterpret_cast<const char*>(&nameLen), sizeof(Size));
                file.write(comp.Name, nameLen);

                file.write(reinterpret_cast<const char*>(&comp.Size), sizeof(Size));
                file.write(reinterpret_cast<const char*>(&comp.Count), sizeof(Size));

                std::streamsize dataSize = TypeCast<std::streamsize>(comp.Size * comp.Count);
                file.write(reinterpret_cast<const char*>(comp.Data), dataSize);
            }

            file.close();
            debug::Log("Saved %zu components", componentCount);
        }

        static inline void Load(const String& filename){
            std::ifstream file(filename, std::ios::binary);

            if(!file.is_open()){
                debug::Log("No save file found: %s", filename.c_str());
                return;
            }

            Size componentCount = 0;
            file.read(reinterpret_cast<char*>(&componentCount), sizeof(Size));

            const List<ComponentInfo>& components = GetAllComponents();
            for(Size i = 0; i < componentCount; ++i){
                Size nameLen = 0;
                file.read(reinterpret_cast<char*>(&nameLen), sizeof(Size));

                String name(nameLen, '\0');
                file.read(name.data(), TypeCast<std::streamsize>(nameLen));

                std::streamsize size = 0;
                std::streamsize count = 0;

                file.read(reinterpret_cast<char*>(&size), sizeof(Size));
                file.read(reinterpret_cast<char*>(&count), sizeof(Size));

                List<ComponentInfo>::const_iterator iter = std::find_if(
                    components.begin(), components.end(),[&name](const ComponentInfo& c){
                        return name == c.Name;
                    });

                if(iter == components.end()){
                    file.seekg(size * count, std::ios::cur);
                    debug::Log("Unknown component: %s", name.c_str());
                    continue;
                }
                file.read(reinterpret_cast<char*>(iter->Data), size * count);
            }

            file.close();
            debug::Log("Loaded %zu components", componentCount);
        }
    }

    namespace resource{
        static inline Array<String, MAX_RESOURCES> Textures;
        static inline Array<String, MAX_RESOURCES> Sounds;
        static inline Array<String, MAX_RESOURCES> Fonts;
    }

    namespace texture{
        static inline Tag Create(const String& filepath){
            for(Tag tag = 1; tag < MAX_RESOURCES; ++tag){
                if(resource::Textures[tag] == ""){
                    resource::Textures[tag] = String(ASSET_DIR) + '/' + filepath;
                    return tag;
                }
            }

            debug::Log("resource Overflow!");
            return 0; // Overflow resource
        }

        namespace{
            inline Rectangle DrawRect(const ID id, const Point2<Real>& size){
                Rectangle rect(TypeCast<Point2<Real>>(size));
                rect.setFillColor(sprite::Colors[id]);
                return rect;
            }

            inline Circle DrawCircle(const ID id, const Point2<Real>& size){
                Circle circle(TypeCast<Real>(size.x) / 2);
                circle.setFillColor(sprite::Colors[id]);
                return circle;
            }

            inline PointArr DrawTriangle(const ID id, const Point2<Real>& size){
                PointArr arr(PrimType::Triangles, 3);
                arr[0].position = {0, size.y};
                arr[0].color = sprite::Colors[id];

                arr[1].position = size;
                arr[1].color = sprite::Colors[id];

                arr[2].position = {size.x / 2, 0};
                arr[2].color = sprite::Colors[id];

                return arr;
            }

            inline PointArr DrawArch(const ID id, const Point2<Real>& size){
                const Byte numPoints = 100;
                PointArr arr(PrimType::TriangleFan, numPoints + 2);

                Point2<Real> center = {size.x / 2.0f, size.y / 2.0f};
                Real radius = size.x / 2.0f;

                arr[0].position = center;
                arr[0].color = sprite::Colors[id];

                const Real viewAngle = sprite::Archs[id];
                const Real startAngle = -90.0f - (viewAngle / 2.0f);
                const Real angleInterval = viewAngle / numPoints;

                for(Byte start = 0; start <= numPoints; ++start){
                    const Real angle = startAngle + (TypeCast<Real>(start) * angleInterval);

                    arr[start + 1].position = {
                        center.x + radius * math::Cos(angle),
                        center.y - radius * math::Sin(angle)
                    };

                    arr[start + 1].color = sprite::Colors[id];
                }
                return arr;
            }

            inline PointArr DrawConvexHull(const ID id, const Point2<Real>& size){
                const Uint sidePoint = sprite::NumSide[id];
                PointArr arr(PrimType::TriangleFan, sidePoint + 2);

                const Point2<Real> center = {size.x / 2, size.y / 2};
                const Real radius = size.x / 2;

                arr[0].position = center;
                arr[0].color = sprite::Colors[id];

                const Real angleInterval = 360.0f / TypeCast<Real>(sidePoint);
                for(Byte start = 0; start <= sidePoint; ++start){
                    const Real angle = TypeCast<Real>(start) * angleInterval;

                    arr[start + 1].position = {
                        center.x + radius * math::Cos(angle),
                        center.y - radius * math::Sin(angle)
                    };

                    arr[start + 1].color = sprite::Colors[id];
                }
                return arr;
            }
        }

        static inline void AllocFromType(const ID id){
            Point2<Real> size = physics::Bounds[id];
            if(size.x == 0 && size.y == 0){
                debug::Log("This entity doesn't have size (ID : %d)", id);
                return;
            }

            RenderTex shape;
            if (!shape.resize({TypeCast<Uint>(size.x), TypeCast<Uint>(size.y)})) {
                debug::Log("Failed to create render texture for entity (ID: %d)", id);
                return;
            }
            shape.clear(Color::Transparent);

            switch(sprite::Types[id]){
            case sprite::Type::Rectangle:{
                shape.draw(DrawRect(id, size));
                break;
            }
            case sprite::Type::Circle:{
                shape.draw(DrawCircle(id, size));
                break;
            }
            case sprite::Type::Triangle:{
                shape.draw(DrawTriangle(id, size));
                break;
            }
            case sprite::Type::Arch:{
                shape.draw(DrawArch(id, size));
                break;
            }
            case sprite::Type::ConvexHull:{
                shape.draw(DrawConvexHull(id, size));
                break;
            }
            default:
                debug::Log("This entity doesn't have shape (ID : %d)", id);
                return;
            }

            shape.display();

            sprite::Textures[id] = std::make_unique<Texture>(shape.getTexture());
        }

        static inline void AllocFromFile(const ID id, const Tag tag){
            const String path = resource::Textures[tag];
            if(path == "") {
                debug::Log("Texture doesn't initialized : %s", path.c_str());
                return;
            }

            std::unique_ptr<Texture> texture = std::make_unique<Texture>();
            if (!texture->loadFromFile(path)) {
                debug::Log("Texture doesn't exist : %s", path.c_str());
                return;
            }
            sprite::Textures[id] = std::move(texture);
        }

        static inline void Delete(const ID id){
            sprite::Textures[id].reset();
        }

        static inline void Clear(){
            for(ID id = 1; id < MAX_ENTITIES; ++id)
                Delete(id);
        }
    }

    namespace sound{
        static inline Array<std::unique_ptr<SoundBuf>, MAX_RESOURCES> Buffers;
        static inline Array<std::unique_ptr<Sound>, MAX_RESOURCES> Sounds;
        static inline Array<Music, MAX_RESOURCES> Musics;

        static inline Tag Create(const String& filepath){
            for(Tag tag = 1; tag < MAX_RESOURCES; ++tag){
                if(resource::Sounds[tag] == ""){
                    resource::Sounds[tag] = String(ASSET_DIR) + '/' + filepath;
                    return tag;
                }
            }

            debug::Log("resource Overflow!");
            return 0; // Overflow resource
        }

        static inline void AllocSound(const Tag tag){
            Buffers[tag] = std::make_unique<SoundBuf>();
            if(!Buffers[tag]->loadFromFile(resource::Sounds[tag])){
                debug::Log("Sound Source doesn't exist : %s", resource::Sounds[tag].c_str());
                return;
            }

            Sounds[tag] = std::make_unique<Sound>(*Buffers[tag]);
        }

        static inline void AllocMusic(const Tag tag, Bool shouldLoop = true){
            if(!Musics[tag].openFromFile(resource::Sounds[tag])){
                debug::Log("Music Source doesn't exist : %s", resource::Sounds[tag].c_str());
                return;
            }

            Musics[tag].setLooping(shouldLoop);
            Musics[tag].play();
        }

        static inline void Play(const Tag tag, Bool shouldLoop = false) {
            if(!Sounds[tag]) return;
            Sounds[tag]->setLooping(shouldLoop);
            Sounds[tag]->play();
        }

        static inline void Pause(const Tag tag) {
            if(Sounds[tag]) Sounds[tag]->pause();
        }

        static inline void Stop(const Tag tag) {
            if(Sounds[tag]) Sounds[tag]->stop();
            if(Musics[tag].getStatus() != SoundSrc::Status::Stopped) Musics[tag].stop();
        }

        static inline void Delete(const Tag tag){
            Stop(tag);
            Buffers[tag].reset();
            Sounds[tag].reset();
        }

        static inline void Clear(){
            for(Tag tag = 1; tag < MAX_RESOURCES; ++tag)
                Delete(tag);
        }
    }

    namespace font{
        static inline Array<std::unique_ptr<Font>, MAX_RESOURCES> Sources;
        static inline Array<std::unique_ptr<Text>, MAX_RESOURCES> Texts;

        static inline Tag Create(const String& filepath){
            for(Tag tag = 1; tag < MAX_RESOURCES; ++tag){
                if(resource::Fonts[tag] == ""){
                    resource::Fonts[tag] = String(ASSET_DIR) + '/' + filepath;
                    return tag;
                }
            }

            debug::Log("resource Overflow!");
            return 0; // Overflow resouce
        }

        static inline void Alloc(const Tag tag){
            Sources[tag] = std::make_unique<Font>();
            if(!Sources[tag]->openFromFile(resource::Fonts[tag])){
                debug::Log("Font doesn't exist : %s", resource::Fonts[tag].c_str());
                Sources[tag].reset();
                return;
            }
            Texts[tag] = std::make_unique<Text>(*Sources[tag]);
        }

        static inline void Delete(const Tag tag){
            Sources[tag].reset();
            Texts[tag].reset();
        }

        static inline void Clear(){
            for(Tag tag = 1; tag < MAX_RESOURCES; ++tag)
                Delete(tag);
        }
    }

    static inline void Clear(){
        transform::Clear();
        sprite::Clear();
        animation::Clear();
        stats::Clear();
        event::Clear();
        particle::Clear();

        texture::Clear();
        sound::Clear();
        font::Clear();

        entity::Clear();
    }
}
