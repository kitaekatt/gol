use anyhow::Result;
use crate::client::GameOfLifeClient;

pub mod load;
pub mod run;
pub mod status;
pub mod stop;
pub mod interactive;
pub mod simulation;
pub mod pattern;
pub mod control;

pub async fn handle_load_command(client: &mut GameOfLifeClient, pattern: &str) -> Result<()> {
    let mut pattern_commands = pattern::PatternCommands::new(client.clone());
    let mut simulation_commands = simulation::SimulationCommands::new(client.clone());
    
    let pattern_file = if pattern.starts_with('/') || pattern.contains(':') {
        // Full path provided
        pattern.to_string()
    } else if pattern.ends_with(".json") {
        // Already has extension, use patterns directory
        format!("../patterns/{}", pattern)
    } else {
        // Add .json extension and use patterns directory
        format!("../patterns/{}.json", pattern)
    };
    
    println!("Loading pattern from: {}", pattern_file);
    
    // Try to create a simulation first (in case it doesn't exist)
    let simulation_id = match simulation_commands.create(50, 50, None).await {
        Ok(response) => {
            println!("Created new simulation");
            response.id
        }
        Err(_) => {
            println!("Using default simulation");
            "default".to_string()
        }
    };
    
    match pattern_commands.load_from_file(simulation_id, &pattern_file, 0, 0).await {
        Ok(_) => println!("Pattern loaded successfully"),
        Err(e) => println!("Error loading pattern: {}", e),
    }
    
    Ok(())
}

pub async fn handle_run_command(client: &mut GameOfLifeClient, generations: Option<u32>, delay: Option<u64>) -> Result<()> {
    let mut control_commands = control::ControlCommands::new(client.clone());
    
    if let Some(delay_ms) = delay {
        control_commands.set_speed(delay_ms);
    }
    
    println!("Running simulation with {} generations, {} ms delay", 
             generations.unwrap_or(0), delay.unwrap_or(100));
    
    control_commands.play(None).await
}

pub async fn handle_status_command(client: &mut GameOfLifeClient) -> Result<String> {
    let mut simulation_commands = simulation::SimulationCommands::new(client.clone());
    simulation_commands.status().await
}

pub async fn handle_stop_command(_client: &mut GameOfLifeClient) -> Result<()> {
    println!("Stopping simulation");
    Ok(())
}

pub async fn handle_interactive_command(_client: &mut GameOfLifeClient) -> Result<()> {
    use crate::ui::TerminalUI;
    let mut ui = TerminalUI::new()?;
    ui.run_interactive().await
}