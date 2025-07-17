use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CellData {
    pub x: i32,
    pub y: i32,
    pub alive: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GameState {
    pub generation: u64,
    pub cells: Vec<CellData>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PatternData {
    pub name: String,
    pub cells: Vec<(i32, i32)>,
}