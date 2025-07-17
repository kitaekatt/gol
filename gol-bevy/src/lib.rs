//! Game of Life Bevy ECS Implementation
//! 
//! This library provides a Game of Life implementation using Bevy ECS.
//! It includes components, systems, and resources for simulating Conway's Game of Life.

pub mod components;
pub mod systems;
pub mod resources;
pub mod api;
pub mod grpc;

pub use components::*;
pub use systems::*;
pub use resources::*;
pub use api::*;
pub use grpc::*;