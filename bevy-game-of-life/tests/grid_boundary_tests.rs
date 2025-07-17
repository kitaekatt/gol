use bevy_game_of_life::systems::game_of_life::*;
use bevy_game_of_life::components::grid::*;
use std::collections::HashSet;

#[test]
fn test_grid_boundaries_without_wrapping() {
    // Test that cells at grid edges behave correctly without wrapping
    let grid_config = GridBoundary {
        width: 5,
        height: 5,
        wrap_edges: false,
    };
    
    // Cell at corner (0,0) with one neighbor at (0,1)
    let live_cells = vec![(0, 0), (0, 1)];
    let live_set: HashSet<(i32, i32)> = live_cells.into_iter().collect();
    
    // Corner cell should have 1 neighbor (not counting wrapped positions)
    let neighbors = count_live_neighbors_bounded((0, 0), &live_set, &grid_config);
    assert_eq!(neighbors, 1);
    
    // Cell at (0,1) should have 1 neighbor
    let neighbors = count_live_neighbors_bounded((0, 1), &live_set, &grid_config);
    assert_eq!(neighbors, 1);
}

#[test]
fn test_grid_boundaries_with_wrapping() {
    // Test that cells at grid edges wrap around correctly
    let grid_config = GridBoundary {
        width: 3,
        height: 3,
        wrap_edges: true,
    };
    
    // Place cells at corners that should see each other when wrapping
    let live_cells = vec![(0, 0), (2, 2)];
    let live_set: HashSet<(i32, i32)> = live_cells.into_iter().collect();
    
    // Corner cell (0,0) should see (2,2) as neighbor when wrapping
    let neighbors = count_live_neighbors_bounded((0, 0), &live_set, &grid_config);
    assert_eq!(neighbors, 1);
    
    // Cell (2,2) should see (0,0) as neighbor when wrapping  
    let neighbors = count_live_neighbors_bounded((2, 2), &live_set, &grid_config);
    assert_eq!(neighbors, 1);
}

#[test]
fn test_out_of_bounds_cells_without_wrapping() {
    // Test behavior when trying to access cells outside grid boundaries
    let grid_config = GridBoundary {
        width: 3,
        height: 3,
        wrap_edges: false,
    };
    
    let live_cells = vec![(1, 1)]; // Single cell in center
    let live_set: HashSet<(i32, i32)> = live_cells.into_iter().collect();
    
    // Cell outside grid should have 0 neighbors (or error)
    let neighbors = count_live_neighbors_bounded((-1, -1), &live_set, &grid_config);
    assert_eq!(neighbors, 0);
    
    let neighbors = count_live_neighbors_bounded((5, 5), &live_set, &grid_config);
    assert_eq!(neighbors, 0);
}

#[test]
fn test_edge_wrapping_horizontal() {
    // Test horizontal edge wrapping
    let grid_config = GridBoundary {
        width: 5,
        height: 3,
        wrap_edges: true,
    };
    
    // Place cells at left and right edges
    let live_cells = vec![(0, 1), (4, 1)]; // Left and right edge of middle row
    let live_set: HashSet<(i32, i32)> = live_cells.into_iter().collect();
    
    // Left edge cell should see right edge as neighbor when wrapping
    let neighbors = count_live_neighbors_bounded((0, 1), &live_set, &grid_config);
    assert_eq!(neighbors, 1);
    
    // Right edge cell should see left edge as neighbor when wrapping
    let neighbors = count_live_neighbors_bounded((4, 1), &live_set, &grid_config);
    assert_eq!(neighbors, 1);
}

#[test]
fn test_edge_wrapping_vertical() {
    // Test vertical edge wrapping
    let grid_config = GridBoundary {
        width: 3,
        height: 5,
        wrap_edges: true,
    };
    
    // Place cells at top and bottom edges
    let live_cells = vec![(1, 0), (1, 4)]; // Top and bottom edge of middle column
    let live_set: HashSet<(i32, i32)> = live_cells.into_iter().collect();
    
    // Top edge cell should see bottom edge as neighbor when wrapping
    let neighbors = count_live_neighbors_bounded((1, 0), &live_set, &grid_config);
    assert_eq!(neighbors, 1);
    
    // Bottom edge cell should see top edge as neighbor when wrapping
    let neighbors = count_live_neighbors_bounded((1, 4), &live_set, &grid_config);
    assert_eq!(neighbors, 1);
}

