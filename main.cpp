//
// Created by qiyu on 11/25/17.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>

#include <hiredis.h>
#include "redisclient.hpp"

enum class type_t
{
    STRING = 1,
    ARRAY = 2,
    INTEGER = 3,
    NIL = 4,
    STATUS = 5,
    ERROR = 6
};

void check(redisReply *c_reply){
    std::string str="";
    int integer=0;
    type_t type = static_cast<type_t>(c_reply->type);
    switch (type) {
        case type_t::ERROR:
        case type_t::STRING:
        case type_t::STATUS:
            str = std::string(c_reply->str, c_reply->len);
            break;
        case type_t::INTEGER:
            integer = c_reply->integer;
            break;
        case type_t::ARRAY:
            for (size_t i=0; i < c_reply->elements; ++i) {
//                _elements.push_back(reply(c_reply->element[i]));
            }
            break;
        default:
            break;
    }

    std::cout<<str<<" "<<integer<<std::endl;
}

void test_hiredis(){
    unsigned int j;
    redisContext *c;
    redisReply *reply;
    const char *hostname = "127.0.0.1";
    int port = 6379;

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    c = redisConnectWithTimeout(hostname, port, timeout);
    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    /* PING server */
    reply = (redisReply *)redisCommand(c,"PING");
    printf("PING: %s\n", reply->str);
    check(reply);
    freeReplyObject(reply);

    /* Set a key */
    reply = (redisReply *)redisCommand(c,"SET %s %s", "foo", "hello world");
    printf("SET: %s\n", reply->str);
    check(reply);
    freeReplyObject(reply);

    reply = (redisReply *)redisCommand(c,"SET %s %d", "foo", 2345);
    printf("SET: %s\n", reply->str);
    check(reply);
    freeReplyObject(reply);

    /* Set a key using binary safe API */
    reply = (redisReply *)redisCommand(c,"SET %b %b", "bar", (size_t) 3, "hello", (size_t) 5);
    printf("SET (binary API): %s\n", reply->str);
    check(reply);
    freeReplyObject(reply);

    /* Try a GET */
    std::string str = "GET ";str+="foo";
    reply = (redisReply *)redisCommand(c, str.data());
    printf("GET foo: %s\n", reply->str);
    check(reply);
    freeReplyObject(reply);

    reply = (redisReply *)redisCommand(c,"DEL foo");
    printf("DEL foo: %s\n", reply->str);
    check(reply);
    freeReplyObject(reply);

    reply = (redisReply *)redisCommand(c,"GET foo");
    printf("GET foo: %s\n", reply->str);
    check(reply);
    freeReplyObject(reply);
    /* Disconnects and frees the context */
    redisFree(c);
}

void test_redisclient(){

    redisclient::redis_client client; //just support numeric and string
    bool r = client.connect("127.0.0.1", 6379);

    r = client.set("a", 10);
    r = client.set("b", 2.5);
    r = client.set("c", "test");

    std::string key("a\0 a", 4);
    std::string val("a\0 a\0a", 6);
    r = client.set(key, val);
    try {
        auto result1 = client.get<int>("a");
        auto result2 = client.get<double>("b");
        auto result3 = client.get<std::string>("c");
        auto result0 = client.get<std::string>(key);

        auto v = client.keys("*");
        r = client.del("a");
        r = client.del("b");
        r = client.del("c");
        r = client.del(key);
        auto v1 = client.keys("*");
        auto result4 = client.get<int>("a");
        auto result5 = client.get<double>("b");
        auto result6 = client.get<std::string>(key);
    }catch(const std::exception& e){
        std::cout<<e.what()<<std::endl;
    }
}

int main(){
    test_redisclient();
    test_hiredis();

    return 0;
}