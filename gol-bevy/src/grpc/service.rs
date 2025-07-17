use tonic::{Request, Response, Status, Code};
use std::pin::Pin;
use std::sync::Arc;
use tokio::sync::Mutex;
use tokio_stream::{Stream, StreamExt};
use bevy::prelude::*;

use crate::grpc::proto::*;
use crate::resources::Simulations;
use crate::components::{Position, CellState};

pub struct GameOfLifeServiceImpl {
    pub simulations: Arc<Mutex<Simulations>>,
}

impl GameOfLifeServiceImpl {
    pub fn new() -> Self {
        Self {
            simulations: Arc::new(Mutex::new(Simulations::new())),
        }
    }
}

#[tonic::async_trait]
impl game_of_life_service_server::GameOfLifeService for GameOfLifeServiceImpl {
    async fn get_status(&self, _request: Request<StatusRequest>) -> Result<Response<StatusResponse>, Status> {
        let simulations = self.simulations.lock().await;
        let response = StatusResponse {
            status: "healthy".to_string(),
            version: "1.0.0".to_string(),
            implementation: "bevy".to_string(),
            uptime_seconds: simulations.uptime_seconds(),
        };
        Ok(Response::new(response))
    }

    async fn create_simulation(&self, request: Request<CreateSimulationRequest>) -> Result<Response<SimulationResponse>, Status> {
        let req = request.into_inner();
        let mut simulations = self.simulations.lock().await;
        
        if req.width <= 0 || req.height <= 0 {
            return Err(Status::new(Code::InvalidArgument, "Width and height must be positive"));
        }
        
        if req.width > 1000 || req.height > 1000 {
            return Err(Status::new(Code::InvalidArgument, "Grid size too large (max 1000x1000)"));
        }
        
        let id = simulations.create_simulation(req.width, req.height, 
            if req.initial_pattern.is_empty() { None } else { Some(req.initial_pattern) });
        
        let simulation = simulations.get_simulation(&id).unwrap();
        let response = SimulationResponse {
            id: id.clone(),
            generation: simulation.generation as i64,
            live_cells: simulation.get_live_cell_count(),
            grid: Some(GridInfo {
                width: simulation.width,
                height: simulation.height,
            }),
            cells: simulation.get_live_cells().into_iter().map(|(x, y)| {
                let cell_state = simulation.cells.get(&(x, y)).unwrap();
                Cell {
                    x,
                    y,
                    alive: cell_state.alive,
                    neighbors: cell_state.neighbor_count as i32,
                }
            }).collect(),
        };
        
        Ok(Response::new(response))
    }

    async fn get_simulation(&self, request: Request<GetSimulationRequest>) -> Result<Response<SimulationResponse>, Status> {
        let req = request.into_inner();
        let simulations = self.simulations.lock().await;
        
        let simulation = simulations.get_simulation(&req.id)
            .ok_or_else(|| Status::new(Code::NotFound, "Simulation not found"))?;
        
        let response = SimulationResponse {
            id: req.id.clone(),
            generation: simulation.generation as i64,
            live_cells: simulation.get_live_cell_count(),
            grid: Some(GridInfo {
                width: simulation.width,
                height: simulation.height,
            }),
            cells: simulation.get_live_cells().into_iter().map(|(x, y)| {
                let cell_state = simulation.cells.get(&(x, y)).unwrap();
                Cell {
                    x,
                    y,
                    alive: cell_state.alive,
                    neighbors: cell_state.neighbor_count as i32,
                }
            }).collect(),
        };
        
        Ok(Response::new(response))
    }

