#include "nlohmann/json.hpp"
#include <iostream>

int
main(int argc, char *argv[])
{
    std::string jsonStr = R"(
    {
        "pi": 3.141,
        "happy": true,
        "name": "Niels",
        "nothing": null,
        "answer": {
            "everything": 42
        },
        "list": [1, 0, 2],
        "object": {
            "currency": "USD",
            "value": 42.99
        }
    }
    )";
    nlohmann::json j = nlohmann::json::parse(jsonStr);
    std::cout << j.dump(4) << std::endl;
    return 0;
}