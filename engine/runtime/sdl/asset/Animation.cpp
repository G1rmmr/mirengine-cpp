#include "asset/Animation.hpp"
#include "component/Sprite.hpp"
#include "core/Manager.hpp"
#include <container/Map.hpp>
#include <container/SparseSet.hpp>
#include <cmath>

namespace std {
    template <std::size_t C>
    struct hash<mir::String<C>> {
        std::size_t operator()(const mir::String<C>& str) const noexcept {
            return std::hash<std::string_view>{}(str.View());
        }
    };
}

namespace mir::animation {
    namespace {
        struct AnimationData {
            Frames Frames;
        };

        struct ActiveAnimation {
            Id EntityId;
            String<> AnimName;
            float Speed = 1.f;
            bool Loop = true;
            std::size_t CurrentFrameIndex = 0;
            float ElapsedTime = 0.f;
            bool IsPlaying = false;
        };

        inline Map<String<>, AnimationData, 1024> animationRegistry;
        inline SparseSet<ActiveAnimation, MAX_ID> activeAnimations;
        bool isSystemAdded = false;

        struct PlayPayload {
            ActiveAnimation Animation;
        };

        void RemoveActiveAnimation(const Id id) noexcept {
            if (!activeAnimations.Contains(id.Index)) {
                return;
            }

            const ActiveAnimation& active = activeAnimations.Get(id.Index);
            if (active.EntityId.Generation == id.Generation) {
                activeAnimations.Remove(id.Index);
            }
        }

        void ApplyPlay(const void* rawPayload) noexcept {
            const auto& payload = *static_cast<const PlayPayload*>(rawPayload);
            if (mir::core::Manager::Instance().IsValidEntity(payload.Animation.EntityId)) {
                activeAnimations.Assign(payload.Animation.EntityId.Index, payload.Animation);
            }
        }

        void UpdateSystem(const float deltaTime) noexcept {
            for (auto& activeAnim : activeAnimations) {
                if (!activeAnim.IsPlaying) {
                    continue;
                }

                if (activeAnim.Speed <= 0.f) {
                    continue;
                }

                const auto* animData = animationRegistry.Find(activeAnim.AnimName);
                if (animData == nullptr || animData->Frames.Size() == 0) {
                    activeAnim.IsPlaying = false;
                    continue;
                }

                const float frameDuration = 0.1f / activeAnim.Speed;
                activeAnim.ElapsedTime += deltaTime;

                if (activeAnim.ElapsedTime >= frameDuration) {
                    const std::size_t totalFrames = animData->Frames.Size();
                    const int framesToAdvance = static_cast<int>(activeAnim.ElapsedTime / frameDuration);
                    activeAnim.ElapsedTime = std::fmod(activeAnim.ElapsedTime, frameDuration);

                    activeAnim.CurrentFrameIndex += framesToAdvance;

                    if (activeAnim.CurrentFrameIndex >= totalFrames) {
                        if (activeAnim.Loop) {
                            activeAnim.CurrentFrameIndex %= totalFrames;
                        } else {
                            activeAnim.CurrentFrameIndex = totalFrames - 1;
                            activeAnim.IsPlaying = false;
                        }
                    }

                    const Frame& frame = animData->Frames[activeAnim.CurrentFrameIndex];
                    sprite::SetSourceRect(activeAnim.EntityId, frame.X, frame.Y, frame.Width, frame.Height);
                }
            }
        }

        void EnsureSystemAdded() noexcept {
            if (!isSystemAdded) {
                isSystemAdded = mir::core::Manager::Instance().AddSystem(UpdateSystem);
            }
        }
    }

    void Register(const String<>& name, const Frames& frames) noexcept {
        AnimationData data;
        data.Frames = frames;
        animationRegistry.Insert(name, data);
    }

    void Play(const Id id, const String<>& animName, float speed, bool loop) noexcept {
        if (!mir::core::Manager::Instance().IsValidEntity(id)) {
            return;
        }

        EnsureSystemAdded();

        const auto* animData = animationRegistry.Find(animName);
        if (animData == nullptr || animData->Frames.Size() == 0) {
            return;
        }

        ActiveAnimation activeAnim{
            id,
            animName,
            speed,
            loop,
            0,
            0.f,
            true
        };

        if (!isSystemAdded || !mir::core::Manager::Instance().AddComponent<PlayPayload>(
                &ApplyPlay, PlayPayload{activeAnim}, &RemoveActiveAnimation)) {
            return;
        }

        const Frame& frame = animData->Frames[0];
        sprite::SetSourceRect(id, frame.X, frame.Y, frame.Width, frame.Height);
    }

    void Stop(const Id id) noexcept {
        if (activeAnimations.Contains(id)) {
            activeAnimations.Remove(id);
        }
    }
}
