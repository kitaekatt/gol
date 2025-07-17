use tonic::{Request, Response, Status};

use gol_bevy::grpc::proto::game_of_life_service_server::GameOfLifeService;
use gol_bevy::grpc::proto::*;
use gol_bevy::grpc::GameOfLifeServiceImpl;

/// Helper to create a test service
fn create_test_service() -> GameOfLifeServiceImpl {
    GameOfLifeServiceImpl::new()
}

#[tokio::test]
async fn test_get_status() {
    let service = create_test_service();
    let request = Request::new(StatusRequest {});
    
    let response = service.get_status(request).await.unwrap();
    let status = response.into_inner();
    
    assert_eq!(status.status, "healthy");
    assert_eq!(status.version, "1.0.0");
    assert_eq!(status.implementation, "bevy");
    assert!(status.uptime_seconds >= 0);
}

#[tokio::test]
async fn test_create_simulation() {
    let service = create_test_service();
    let request = Request::new(CreateSimulationRequest {
        width: 100,
        height: 100,
        initial_pattern: String::new(),
    });
    
    let response = service.create_simulation(request).await.unwrap();
    let simulation = response.into_inner();
    
    assert!(!simulation.id.is_empty());
    assert_eq!(simulation.generation, 0);
    assert_eq!(simulation.live_cells, 0);
    assert!(simulation.grid.is_some());
    assert_eq!(simulation.grid.unwrap().width, 100);
    assert_eq!(simulation.grid.unwrap().height, 100);
    assert!(simulation.cells.is_empty());
}

#[tokio::test]
async fn test_create_simulation_invalid_dimensions() {
    let service = create_test_service();
    
    // Test zero width
    let request = Request::new(CreateSimulationRequest {
        width: 0,
        height: 100,
        initial_pattern: String::new(),
    });
    
    let result = service.create_simulation(request).await;
    assert!(result.is_err());
    
    // Test negative height
    let request = Request::new(CreateSimulationRequest {
        width: 100,
        height: -1,
        initial_pattern: String::new(),
    });
    
    let result = service.create_simulation(request).await;
    assert!(result.is_err());
    
    // Test too large dimensions
    let request = Request::new(CreateSimulationRequest {
        width: 2000,
        height: 2000,
        initial_pattern: String::new(),
    });
    
    let result = service.create_simulation(request).await;
    assert!(result.is_err());
}

#[tokio::test]
async fn test_get_simulation() {
    let service = create_test_service();
    
    // First create a simulation
    let create_request = Request::new(CreateSimulationRequest {
        width: 50,
        height: 50,
        initial_pattern: String::new(),
    });
    
    let create_response = service.create_simulation(create_request).await.unwrap();
    let created_simulation = create_response.into_inner();
    
    // Then get it
    let get_request = Request::new(GetSimulationRequest {
        id: created_simulation.id.clone(),
    });
    
    let get_response = service.get_simulation(get_request).await.unwrap();
    let simulation = get_response.into_inner();
    
    assert_eq!(simulation.id, created_simulation.id);
    assert_eq!(simulation.generation, 0);
    assert_eq!(simulation.live_cells, 0);
    assert!(simulation.grid.is_some());
    assert_eq!(simulation.grid.unwrap().width, 50);
    assert_eq!(simulation.grid.unwrap().height, 50);
}

#[tokio::test]
async fn test_get_simulation_not_found() {
    let service = create_test_service();
    let request = Request::new(GetSimulationRequest {
        id: "non-existent-id".to_string(),
    });
    
    let result = service.get_simulation(request).await;
    assert!(result.is_err());
}

#[tokio::test]
async fn test_update_simulation() {
    let service = create_test_service();
    
    // Create a simulation
    let create_request = Request::new(CreateSimulationRequest {
        width: 50,
        height: 50,
        initial_pattern: String::new(),
    });
    
    let create_response = service.create_simulation(create_request).await.unwrap();
    let created_simulation = create_response.into_inner();
    
    // Update it with some cells (blinker pattern)
    let update_request = Request::new(UpdateSimulationRequest {
        id: created_simulation.id.clone(),
        generation: 1,
        cells: vec![
            Cell { x: 25, y: 24, alive: true, neighbors: 0 },
            Cell { x: 25, y: 25, alive: true, neighbors: 0 },
            Cell { x: 25, y: 26, alive: true, neighbors: 0 },
        ],
    });
    
    let update_response = service.update_simulation(update_request).await.unwrap();
    let updated_simulation = update_response.into_inner();
    
    assert_eq!(updated_simulation.id, created_simulation.id);
    assert_eq!(updated_simulation.generation, 1);
    assert_eq!(updated_simulation.live_cells, 3);
    assert_eq!(updated_simulation.cells.len(), 3);
}

