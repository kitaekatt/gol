# Game of Life - Console Client

## Implementation Overview

This is a unified console client for the Game of Life multi-implementation project. The client provides a rich terminal-based interface that can connect to any of the three server implementations (Bevy, EnTT, or Flecs) via gRPC, enabling seamless testing and comparison of different ECS architectures.

**Key Features:**
- Terminal-based UI using Ratatui for rich visualization
- gRPC client connectivity to all server implementations
- Real-time simulation visualization and control
- Pattern loading and management
- Performance benchmarking capabilities
- Multi-server support with easy switching

## Development Environment Setup

### Prerequisites
- **Rust**: Version 1.70+ (use `rustup` to install)
- **Cargo**: Included with Rust
- **Protocol Buffers**: For gRPC code generation
  ```bash
  # Ubuntu/Debian
  sudo apt install protobuf-compiler
  
  # macOS
  brew install protobuf
  
  # Windows
  # Download from https://github.com/protocolbuffers/protobuf/releases
  ```

### Installation
1. **Clone the repository** (if not already done)
2. **Navigate to the console client**:
   ```bash
   cd gol-console-client
   ```
3. **Install dependencies**:
   ```bash
   cargo build
   ```

### Dependencies
- **Tonic**: gRPC client implementation
- **Prost**: Protocol Buffers for Rust
- **Tokio**: Async runtime for client operations
- **Clap**: Command-line argument parsing
- **Ratatui**: Terminal UI framework
- **Crossterm**: Cross-platform terminal manipulation
- **Serde**: JSON serialization for configuration
- **Anyhow**: Error handling

## Architecture Summary

### Core Components

#### gRPC Client Layer
```rust
pub struct GameOfLifeClient {
    client: GameOfLifeServiceClient<tonic::transport::Channel>,
    endpoint: String,
    implementation: String,
}

impl GameOfLifeClient {
    pub async fn connect(endpoint: &str) -> Result<Self>;
    pub async fn create_simulation(&mut self, config: SimulationConfig) -> Result<String>;
    pub async fn step_simulation(&mut self, id: &str, steps: i32) -> Result<StepResponse>;
    pub async fn get_simulation(&mut self, id: &str) -> Result<SimulationResponse>;
    pub async fn load_pattern(&mut self, id: &str, pattern: &Pattern, pos: Position) -> Result<()>;
}
```

#### Terminal UI Components
- **App**: Main application state and event handling
- **Grid**: Simulation grid visualization
- **Menu**: Command interface and controls
- **StatusBar**: Current simulation status and metrics
- **ServerSelector**: Multi-server connection management

#### Command System
```rust
pub enum Command {
    Connect(String),           // Connect to server
    CreateSimulation(i32, i32), // Create new simulation
    LoadPattern(String, i32, i32), // Load pattern at position
    Step(i32),                // Step simulation N times
    Play,                     // Start continuous simulation
    Pause,                    // Pause simulation
    Reset,                    // Reset simulation
    Benchmark,                // Run performance benchmark
    Quit,                     // Exit application
}
```

### UI Layout
```
┌─────────────────────────────────────────────────────────────────────────────────┐
│ Game of Life Console Client - Connected to: Bevy Server (localhost:50051)      │
├─────────────────────────────────────────────────────────────────────────────────┤
│ Generation: 142 | Live Cells: 1,337 | FPS: 30 | Memory: 1.2MB                 │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│  ████████░░░░░░░░████████                                                      │
│  ██░░░░░░░░░░░░░░░░░░░░██                                                      │
│  ██░░██████████████░░██                                                        │
│  ██░░██░░░░░░░░░░██░░██                                                        │
│  ██░░██░░████░░░░██░░██                                                        │
│  ██░░██░░░░░░░░░░██░░██                                                        │
│  ██░░██████████████░░██                                                        │
│  ██░░░░░░░░░░░░░░░░░░░░██                                                      │
│  ████████░░░░░░░░████████                                                      │
│                                                                                 │
├─────────────────────────────────────────────────────────────────────────────────┤
│ Commands: [S]tep [P]lay/Pause [R]eset [L]oad Pattern [B]enchmark [Q]uit       │
│ Servers: [1]Bevy [2]EnTT [3]Flecs | Current: Bevy                             │
└─────────────────────────────────────────────────────────────────────────────────┘
```

## Build Instructions

### Development Build
```bash
cargo build
```

### Release Build
```bash
cargo build --release
```

### Running the Client
```bash
# Connect to default server (localhost:50051)
cargo run

# Connect to specific server
cargo run -- --server localhost:50052

# Connect to specific implementation
cargo run -- --implementation bevy

# Load initial pattern
cargo run -- --pattern glider

# Enable debug logging
RUST_LOG=debug cargo run
```

### Command Line Options
- `--server`: Server endpoint (default: localhost:50051)
- `--implementation`: Preferred implementation (bevy, entt, flecs)
- `--pattern`: Initial pattern to load
- `--width`: Grid width (default: 80)
- `--height`: Grid height (default: 24)
- `--fps`: Target FPS for continuous simulation (default: 10)

