use std::path::PathBuf;
use tracing_subscriber::{fmt, prelude::*, EnvFilter};

use clap::{Parser, Subcommand};

#[derive(Parser)]
#[command(version, about, long_about = None)]
struct Cli {
    /// smoldyn model
    model: PathBuf,

    /// Turn debugging information on
    #[arg(short, long, action = clap::ArgAction::Count)]
    debug: u8,

    #[command(subcommand)]
    command: Option<Commands>,
}

#[derive(Subcommand)]
enum Commands {
    /// does testing things
    Simulate,
}

fn main() {
    let cli = Cli::parse();

    // You can see how many times a particular flag or argument occurred
    // Note, only flags can have multiple occurrences
    let log_level = match cli.debug {
        0 => "warn",
        1 => "info",
        2 => "debug",
        _ => "trace",
    };

    std::env::set_var("SMOLDYN_LOG", log_level);
    tracing_subscriber::registry()
        .with(fmt::layer())
        .with(EnvFilter::from_env("SMOLDYN_LOG"))
        .init();

    smoldyn::run(&cli.model).expect("failed to run model");
}
