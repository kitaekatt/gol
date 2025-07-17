#[cfg(test)]
mod integration_tests {
    use bevy::prelude::*;
    use crate::components::{Position, CellState};
    
    #[test]
    fn test_components_integrate_with_bevy_ecs() {
        // Create a Bevy app for testing
        let mut app = App::new();
        app.add_plugins(MinimalPlugins);
        
        // Spawn an entity with our components
        let entity = app.world_mut().spawn((
            Position::new(5, 10),
            CellState::with_generation_and_neighbors(1, 3)
        )).id();
        
        // Query for the components
        let world = app.world_mut();
        let mut query = world.query::<(&Position, &CellState)>();
        let (position, cell_state) = query.get(world, entity).unwrap();
        
        // Verify the components are correctly stored
        assert_eq!(position.x, 5);
        assert_eq!(position.y, 10);
        assert_eq!(cell_state.generation, 1);
        assert_eq!(cell_state.neighbor_count, 3);
        assert!(cell_state.alive);
    }
    
    #[test]
    fn test_sparse_representation() {
        // Create a Bevy app for testing
        let mut app = App::new();
        app.add_plugins(MinimalPlugins);
        
        // Spawn multiple entities representing live cells
        let positions = vec![
            Position::new(0, 0),
            Position::new(1, 0),
            Position::new(2, 0),
        ];
        
        for pos in positions {
            app.world_mut().spawn((pos, CellState::new()));
        }
        
        // Query for all live cells
        let world = app.world_mut();
        let mut query = world.query::<(&Position, &CellState)>();
        let cells: Vec<_> = query.iter(world).collect();
        
        // Verify we have exactly 3 live cells (sparse representation)
        assert_eq!(cells.len(), 3);
        
        // Verify positions are correct
        let positions: Vec<Position> = cells.iter().map(|(pos, _)| **pos).collect();
        assert!(positions.contains(&Position::new(0, 0)));
        assert!(positions.contains(&Position::new(1, 0)));
        assert!(positions.contains(&Position::new(2, 0)));
    }
    
    #[test]
    fn test_component_queries() {
        let mut app = App::new();
        app.add_plugins(MinimalPlugins);
        
        // Create entities with different states
        app.world_mut().spawn((Position::new(0, 0), CellState::with_neighbors(2)));
        app.world_mut().spawn((Position::new(1, 0), CellState::with_neighbors(3)));
        app.world_mut().spawn((Position::new(2, 0), CellState::with_neighbors(4)));
        
        // Query for cells that should survive
        let world = app.world_mut();
        let mut query = world.query::<(&Position, &CellState)>();
        let survivors: Vec<_> = query.iter(world)
            .filter(|(_, cell)| cell.should_survive())
            .collect();
        
        // Only cells with 2 or 3 neighbors should survive
        assert_eq!(survivors.len(), 2);
    }
    
    #[test]
    fn test_component_mutation() {
        let mut app = App::new();
        app.add_plugins(MinimalPlugins);
        
        // Spawn an entity
        let entity = app.world_mut().spawn((
            Position::new(0, 0),
            CellState::with_neighbors(2)
        )).id();
        
        // Mutate the cell state
        {
            let mut cell_state = app.world_mut().get_mut::<CellState>(entity).unwrap();
            cell_state.neighbor_count = 3;
        }
        
        // Verify the mutation
        let cell_state = app.world().get::<CellState>(entity).unwrap();
        assert_eq!(cell_state.neighbor_count, 3);
    }
}