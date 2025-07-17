#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "core/GameConfig.h"
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

TEST_CASE("GameConfig construction with defaults", "[GameConfig]") {
    GameConfig config;
    
    SECTION("Default grid settings") {
        REQUIRE(config.getGridWidth() == 100);
        REQUIRE(config.getGridHeight() == 100);
        REQUIRE(config.getWrapEdges() == false);
    }
    
    SECTION("Default simulation settings") {
        REQUIRE(config.getMaxGenerations() == 1000);
        REQUIRE(config.getAutoPauseOnStable() == true);
        REQUIRE(config.getStableDetectionCycles() == 10);
        REQUIRE(config.getStepDelayMs() == 100);
    }
    
    SECTION("Default performance settings") {
        REQUIRE(config.getTargetFps() == 60);
        REQUIRE(config.getMemoryLimitMb() == 100);
        REQUIRE(config.getEnableSpatialOptimization() == true);
        REQUIRE(config.getBatchSize() == 1000);
    }
}

TEST_CASE("GameConfig JSON serialization", "[GameConfig]") {
    GameConfig config;
    config.setGridWidth(200);
    config.setGridHeight(150);
    config.setWrapEdges(true);
    config.setMaxGenerations(500);
    config.setTargetFps(30);
    
    SECTION("Serialize to JSON") {
        json j = config.toJson();
        
        REQUIRE(j["grid"]["width"] == 200);
        REQUIRE(j["grid"]["height"] == 150);
        REQUIRE(j["grid"]["wrap_edges"] == true);
        REQUIRE(j["simulation"]["max_generations"] == 500);
        REQUIRE(j["performance"]["target_fps"] == 30);
    }
    
    SECTION("Serialize then deserialize preserves values") {
        json j = config.toJson();
        GameConfig restored;
        restored.fromJson(j);
        
        REQUIRE(restored.getGridWidth() == 200);
        REQUIRE(restored.getGridHeight() == 150);
        REQUIRE(restored.getWrapEdges() == true);
        REQUIRE(restored.getMaxGenerations() == 500);
        REQUIRE(restored.getTargetFps() == 30);
    }
}

TEST_CASE("GameConfig JSON deserialization", "[GameConfig]") {
    json testJson = {
        {"grid", {
            {"width", 50},
            {"height", 75},
            {"wrap_edges", true}
        }},
        {"simulation", {
            {"max_generations", 2000},
            {"auto_pause_on_stable", false}
        }},
        {"performance", {
            {"target_fps", 120},
            {"memory_limit_mb", 200}
        }}
    };
    
    GameConfig config;
    config.fromJson(testJson);
    
    REQUIRE(config.getGridWidth() == 50);
    REQUIRE(config.getGridHeight() == 75);
    REQUIRE(config.getWrapEdges() == true);
    REQUIRE(config.getMaxGenerations() == 2000);
    REQUIRE(config.getAutoPauseOnStable() == false);
    REQUIRE(config.getTargetFps() == 120);
    REQUIRE(config.getMemoryLimitMb() == 200);
}

TEST_CASE("GameConfig partial JSON deserialization uses defaults", "[GameConfig]") {
    json partialJson = {
        {"grid", {
            {"width", 300}
        }}
    };
    
    GameConfig config;
    config.fromJson(partialJson);
    
    REQUIRE(config.getGridWidth() == 300);  // From JSON
    REQUIRE(config.getGridHeight() == 100); // Default value
    REQUIRE(config.getWrapEdges() == false); // Default value
}

TEST_CASE("GameConfig file operations", "[GameConfig]") {
    GameConfig config;
    config.setGridWidth(80);
    config.setGridHeight(60);
    config.setMaxGenerations(1500);
    
    const std::string testFile = "test_config.json";
    
    SECTION("Save and load from file") {
        config.saveToFile(testFile);
        
        GameConfig loaded;
        loaded.loadFromFile(testFile);
        
        REQUIRE(loaded.getGridWidth() == 80);
        REQUIRE(loaded.getGridHeight() == 60);
        REQUIRE(loaded.getMaxGenerations() == 1500);
        
        // Clean up
        std::remove(testFile.c_str());
    }
}

TEST_CASE("GameConfig validation", "[GameConfig]") {
    GameConfig config;
    
    SECTION("Valid configuration passes validation") {
        REQUIRE(config.isValid());
    }
    
    SECTION("Invalid grid dimensions fail validation") {
        config.setGridWidth(0);
        REQUIRE_FALSE(config.isValid());
        
        config.setGridWidth(100);
        config.setGridHeight(-1);
        REQUIRE_FALSE(config.isValid());
    }
    
    SECTION("Invalid performance settings fail validation") {
        config.setTargetFps(0);
        REQUIRE_FALSE(config.isValid());
        
        config.setTargetFps(60);
        config.setMemoryLimitMb(-1);
        REQUIRE_FALSE(config.isValid());
    }
}