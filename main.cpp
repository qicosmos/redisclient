//
// Created by qiyu on 11/25/17.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>

#include <hiredis.h>

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

    /* Set a key using binary safe API */
    reply = (redisReply *)redisCommand(c,"SET %b %b", "bar", (size_t) 3, "hello", (size_t) 5);
    printf("SET (binary API): %s\n", reply->str);
    check(reply);
    freeReplyObject(reply);

    /* Try a GET */
    reply = (redisReply *)redisCommand(c,"GET foo");
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

int main(){
    test_hiredis();

    return 0;
}