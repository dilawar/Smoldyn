use std::{
    path::PathBuf,
    sync::{Arc, atomic::AtomicBool},
};
use tracing_subscriber::{EnvFilter, fmt, prelude::*};

use clap::{Parser, Subcommand};

#[derive(Parser)]
#[command(about, long_about = None)]
struct Cli {
    /// smoldyn model
    model: Option<PathBuf>,

    /// Turn debugging information on
    #[arg(short, long, action = clap::ArgAction::Count)]
    debug: u8,

    /// version
    #[arg(long)]
    version: bool,

    /// Simulated time between draws
    #[arg(long, default_value_t = 1.0)]
    plot_dt: f64,

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

    unsafe {
        std::env::set_var("SMOLDYN_LOG", log_level);
    }

    tracing_subscriber::registry()
        .with(fmt::layer())
        .with(EnvFilter::from_env("SMOLDYN_LOG"))
        .init();

    if cli.version {
        return show_version();
    }

    if let Some(model) = cli.model {
        let stop_me = Arc::new(AtomicBool::new(false));
        smoldyn::run(&model, "", cli.plot_dt, stop_me).expect("failed to run model");
    }
}

fn show_version() {
    println!("{}", smoldyn::version());
}
