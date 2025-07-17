#[cfg(test)]
mod system_tests {
    use bevy::prelude::*;
    use crate::components::{Position, CellState};
    use crate::resources::{SimulationState, GridConfig};
    use crate::systems::{neighbor_counting_system, cell_lifecycle_system, cleanup_system};
    use std::collections::HashSet;
    
    fn create_test_app() -> App {
        let mut app = App::new();
        app.add_plugins(MinimalPlugins);
        app.init_resource::<SimulationState>();
        app.init_resource::<GridConfig>();
        app.add_systems(Update, (
            neighbor_counting_system,
            cell_lifecycle_system,
            cleanup_system,
        ).chain());
        app
    }
    
    fn create_neighbor_test_app() -> App {
        let mut app = App::new();
        app.add_plugins(MinimalPlugins);
        app.init_resource::<SimulationState>();
        app.init_resource::<GridConfig>();
        app.add_systems(Update, neighbor_counting_system);
        app
    }
    
    fn spawn_pattern(app: &mut App, pattern: &[(i32, i32)]) {
        for (x, y) in pattern {
            app.world_mut().spawn((
                Position::new(*x, *y),
                CellState::new()
            ));
        }
    }
    
    fn get_live_positions(app: &mut App) -> HashSet<(i32, i32)> {
        let mut positions = HashSet::new();
        let world = app.world_mut();
        let mut query = world.query::<&Position>();
        
        for position in query.iter(world) {
            positions.insert((position.x, position.y));
        }
        
        positions
    }
    
    #[test]
    fn test_neighbor_calculation_system() {
        let mut app = create_neighbor_test_app();
        
        // Create a simple 3x3 block pattern
        spawn_pattern(&mut app, &[
            (0, 0), (1, 0), (2, 0),
            (0, 1), (1, 1), (2, 1),
            (0, 2), (1, 2), (2, 2),
        ]);
        
        // Run neighbor calculation
        app.update();
        
        // Check that neighbor counts are calculated correctly
        let world = app.world_mut();
        let mut query = world.query::<(&Position, &CellState)>();
        
        for (position, cell_state) in query.iter(world) {
            let expected_neighbors = match (position.x, position.y) {
                (1, 1) => 8, // Center cell has 8 neighbors
                (0, 0) | (2, 0) | (0, 2) | (2, 2) => 3, // Corner cells have 3 neighbors
                _ => 5, // Edge cells have 5 neighbors
            };
            
            if cell_state.alive {
                assert_eq!(cell_state.neighbor_count, expected_neighbors,
                    "Cell at ({}, {}) should have {} neighbors, got {}",
                    position.x, position.y, expected_neighbors, cell_state.neighbor_count);
            }
        }
    }
    
    #[test]
    fn test_blinker_oscillator() {
        let mut app = create_test_app();
        
        // Create a blinker pattern (vertical line)
        spawn_pattern(&mut app, &[(1, 0), (1, 1), (1, 2)]);
        
        // Run one generation
        app.update();
        
        // Should become horizontal line
        let positions = get_live_positions(&mut app);
        let expected: HashSet<(i32, i32)> = [(0, 1), (1, 1), (2, 1)].iter().cloned().collect();
        assert_eq!(positions, expected, "Blinker should be horizontal after one generation");
        
        // Run another generation
        app.update();
        
        // Should become vertical line again
        let positions = get_live_positions(&mut app);
        let expected: HashSet<(i32, i32)> = [(1, 0), (1, 1), (1, 2)].iter().cloned().collect();
        assert_eq!(positions, expected, "Blinker should be vertical after two generations");
    }
    
    #[test]
    fn test_block_still_life() {
        let mut app = create_test_app();
        
        // Create a block pattern (2x2 square)
        spawn_pattern(&mut app, &[(0, 0), (1, 0), (0, 1), (1, 1)]);
        
        let initial_positions = get_live_positions(&mut app);
        
        // Run several generations
        for _ in 0..5 {
            app.update();
        }
        
        let final_positions = get_live_positions(&mut app);
        
        // Block should remain unchanged
        assert_eq!(initial_positions, final_positions, "Block should remain stable");
    }
    
