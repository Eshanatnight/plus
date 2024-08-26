use thiserror::Error;

#[derive(Debug, Error)]
pub enum PlusError {
    #[error("Fatal: {0}")]
    FatalError(#[from] anyhow::Error),
}
