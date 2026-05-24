#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <algorithm>
#include <utility>

#include "../core/Entity.hpp"
#include "../util/Types.hpp"

namespace mir{
    namespace event{
        namespace type{
            struct Collision { ID Other; };
            struct Attack { ID Other; };
            struct Damaged { ID Other; };
            struct Death {};
            struct Jump {};
            struct ButtonClick {};

            enum class Key{
                Escape,
                Space,
                Enter,
                W,
                A,
                S,
                D,
                F1,
                F2
            };

            struct KeyPressed{
                Key Input;
            };

            struct KeyReleased{
                Key Input;
            };

            struct MousePressed{
                Int X;
                Int Y;
            };

            struct MouseReleased{
                Int X;
                Int Y;
            };
        }

        namespace{
            using SubID = Uint;

            class Base{
            public:
                Base() = default;
                Base(const Base& other) = default;
                Base& operator=(const Base& other) = default;

                Base(Base&& other) noexcept = default;
                Base& operator=(Base&& other) noexcept = default;

                virtual ~Base() = default;
            };

            template<typename T>
            class Listener : public Base{
            public:
                explicit Listener(Action<const T&> callBack) : callback(std::move(callBack)) {}
                void Exec(const T& event){ callback(event); }

            private:
                Action<const T&> callback;
            };

            static inline Dictionary<std::type_index,
                Dictionary<SubID, std::unique_ptr<Base>>> Listeners;

            static inline SubID NextID = 0;
        }

        template<typename T>
        static inline SubID Subscribe(Action<const T&> callback){
            std::type_index typeId = std::type_index(typeid(T));
            SubID id = NextID++;

            Listeners[typeId][id] = std::make_unique<Listener<T>>(std::move(callback));
            return id;
        }

        template<typename T> static inline void Unsubscribe() {
            std::type_index typeId = std::type_index(typeid(T));
            Listeners.erase(typeId);
        }

        template<typename T>
        static inline void Publish(const T& event){
            std::type_index typeId = std::type_index(typeid(T));
            if(Listeners.find(typeId) == Listeners.end()) return;

            List<Base*> currentListeners;
            currentListeners.reserve(Listeners.at(typeId).size());

            for(const auto& [_, pointer] : Listeners.at(typeId)){
                currentListeners.push_back(pointer.get());
            }

            for(Base* listener : currentListeners){
                if(listener) TypeCast<Listener<T>*>(listener)->Exec(event);
            }
        }

        static inline void Clear(){ Listeners.clear(); }
    }
}