    #[test]
    fn test_glider_spaceship() {
        let mut app = create_test_app();
        
        // Create a glider pattern
        spawn_pattern(&mut app, &[
            (1, 0),
            (2, 1),
            (0, 2), (1, 2), (2, 2),
        ]);
        
        // Run 4 generations (glider period)
        for _ in 0..4 {
            app.update();
        }
        
        // Glider should have moved one cell diagonally
        let positions = get_live_positions(&mut app);
        let expected: HashSet<(i32, i32)> = [
            (2, 1),
            (3, 2),
            (1, 3), (2, 3), (3, 3),
        ].iter().cloned().collect();
        
        assert_eq!(positions, expected, "Glider should move diagonally");
    }
    
    #[test]
    fn test_death_by_underpopulation() {
        let mut app = create_test_app();
        
        // Create a single cell (should die from underpopulation)
        spawn_pattern(&mut app, &[(0, 0)]);
        
        app.update();
        
        let positions = get_live_positions(&mut app);
        assert!(positions.is_empty(), "Single cell should die from underpopulation");
    }
    
    #[test]
    fn test_death_by_overpopulation() {
        let mut app = create_test_app();
        
        // Create a pattern where center cell has too many neighbors
        spawn_pattern(&mut app, &[
            (-1, -1), (0, -1), (1, -1),
            (-1, 0),  (0, 0),  (1, 0),
            (-1, 1),  (0, 1),  (1, 1),
        ]);
        
        app.update();
        
        let positions = get_live_positions(&mut app);
        
        // Center cell should die from overpopulation
        assert!(!positions.contains(&(0, 0)), "Center cell should die from overpopulation");
    }
    
    #[test]
    fn test_birth_by_reproduction() {
        let mut app = create_test_app();
        
        // Create a pattern where an empty cell has exactly 3 neighbors
        spawn_pattern(&mut app, &[
            (0, 0), (1, 0), (0, 1)
        ]);
        
        app.update();
        
        let positions = get_live_positions(&mut app);
        
        // Cell at (1, 1) should be born
        assert!(positions.contains(&(1, 1)), "Cell should be born with 3 neighbors");
    }
    
    #[test]
    fn test_sparse_representation_efficiency() {
        let mut app = create_test_app();
        
        // Create a small pattern in a large theoretical grid
        spawn_pattern(&mut app, &[(1000, 1000), (1001, 1000), (1000, 1001)]);
        
        let _initial_count = {
            let world = app.world_mut();
            let mut query = world.query::<&Position>();
            query.iter(world).count()
        };
        
        app.update();
        
        let final_count = {
            let world = app.world_mut();
            let mut query = world.query::<&Position>();
            query.iter(world).count()
        };
        
        // Should only process cells near the live ones, not the entire theoretical grid
        assert!(final_count < 20, "Should maintain sparse representation efficiency");
    }
    
    #[test]
    fn test_generation_counter() {
        let mut app = create_test_app();
        
        // Create a stable pattern
        spawn_pattern(&mut app, &[(0, 0), (1, 0), (0, 1), (1, 1)]);
        
        let initial_generation = app.world().resource::<SimulationState>().generation;
        
        // Run several generations
        for _ in 0..3 {
            app.update();
        }
        
        let final_generation = app.world().resource::<SimulationState>().generation;
        
        assert_eq!(final_generation, initial_generation + 3, "Generation counter should increment");
    }
    
    #[test]
    fn test_empty_grid() {
        let mut app = create_test_app();
        
        // Start with empty grid
        app.update();
        
        let positions = get_live_positions(&mut app);
        assert!(positions.is_empty(), "Empty grid should remain empty");
    }
}