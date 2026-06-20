#pragma once

#include <Mir>

namespace network{
    const mir::String HOST = "127.0.0.1";
    const mir::Uint PORT = 8080;

    static inline void PostScore(const mir::String& name, const mir::Uint score){
        mir::Dictionary<mir::String, mir::String> params = {
            {"name", name},
            {"score", mir::ToString(score)}
        };

        mir::http::Set set = { [](const mir::HTTP::Response& response){
            if(response.getStatus() == mir::HTTP::Response::Status::Ok ||
               response.getStatus() == mir::HTTP::Response::Status::Created){
                mir::debug::Log("[Network] PostScore Response: %s", response.getBody().c_str());
            }
            else
                mir::debug::Log("[Network] PostScore Failed: %s", response.getBody().c_str());
        }, HOST, "/submit", PORT};

        mir::http::Request(mir::http::Type::Post, params, set);
    }

    static inline void GetTopPlayers(){
        mir::Dictionary<mir::String, mir::String> params = {};
        mir::http::Set set = {[](const mir::HTTP::Response& response){
            if(response.getStatus() == mir::HTTP::Response::Status::Ok)
                mir::debug::Log("[Network] Top 5 Players:\n%s", response.getBody().c_str());
            else
                mir::debug::Log("[Network] GetTopPlayers Failed: %s", response.getBody().c_str());
        }, HOST, "/top5", PORT};
        mir::http::Request(mir::http::Type::Get, params, set);
    }
}
