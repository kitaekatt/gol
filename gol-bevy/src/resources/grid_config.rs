use bevy::prelude::*;

#[derive(Resource, Clone, Debug)]
pub struct GridConfig {
    pub width: u32,
    pub height: u32,
    pub wrap_edges: bool,
}

impl Default for GridConfig {
    fn default() -> Self {
        Self {
            width: 50,
            height: 50,
            wrap_edges: false,
        }
    }
}

impl GridConfig {
    pub fn new(width: u32, height: u32, wrap_edges: bool) -> Self {
        Self {
            width,
            height,
            wrap_edges,
        }
    }
}