#[tokio::test]
async fn test_delete_simulation() {
    let service = create_test_service();
    
    // Create a simulation
    let create_request = Request::new(CreateSimulationRequest {
        width: 50,
        height: 50,
        initial_pattern: String::new(),
    });
    
    let create_response = service.create_simulation(create_request).await.unwrap();
    let created_simulation = create_response.into_inner();
    
    // Delete it
    let delete_request = Request::new(DeleteSimulationRequest {
        id: created_simulation.id.clone(),
    });
    
    let delete_response = service.delete_simulation(delete_request).await.unwrap();
    let result = delete_response.into_inner();
    
    assert!(result.success);
    assert_eq!(result.message, "Simulation deleted successfully");
    
    // Try to get it again - should fail
    let get_request = Request::new(GetSimulationRequest {
        id: created_simulation.id,
    });
    
    let get_result = service.get_simulation(get_request).await;
    assert!(get_result.is_err());
}

#[tokio::test]
async fn test_step_simulation() {
    let service = create_test_service();
    
    // Create a simulation
    let create_request = Request::new(CreateSimulationRequest {
        width: 50,
        height: 50,
        initial_pattern: String::new(),
    });
    
    let create_response = service.create_simulation(create_request).await.unwrap();
    let created_simulation = create_response.into_inner();
    
    // Add blinker pattern
    let update_request = Request::new(UpdateSimulationRequest {
        id: created_simulation.id.clone(),
        generation: 0,
        cells: vec![
            Cell { x: 25, y: 24, alive: true, neighbors: 0 },
            Cell { x: 25, y: 25, alive: true, neighbors: 0 },
            Cell { x: 25, y: 26, alive: true, neighbors: 0 },
        ],
    });
    
    service.update_simulation(update_request).await.unwrap();
    
    // Step the simulation
    let step_request = Request::new(StepSimulationRequest {
        id: created_simulation.id.clone(),
        steps: 1,
    });
    
    let step_response = service.step_simulation(step_request).await.unwrap();
    let result = step_response.into_inner();
    
    assert_eq!(result.generation, 1);
    assert_eq!(result.live_cells, 3); // Blinker should still have 3 cells after one step
    assert!(result.changed_cells >= 0);
}

#[tokio::test]
async fn test_load_pattern() {
    let service = create_test_service();
    
    // Create a simulation
    let create_request = Request::new(CreateSimulationRequest {
        width: 50,
        height: 50,
        initial_pattern: String::new(),
    });
    
    let create_response = service.create_simulation(create_request).await.unwrap();
    let created_simulation = create_response.into_inner();
    
    // Load a pattern (glider)
    let load_request = Request::new(LoadPatternRequest {
        id: created_simulation.id.clone(),
        pattern: Some(Pattern {
            name: "glider".to_string(),
            description: "A simple glider pattern".to_string(),
            author: "John Conway".to_string(),
            cells: vec![
                Position { x: 0, y: 1 },
                Position { x: 1, y: 2 },
                Position { x: 2, y: 0 },
                Position { x: 2, y: 1 },
                Position { x: 2, y: 2 },
            ],
        }),
        position: Some(Position { x: 10, y: 10 }),
    });
    
    let load_response = service.load_pattern(load_request).await.unwrap();
    let result = load_response.into_inner();
    
    assert!(result.success);
    assert_eq!(result.cells_added, 5);
    assert!(result.message.contains("glider"));
}

#[tokio::test]
async fn test_blinker_pattern_behavior() {
    let service = create_test_service();
    
    // Create a simulation
    let create_request = Request::new(CreateSimulationRequest {
        width: 50,
        height: 50,
        initial_pattern: String::new(),
    });
    
    let create_response = service.create_simulation(create_request).await.unwrap();
    let created_simulation = create_response.into_inner();
    
    // Add vertical blinker pattern
    let update_request = Request::new(UpdateSimulationRequest {
        id: created_simulation.id.clone(),
        generation: 0,
        cells: vec![
            Cell { x: 25, y: 24, alive: true, neighbors: 0 },
            Cell { x: 25, y: 25, alive: true, neighbors: 0 },
            Cell { x: 25, y: 26, alive: true, neighbors: 0 },
        ],
    });
    
    service.update_simulation(update_request).await.unwrap();
    
    // Step once - should become horizontal
    let step_request = Request::new(StepSimulationRequest {
        id: created_simulation.id.clone(),
        steps: 1,
    });
    
    let step_response = service.step_simulation(step_request).await.unwrap();
    let result = step_response.into_inner();
    
    assert_eq!(result.generation, 1);
    assert_eq!(result.live_cells, 3);
    
    // Get the simulation to check the pattern
    let get_request = Request::new(GetSimulationRequest {
        id: created_simulation.id.clone(),
    });
    
    let get_response = service.get_simulation(get_request).await.unwrap();
    let simulation = get_response.into_inner();
    
    // Should have horizontal blinker pattern
    assert_eq!(simulation.cells.len(), 3);
    let mut positions: Vec<(i32, i32)> = simulation.cells.iter().map(|c| (c.x, c.y)).collect();
    positions.sort();
    assert_eq!(positions, vec![(24, 25), (25, 25), (26, 25)]);
}

