use bevy_game_of_life::systems::game_of_life::*;
use bevy_game_of_life::components::cell::*;
use std::collections::HashSet;

#[test]
fn test_conway_rules_underpopulation() {
    // Any live cell with fewer than two live neighbors dies (underpopulation)
    
    // Cell with 0 neighbors
    assert!(!should_cell_survive(true, 0));
    
    // Cell with 1 neighbor
    assert!(!should_cell_survive(true, 1));
}

#[test]
fn test_conway_rules_survival() {
    // Any live cell with two or three live neighbors survives
    
    // Cell with 2 neighbors
    assert!(should_cell_survive(true, 2));
    
    // Cell with 3 neighbors  
    assert!(should_cell_survive(true, 3));
}

#[test]
fn test_conway_rules_overpopulation() {
    // Any live cell with more than three live neighbors dies (overpopulation)
    
    // Cell with 4 neighbors
    assert!(!should_cell_survive(true, 4));
    
    // Cell with 5 neighbors
    assert!(!should_cell_survive(true, 5));
    
    // Cell with 8 neighbors (maximum possible)
    assert!(!should_cell_survive(true, 8));
}

#[test]
fn test_conway_rules_reproduction() {
    // Any dead cell with exactly three live neighbors becomes alive (reproduction)
    
    // Dead cell with 3 neighbors
    assert!(should_cell_survive(false, 3));
    
    // Dead cell with other neighbor counts should stay dead
    assert!(!should_cell_survive(false, 0));
    assert!(!should_cell_survive(false, 1));
    assert!(!should_cell_survive(false, 2));
    assert!(!should_cell_survive(false, 4));
    assert!(!should_cell_survive(false, 5));
}

#[test]
fn test_neighbor_calculation_basic() {
    // Test neighbor counting for a simple 3x3 grid
    let live_cells = vec![
        (0, 0), (0, 1), (0, 2),
        (1, 0),         (1, 2),
        (2, 0), (2, 1), (2, 2),
    ];
    
    let live_set: HashSet<(i32, i32)> = live_cells.into_iter().collect();
    
    // Center cell (1, 1) should have 8 neighbors
    assert_eq!(count_live_neighbors((1, 1), &live_set, false), 8);
    
    // Corner cell (0, 0) should have 2 neighbors: (0, 1) and (1, 0)
    assert_eq!(count_live_neighbors((0, 0), &live_set, false), 2);
    
    // Edge cell (0, 1) should have 4 neighbors: (0, 0), (0, 2), (1, 0), (1, 2)
    assert_eq!(count_live_neighbors((0, 1), &live_set, false), 4);
}

#[test]
fn test_neighbor_calculation_with_wrapping() {
    // Test neighbor counting with edge wrapping enabled
    let live_cells = vec![(0, 0), (0, 1), (1, 0)];
    let live_set: HashSet<(i32, i32)> = live_cells.into_iter().collect();
    
    // In a 3x3 grid with wrapping, corner (2, 2) should see (0, 0), (0, 1), (1, 0) as neighbors
    let grid_width = 3;
    let grid_height = 3;
    
    let neighbors = count_live_neighbors_with_wrapping((2, 2), &live_set, grid_width, grid_height);
    assert_eq!(neighbors, 3);
}

#[test]
fn test_neighbor_calculation_without_wrapping() {
    // Test that cells outside grid boundaries don't count as neighbors
    let live_cells = vec![(0, 0)];
    let live_set: HashSet<(i32, i32)> = live_cells.into_iter().collect();
    
    // Cell at (-1, -1) doesn't exist, so (0, 0) has 0 neighbors
    assert_eq!(count_live_neighbors((0, 0), &live_set, false), 0);
    
    // Add a neighbor at (0, 1)
    let live_cells = vec![(0, 0), (0, 1)];
    let live_set: HashSet<(i32, i32)> = live_cells.into_iter().collect();
    
    // Now (0, 0) should have 1 neighbor
    assert_eq!(count_live_neighbors((0, 0), &live_set, false), 1);
}

