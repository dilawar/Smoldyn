//! smoldyn library.

mod common;

pub mod simulation;
pub use simulation::*;

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("Smoldyn/libsmoldyn.h");

        type simstruct;
        unsafe fn smolPrepareSimFromFile(
            filepath: *const c_char,
            filename: *const c_char,
            flags: *const c_char,
        ) -> *mut simstruct;

        unsafe fn smolGetVersion() -> f64;
    }
}

/// Return smoldyn version
pub fn version() -> String {
    let version = unsafe { crate::ffi::smolGetVersion() };

    format!("{version}")
}
