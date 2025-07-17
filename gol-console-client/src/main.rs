use clap::{Parser, Subcommand};
use anyhow::Result;

mod client;
mod ui;
mod commands;

#[derive(Parser)]
#[command(name = "gol-console-client")]
#[command(about = "A Game of Life console client supporting multiple gRPC backends")]
#[command(version = "0.1.0")]
struct Cli {
    #[arg(long, default_value = "bevy")]
    backend: String,
    
    #[arg(long, default_value = "50051")]
    port: u16,
    
    #[arg(long, default_value = "localhost")]
    host: String,
    
    #[command(subcommand)]
    command: Option<Commands>,
}

#[derive(Subcommand)]
enum Commands {
    Load {
        #[arg(help = "Path to pattern file")]
        pattern: String,
    },
    Run {
        #[arg(short, long, help = "Number of generations to run")]
        generations: Option<u32>,
        #[arg(short, long, help = "Delay between generations in ms")]
        delay: Option<u64>,
    },
    Status,
    Stop,
    Interactive,
}

#[tokio::main]
async fn main() -> Result<()> {
    let cli = Cli::parse();
    
    let mut client = client::GameOfLifeClient::new(
        cli.backend.clone(),
        cli.host.clone(),
        cli.port,
    );
    
    match &cli.command {
        Some(Commands::Load { pattern }) => {
            commands::handle_load_command(&mut client, pattern).await?;
        }
        Some(Commands::Run { generations, delay }) => {
            commands::handle_run_command(&mut client, *generations, *delay).await?;
        }
        Some(Commands::Status) => {
            let status = commands::handle_status_command(&mut client).await?;
            println!("{}", status);
        }
        Some(Commands::Stop) => {
            commands::handle_stop_command(&mut client).await?;
        }
        Some(Commands::Interactive) => {
            commands::handle_interactive_command(&mut client).await?;
        }
        None => {
            println!("No command specified. Use --help for available commands.");
        }
    }
    
    Ok(())
}
