#include <iostream>
#include <boost/json.hpp>

int main()
{
    // Create a JSON object
    boost::json::object obj;
    obj["name"] = "auth.service";
    obj["version"] = "1.0.0";
    obj["status"] = "running";
    obj["features"] = boost::json::array{"jwt", "ssh-keys", "redis"};

    // Serialize to string
    std::string json_str = boost::json::serialize(obj);

    // Output the JSON
    std::cout << "Service info: " << json_str << std::endl;

    return 0;
}