#[tokio::test]
async fn test_block_pattern_stability() {
    let service = create_test_service();
    
    // Create a simulation
    let create_request = Request::new(CreateSimulationRequest {
        width: 50,
        height: 50,
        initial_pattern: String::new(),
    });
    
    let create_response = service.create_simulation(create_request).await.unwrap();
    let created_simulation = create_response.into_inner();
    
    // Add block pattern (still life)
    let update_request = Request::new(UpdateSimulationRequest {
        id: created_simulation.id.clone(),
        generation: 0,
        cells: vec![
            Cell { x: 25, y: 25, alive: true, neighbors: 0 },
            Cell { x: 25, y: 26, alive: true, neighbors: 0 },
            Cell { x: 26, y: 25, alive: true, neighbors: 0 },
            Cell { x: 26, y: 26, alive: true, neighbors: 0 },
        ],
    });
    
    service.update_simulation(update_request).await.unwrap();
    
    // Step multiple times - should remain stable
    for _ in 0..5 {
        let step_request = Request::new(StepSimulationRequest {
            id: created_simulation.id.clone(),
            steps: 1,
        });
        
        let step_response = service.step_simulation(step_request).await.unwrap();
        let result = step_response.into_inner();
        
        assert_eq!(result.live_cells, 4); // Block should always have 4 cells
    }
}

#[tokio::test]
async fn test_empty_simulation_behavior() {
    let service = create_test_service();
    
    // Create empty simulation
    let create_request = Request::new(CreateSimulationRequest {
        width: 50,
        height: 50,
        initial_pattern: String::new(),
    });
    
    let create_response = service.create_simulation(create_request).await.unwrap();
    let created_simulation = create_response.into_inner();
    
    // Step an empty simulation
    let step_request = Request::new(StepSimulationRequest {
        id: created_simulation.id.clone(),
        steps: 1,
    });
    
    let step_response = service.step_simulation(step_request).await.unwrap();
    let result = step_response.into_inner();
    
    assert_eq!(result.generation, 1);
    assert_eq!(result.live_cells, 0);
    assert_eq!(result.changed_cells, 0);
}

#[tokio::test]
async fn test_multiple_simulations() {
    let service = create_test_service();
    
    // Create multiple simulations
    let mut simulation_ids = Vec::new();
    
    for i in 0..3 {
        let create_request = Request::new(CreateSimulationRequest {
            width: 50 + i * 10,
            height: 50 + i * 10,
            initial_pattern: String::new(),
        });
        
        let create_response = service.create_simulation(create_request).await.unwrap();
        let created_simulation = create_response.into_inner();
        simulation_ids.push(created_simulation.id);
    }
    
    // Verify all simulations exist
    for id in &simulation_ids {
        let get_request = Request::new(GetSimulationRequest {
            id: id.clone(),
        });
        
        let get_response = service.get_simulation(get_request).await.unwrap();
        let simulation = get_response.into_inner();
        assert_eq!(simulation.id, *id);
    }
    
    // Delete one simulation
    let delete_request = Request::new(DeleteSimulationRequest {
        id: simulation_ids[1].clone(),
    });
    
    let delete_response = service.delete_simulation(delete_request).await.unwrap();
    assert!(delete_response.into_inner().success);
    
    // Verify it's gone but others remain
    let get_request = Request::new(GetSimulationRequest {
        id: simulation_ids[1].clone(),
    });
    
    let get_result = service.get_simulation(get_request).await;
    assert!(get_result.is_err());
    
    // Other simulations should still exist
    for (i, id) in simulation_ids.iter().enumerate() {
        if i != 1 {
            let get_request = Request::new(GetSimulationRequest {
                id: id.clone(),
            });
            
            let get_response = service.get_simulation(get_request).await;
            assert!(get_response.is_ok());
        }
    }
}