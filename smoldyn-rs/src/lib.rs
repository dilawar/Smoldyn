///! smoldyn library.
use libc::c_char;
use std::ffi::{CStr, CString};

mod common;

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("Smoldyn/libsmoldyn.h");

        type simstruct;
        unsafe fn simInitAndLoad(
            fileroot: *const c_char,
            filename: *const c_char,
            sim: *mut simstruct,
            flags: *const c_char,
        );
    }
}

use std::path::Path;

/// Run smoldyn simulator
pub fn run(model: &Path, flags: &str) -> anyhow::Result<()> {
    tracing::info!("Running model {:?}", model);

    let fileroot = CString::new(common::path_to_bytes(model.parent().unwrap())).unwrap();
    let filename = CString::new(common::path_to_bytes(
        model
            .file_name()
            .expect("failed to get filename of modelfile"),
    ))
    .unwrap();

    let flags = CString::new(flags).unwrap();
    let c_flags: *const c_char = flags.as_ptr() as *const c_char;

    unsafe {
        let sim = simptr::new();
        ffi::simInitAndLoad(fileroot.into_raw(), filename.into_raw(), sim, c_flags);
    }

    Ok(())
}
