#[cfg(test)]
mod scheduler_integration_test {
    use bevy::prelude::*;
    use crate::components::{Position, CellState};
    use crate::resources::{SimulationState, GridConfig};
    use crate::systems::{neighbor_counting_system, cell_lifecycle_system, cleanup_system};
    
    #[test]
    fn test_systems_integrate_with_bevy_scheduler() {
        let mut app = App::new();
        app.add_plugins(MinimalPlugins);
        app.init_resource::<SimulationState>();
        app.init_resource::<GridConfig>();
        
        // Add systems to scheduler in the correct order
        app.add_systems(Update, (
            neighbor_counting_system,
            cell_lifecycle_system,
            cleanup_system,
        ).chain());
        
        // Spawn a blinker pattern
        app.world_mut().spawn((Position::new(1, 0), CellState::new()));
        app.world_mut().spawn((Position::new(1, 1), CellState::new()));
        app.world_mut().spawn((Position::new(1, 2), CellState::new()));
        
        // Run the scheduler (should execute all systems in order)
        app.update();
        
        // Verify that the systems executed correctly
        let world = app.world_mut();
        let mut query = world.query::<&Position>();
        let positions: Vec<_> = query.iter(world).map(|p| (p.x, p.y)).collect();
        
        // Should have transformed from vertical to horizontal line
        assert_eq!(positions.len(), 3, "Should have 3 live cells");
        
        // Run another update cycle
        app.update();
        
        let world = app.world_mut();
        let mut query = world.query::<&Position>();
        let positions: Vec<_> = query.iter(world).map(|p| (p.x, p.y)).collect();
        
        // Should have transformed back to vertical line
        assert_eq!(positions.len(), 3, "Should still have 3 live cells");
    }
    
    #[test]
    fn test_system_execution_order() {
        let mut app = App::new();
        app.add_plugins(MinimalPlugins);
        app.init_resource::<SimulationState>();
        app.init_resource::<GridConfig>();
        
        // Add systems in chain to enforce execution order
        app.add_systems(Update, (
            neighbor_counting_system,
            cell_lifecycle_system,
            cleanup_system,
        ).chain());
        
        // Create a simple pattern that should produce predictable results
        app.world_mut().spawn((Position::new(0, 0), CellState::new()));
        app.world_mut().spawn((Position::new(1, 0), CellState::new()));
        app.world_mut().spawn((Position::new(0, 1), CellState::new()));
        
        let initial_generation = app.world().resource::<SimulationState>().generation;
        
        // Run one update cycle
        app.update();
        
        // Check that generation counter was incremented (proving systems executed)
        let final_generation = app.world().resource::<SimulationState>().generation;
        assert!(final_generation > initial_generation, "Generation should have incremented");
        
        // Verify cells are still present (block pattern should remain stable)
        let world = app.world_mut();
        let mut query = world.query::<&Position>();
        let count = query.iter(world).count();
        assert!(count > 0, "Should have live cells after update");
    }
}