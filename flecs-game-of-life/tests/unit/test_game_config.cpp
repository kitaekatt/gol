#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <flecs_gol/game_config.h>
#include <nlohmann/json.hpp>
#include <sstream>

using namespace flecs_gol;
using json = nlohmann::json;

TEST_CASE("GameConfig Default Construction", "[config]") {
    GameConfig config;
    
    SECTION("Has reasonable defaults") {
        REQUIRE(config.getGridMinX() == -500);
        REQUIRE(config.getGridMaxX() == 500);
        REQUIRE(config.getGridMinY() == -500);
        REQUIRE(config.getGridMaxY() == 500);
        REQUIRE(config.getTargetFPS() == 10);
        REQUIRE(config.getMaxEntities() == 1000000);
        REQUIRE_FALSE(config.getWrapEdges());
        REQUIRE_FALSE(config.getEnableProfiling());
    }
}

TEST_CASE("GameConfig JSON Serialization", "[config]") {
    GameConfig config;
    config.setGridBoundaries(-100, 100, -50, 50);
    config.setTargetFPS(60);
    config.setMaxEntities(500000);
    config.setWrapEdges(true);
    config.setEnableProfiling(true);
    
    SECTION("Serialize to JSON") {
        json j = config.toJson();
        
        REQUIRE(j["grid"]["boundaries"]["minX"] == -100);
        REQUIRE(j["grid"]["boundaries"]["maxX"] == 100);
        REQUIRE(j["grid"]["boundaries"]["minY"] == -50);
        REQUIRE(j["grid"]["boundaries"]["maxY"] == 50);
        REQUIRE(j["grid"]["wrapEdges"] == true);
        REQUIRE(j["simulation"]["targetFPS"] == 60);
        REQUIRE(j["performance"]["maxEntities"] == 500000);
        REQUIRE(j["performance"]["enableProfiling"] == true);
    }
}

TEST_CASE("GameConfig JSON Deserialization", "[config]") {
    json testJson = R"({
        "grid": {
            "boundaries": {
                "minX": -200,
                "maxX": 300,
                "minY": -150,
                "maxY": 250
            },
            "wrapEdges": true
        },
        "simulation": {
            "targetFPS": 30,
            "maxGenerations": 1000
        },
        "performance": {
            "maxEntities": 750000,
            "enableProfiling": false
        }
    })"_json;
    
    SECTION("Deserialize from JSON") {
        GameConfig config = GameConfig::fromJson(testJson);
        
        REQUIRE(config.getGridMinX() == -200);
        REQUIRE(config.getGridMaxX() == 300);
        REQUIRE(config.getGridMinY() == -150);
        REQUIRE(config.getGridMaxY() == 250);
        REQUIRE(config.getWrapEdges() == true);
        REQUIRE(config.getTargetFPS() == 30);
        REQUIRE(config.getMaxGenerations() == 1000);
        REQUIRE(config.getMaxEntities() == 750000);
        REQUIRE_FALSE(config.getEnableProfiling());
    }
}

TEST_CASE("GameConfig File I/O", "[config]") {
    GameConfig originalConfig;
    originalConfig.setGridBoundaries(-75, 75, -25, 25);
    originalConfig.setTargetFPS(15);
    originalConfig.setWrapEdges(true);
    
    const std::string testFilePath = "test_config.json";
    
    SECTION("Save and load from file") {
        // Save to file
        REQUIRE(originalConfig.saveToFile(testFilePath));
        
        // Load from file
        auto loadResult = GameConfig::loadFromFile(testFilePath);
        REQUIRE(loadResult.has_value());
        
        GameConfig loadedConfig = loadResult.value();
        REQUIRE(loadedConfig.getGridMinX() == originalConfig.getGridMinX());
        REQUIRE(loadedConfig.getGridMaxX() == originalConfig.getGridMaxX());
        REQUIRE(loadedConfig.getGridMinY() == originalConfig.getGridMinY());
        REQUIRE(loadedConfig.getGridMaxY() == originalConfig.getGridMaxY());
        REQUIRE(loadedConfig.getTargetFPS() == originalConfig.getTargetFPS());
        REQUIRE(loadedConfig.getWrapEdges() == originalConfig.getWrapEdges());
        
        // Clean up
        std::remove(testFilePath.c_str());
    }
    
    SECTION("Handle missing file") {
        auto result = GameConfig::loadFromFile("nonexistent_file.json");
        REQUIRE_FALSE(result.has_value());
    }
}

TEST_CASE("GameConfig Validation", "[config]") {
    SECTION("Valid config passes validation") {
        GameConfig config;
        config.setGridBoundaries(-100, 100, -100, 100);
        config.setTargetFPS(60);
        config.setMaxEntities(1000);
        
        REQUIRE(config.validate());
    }
    
    SECTION("Invalid grid boundaries fail validation") {
        GameConfig config;
        config.setGridBoundaries(100, -100, -100, 100); // minX > maxX
        
        REQUIRE_FALSE(config.validate());
    }
    
    SECTION("Invalid FPS fails validation") {
        GameConfig config;
        config.setTargetFPS(0);
        
        REQUIRE_FALSE(config.validate());
    }
    
    SECTION("Invalid max entities fails validation") {
        GameConfig config;
        config.setMaxEntities(0);
        
        REQUIRE_FALSE(config.validate());
    }
}

TEST_CASE("GameConfig Boundary Calculations", "[config]") {
    GameConfig config;
    config.setGridBoundaries(-50, 150, -75, 25);
    
    SECTION("Calculate grid dimensions") {
        REQUIRE(config.getGridWidth() == 201); // 150 - (-50) + 1
        REQUIRE(config.getGridHeight() == 101); // 25 - (-75) + 1
    }
    
    SECTION("Check point containment") {
        REQUIRE(config.isPointInBounds(0, 0));
        REQUIRE(config.isPointInBounds(-50, -75));
        REQUIRE(config.isPointInBounds(150, 25));
        REQUIRE_FALSE(config.isPointInBounds(-51, 0));
        REQUIRE_FALSE(config.isPointInBounds(0, -76));
        REQUIRE_FALSE(config.isPointInBounds(151, 0));
        REQUIRE_FALSE(config.isPointInBounds(0, 26));
    }
}