    async fn update_simulation(&self, request: Request<UpdateSimulationRequest>) -> Result<Response<SimulationResponse>, Status> {
        let req = request.into_inner();
        let mut simulations = self.simulations.lock().await;
        
        let simulation = simulations.get_simulation_mut(&req.id)
            .ok_or_else(|| Status::new(Code::NotFound, "Simulation not found"))?;
        
        if req.generation > 0 {
            simulation.generation = req.generation as u64;
        }
        
        if !req.cells.is_empty() {
            simulation.cells.clear();
            for cell in req.cells {
                if cell.x >= 0 && cell.x < simulation.width && cell.y >= 0 && cell.y < simulation.height {
                    simulation.cells.insert((cell.x, cell.y), CellState {
                        alive: cell.alive,
                        generation: simulation.generation,
                        neighbor_count: cell.neighbors as u8,
                    });
                }
            }
        }
        
        let response = SimulationResponse {
            id: req.id.clone(),
            generation: simulation.generation as i64,
            live_cells: simulation.get_live_cell_count(),
            grid: Some(GridInfo {
                width: simulation.width,
                height: simulation.height,
            }),
            cells: simulation.get_live_cells().into_iter().map(|(x, y)| {
                let cell_state = simulation.cells.get(&(x, y)).unwrap();
                Cell {
                    x,
                    y,
                    alive: cell_state.alive,
                    neighbors: cell_state.neighbor_count as i32,
                }
            }).collect(),
        };
        
        Ok(Response::new(response))
    }

    async fn delete_simulation(&self, request: Request<DeleteSimulationRequest>) -> Result<Response<DeleteResponse>, Status> {
        let req = request.into_inner();
        let mut simulations = self.simulations.lock().await;
        
        let success = simulations.delete_simulation(&req.id);
        let response = DeleteResponse {
            success,
            message: if success {
                "Simulation deleted successfully".to_string()
            } else {
                "Simulation not found".to_string()
            },
        };
        
        Ok(Response::new(response))
    }

    async fn step_simulation(&self, request: Request<StepSimulationRequest>) -> Result<Response<StepResponse>, Status> {
        let req = request.into_inner();
        let mut simulations = self.simulations.lock().await;
        
        let simulation = simulations.get_simulation_mut(&req.id)
            .ok_or_else(|| Status::new(Code::NotFound, "Simulation not found"))?;
        
        let steps = if req.steps <= 0 { 1 } else { req.steps };
        let initial_cells = simulation.get_live_cell_count();
        
        // Apply Game of Life rules for the specified number of steps
        for _ in 0..steps {
            simulation.generation += 1;
            
            // Calculate neighbors for all cells
            let mut neighbor_counts: std::collections::HashMap<(i32, i32), u8> = std::collections::HashMap::new();
            
            for ((x, y), cell) in &simulation.cells {
                if cell.alive {
                    let neighbors = [
                        (x - 1, y - 1), (*x, y - 1), (x + 1, y - 1),
                        (x - 1, *y),                  (x + 1, *y),
                        (x - 1, y + 1), (*x, y + 1), (x + 1, y + 1),
                    ];
                    
                    for (nx, ny) in neighbors {
                        if nx >= 0 && nx < simulation.width && ny >= 0 && ny < simulation.height {
                            *neighbor_counts.entry((nx, ny)).or_insert(0) += 1;
                        }
                    }
                }
            }
            
            // Apply Game of Life rules
            let mut new_cells = std::collections::HashMap::new();
            
            // Check all positions that might have cells
            for ((x, y), neighbor_count) in neighbor_counts {
                let currently_alive = simulation.cells.get(&(x, y)).map(|c| c.alive).unwrap_or(false);
                
                let will_be_alive = if currently_alive {
                    neighbor_count == 2 || neighbor_count == 3
                } else {
                    neighbor_count == 3
                };
                
                if will_be_alive {
                    new_cells.insert((x, y), CellState {
                        alive: true,
                        generation: simulation.generation,
                        neighbor_count,
                    });
                }
            }
            
            simulation.cells = new_cells;
        }
        
        let final_cells = simulation.get_live_cell_count();
        let changed_cells = (initial_cells as i64 - final_cells as i64).abs();
        
        let response = StepResponse {
            generation: simulation.generation as i64,
            live_cells: final_cells,
            changed_cells,
        };
        
        Ok(Response::new(response))
    }

