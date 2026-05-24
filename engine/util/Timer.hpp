#pragma once

#include <vector>
#include <functional>
#include <chrono>
#include <ctime>
#include <utility>

#include <SFML/System/Clock.hpp>

#include "Types.hpp"

namespace mir{
    namespace time{
        namespace{
            struct TimerTask{
                Action<> CallBack;
                Real IntervalSec;
                Real RemainSec;
                Bool IsLooping;
            };

            inline List<TimerTask> TimerTasks;
        }

        struct Set{
            Int Hour;
            Int Minute;
            Int Second;
            Int MiliSec;
        };

        static inline Set GetLocalTime(){
            const std::chrono::time_point now = std::chrono::system_clock::now();
            const time_t time = std::chrono::system_clock::to_time_t(now);
            const std::chrono::milliseconds ms = duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % 1000;

            std::tm* localTime = std::localtime(&time);

            return {
                localTime->tm_hour,
                localTime->tm_min,
                localTime->tm_sec,
                TypeCast<Int>(ms.count())
            };
        }

        static inline Real GetDelta(){
            static sf::Clock deltaClock;
            return deltaClock.restart().asSeconds();
        }

        static inline void Register(
            const Real seconds,
            Action<> callback,
            Bool isLooping = false){
            TimerTasks.push_back({std::move(callback), seconds, seconds, isLooping});
        }

        static inline void Update(const Real deltaTime){
            if(TimerTasks.empty()) return;

            for(Size i = 0; i < TimerTasks.size(); ++i){
                TimerTasks[i].RemainSec -= deltaTime;

                if(TimerTasks[i].RemainSec > 0) continue;
                if(TimerTasks[i].CallBack) TimerTasks[i].CallBack();
                if(TimerTasks[i].IsLooping){
                    TimerTasks[i].RemainSec += TimerTasks[i].IntervalSec;
                    continue;
                }
            }

            std::erase_if(TimerTasks, [](const TimerTask& task) {
                return !task.IsLooping && task.RemainSec <= 0;
            });
        }
    }
}
