//! smoldyn library.

mod common;

pub mod simulation;
pub use simulation::*;

#[cxx::bridge]
mod ffi {
    /// Snapshot of every live molecule at one instant.
    ///
    /// `positions` is flattened row-major: molecule `i` occupies
    /// `positions[i * dim .. i * dim + dim]`. `species` indexes into
    /// `species_names`; `state` is a `MolecState` discriminant.
    #[derive(Clone, Debug)]
    struct MoleculeSnapshot {
        dim: i32,
        species_names: Vec<String>,
        serno: Vec<u64>,
        species: Vec<i32>,
        state: Vec<i32>,
        positions: Vec<f64>,
    }

    unsafe extern "C++" {
        include!("Smoldyn/libsmoldyn.h");
        include!("simstruct.h");
        include!("molecules.h");

        type simstruct;

        unsafe fn smolPrepareSimFromFile(
            filepath: *const c_char,
            filename: *const c_char,
            flags: *const c_char,
        ) -> *mut simstruct;

        unsafe fn smolGetVersion() -> f64;

        fn simstruct_dim(sim: &simstruct) -> i32;
        fn simstruct_condition(sim: &simstruct) -> i32;
        fn simstruct_time(sim: &simstruct) -> f64;
        fn simstruct_tmin(sim: &simstruct) -> f64;
        fn simstruct_tmax(sim: &simstruct) -> f64;
        fn simstruct_tbreak(sim: &simstruct) -> f64;
        fn simstruct_dt(sim: &simstruct) -> f64;
        fn simstruct_accur(sim: &simstruct) -> f64;
        fn simstruct_elapsedtime(sim: &simstruct) -> f64;
        fn simstruct_quitatend(sim: &simstruct) -> bool;
        fn simstruct_randseed(sim: &simstruct) -> i64;
        fn simstruct_nvar(sim: &simstruct) -> i32;
        fn simstruct_maxvar(sim: &simstruct) -> i32;
        fn simstruct_eventcount(sim: &simstruct, index: i32) -> i32;
        fn simstruct_eventcount_max() -> i32;

        fn simstruct_molecules(sim: &simstruct) -> MoleculeSnapshot;
    }
}

/// Return smoldyn version
pub fn version() -> String {
    let version = unsafe { crate::ffi::smolGetVersion() };

    format!("{version}")
}
