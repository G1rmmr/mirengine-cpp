#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <atomic>

#include <SFML/Network/Http.hpp>

#include "../util/Types.hpp"
#include "../util/Debugger.hpp"

namespace mir{
    namespace http{
        enum class Type{
            Get,
            Post
        };

        struct Set{
            Action<const HTTP::Response&> Callback;
            String Host;
            String EndPoint;
            Uint Port;
        };

        namespace{
            inline std::atomic<Int> ActivatedRequsts = 0;

            static inline String BuildQuery(Dictionary<String, String> params){
                if(params.empty()) {
                    debug::Log("HTTP Parameters are empty!");
                    return "";
                }

                std::stringstream stream;
                bool isFirst = true;

                for(const auto& [key, val] : params){
                    if(!isFirst) stream << "&";
                    stream << key << "=" << val;
                    isFirst = false;
                }

                params.clear();
                return stream.str();
            }
        }

        static inline void Request(Type type, const Dictionary<String, String>& params, const Set& set){
            ActivatedRequsts++;

            FireAndForget([type, params, set](){
                HTTP http(set.Host, set.Port);
                HTTP::Request request;

                const String query = BuildQuery(params);

                switch(type){
                case Type::Get:
                    request = {
                        set.EndPoint + (query.empty() ? "" : "?" + query),
                        HTTP::Request::Method::Get
                    };
                    break;
                case Type::Post:
                    request = {
                        set.EndPoint,
                        HTTP::Request::Method::Post,
                        query
                    };
                    request.setField("Content-Type", "application/x-www-form-urlencoded");
                    break;

                default: break;
                }

                HTTP::Response response = http.sendRequest(request);

                if(response.getStatus() == HTTP::Response::Status::Ok ||
                    response.getStatus() == HTTP::Response::Status::Created){
                    debug::Log("[Network] Success: %s (Status: %d)",
                        set.EndPoint.c_str(), TypeCast<Int>(response.getStatus()));
                }
                else{
                    debug::Log("[Network] Failed: %s (Status: %d, Body: %s)",
                        set.EndPoint.c_str(), TypeCast<Int>(response.getStatus()), response.getBody().c_str());
                }

                if(set.Callback) set.Callback(response);

                ActivatedRequsts--;
            });
        }

        static inline void Wait(const Uint miliSec){
            const std::chrono::milliseconds mili = TypeCast<std::chrono::milliseconds>(miliSec);
            while(ActivatedRequsts != 0)
                std::this_thread::sleep_for(mili);
        }
    }
}
