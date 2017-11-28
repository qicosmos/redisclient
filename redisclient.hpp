//
// Created by qiyu on 11/25/17.
//

#ifndef REDISCLIENT_REDISCLIENT_HPP
#define REDISCLIENT_REDISCLIENT_HPP

#include <string>
#include <iostream>
#include <inttypes.h>
#include <hiredis.h>
#include <vector>

namespace redisclient{
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
                reply = (redisReply *)redisCommand(con_,"SET %b %d", key.data(), key.size(), std::forward<T>(val));
            }
            else if constexpr(std::is_floating_point_v<U>){
                reply = (redisReply *)redisCommand(con_,"SET %b %f", key.data(), key.size(), std::forward<T>(val));
            }
            else if constexpr(detail::is_string_v<U>){
                reply = (redisReply *)redisCommand(con_,"SET %b %b", key.data(), key.size(),
                                                   std::forward<T>(val).data(), std::forward<T>(val).size());
            }
            else if constexpr(detail::is_cstr_v<U>){
                reply = (redisReply *)redisCommand(con_,"SET %b %b", key.data(), key.size(),
                                                   std::forward<T>(val), strlen(std::forward<T>(val)));
            }
            else if constexpr(detail::is_char_array_v<U>){
                reply = (redisReply *)redisCommand(con_,"SET %b %b", key.data(), key.size(),
                                                   std::forward<T>(val), sizeof(std::forward<T>(val)));
            }
            else if constexpr(detail::is_int64_v<U>){
                reply = (redisReply *)redisCommand(con_,"SET %b ""%" PRId64, key.data(), key.size(), std::forward<T>(val));
            }
            else if constexpr(detail::is_uint64_v<U>){
                reply = (redisReply *)redisCommand(con_,"SET %b ""%" PRIu64, key.data(), key.size(), std::forward<T>(val));
            }
            else{
                std::cout<<"don't support the type now"<<std::endl;
                freeReplyObject(reply);
                return false;
            }

            if(reply== nullptr)
                return false;

            guard_reply guard(reply);
            if (!(reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str,"OK") == 0)){
                return false;
            }

            return true;
        }

        template<typename T>
        T get(const std::string& key){
            using U = std::remove_const_t<std::remove_reference_t<T>>;
            redisReply *reply = (redisReply *)redisCommand(con_, "GET %b", key.data(), key.size());

            if(reply== nullptr){
                throw(std::invalid_argument("redisCommand failed"));
            }

            guard_reply guard(reply);
            if(reply->str== nullptr){
                throw(std::invalid_argument("no value"));
            }

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
                return std::string(reply->str, reply->len);
            }else{
                std::cout<<"don't support the type now"<<std::endl;
                throw(std::invalid_argument("don't support the type now"));
            }
        }

        bool del(const std::string& key){
            redisReply *reply = (redisReply *)redisCommand(con_, "DEL %b", key.data(), key.size());

            if(reply== nullptr)
                return false;

            guard_reply guard(reply);
            return reply->integer>0;
        }

        std::vector<std::string> keys(const std::string& key){
            redisReply *reply = (redisReply *)redisCommand(con_, "KEYS %b", key.data(), key.size());

            if(reply== nullptr)
                return {};

            guard_reply guard(reply);
            if(reply->elements==0)
                return {};

            std::vector<std::string> v;
            for (size_t i = 0; i < reply->elements; ++i) {
                redisReply *item = reply->element[i];
                v.push_back(std::string(item->str, item->len));
            }

            return v;
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
