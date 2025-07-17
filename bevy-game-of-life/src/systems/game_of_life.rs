use std::collections::{HashMap, HashSet};
use crate::components::grid::GridBoundary;

// Core Conway's Game of Life rule implementation
pub fn should_cell_survive(currently_alive: bool, neighbor_count: u8) -> bool {
    match (currently_alive, neighbor_count) {
        // Live cell with 2 or 3 neighbors survives
        (true, 2) | (true, 3) => true,
        // Dead cell with exactly 3 neighbors becomes alive
        (false, 3) => true,
        // All other cases: cell dies or stays dead
        _ => false,
    }
}

// Count live neighbors for a position in an infinite grid
pub fn count_live_neighbors(
    position: (i32, i32),
    live_cells: &HashSet<(i32, i32)>,
    _wrap_edges: bool,
) -> u8 {
    let (x, y) = position;
    let mut count = 0;
    
    // Check all 8 neighbors
    for dx in -1..=1 {
        for dy in -1..=1 {
            if dx == 0 && dy == 0 {
                continue; // Skip the cell itself
            }
            
            let neighbor_pos = (x + dx, y + dy);
            if live_cells.contains(&neighbor_pos) {
                count += 1;
            }
        }
    }
    
    count
}

// Count live neighbors with grid boundary handling
pub fn count_live_neighbors_bounded(
    position: (i32, i32),
    live_cells: &HashSet<(i32, i32)>,
    boundary: &GridBoundary,
) -> u8 {
    let (x, y) = position;
    let mut count = 0;
    
    // Check all 8 neighbors
    for dx in -1..=1 {
        for dy in -1..=1 {
            if dx == 0 && dy == 0 {
                continue; // Skip the cell itself
            }
            
            let neighbor_x = x + dx;
            let neighbor_y = y + dy;
            
            let neighbor_pos = if boundary.wrap_edges {
                // Handle wrapping
                let wrapped_x = if neighbor_x < 0 {
                    boundary.width + (neighbor_x % boundary.width)
                } else {
                    neighbor_x % boundary.width
                };
                
                let wrapped_y = if neighbor_y < 0 {
                    boundary.height + (neighbor_y % boundary.height)
                } else {
                    neighbor_y % boundary.height
                };
                
                (wrapped_x, wrapped_y)
            } else {
                // Check if neighbor is within bounds
                if neighbor_x >= 0 && neighbor_x < boundary.width && 
                   neighbor_y >= 0 && neighbor_y < boundary.height {
                    (neighbor_x, neighbor_y)
                } else {
                    continue; // Skip out-of-bounds neighbors
                }
            };
            
            if live_cells.contains(&neighbor_pos) {
                count += 1;
            }
        }
    }
    
    count
}

// Count neighbors with wrapping for specific grid size
pub fn count_live_neighbors_with_wrapping(
    position: (i32, i32),
    live_cells: &HashSet<(i32, i32)>,
    grid_width: i32,
    grid_height: i32,
) -> u8 {
    let boundary = GridBoundary {
        width: grid_width,
        height: grid_height,
        wrap_edges: true,
    };
    count_live_neighbors_bounded(position, live_cells, &boundary)
}

// Apply Game of Life rules to get next generation
pub fn apply_game_of_life_rules(
    live_cells: &[(i32, i32)],
    wrap_edges: bool,
    grid_width: Option<i32>,
    grid_height: Option<i32>,
) -> Vec<(i32, i32)> {
    let live_set: HashSet<(i32, i32)> = live_cells.iter().cloned().collect();
    let mut next_generation = Vec::new();
    
    // Get all cells that need to be checked (live cells + their neighbors)
    let mut cells_to_check = HashSet::new();
    
    for &(x, y) in &live_set {
        // Add the live cell itself
        cells_to_check.insert((x, y));
        
        // Add all neighbors
        for dx in -1..=1 {
            for dy in -1..=1 {
                let neighbor_pos = (x + dx, y + dy);
                cells_to_check.insert(neighbor_pos);
            }
        }
    }
    
    // Apply rules to each cell
    for &position in &cells_to_check {
        let currently_alive = live_set.contains(&position);
        
        let neighbor_count = if wrap_edges && grid_width.is_some() && grid_height.is_some() {
            count_live_neighbors_with_wrapping(
                position,
                &live_set,
                grid_width.unwrap(),
                grid_height.unwrap(),
            )
        } else {
            count_live_neighbors(position, &live_set, false)
        };
        
        if should_cell_survive(currently_alive, neighbor_count) {
            next_generation.push(position);
        }
    }
    
    next_generation
}

