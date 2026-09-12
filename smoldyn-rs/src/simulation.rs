//! Simulation related functions

use std::ffi::CString;
use std::path::Path;

use crate::common;
use crate::ffi;

unsafe extern "C" {
    /// `enum ErrorCode smolRunSim(simptr sim)`; `ErrorCode` has the ABI of `int`.
    fn smolRunSim(sim: *mut ffi::simstruct) -> i32;
    /// `enum ErrorCode smolFreeSim(simptr sim)`.
    fn smolFreeSim(sim: *mut ffi::simstruct) -> i32;
}


/// Run smoldyn simulator
pub fn run(model: &Path, flags: &str) -> anyhow::Result<()> {
    tracing::info!("Running model {:?}", model);

    let mut fileroot = common::path_to_bytes(model.parent().expect("invalid filename"));
    fileroot.push(std::path::MAIN_SEPARATOR as u8);
    let fileroot = CString::new(fileroot).expect("invalid filename");
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

    anyhow::ensure!(!sim.is_null(), format!("failed to load model {model:?}"));

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
