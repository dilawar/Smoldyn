//! Simulation related functions

use std::ffi::CString;
use std::path::Path;
use std::sync::Arc;
use std::sync::atomic::{AtomicBool, Ordering};

use crate::common::{self, ConstSimptr, MutSimptr};
use crate::ffi;

unsafe extern "C" {
    /// `enum ErrorCode smolRunSim(simptr sim)`; `ErrorCode` has the ABI of `int`.
    fn smolRunSim(sim: *mut ffi::simstruct) -> i32;
}

/// Run smoldyn simulator
pub fn run(model: &Path, flags: &str, stop_me: Arc<AtomicBool>) -> anyhow::Result<()> {
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
    let sim = MutSimptr(sim);

    anyhow::ensure!(!sim.is_null(), format!("failed to load model {model:?}"));

    // create a thread to read simptr
    let watcher_stop = stop_me.clone();
    let const_simptr = ConstSimptr::from(&sim);
    let t = std::thread::spawn(move || {
        watch_simptr(const_simptr, watcher_stop);
    });

    // start simulation.
    let error_code = unsafe { smolRunSim(sim.0) };
    anyhow::ensure!(
        error_code == 0,
        "failed to run model {model:?} (smoldyn error code {error_code})",
    );
    stop_me.store(true, Ordering::Relaxed);

    t.join().expect("failed to join");

    Ok(())
}

fn watch_simptr(_sim: ConstSimptr, stop_me: Arc<AtomicBool>) {
    loop {
        println!("watching simptr");
        std::thread::sleep(std::time::Duration::from_secs(1));
        if stop_me.load(Ordering::Relaxed) {
            break;
        }
    }
}
