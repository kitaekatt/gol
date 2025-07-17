//! Game of Life ECS Components
//! 
//! This module contains the core ECS components for the Game of Life simulation.
//! - `Position`: Represents a cell's position in the grid
//! - `CellState`: Represents the state of a cell (alive, generation, neighbor count)

pub mod position;
pub mod cell_state;

#[cfg(test)]
mod integration_tests;

pub use position::Position;
pub use cell_state::CellState;

// Type aliases for convenience
pub type CellEntity = (Position, CellState);
pub type CellPosition = Position;