#[test]
fn test_wrapped_glider_pattern() {
    // Test glider pattern behavior with wrapping vs non-wrapping
    let glider = vec![
        (1, 0),
        (2, 1),
        (0, 2), (1, 2), (2, 2)
    ];
    
    // Without wrapping (infinite grid)
    let next_gen_infinite = apply_game_of_life_rules(&glider, false, None, None);
    
    // With wrapping in small grid
    let grid_config = GridBoundary {
        width: 5,
        height: 5,
        wrap_edges: true,
    };
    let next_gen_wrapped = apply_game_of_life_rules_bounded(&glider, &grid_config);
    
    // Results should be different due to edge effects
    let infinite_set: HashSet<(i32, i32)> = next_gen_infinite.into_iter().collect();
    let wrapped_set: HashSet<(i32, i32)> = next_gen_wrapped.into_iter().collect();
    
    // In small grid with wrapping, behavior will be different
    // This mainly tests that both functions work and produce valid output
    assert!(!infinite_set.is_empty());
    assert!(!wrapped_set.is_empty());
}

#[test]
fn test_grid_coordinate_validation() {
    let grid_config = GridBoundary {
        width: 10,
        height: 10,
        wrap_edges: false,
    };
    
    // Test valid coordinates
    assert!(is_valid_coordinate((0, 0), &grid_config));
    assert!(is_valid_coordinate((9, 9), &grid_config));
    assert!(is_valid_coordinate((5, 5), &grid_config));
    
    // Test invalid coordinates
    assert!(!is_valid_coordinate((-1, 0), &grid_config));
    assert!(!is_valid_coordinate((0, -1), &grid_config));
    assert!(!is_valid_coordinate((10, 0), &grid_config));
    assert!(!is_valid_coordinate((0, 10), &grid_config));
    assert!(!is_valid_coordinate((10, 10), &grid_config));
}

#[test]
fn test_wrapped_coordinate_mapping() {
    let grid_config = GridBoundary {
        width: 5,
        height: 3,
        wrap_edges: true,
    };
    
    // Test horizontal wrapping
    assert_eq!(wrap_coordinate((-1, 1), &grid_config), (4, 1));
    assert_eq!(wrap_coordinate((5, 1), &grid_config), (0, 1));
    assert_eq!(wrap_coordinate((7, 1), &grid_config), (2, 1));
    
    // Test vertical wrapping
    assert_eq!(wrap_coordinate((1, -1), &grid_config), (1, 2));
    assert_eq!(wrap_coordinate((1, 3), &grid_config), (1, 0));
    assert_eq!(wrap_coordinate((1, 5), &grid_config), (1, 2));
    
    // Test corner wrapping
    assert_eq!(wrap_coordinate((-1, -1), &grid_config), (4, 2));
    assert_eq!(wrap_coordinate((5, 3), &grid_config), (0, 0));
}

#[test]
fn test_blinker_at_edge_without_wrapping() {
    // Test blinker pattern at grid edge without wrapping
    let grid_config = GridBoundary {
        width: 3,
        height: 3,
        wrap_edges: false,
    };
    
    // Vertical blinker at right edge
    let blinker = vec![(2, 0), (2, 1), (2, 2)];
    let next_gen = apply_game_of_life_rules_bounded(&blinker, &grid_config);
    
    // Should become horizontal, but truncated at edge
    let expected = vec![(1, 1), (2, 1)]; // Only cells within grid
    let next_set: HashSet<(i32, i32)> = next_gen.into_iter().collect();
    let expected_set: HashSet<(i32, i32)> = expected.into_iter().collect();
    
    assert_eq!(next_set, expected_set);
}

#[test]
fn test_blinker_at_edge_with_wrapping() {
    // Test blinker pattern at grid edge with wrapping
    let grid_config = GridBoundary {
        width: 3,
        height: 3,
        wrap_edges: true,
    };
    
    // Vertical blinker at right edge
    let blinker = vec![(2, 0), (2, 1), (2, 2)];
    let next_gen = apply_game_of_life_rules_bounded(&blinker, &grid_config);
    
    // In a 3x3 grid with wrapping, this vertical blinker creates a more complex pattern
    // due to the small grid size and edge wrapping effects
    let expected = vec![(0, 0), (0, 1), (0, 2), (1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2)]; // All cells become alive due to wrapping
    let next_set: HashSet<(i32, i32)> = next_gen.into_iter().collect();
    let expected_set: HashSet<(i32, i32)> = expected.into_iter().collect();
    
    assert_eq!(next_set, expected_set);
}