// Apply rules with explicit boundary handling
pub fn apply_game_of_life_rules_bounded(
    live_cells: &[(i32, i32)],
    boundary: &GridBoundary,
) -> Vec<(i32, i32)> {
    let live_set: HashSet<(i32, i32)> = live_cells.iter().cloned().collect();
    let mut next_generation = Vec::new();
    
    // Get all cells that need to be checked
    let mut cells_to_check = HashSet::new();
    
    for &(x, y) in &live_set {
        // Add the live cell itself
        cells_to_check.insert((x, y));
        
        // Add all neighbors (respecting boundaries)
        for dx in -1..=1 {
            for dy in -1..=1 {
                let neighbor_x = x + dx;
                let neighbor_y = y + dy;
                
                if boundary.wrap_edges {
                    // With wrapping, wrap coordinates to valid range
                    let wrapped = wrap_coordinate((neighbor_x, neighbor_y), boundary);
                    cells_to_check.insert(wrapped);
                } else {
                    // Without wrapping, only check cells within bounds
                    if neighbor_x >= 0 && neighbor_x < boundary.width && 
                       neighbor_y >= 0 && neighbor_y < boundary.height {
                        cells_to_check.insert((neighbor_x, neighbor_y));
                    }
                }
            }
        }
    }
    
    // Apply rules to each cell
    for &position in &cells_to_check {
        // Skip positions outside bounds when not wrapping
        if !boundary.wrap_edges {
            let (x, y) = position;
            if x < 0 || x >= boundary.width || y < 0 || y >= boundary.height {
                continue;
            }
        }
        
        let currently_alive = live_set.contains(&position);
        let neighbor_count = count_live_neighbors_bounded(position, &live_set, boundary);
        
        if should_cell_survive(currently_alive, neighbor_count) {
            next_generation.push(position);
        }
    }
    
    next_generation
}

// Helper functions for boundary validation
pub fn is_valid_coordinate(position: (i32, i32), boundary: &GridBoundary) -> bool {
    let (x, y) = position;
    x >= 0 && x < boundary.width && y >= 0 && y < boundary.height
}

pub fn wrap_coordinate(position: (i32, i32), boundary: &GridBoundary) -> (i32, i32) {
    let (x, y) = position;
    
    let wrapped_x = if x < 0 {
        boundary.width + (x % boundary.width)
    } else {
        x % boundary.width
    };
    
    let wrapped_y = if y < 0 {
        boundary.height + (y % boundary.height)
    } else {
        y % boundary.height
    };
    
    (wrapped_x, wrapped_y)
}

// Pattern generation helpers
pub fn generate_glider_pattern(offset_x: i32, offset_y: i32) -> Vec<(i32, i32)> {
    vec![
        (1 + offset_x, 0 + offset_y),
        (2 + offset_x, 1 + offset_y),
        (0 + offset_x, 2 + offset_y),
        (1 + offset_x, 2 + offset_y),
        (2 + offset_x, 2 + offset_y),
    ]
}

pub fn generate_blinker_pattern(offset_x: i32, offset_y: i32) -> Vec<(i32, i32)> {
    vec![
        (1 + offset_x, 0 + offset_y),
        (1 + offset_x, 1 + offset_y),
        (1 + offset_x, 2 + offset_y),
    ]
}

