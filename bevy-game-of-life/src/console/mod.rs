pub mod controller;
pub mod renderer;
pub mod input;

pub use controller::{SimulationController, SimulationSnapshot, PerformanceMetrics};
pub use renderer::{ConsoleRenderer, RenderConfig};
pub use input::{ConsoleInput, InputEvent, InputState};