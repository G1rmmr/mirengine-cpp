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
                    stream << key.c_str() << "=" << val.c_str();
                    isFirst = false;
                }

                params.clear();
                return String(stream.str());
            }
        }

        static inline void Request(Type type, const Dictionary<String, String>& params, const Set& set){
            ActivatedRequsts++;

            FireAndForget([type, params, set](){
                HTTP http(set.Host.c_str(), set.Port);
                HTTP::Request request;

                const String query = BuildQuery(params);

                request.setMethod(type == Type::Get ? HTTP::Request::Method::Get : HTTP::Request::Method::Post);
                
                if (type == Type::Get) {
                    String uri = set.EndPoint;
                    if (!query.empty()) {
                        uri += "?";
                        uri += query;
                    }
                    request.setUri(uri.c_str());
                } else {
                    request.setUri(set.EndPoint.c_str());
                    request.setBody(query.c_str());
                    request.setField("Content-Type", "application/x-www-form-urlencoded");
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
