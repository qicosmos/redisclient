# C++17实现的一个简单的redis客户端

## motiviation

实现最常用的redis操作，只支持数字类型字符串类型(包括std::string, c字符串，和字符串数组)，简单好用。

## 基本接口

	bool connect(const std::string& hostname, int port, int timeout=3);
	
	template<typename T>
	bool set(const std::string& key, T&& val);
	
	template<typename T>
	T get(const std::string& key);

有这三个接口后用户就可以很方便地使用redis了。

注意：get接口会抛异常，当字符串转换为对应的值失败时会抛异常，当要取的k-v不存在时也会抛异常。

## 基本用法

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
        r = client.del("a");
        r = client.del("b");
        r = client.del("c");
        r = client.del(key);

        auto result4 = client.get<int>("a");
        auto result5 = client.get<double>("b");
        auto result6 = client.get<std::string>(key);
    }catch(const std::exception& e){
        std::cout<<e.what()<<std::endl;
	}

## roadmap

1. 增加conneciton pool
2. 增加更多redis访问接口
