use bevy::prelude::*;
use bevy_game_of_life::components::cell::*;
use bevy_game_of_life::components::grid::*;
use bevy_game_of_life::systems::game_of_life::*;
use bevy_game_of_life::resources::config::*;

#[test]
fn test_cell_state_component() {
    let cell_state = CellState::new(true);
    
    assert!(cell_state.alive);
    assert_eq!(cell_state.age, 0);
    
    let mut aged_cell = cell_state;
    aged_cell.increment_age();
    assert_eq!(aged_cell.age, 1);
}

#[test]
fn test_grid_position_component() {
    let position = GridPosition::new(10, 20);
    
    assert_eq!(position.x, 10);
    assert_eq!(position.y, 20);
    
    let neighbors = position.get_neighbor_positions();
    assert_eq!(neighbors.len(), 8);
    
    // Check that all 8 neighbors are included
    assert!(neighbors.contains(&GridPosition::new(9, 19)));   // top-left
    assert!(neighbors.contains(&GridPosition::new(10, 19)));  // top
    assert!(neighbors.contains(&GridPosition::new(11, 19)));  // top-right
    assert!(neighbors.contains(&GridPosition::new(9, 20)));   // left
    assert!(neighbors.contains(&GridPosition::new(11, 20)));  // right
    assert!(neighbors.contains(&GridPosition::new(9, 21)));   // bottom-left
    assert!(neighbors.contains(&GridPosition::new(10, 21)));  // bottom
    assert!(neighbors.contains(&GridPosition::new(11, 21)));  // bottom-right
}

#[test]
fn test_neighbor_count_component() {
    let mut neighbor_count = NeighborCount::new();
    
    assert_eq!(neighbor_count.count, 0);
    assert!(neighbor_count.dirty);
    
    neighbor_count.set_count(5);
    assert_eq!(neighbor_count.count, 5);
    assert!(!neighbor_count.dirty);
    
    neighbor_count.mark_dirty();
    assert!(neighbor_count.dirty);
}

#[test]
fn test_bevy_app_with_game_components() {
    let mut app = App::new();
    
    // Add minimal Bevy plugins for testing
    app.add_plugins(MinimalPlugins);
    
    // Spawn test entities with components
    let entity1 = app.world.spawn((
        CellState::new(true),
        GridPosition::new(0, 0),
        NeighborCount::new(),
    )).id();
    
    let _entity2 = app.world.spawn((
        CellState::new(false),
        GridPosition::new(1, 1),
        NeighborCount::new(),
    )).id();
    
    // Verify entities exist with correct components
    let mut cell_query = app.world.query::<(&CellState, &GridPosition, &NeighborCount)>();
    let entities: Vec<_> = cell_query.iter(&app.world).collect();
    
    assert_eq!(entities.len(), 2);
    
    // Test component access
    let cell_state = app.world.get::<CellState>(entity1).unwrap();
    assert!(cell_state.alive);
    
    let position = app.world.get::<GridPosition>(entity1).unwrap();
    assert_eq!(position.x, 0);
    assert_eq!(position.y, 0);
}

#[test]
fn test_game_config_resource() {
    let mut app = App::new();
    app.add_plugins(MinimalPlugins);
    
    // Add config resource
    let config = GameConfig::default();
    app.insert_resource(config);
    
    // Verify resource exists and can be accessed
    let resource = app.world.get_resource::<GameConfig>().unwrap();
    assert_eq!(resource.grid.width, 100);
    assert_eq!(resource.grid.height, 100);
}

#[test]
fn test_spatial_grid_resource() {
    let mut app = App::new();
    app.add_plugins(MinimalPlugins);
    
    // Add spatial grid resource
    let spatial_grid = SpatialGrid::new();
    app.insert_resource(spatial_grid);
    
    // Test adding entities to spatial grid
    let _entity = app.world.spawn((
        CellState::new(true),
        GridPosition::new(5, 10),
    )).id();
    
    // In a real system, we'd update the spatial grid through a system
    // For testing, we'll verify the resource exists
    let spatial_resource = app.world.get_resource::<SpatialGrid>().unwrap();
    assert!(spatial_resource.cells.is_empty()); // Empty initially
}

#[test]
fn test_simulation_timer_resource() {
    let mut app = App::new();
    app.add_plugins(MinimalPlugins);
    
    // Add simulation timer
    let timer = SimulationTimer::new(10); // 10 steps per second
    app.insert_resource(timer);
    
    // Verify timer resource
    let timer_resource = app.world.get_resource::<SimulationTimer>().unwrap();
    assert_eq!(timer_resource.steps_per_second(), 10);
}

#[test] 
fn test_component_queries() {
    let mut app = App::new();
    app.add_plugins(MinimalPlugins);
    
    // Spawn some test entities
    app.world.spawn((
        CellState::new(true),
        GridPosition::new(0, 0),
        NeighborCount::new(),
    ));
    
    app.world.spawn((
        CellState::new(false),
        GridPosition::new(1, 0),
        NeighborCount::new(),
    ));
    
    app.world.spawn((
        CellState::new(true),
        GridPosition::new(0, 1),
        NeighborCount::new(),
    ));
    
    // Test query for live cells only
    let mut live_cell_query = app.world.query_filtered::<&GridPosition, With<CellState>>();
    let live_positions: Vec<_> = live_cell_query.iter(&app.world).collect();
    assert_eq!(live_positions.len(), 3); // All cells have CellState
    
    // Test query with component filter
    let mut query = app.world.query::<(&CellState, &GridPosition)>();
    let mut live_count = 0;
    
    for (cell_state, _position) in query.iter(&app.world) {
        if cell_state.alive {
            live_count += 1;
        }
    }
    
    assert_eq!(live_count, 2);
}

#[test]
fn test_entity_lifecycle() {
    let mut app = App::new();
    app.add_plugins(MinimalPlugins);
    
    // Spawn entity
    let entity = app.world.spawn((
        CellState::new(true),
        GridPosition::new(5, 5),
    )).id();
    
    // Verify entity exists
    assert!(app.world.get_entity(entity).is_some());
    
    // Despawn entity
    app.world.despawn(entity);
    
    // Verify entity no longer exists
    assert!(app.world.get_entity(entity).is_none());
}

#[test]
fn test_component_removal_and_insertion() {
    let mut app = App::new();
    app.add_plugins(MinimalPlugins);
    
    // Spawn entity with components
    let entity = app.world.spawn((
        CellState::new(true),
        GridPosition::new(0, 0),
    )).id();
    
    // Add neighbor count component
    app.world.entity_mut(entity).insert(NeighborCount::new());
    
    // Verify all components exist
    assert!(app.world.get::<CellState>(entity).is_some());
    assert!(app.world.get::<GridPosition>(entity).is_some());
    assert!(app.world.get::<NeighborCount>(entity).is_some());
    
    // Remove neighbor count component
    app.world.entity_mut(entity).remove::<NeighborCount>();
    
    // Verify component was removed
    assert!(app.world.get::<NeighborCount>(entity).is_none());
    assert!(app.world.get::<CellState>(entity).is_some());
    assert!(app.world.get::<GridPosition>(entity).is_some());
}