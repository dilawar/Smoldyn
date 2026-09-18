//! Top-level data-structure

use std::collections::HashMap;
use std::path::PathBuf;

#[derive(Default, Debug)]
pub struct Simulation {
    pub model_path: PathBuf,
    pub boundary: Boundary,
    pub count: usize,
    pub current_time: f64,
}

impl Simulation {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn initialize(&mut self) -> anyhow::Result<()> {
        todo!()
    }

    pub fn run(
        &mut self,
        stoptime: f64,
        dt: f64,
        display: bool,
        overwrite: bool,
    ) -> anyhow::Result<()> {
        todo!()
    }

    pub fn run_until(
        &mut self,
        breaktime: f64,
        dt: f64,
        display: bool,
        overwrite: bool,
    ) -> anyhow::Result<()> {
        todo!()
    }

    pub fn set_current_time(&mut self, current_time: f64) {
        self.current_time = current_time
    }

    pub fn get_current_time(&self) -> f64 {
        self.current_time
    }

    pub fn add_command(
        &mut self,
        command: &str,
        cmd_type: char,
        kwargs: HashMap<String, f64>,
    ) -> anyhow::Result<()> {
        todo!()
    }
}

#[derive(Default, Debug, PartialEq)]
pub struct Boundary {
    low: [f64; 3],
    high: [f64; 3],
}

#[cfg(test)]
mod test {

    #[test]
    fn name() {
        todo!();
    }
}