## Testing Guidelines

### Running Tests
```bash
# Run all tests
cargo test

# Run unit tests only
cargo test --lib

# Run integration tests
cargo test --test '*'

# Run with output
cargo test -- --nocapture

# Run specific test
cargo test test_grpc_connection
```

### Test Categories
- **Unit Tests**: Individual component behavior
- **Integration Tests**: gRPC client functionality
- **UI Tests**: Terminal interface validation
- **End-to-End Tests**: Full client-server interaction
- **Performance Tests**: Benchmarking and stress testing

### Mock Server Testing
```rust
#[cfg(test)]
mod tests {
    use super::*;
    use tonic::transport::Server;
    
    #[tokio::test]
    async fn test_client_connection() {
        // Start mock server
        let (mut client, _server) = setup_mock_server().await;
        
        // Test connection
        let status = client.get_status().await.unwrap();
        assert_eq!(status.status, "healthy");
    }
}
```

## gRPC Client Documentation

### Multi-Server Support
The client can connect to any of the three server implementations:

```rust
pub enum ServerImplementation {
    Bevy,
    EnTT,
    Flecs,
}

pub struct ServerConfig {
    pub endpoint: String,
    pub implementation: ServerImplementation,
    pub timeout: Duration,
}
```

### Connection Management
```rust
impl GameOfLifeClient {
    pub async fn connect_to_server(config: ServerConfig) -> Result<Self> {
        let channel = tonic::transport::Endpoint::from_shared(config.endpoint)?
            .timeout(config.timeout)
            .connect()
            .await?;
        
        let client = GameOfLifeServiceClient::new(channel);
        
        // Verify connection with health check
        let status = client.get_status(StatusRequest {}).await?;
        
        Ok(Self {
            client,
            endpoint: config.endpoint,
            implementation: status.implementation,
        })
    }
}
```

### Streaming Support
```rust
pub async fn stream_simulation(&mut self, id: &str) -> Result<impl Stream<Item = SimulationUpdate>> {
    let request = StreamRequest {
        id: id.to_string(),
        auto_step: true,
        step_interval_ms: 100,
    };
    
    let response = self.client.stream_simulation(request).await?;
    Ok(response.into_inner())
}
```

## UI Framework and Controls

### Terminal UI Components
Built using Ratatui for rich terminal interfaces:

#### Grid Visualization
```rust
pub struct GridWidget {
    cells: Vec<Vec<bool>>,
    zoom: f32,
    offset: (i32, i32),
}

impl GridWidget {
    pub fn render(&self, area: Rect, buf: &mut Buffer) {
        // Render live cells as filled blocks
        for (y, row) in self.cells.iter().enumerate() {
            for (x, &alive) in row.iter().enumerate() {
                if alive {
                    buf.set_string(x as u16, y as u16, "██", Style::default().fg(Color::Green));
                } else {
                    buf.set_string(x as u16, y as u16, "░░", Style::default().fg(Color::DarkGray));
                }
            }
        }
    }
}
```

#### Input Handling
```rust
pub enum InputEvent {
    Key(KeyEvent),
    Mouse(MouseEvent),
    Resize(u16, u16),
}

pub struct InputHandler {
    current_mode: InputMode,
}

impl InputHandler {
    pub fn handle_input(&mut self, event: InputEvent) -> Option<Command> {
        match event {
            InputEvent::Key(key) => match key.code {
                KeyCode::Char('s') => Some(Command::Step(1)),
                KeyCode::Char('p') => Some(Command::Play),
                KeyCode::Char('r') => Some(Command::Reset),
                KeyCode::Char('l') => Some(Command::LoadPattern("glider".to_string(), 10, 10)),
                KeyCode::Char('q') => Some(Command::Quit),
                KeyCode::Char('1') => Some(Command::Connect("bevy".to_string())),
                KeyCode::Char('2') => Some(Command::Connect("entt".to_string())),
                KeyCode::Char('3') => Some(Command::Connect("flecs".to_string())),
                _ => None,
            },
            _ => None,
        }
    }
}
```

### Multi-Server Connectivity
```rust
pub struct ServerManager {
    servers: HashMap<String, ServerConfig>,
    current_server: String,
}

impl ServerManager {
    pub fn new() -> Self {
        let mut servers = HashMap::new();
        servers.insert("bevy".to_string(), ServerConfig {
            endpoint: "http://localhost:50051".to_string(),
            implementation: ServerImplementation::Bevy,
            timeout: Duration::from_secs(5),
        });
        servers.insert("entt".to_string(), ServerConfig {
            endpoint: "http://localhost:50052".to_string(),
            implementation: ServerImplementation::EnTT,
            timeout: Duration::from_secs(5),
        });
        servers.insert("flecs".to_string(), ServerConfig {
            endpoint: "http://localhost:50053".to_string(),
            implementation: ServerImplementation::Flecs,
            timeout: Duration::from_secs(5),
        });
        
        Self {
            servers,
            current_server: "bevy".to_string(),
        }
    }
    
    pub async fn switch_server(&mut self, name: &str) -> Result<GameOfLifeClient> {
        let config = self.servers.get(name)
            .ok_or_else(|| anyhow::anyhow!("Unknown server: {}", name))?;
        
        let client = GameOfLifeClient::connect_to_server(config.clone()).await?;
        self.current_server = name.to_string();
        Ok(client)
    }
}
```

