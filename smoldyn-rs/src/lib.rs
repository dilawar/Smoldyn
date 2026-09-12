//! smoldyn library.
use std::ffi::CString;

mod common;

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

use std::path::Path;

use crate::ffi::smolGetVersion;

unsafe extern "C" {
    /// `enum ErrorCode smolRunSim(simptr sim)`; `ErrorCode` has the ABI of `int`.
    fn smolRunSim(sim: *mut ffi::simstruct) -> i32;
    /// `enum ErrorCode smolFreeSim(simptr sim)`.
    fn smolFreeSim(sim: *mut ffi::simstruct) -> i32;
}

/// Run smoldyn simulator
pub fn run(model: &Path, flags: &str) -> anyhow::Result<()> {
    tracing::info!("Running model {:?}", model);

    let mut fileroot = common::path_to_bytes(model.parent().unwrap());
    fileroot.push(std::path::MAIN_SEPARATOR as u8);
    let fileroot = CString::new(fileroot).unwrap();
    let filename = CString::new(common::path_to_bytes(
        model
            .file_name()
            .expect("failed to get filename of modelfile"),
    ))
    .unwrap();
    let flags = CString::new(flags).unwrap();

    let sim = unsafe {
        ffi::smolPrepareSimFromFile(fileroot.as_ptr(), filename.as_ptr(), flags.as_ptr())
    };

    if sim.is_null() {
        anyhow::bail!("failed to load model {:?}", model);
    }

    let error_code = unsafe { smolRunSim(sim) };
    unsafe { smolFreeSim(sim) };

    if error_code != 0 {
        anyhow::bail!(
            "failed to run model {:?} (smoldyn error code {})",
            model,
            error_code
        );
    }

    Ok(())
}

/// Return smoldyn version
pub fn version() -> String {
    let version = unsafe { smolGetVersion() };

    format!("{version}")
}
