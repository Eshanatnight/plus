mod cli;
mod error;

use anyhow::anyhow;
use clap::Parser;
use cli::{Cli, Command};
use crossterm::style::Stylize;
use error::PlusError;
use std::env;
use std::fs::DirBuilder;

fn main() -> anyhow::Result<()> {
    let args = Cli::parse();

    let mut path = env::current_dir()?;
    match args.command {
        Command::Init => {
            // INFO: When Initializing if a git Repository already exists Only inialize project and
            // not a git repository (same with worktrees)
            let _res = git2::Repository::init(path)?;
        },
        Command::New { name } => {
            path.push(&name);
            // FIX: use then / then_some rather than if
            if path.exists() {
                let msg = format!(
                    "Error: Failed to Initialize Project. Directory `{}` already exists",
                    name
                )
                .red();
                eprintln!("{}", msg);
                return Ok(());
            }

            path.push("src");
            DirBuilder::new().recursive(true).create(&path)?;

            // FIX: use then / then_some rather than if
            if !path.pop() {
                let msg = format!(
                    "Parent of Repository Path: `{}` does not exists",
                    path.to_str().expect("Path should be a valid string")
                )
                .dark_red();

                return Err(PlusError::FatalError(anyhow!(msg)).into());
            }
            let _res = git2::Repository::init(&path)?;
        },
    }

    Ok(())
}
