use super::{SimulationController, ConsoleRenderer, ConsoleInput, InputEvent, ViewState, RenderOptions};
use crate::resources::GameConfig;
use std::time::{Duration, Instant};
use std::thread;
use std::io;

/// Main console application that coordinates the Controller and View layers
/// Demonstrates clean separation between simulation logic and presentation
pub struct ConsoleApp {
    controller: SimulationController,
    renderer: ConsoleRenderer,
    input: ConsoleInput,
    view_state: ViewState,
    target_fps: f64,
    frame_duration: Duration,
    last_frame: Instant,
    show_help: bool,
    running: bool,
}

/// Configuration for the console application
pub struct ConsoleConfig {
    pub simulation_fps: f64,
    pub render_fps: f64,
    pub render_options: RenderOptions,
    pub initial_pattern: Option<String>,
}

impl Default for ConsoleConfig {
    fn default() -> Self {
        Self {
            simulation_fps: 10.0,
            render_fps: 30.0,
            render_options: RenderOptions::default(),
            initial_pattern: Some("glider".to_string()),
        }
    }
}

impl ConsoleApp {
    /// Create a new console application with default settings
    pub fn new() -> io::Result<Self> {
        Self::with_config(ConsoleConfig::default())
    }
    
    /// Create a new console application with custom configuration
    pub fn with_config(config: ConsoleConfig) -> io::Result<Self> {
        // Create simulation controller
        let mut sim_config = GameConfig::default();
        if let Some(pattern) = &config.initial_pattern {
            sim_config.initial_pattern.path = pattern.clone();
        }
        
        let mut controller = SimulationController::with_config(sim_config);
        controller.set_fps(config.simulation_fps);
        
        // Create renderer and input
        let renderer = ConsoleRenderer::with_options(config.render_options);
        let input = ConsoleInput::new()?;
        
        let frame_duration = Duration::from_secs_f64(1.0 / config.render_fps);
        
        Ok(Self {
            controller,
            renderer,
            input,
            view_state: ViewState::new(),
            target_fps: config.render_fps,
            frame_duration,
            last_frame: Instant::now(),
            show_help: false,
            running: true,
        })
    }
    
    /// Run the main application loop
    pub fn run(&mut self) -> io::Result<()> {
        println!("Starting Bevy Game of Life Console...");
        println!("Press 'h' for help, 'q' to quit");
        thread::sleep(Duration::from_millis(1000));
        
        while self.running {
            // Handle input events
            self.handle_input();
            
            // Update simulation
            self.controller.update();
            
            // Render if enough time has passed
            let now = Instant::now();
            if now.duration_since(self.last_frame) >= self.frame_duration {
                self.render()?;
                self.last_frame = now;
            }
            
            // Small sleep to prevent busy waiting
            thread::sleep(Duration::from_millis(1));
        }
        
        println!("\nGoodbye!");
        Ok(())
    }
    
    /// Handle all pending input events
    fn handle_input(&mut self) {
        while let Some(event) = self.input.get_event() {
            match event {
                InputEvent::Quit => {
                    self.running = false;
                }
                
                InputEvent::PlayPause => {
                    if self.controller.is_running() {
                        self.controller.pause();
                    } else {
                        self.controller.start();
                    }
                }
                
                InputEvent::Step => {
                    self.controller.step();
                }
                
                InputEvent::Reset => {
                    self.controller.reset();
                    self.view_state.reset();
                }
                
                InputEvent::PanUp => {
                    self.view_state.pan(0, -2);
                }
                
                InputEvent::PanDown => {
                    self.view_state.pan(0, 2);
                }
                
                InputEvent::PanLeft => {
                    self.view_state.pan(-2, 0);
                }
                
                InputEvent::PanRight => {
                    self.view_state.pan(2, 0);
                }
                
                InputEvent::ZoomIn => {
                    self.view_state.zoom_in();
                    let (width, height) = self.view_state.get_viewport_size();
                    self.renderer.set_viewport_size(width, height);
                }
                
                InputEvent::ZoomOut => {
                    self.view_state.zoom_out();
                    let (width, height) = self.view_state.get_viewport_size();
                    self.renderer.set_viewport_size(width, height);
                }
                
                InputEvent::ToggleGrid => {
                    self.renderer.toggle_grid();
                }
                
                InputEvent::ToggleCoordinates => {
                    self.renderer.toggle_coordinates();
                }
                
                InputEvent::LoadGlider => {
                    self.controller.load_pattern("glider");
                    self.view_state.reset();
                }
                
                InputEvent::LoadBlinker => {
                    self.controller.load_pattern("blinker");
                    self.view_state.reset();
                }
                
                InputEvent::LoadGosperGun => {
                    self.controller.load_pattern("gosper_gun");
                    self.view_state.reset();
                }
                
                InputEvent::ClearCells => {
                    self.controller.set_cells(&[]);
                }
                
                InputEvent::ShowHelp => {
                    self.show_help = !self.show_help;
                }
                
                InputEvent::Unknown(_) => {
                    // Ignore unknown input
                }
            }
        }
    }
    
