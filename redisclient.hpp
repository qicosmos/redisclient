//
// Created by qiyu on 11/25/17.
//

#ifndef REDISCLIENT_REDISCLIENT_HPP
#define REDISCLIENT_REDISCLIENT_HPP

#include <string>
#include <iostream>
#include <inttypes.h>
#include <hiredis.h>
/*
 * reply = (redisReply *)redisCommand(c,"SET %s %s", "foo", "hello world");
    printf("SET: %s\n", reply->str);
    check(reply);
    freeReplyObject(reply);
 */
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
            else if constexpr(detail::is_cstr_v<U>){
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

//        template<typename T>
        std::string get(const std::string& key){
            using namespace std::string_literals;
//            using U = std::remove_const_t<std::remove_reference_t<T>>;
            std::string cmd = "GET "s + key;
            redisReply *reply = (redisReply *)redisCommand(con_, cmd.data());
            std::string str = reply->str;
            freeReplyObject(reply);
            return str;
//            if constexpr(std::is_arithmetic_v<U>){
//                reply = (redisReply *)redisCommand(con_, cmd.data());
//            }
//            else if constexpr(detail::is_string_v<U>){
//                reply = (redisReply *)redisCommand(con_, cmd.data());
//            }
//            return T{};
        }

        bool del(const std::string& key){
            using namespace std::string_literals;
//            using U = std::remove_const_t<std::remove_reference_t<T>>;
            std::string cmd = "DEL "s + key;
            redisReply *reply = (redisReply *)redisCommand(con_, cmd.data());

            if(reply== nullptr)
                return false;

            bool b = reply->integer>0;
            freeReplyObject(reply);
            return b;
        }

    private:
        redisContext *con_ = nullptr;
    };
}

#endif //REDISCLIENT_REDISCLIENT_HPP
