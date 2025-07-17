pub mod service;
pub mod proto {
    tonic::include_proto!("game_of_life");
}

pub use service::GameOfLifeServiceImpl;
pub use proto::*;