    /// Render the current state
    fn render(&mut self) -> io::Result<()> {
        if self.show_help {
            self.renderer.render_help()?;
        } else {
            let snapshot = self.controller.get_state();
            self.renderer.render_with_center(
                &snapshot,
                Some(self.view_state.center_x),
                Some(self.view_state.center_y),
            )?;
            
            // Show performance metrics
            let metrics = self.controller.get_performance();
            self.renderer.render_performance(&metrics)?;
        }
        
        Ok(())
    }
    
    /// Run the application in benchmark mode (no input/rendering)
    pub fn benchmark(&mut self, generations: u64) -> io::Result<()> {
        println!("Running benchmark for {} generations...", generations);
        
        let start_time = Instant::now();
        self.controller.start();
        
        let mut last_generation = 0;
        let mut update_count = 0;
        
        while last_generation < generations {
            self.controller.update();
            update_count += 1;
            
            // Only check generation every 100 updates to avoid expensive calls
            if update_count % 100 == 0 {
                let current_generation = self.controller.get_state().generation;
                if current_generation > last_generation {
                    last_generation = current_generation;
                    
                    if last_generation % 50 == 0 {
                        let elapsed = start_time.elapsed();
                        let rate = last_generation as f64 / elapsed.as_secs_f64();
                        println!("Generation {}: {:.1} gen/sec", last_generation, rate);
                    }
                }
            }
            
            // Small sleep to prevent busy waiting
            thread::sleep(Duration::from_millis(1));
        }
        
        let total_time = start_time.elapsed();
        let final_metrics = self.controller.get_performance();
        
        println!("\nBenchmark Results:");
        println!("Total time: {:.2}s", total_time.as_secs_f64());
        println!("Average rate: {:.1} generations/second", generations as f64 / total_time.as_secs_f64());
        println!("Final population: {}", self.controller.get_state().population);
        println!("Final FPS: {:.1}", final_metrics.fps);
        
        Ok(())
    }
    
    /// Test the controller without any view dependencies
    pub fn test_headless(&mut self) -> io::Result<()> {
        println!("Testing headless operation...");
        
        // Test basic operations
        println!("Starting simulation...");
        self.controller.start();
        
        // Run for a few updates
        for i in 0..10 {
            self.controller.update();
            let state = self.controller.get_state();
            println!("Update {}: Gen {}, Pop {}", i, state.generation, state.population);
            thread::sleep(Duration::from_millis(100));
        }
        
        // Test pause/resume
        println!("Pausing...");
        self.controller.pause();
        let gen_before = self.controller.get_state().generation;
        
        thread::sleep(Duration::from_millis(200));
        self.controller.update();
        
        let gen_after = self.controller.get_state().generation;
        assert_eq!(gen_before, gen_after, "Generation should not advance when paused");
        println!("Pause test passed");
        
        // Test step
        println!("Testing step...");
        
        // Load a pattern that will definitely change (blinker)
        self.controller.load_pattern("blinker");
        let blinker_gen = self.controller.get_state().generation;
        
        self.controller.step();
        let gen_stepped = self.controller.get_state().generation;
        assert!(gen_stepped > blinker_gen, "Step should advance generation with active pattern");
        println!("Step test passed");
        
        // Test reset
        println!("Testing reset...");
        self.controller.reset();
        let gen_reset = self.controller.get_state().generation;
        assert_eq!(gen_reset, 0, "Reset should return to generation 0");
        println!("Reset test passed");
        
        // Test pattern loading
        println!("Testing pattern loading...");
        self.controller.load_pattern("blinker");
        let state = self.controller.get_state();
        assert!(state.population > 0, "Pattern should have live cells");
        println!("Pattern loading test passed");
        
        println!("All headless tests passed!");
        Ok(())
    }
}