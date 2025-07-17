//! Game of Life ECS Systems
//! 
//! This module contains the core ECS systems for the Game of Life simulation.
//! - `neighbor_system`: Calculate neighbor counts for all cells
//! - `lifecycle_system`: Apply Conway's Game of Life rules
//!
//! The systems use sparse representation for efficiency - only live cells and
//! their neighbors are processed.

use bevy::prelude::*;

pub mod neighbor_system;
pub mod lifecycle_system;

#[cfg(test)]
mod system_tests;

#[cfg(test)]
mod scheduler_integration_test;

pub use neighbor_system::{neighbor_calculation_system, neighbor_counting_system};
pub use lifecycle_system::{lifecycle_system, cell_lifecycle_system, cleanup_system};

// System sets for organizing execution order
#[derive(SystemSet, Debug, Clone, PartialEq, Eq, Hash)]
pub enum GameOfLifeSystemSet {
    NeighborCalculation,
    LifecycleUpdate,
    Cleanup,
}