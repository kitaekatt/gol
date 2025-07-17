use anyhow::{Result, Context};
use serde::{Deserialize, Serialize};
use std::fs;
use std::path::Path;
use crate::client::GameOfLifeClient;
use crate::client::game_of_life::{Pattern, Position, LoadPatternResponse};

#[derive(Debug, Serialize, Deserialize)]
pub struct PatternFile {
    pub name: String,
    pub description: String,
    pub author: String,
    pub cells: Vec<PatternCell>,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct PatternCell {
    pub x: i32,
    pub y: i32,
}

pub struct PatternCommands {
    client: GameOfLifeClient,
}

impl PatternCommands {
    pub fn new(client: GameOfLifeClient) -> Self {
        Self { client }
    }
    
    pub async fn load_from_file(&mut self, simulation_id: String, file_path: &str, x: i32, y: i32) -> Result<LoadPatternResponse> {
        let pattern_file = self.read_pattern_file(file_path)?;
        let pattern = self.convert_to_grpc_pattern(pattern_file)?;
        let position = Position { x, y };
        
        self.client.connect().await?;
        let response = self.client.load_pattern(simulation_id, pattern, position).await?;
        
        if response.success {
            println!("Pattern loaded successfully");
            println!("Added {} cells", response.cells_added);
        } else {
            println!("Failed to load pattern: {}", response.message);
        }
        
        Ok(response)
    }
    
    pub fn read_pattern_file(&self, file_path: &str) -> Result<PatternFile> {
        let path = Path::new(file_path);
        if !path.exists() {
            return Err(anyhow::anyhow!("Pattern file not found: {}", file_path));
        }
        
        let content = fs::read_to_string(path)
            .context("Failed to read pattern file")?;
            
        let pattern: PatternFile = serde_json::from_str(&content)
            .context("Failed to parse pattern file as JSON")?;
            
        Ok(pattern)
    }
    
    pub fn convert_to_grpc_pattern(&self, pattern_file: PatternFile) -> Result<Pattern> {
        let cells: Vec<Position> = pattern_file.cells
            .into_iter()
            .map(|cell| Position { x: cell.x, y: cell.y })
            .collect();
            
        Ok(Pattern {
            name: pattern_file.name,
            description: pattern_file.description,
            author: pattern_file.author,
            cells,
        })
    }
    
    pub fn list_available_patterns(&self, patterns_dir: &str) -> Result<Vec<String>> {
        let dir = Path::new(patterns_dir);
        if !dir.exists() {
            return Ok(vec![]);
        }
        
        let mut patterns = Vec::new();
        for entry in fs::read_dir(dir)? {
            let entry = entry?;
            let path = entry.path();
            if path.extension().and_then(|s| s.to_str()) == Some("json") {
                if let Some(name) = path.file_stem().and_then(|s| s.to_str()) {
                    patterns.push(name.to_string());
                }
            }
        }
        
        patterns.sort();
        Ok(patterns)
    }
    
    pub fn show_pattern_info(&self, file_path: &str) -> Result<()> {
        let pattern = self.read_pattern_file(file_path)?;
        println!("Pattern: {}", pattern.name);
        println!("Description: {}", pattern.description);
        println!("Author: {}", pattern.author);
        println!("Cells: {}", pattern.cells.len());
        
        let mut min_x = i32::MAX;
        let mut max_x = i32::MIN;
        let mut min_y = i32::MAX;
        let mut max_y = i32::MIN;
        
        for cell in &pattern.cells {
            min_x = min_x.min(cell.x);
            max_x = max_x.max(cell.x);
            min_y = min_y.min(cell.y);
            max_y = max_y.max(cell.y);
        }
        
        if !pattern.cells.is_empty() {
            println!("Dimensions: {}x{}", max_x - min_x + 1, max_y - min_y + 1);
        }
        
        Ok(())
    }
}