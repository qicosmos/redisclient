# redisclient
A c++17 simple redis client

## quick example

    redisclient::redis_client client; //just support numeric and string

    bool r = client.connect("127.0.0.1", 6379);

    r = client.set("a", 10);
    r = client.set("b", 2.5);
    r = client.set("c", "test");

    try {
        auto result1 = client.get<int>("a");
        auto result2 = client.get<double>("b");
        auto result3 = client.get<std::string>("c");

        r = client.del("a");
        r = client.del("b");
        r = client.del("c");

        auto result4 = client.get<int>("a");
        auto result5 = client.get<double>("b");
    }catch(const std::exception& e){
        std::cout<<e.what()<<std::endl;
    }