## Performance Targets

### Network Performance
- **Connection Time**: <1 second to establish gRPC connection
- **Response Time**: <100ms for simulation operations
- **Throughput**: Handle 30+ FPS for real-time visualization
- **Reliability**: Automatic reconnection on network failures

### UI Performance
- **Frame Rate**: Maintain 30 FPS for smooth animation
- **Memory Usage**: <10MB for UI state and buffers
- **CPU Usage**: <10% CPU for rendering and input handling
- **Scalability**: Support grids up to 200x50 in terminal

### Benchmarking
```rust
pub struct BenchmarkRunner {
    client: GameOfLifeClient,
    patterns: Vec<Pattern>,
}

impl BenchmarkRunner {
    pub async fn run_performance_benchmark(&mut self) -> BenchmarkResults {
        let mut results = BenchmarkResults::default();
        
        for pattern in &self.patterns {
            let start = Instant::now();
            let sim_id = self.client.create_simulation(100, 100).await?;
            self.client.load_pattern(&sim_id, pattern, Position { x: 10, y: 10 }).await?;
            
            // Run 1000 steps
            for _ in 0..1000 {
                self.client.step_simulation(&sim_id, 1).await?;
            }
            
            let duration = start.elapsed();
            results.add_result(pattern.name.clone(), duration);
        }
        
        results
    }
}
```

## Troubleshooting

### Common Issues

#### Connection Errors
**Issue**: `gRPC connection failed`
**Solution**: Verify server is running:
```bash
# Check if server is listening
netstat -tlnp | grep 50051

# Test connection with grpcurl
grpcurl -plaintext localhost:50051 list
```

**Issue**: `Protocol buffer compilation failed`
**Solution**: Install protobuf compiler:
```bash
sudo apt install protobuf-compiler
```

#### UI Issues
**Issue**: `Terminal rendering corrupted`
**Solution**: Reset terminal and check size:
```bash
reset
tput cols; tput lines
```

**Issue**: `Input not responding`
**Solution**: Check terminal mode:
```bash
stty -echo -icanon
```

#### Performance Issues
**Issue**: `High CPU usage during visualization`
**Solution**: Reduce FPS or grid size:
```bash
cargo run -- --fps 5 --width 40 --height 20
```

**Issue**: `Memory leak during long sessions`
**Solution**: Enable debug logging to identify leaks:
```bash
RUST_LOG=debug cargo run
```

### Debugging
Enable debug logging:
```bash
RUST_LOG=debug cargo run
```

Network debugging:
```bash
RUST_LOG=tonic=debug cargo run
```

UI debugging:
```bash
RUST_LOG=ratatui=debug cargo run
```

### Getting Help
- **Documentation**: Run `cargo doc --open` for API docs
- **Examples**: Check `examples/` directory for usage patterns
- **Issues**: Report bugs in project issue tracker
- **Performance**: Use `cargo flamegraph` for profiling

## Development Notes

### Code Organization
```
src/
├── main.rs              # Application entry point
├── lib.rs               # Library interface
├── client/              # gRPC client implementation
│   ├── mod.rs
│   ├── connection.rs
│   └── service.rs
├── ui/                  # Terminal UI components
│   ├── mod.rs
│   ├── app.rs
│   ├── grid.rs
│   ├── menu.rs
│   └── status.rs
├── commands/            # Command processing
│   ├── mod.rs
│   ├── simulation.rs
│   ├── pattern.rs
│   └── benchmark.rs
├── config/              # Configuration management
│   ├── mod.rs
│   └── settings.rs
└── utils/               # Utility functions
    ├── mod.rs
    ├── pattern_loader.rs
    └── server_discovery.rs
```

### Best Practices
- **Async/Await**: Use async patterns for gRPC communication
- **Error Handling**: Implement comprehensive error handling with anyhow
- **State Management**: Maintain clean separation between UI and client state
- **Performance**: Profile regularly and optimize rendering loops
- **Testing**: Write integration tests for gRPC client functionality

### Configuration Management
```rust
#[derive(Debug, Deserialize)]
pub struct ClientConfig {
    pub default_server: String,
    pub servers: HashMap<String, ServerConfig>,
    pub ui: UIConfig,
    pub performance: PerformanceConfig,
}

#[derive(Debug, Deserialize)]
pub struct UIConfig {
    pub fps: u32,
    pub grid_width: u32,
    pub grid_height: u32,
    pub cell_char: String,
}
```

This console client provides a powerful interface for interacting with all three Game of Life server implementations while maintaining a consistent and intuitive user experience.