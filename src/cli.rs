use clap::{Parser, Subcommand};

#[derive(Parser, Debug)]
#[command(name = "plus")]
#[command(version, about, long_about = None)]
pub struct Cli {
    #[command(subcommand)]
    pub command: Command,
}

#[derive(Debug, Subcommand)]
pub enum Command {
    /// create a new project
    #[command(arg_required_else_help = true)]
    New {
        /// name of the project
        name: String,
    },

    /// initialize a new project in the current directory
    Init,
}
