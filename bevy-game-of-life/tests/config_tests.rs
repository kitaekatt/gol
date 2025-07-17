use bevy_game_of_life::resources::config::*;
use serde_json;
use std::fs;
use tempfile::NamedTempFile;

#[test]
fn test_game_config_default_values() {
    let config = GameConfig::default();
    
    assert_eq!(config.grid.width, 100);
    assert_eq!(config.grid.height, 100);
    assert!(!config.grid.wrap_edges);
    
    assert_eq!(config.simulation.steps_per_second, 10);
    assert!(config.simulation.auto_start);
    assert_eq!(config.simulation.max_generations, None);
    
    assert_eq!(config.rendering.cell_size, 8.0);
    assert!(config.rendering.grid_lines);
    assert_eq!(config.rendering.color_scheme, "classic");
    assert!(config.rendering.smooth_transitions);
}

#[test]
fn test_game_config_serialization() {
    let config = GameConfig::default();
    
    let json = serde_json::to_string(&config).expect("Failed to serialize config");
    assert!(json.contains("\"width\":100"));
    assert!(json.contains("\"height\":100"));
    assert!(json.contains("\"steps_per_second\":10"));
}

#[test]
fn test_game_config_deserialization() {
    let json = r#"{
        "grid": {
            "width": 50,
            "height": 75,
            "wrap_edges": true
        },
        "simulation": {
            "steps_per_second": 20,
            "auto_start": false,
            "max_generations": 1000
        },
        "rendering": {
            "cell_size": 10.0,
            "grid_lines": false,
            "color_scheme": "neon",
            "smooth_transitions": false
        },
        "initial_pattern": {
            "type": "file",
            "path": "assets/patterns/glider.json"
        }
    }"#;
    
    let config: GameConfig = serde_json::from_str(json).expect("Failed to deserialize config");
    
    assert_eq!(config.grid.width, 50);
    assert_eq!(config.grid.height, 75);
    assert!(config.grid.wrap_edges);
    
    assert_eq!(config.simulation.steps_per_second, 20);
    assert!(!config.simulation.auto_start);
    assert_eq!(config.simulation.max_generations, Some(1000));
    
    assert_eq!(config.rendering.cell_size, 10.0);
    assert!(!config.rendering.grid_lines);
    assert_eq!(config.rendering.color_scheme, "neon");
    assert!(!config.rendering.smooth_transitions);
}

#[test]
fn test_config_file_loading() {
    let config = GameConfig {
        grid: GridConfig {
            width: 200,
            height: 150,
            wrap_edges: true,
        },
        simulation: SimulationConfig {
            steps_per_second: 15,
            auto_start: false,
            max_generations: Some(500),
        },
        rendering: RenderingConfig {
            cell_size: 12.0,
            grid_lines: true,
            color_scheme: "retro".to_string(),
            smooth_transitions: true,
        },
        initial_pattern: PatternConfig {
            pattern_type: "file".to_string(),
            path: "test_pattern.json".to_string(),
        },
    };
    
    let temp_file = NamedTempFile::new().expect("Failed to create temp file");
    let file_path = temp_file.path();
    
    // Save config to file
    let json = serde_json::to_string_pretty(&config).expect("Failed to serialize config");
    fs::write(file_path, json).expect("Failed to write config file");
    
    // Load config from file
    let loaded_json = fs::read_to_string(file_path).expect("Failed to read config file");
    let loaded_config: GameConfig = serde_json::from_str(&loaded_json).expect("Failed to parse config");
    
    assert_eq!(loaded_config.grid.width, 200);
    assert_eq!(loaded_config.grid.height, 150);
    assert_eq!(loaded_config.simulation.steps_per_second, 15);
    assert_eq!(loaded_config.rendering.cell_size, 12.0);
}

#[test]
fn test_pattern_config_serialization() {
    let pattern_config = PatternConfig {
        pattern_type: "embedded".to_string(),
        path: "glider".to_string(),
    };
    
    let json = serde_json::to_string(&pattern_config).expect("Failed to serialize pattern config");
    assert!(json.contains("\"type\":\"embedded\""));
    assert!(json.contains("\"path\":\"glider\""));
}

#[test]
fn test_invalid_config_handling() {
    let invalid_json = r#"{
        "grid": {
            "width": -10,
            "height": "invalid"
        }
    }"#;
    
    let result = serde_json::from_str::<GameConfig>(invalid_json);
    assert!(result.is_err());
}

#[test]
fn test_partial_config_with_defaults() {
    let partial_json = r#"{
        "grid": {
            "width": 300
        }
    }"#;
    
    // This should fail because we don't have default implementations for partial configs
    // We'll need to implement a merge function or use serde defaults
    let result = serde_json::from_str::<GameConfig>(partial_json);
    assert!(result.is_err());
}