#[test]
fn test_classic_patterns_blinker() {
    // Test the oscillating blinker pattern
    // Generation 0: vertical line at (1,0), (1,1), (1,2)
    let gen0 = vec![(1, 0), (1, 1), (1, 2)];
    
    // Generation 1: horizontal line at (0,1), (1,1), (2,1)
    let expected_gen1 = vec![(0, 1), (1, 1), (2, 1)];
    
    let gen1 = apply_game_of_life_rules(&gen0, false, None, None);
    
    let gen1_set: HashSet<(i32, i32)> = gen1.into_iter().collect();
    let expected_set: HashSet<(i32, i32)> = expected_gen1.into_iter().collect();
    
    assert_eq!(gen1_set, expected_set);
    
    // Generation 2: should return to vertical line
    let gen1_vec: Vec<(i32, i32)> = gen1_set.into_iter().collect();
    let gen2 = apply_game_of_life_rules(&gen1_vec, false, None, None);
    let gen2_set: HashSet<(i32, i32)> = gen2.into_iter().collect();
    let gen0_set: HashSet<(i32, i32)> = gen0.into_iter().collect();
    
    assert_eq!(gen2_set, gen0_set);
}

#[test]
fn test_classic_patterns_block() {
    // Test the stable block pattern (still life)
    let block = vec![(0, 0), (0, 1), (1, 0), (1, 1)];
    
    let next_gen = apply_game_of_life_rules(&block, false, None, None);
    
    let block_set: HashSet<(i32, i32)> = block.into_iter().collect();
    let next_set: HashSet<(i32, i32)> = next_gen.into_iter().collect();
    
    // Block should remain unchanged
    assert_eq!(block_set, next_set);
}

#[test]
fn test_empty_grid() {
    // Empty grid should remain empty
    let empty: Vec<(i32, i32)> = vec![];
    let next_gen = apply_game_of_life_rules(&empty, false, None, None);
    
    assert!(next_gen.is_empty());
}

#[test]
fn test_single_cell_dies() {
    // Single isolated cell should die (underpopulation)
    let single_cell = vec![(5, 5)];
    let next_gen = apply_game_of_life_rules(&single_cell, false, None, None);
    
    assert!(next_gen.is_empty());
}

#[test]
fn test_glider_first_generation() {
    // Test first step of glider pattern
    let glider_gen0 = vec![
        (1, 0),
        (2, 1),
        (0, 2), (1, 2), (2, 2)
    ];
    
    let glider_gen1_expected = vec![
        (0, 1),
        (2, 1),
        (1, 2), (2, 2),
        (1, 3)
    ];
    
    let gen1 = apply_game_of_life_rules(&glider_gen0, false, None, None);
    
    let gen1_set: HashSet<(i32, i32)> = gen1.into_iter().collect();
    let expected_set: HashSet<(i32, i32)> = glider_gen1_expected.into_iter().collect();
    
    assert_eq!(gen1_set, expected_set);
}

#[test]
fn test_boundary_conditions() {
    // Test cells near grid boundaries (without wrapping)
    let cells_near_origin = vec![(-1, -1), (-1, 0), (0, -1), (0, 0)];
    
    // This should work fine with negative coordinates in infinite grid
    let next_gen = apply_game_of_life_rules(&cells_near_origin, false, None, None);
    
    // Only cell at (0, 0) has enough neighbors to survive, others die
    // Actually, let's check: (-1,-1) has neighbors (-1,0), (0,-1), (0,0) = 3 neighbors = survives
    // (-1,0) has neighbors (-1,-1), (0,-1), (0,0) = 3 neighbors = survives
    // (0,-1) has neighbors (-1,-1), (-1,0), (0,0) = 3 neighbors = survives  
    // (0,0) has neighbors (-1,-1), (-1,0), (0,-1) = 3 neighbors = survives
    // So all should survive!
    assert_eq!(next_gen.len(), 4);
}