    async fn load_pattern(&self, request: Request<LoadPatternRequest>) -> Result<Response<LoadPatternResponse>, Status> {
        let req = request.into_inner();
        let mut simulations = self.simulations.lock().await;
        
        let simulation = simulations.get_simulation_mut(&req.id)
            .ok_or_else(|| Status::new(Code::NotFound, "Simulation not found"))?;
        
        let pattern = req.pattern.ok_or_else(|| Status::new(Code::InvalidArgument, "Pattern is required"))?;
        let position = req.position.ok_or_else(|| Status::new(Code::InvalidArgument, "Position is required"))?;
        
        let pattern_cells: Vec<(i32, i32)> = pattern.cells.into_iter()
            .map(|pos| (pos.x, pos.y))
            .collect();
        
        let cells_added = simulation.add_pattern(&pattern_cells, position.x, position.y);
        
        let response = LoadPatternResponse {
            success: cells_added > 0,
            cells_added,
            message: if cells_added > 0 {
                format!("Pattern '{}' loaded successfully", pattern.name)
            } else {
                "No cells were added (pattern outside grid or cells already exist)".to_string()
            },
        };
        
        Ok(Response::new(response))
    }

    type StreamSimulationStream = Pin<Box<dyn Stream<Item = Result<SimulationUpdate, Status>> + Send>>;

    async fn stream_simulation(&self, request: Request<StreamRequest>) -> Result<Response<Self::StreamSimulationStream>, Status> {
        let req = request.into_inner();
        let simulations = self.simulations.clone();
        
        // Verify simulation exists
        {
            let sim_guard = simulations.lock().await;
            if sim_guard.get_simulation(&req.id).is_none() {
                return Err(Status::new(Code::NotFound, "Simulation not found"));
            }
        }
        
        let stream = async_stream::stream! {
            let mut interval = tokio::time::interval(
                tokio::time::Duration::from_millis(
                    if req.step_interval_ms > 0 { req.step_interval_ms as u64 } else { 1000 }
                )
            );
            
            loop {
                interval.tick().await;
                
                let mut sim_guard = simulations.lock().await;
                let simulation = match sim_guard.get_simulation_mut(&req.id) {
                    Some(sim) => sim,
                    None => {
                        yield Err(Status::new(Code::NotFound, "Simulation not found"));
                        break;
                    }
                };
                
                if req.auto_step {
                    // Step the simulation
                    simulation.generation += 1;
                    
                    // Apply Game of Life rules (simplified for streaming)
                    let mut neighbor_counts: std::collections::HashMap<(i32, i32), u8> = std::collections::HashMap::new();
                    
                    for ((x, y), cell) in &simulation.cells {
                        if cell.alive {
                            let neighbors = [
                                (x - 1, y - 1), (*x, y - 1), (x + 1, y - 1),
                                (x - 1, *y),                  (x + 1, *y),
                                (x - 1, y + 1), (*x, y + 1), (x + 1, y + 1),
                            ];
                            
                            for (nx, ny) in neighbors {
                                if nx >= 0 && nx < simulation.width && ny >= 0 && ny < simulation.height {
                                    *neighbor_counts.entry((nx, ny)).or_insert(0) += 1;
                                }
                            }
                        }
                    }
                    
                    let mut new_cells = std::collections::HashMap::new();
                    
                    for ((x, y), neighbor_count) in neighbor_counts {
                        let currently_alive = simulation.cells.get(&(x, y)).map(|c| c.alive).unwrap_or(false);
                        
                        let will_be_alive = if currently_alive {
                            neighbor_count == 2 || neighbor_count == 3
                        } else {
                            neighbor_count == 3
                        };
                        
                        if will_be_alive {
                            new_cells.insert((x, y), CellState {
                                alive: true,
                                generation: simulation.generation,
                                neighbor_count,
                            });
                        }
                    }
                    
                    simulation.cells = new_cells;
                }
                
                let live_cells = simulation.get_live_cell_count();
                let changed_cells: Vec<Cell> = simulation.get_live_cells().into_iter().map(|(x, y)| {
                    let cell_state = simulation.cells.get(&(x, y)).unwrap();
                    Cell {
                        x,
                        y,
                        alive: cell_state.alive,
                        neighbors: cell_state.neighbor_count as i32,
                    }
                }).collect();
                
                yield Ok(SimulationUpdate {
                    generation: simulation.generation as i64,
                    live_cells,
                    changed_cells,
                    simulation_ended: live_cells == 0,
                });
                
                if live_cells == 0 {
                    break;
                }
            }
        };
        
        Ok(Response::new(Box::pin(stream)))
    }
}