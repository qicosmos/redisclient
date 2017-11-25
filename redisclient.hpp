//
// Created by qiyu on 11/25/17.
//

#ifndef REDISCLIENT_REDISCLIENT_HPP
#define REDISCLIENT_REDISCLIENT_HPP

#include <string>

class redis_client{
public:
    bool connect(const std::string& name, int port, int timeout){
        return false;
    }

    void disconnect(){}

    template<typename T>
    bool set(const std::string& key, T&& val){

        return false;
    }

    auto get(const std::string& key){

    }

    bool del(const std::string& key){
        return false;
    }
};
#endif //REDISCLIENT_REDISCLIENT_HPP
