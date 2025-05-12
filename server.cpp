#include <crow.h>
#include <yaml-cpp/yaml.h>
#include <vector>
#include <string>

struct User {
    int id;
    std::string username;
    int age;
    std::string createdAt;
    std::vector<int> following;
    std::vector<int> followers;
};

int main() {
    crow::SimpleApp app;

    // Загрузка данных
    YAML::Node db = YAML::LoadFile("database.yaml");
    std::vector<User> users;
    for (const auto& userNode : db["users"]) {
        User user;
        user.id = userNode["id"].as<int>();
        user.username = userNode["username"].as<std::string>();
        user.age = userNode["age"].as<int>();
        user.createdAt = userNode["createdAt"].as<std::string>();
        user.following = userNode["following"].as<std::vector<int>>();
        user.followers = userNode["followers"].as<std::vector<int>>();
        users.push_back(user);
    }

    CROW_ROUTE(app, "/users/<int>")([&users](const crow::request&, crow::response& res, int id) {
        for (const auto& user : users) {
            if (user.id == id) {
                crow::json::wvalue json;
                json["id"] = user.id;
                json["username"] = user.username;
                json["age"] = user.age;
                json["createdAt"] = user.createdAt;
                json["following"] = user.following;
                json["followers"] = user.followers;
                res.write(json.dump());
                return res.end();
            }
        }
        res.code = 404;
        res.write("User not found");
        return res.end();
    });

    CROW_ROUTE(app, "/users/<int>/following")([&users](const crow::request&, crow::response& res, int id) {
        for (const auto& user : users) {
            if (user.id == id) {
                crow::json::wvalue json;
                json = user.following;
                res.write(json.dump());
                return res.end();
            }
        }
        res.code = 404;
        res.write("User not found");
        return res.end();
    });
    
    CROW_ROUTE(app, "/users/following").methods("POST"_method)
    ([&users](const crow::request& req) {
        try {
            auto json = crow::json::load(req.body);
            if (!json || !json.has("id")) {
                return crow::response(400, "{\"error\":\"Missing 'id' in JSON\"}");
            }

            int id = json["id"].i();
            for (const auto& user : users) {
                if (user.id == id) {
                    crow::json::wvalue result;
                    result["following"] = user.following;
                    return crow::response(200, result);
                }
            }
            return crow::response(404, "{\"error\":\"User not found\"}");
        } catch (...) {
            return crow::response(500, "{\"error\":\"Internal server error\"}");
        }
    });

    app.port(8000).multithreaded().run();
}