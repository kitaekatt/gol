use bevy::prelude::*;
use bevy_game_of_life::plugins::{GameOfLifePlugin, ConfigPlugin, DebugPlugin};

fn main() {
    let mut app = App::new();
    
    // Add minimal Bevy plugins for headless operation
    app.add_plugins(MinimalPlugins);
    
    // Add our custom plugins
    app.add_plugins((
        ConfigPlugin::default(),
        GameOfLifePlugin,
        DebugPlugin,
    ));
    
    // Add startup system
    app.add_systems(Startup, startup_system);
    
    info!("Starting Bevy Game of Life");
    app.run();
}

fn startup_system() {
    info!("Bevy Game of Life initialized successfully!");
    info!("Note: Running in headless mode");
    info!("Console controls (when implemented):");
    info!("  Q + ENTER - Quit game");
    info!("  ENTER - Advance one generation");
    info!("  SPACE + ENTER - Pause/Resume simulation");
    info!("  R + ENTER - Reset simulation");
    info!("  G + ENTER - Spawn glider pattern");
    info!("  B + ENTER - Spawn blinker pattern");
    info!("  C + ENTER - Clear grid");
    info!("  I + ENTER - Print statistics");
    info!("  +/- + ENTER - Adjust simulation speed");
    info!("Watch the performance output to see the simulation running!");
}