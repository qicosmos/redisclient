//
// Created by qiyu on 11/25/17.
//

#ifndef REDISCLIENT_REDISCLIENT_HPP
#define REDISCLIENT_REDISCLIENT_HPP

#include <string>
#include <iostream>
#include <inttypes.h>
#include <hiredis.h>
using namespace std::string_literals;

namespace detail{
    template<typename T>
    constexpr bool  is_int64_v = std::is_same_v<T, std::int64_t>;

    template<typename T>
    constexpr bool  is_uint64_v = std::is_same_v<T, std::uint64_t>;

    template<typename T>
    constexpr bool  is_64_v = std::is_same_v<T, std::int64_t>||std::is_same_v<T, std::uint64_t>;

    template<typename T>
    constexpr bool  is_string_v = std::is_same_v<T, std::string>;

    template<typename T>
    constexpr bool  is_cstr_v = std::is_same_v<T, const char*>;

    template<typename T>
    constexpr bool  is_char_array_v = std::is_array_v<T>&&std::is_same_v<std::remove_reference_t<decltype(*std::declval<T>())>, char>;
}
namespace redisclient{
    class redis_client{
    public:
        bool connect(const std::string& hostname, int port, int timeout=3){
            struct timeval tm = { timeout, 0 };

            con_ = redisConnectWithTimeout(hostname.data(), port, tm);
            if (con_ == NULL || con_->err) {
                if (con_) {
                    std::cout<<"Connection error: "<<con_->errstr<<std::endl;
                    redisFree(con_);
                } else {
                    std::cout<<"Connection error: can't allocate redis context"<<std::endl;
                }
                return false;
            }

            return true;
        }

        void disconnect(){
            if(con_!= nullptr)
                redisFree(con_);
        }

        template<typename T>
        bool set(const std::string& key, T&& val){
            using U = std::remove_const_t<std::remove_reference_t<T>>;
            redisReply *reply = nullptr;
            if constexpr(std::is_integral_v<U>&&!detail::is_64_v<U>){
                reply = (redisReply *)redisCommand(con_,"SET %s %d", key.data(), std::forward<T>(val));
            }
            else if constexpr(std::is_floating_point_v<U>){
                reply = (redisReply *)redisCommand(con_,"SET %s %f", key.data(), std::forward<T>(val));
            }
            else if constexpr(detail::is_string_v<U>){
                reply = (redisReply *)redisCommand(con_,"SET %s %s", key.data(), std::forward<T>(val).data());
            }
            else if constexpr(detail::is_cstr_v<U>||detail::is_char_array_v<U>){
                reply = (redisReply *)redisCommand(con_,"SET %s %s", key.data(), std::forward<T>(val));
            }
            else if constexpr(detail::is_int64_v<U>){
                reply = (redisReply *)redisCommand(con_,"SET %s ""%" PRId64, key.data(), std::forward<T>(val));
            }
            else if constexpr(detail::is_uint64_v<U>){
                reply = (redisReply *)redisCommand(con_,"SET %s ""%" PRIu64, key.data(), std::forward<T>(val));
            }
            else{
                std::cout<<"don't support the type now"<<std::endl;
                freeReplyObject(reply);
                return false;
            }

            if(reply== nullptr)
                return false;

            if (!(reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str,"OK") == 0)){
                return false;
            }
            freeReplyObject(reply);
            return true;
        }

        //blob

        template<typename T>
        T get(const std::string& key){
            using U = std::remove_const_t<std::remove_reference_t<T>>;
            std::string cmd = "GET "s + key;
            redisReply *reply = (redisReply *)redisCommand(con_, cmd.data());

            if(reply== nullptr){
                throw(std::invalid_argument("redisCommand failed"));
            }

            guard_reply guard(reply);
            if constexpr(std::is_integral_v<U>&&!detail::is_64_v<U>){
                return std::atoi(reply->str);
            }
            else if constexpr(std::is_floating_point_v<U>){
                return std::atof(reply->str);
            }
            else if constexpr(detail::is_64_v<U>){
                return std::atoll(reply->str);
            }
            else if constexpr(detail::is_string_v<U>){
                reply->str;
            }else{
                std::cout<<"don't support the type now"<<std::endl;
                freeReplyObject(reply);
                throw(std::invalid_argument("don't support the type now"));
            }
        }

        bool del(const std::string& key){
            std::string cmd = "DEL "s + key;
            redisReply *reply = (redisReply *)redisCommand(con_, cmd.data());

            if(reply== nullptr)
                return false;

            bool b = reply->integer>0;
            freeReplyObject(reply);
            return b;
        }

    private:
        struct guard_reply{
            guard_reply(redisReply *reply):reply_(reply){}
            ~guard_reply(){
                if(reply_!= nullptr)
                    freeReplyObject(reply_);
            }

        private:
            redisReply *reply_ = nullptr;
        };

        redisContext *con_ = nullptr;
    };
}

#endif //REDISCLIENT_REDISCLIENT_HPP