pub fn generate_gosper_gun_pattern(offset_x: i32, offset_y: i32) -> Vec<(i32, i32)> {
    vec![
        (0 + offset_x, 4 + offset_y), (0 + offset_x, 5 + offset_y),
        (1 + offset_x, 4 + offset_y), (1 + offset_x, 5 + offset_y),
        (10 + offset_x, 4 + offset_y), (10 + offset_x, 5 + offset_y), (10 + offset_x, 6 + offset_y),
        (11 + offset_x, 3 + offset_y), (11 + offset_x, 7 + offset_y),
        (12 + offset_x, 2 + offset_y), (12 + offset_x, 8 + offset_y),
        (13 + offset_x, 2 + offset_y), (13 + offset_x, 8 + offset_y),
        (14 + offset_x, 5 + offset_y),
        (15 + offset_x, 3 + offset_y), (15 + offset_x, 7 + offset_y),
        (16 + offset_x, 4 + offset_y), (16 + offset_x, 5 + offset_y), (16 + offset_x, 6 + offset_y),
        (17 + offset_x, 5 + offset_y),
        (20 + offset_x, 2 + offset_y), (20 + offset_x, 3 + offset_y), (20 + offset_x, 4 + offset_y),
        (21 + offset_x, 2 + offset_y), (21 + offset_x, 3 + offset_y), (21 + offset_x, 4 + offset_y),
        (22 + offset_x, 1 + offset_y), (22 + offset_x, 5 + offset_y),
        (24 + offset_x, 0 + offset_y), (24 + offset_x, 1 + offset_y), (24 + offset_x, 5 + offset_y), (24 + offset_x, 6 + offset_y),
        (34 + offset_x, 2 + offset_y), (34 + offset_x, 3 + offset_y),
        (35 + offset_x, 2 + offset_y), (35 + offset_x, 3 + offset_y),
    ]
}

pub fn generate_block_pattern(offset_x: i32, offset_y: i32) -> Vec<(i32, i32)> {
    vec![
        (0 + offset_x, 0 + offset_y),
        (0 + offset_x, 1 + offset_y),
        (1 + offset_x, 0 + offset_y),
        (1 + offset_x, 1 + offset_y),
    ]
}

// Performance optimization: batch neighbor counting
pub fn batch_count_neighbors(
    positions: &[(i32, i32)],
    live_cells: &HashSet<(i32, i32)>,
    wrap_edges: bool,
) -> HashMap<(i32, i32), u8> {
    let mut neighbor_counts = HashMap::new();
    
    for &position in positions {
        let count = count_live_neighbors(position, live_cells, wrap_edges);
        neighbor_counts.insert(position, count);
    }
    
    neighbor_counts
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_should_cell_survive() {
        // Test underpopulation
        assert!(!should_cell_survive(true, 0));
        assert!(!should_cell_survive(true, 1));
        
        // Test survival
        assert!(should_cell_survive(true, 2));
        assert!(should_cell_survive(true, 3));
        
        // Test overpopulation
        assert!(!should_cell_survive(true, 4));
        assert!(!should_cell_survive(true, 8));
        
        // Test reproduction
        assert!(should_cell_survive(false, 3));
        assert!(!should_cell_survive(false, 2));
        assert!(!should_cell_survive(false, 4));
    }
    
    #[test]
    fn test_count_live_neighbors() {
        let live_cells: HashSet<(i32, i32)> = vec![(0, 0), (0, 1), (1, 0)].into_iter().collect();
        
        // Cell at origin should have 2 neighbors
        assert_eq!(count_live_neighbors((0, 0), &live_cells, false), 2);
        
        // Cell at (1, 1) should have 3 neighbors
        assert_eq!(count_live_neighbors((1, 1), &live_cells, false), 3);
        
        // Empty position should have 0 neighbors
        assert_eq!(count_live_neighbors((5, 5), &live_cells, false), 0);
    }
}