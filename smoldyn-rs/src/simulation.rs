//! Simulation related functions

use std::ffi::CString;
use std::path::Path;
use std::sync::Arc;
use std::sync::atomic::{AtomicBool, Ordering};

use crate::common::{self, ConstSimptr, MutSimptr};
use crate::ffi;

unsafe extern "C" {
    /// `enum ErrorCode smolRunSimUntil(simptr sim,double breaktime)`;
    /// `ErrorCode` has the ABI of `int`.
    fn smolRunSimUntil(sim: *mut ffi::simstruct, breaktime: f64) -> i32;
}

/// Run a smoldyn model, drawing the simulation every `plot_dt` of simulated
/// time.
///
/// The simulation is advanced in `plot_dt` chunks with `smolRunSimUntil` and
/// [`draw_simulation`] is called synchronously after each chunk.
pub fn run(
    model: &Path,
    flags: &str,
    plot_dt: f64,
    stop_me: Arc<AtomicBool>,
) -> anyhow::Result<()> {
    tracing::info!("Running model {:?}", model);
    anyhow::ensure!(plot_dt > 0.0, "plot_dt must be positive");

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

    let const_simptr = ConstSimptr::from(&sim);
    let (mut now, tmax) = const_simptr
        .snapshot()
        .map(|state| (state.time, state.tmax))
        .unwrap_or((0.0, 0.0));

    // Advance the simulation one `plot_dt` at a time, drawing after each step.
    while now < tmax {
        if stop_me.load(Ordering::Relaxed) {
            tracing::info!("stopping simulation at t={now}");
            break;
        }

        let next = (now + plot_dt).min(tmax);
        let error_code = unsafe { smolRunSimUntil(sim.0, next) };
        anyhow::ensure!(
            error_code == 0,
            "failed to run model {model:?} until t={next} (smoldyn error code {error_code})",
        );

        draw_simulation(&const_simptr);
        now = next;
    }

    Ok(())
}

/// Draw the current state of the simulation.
///
/// This is called between `plot_dt` chunks, so it is safe to read the
/// `simstruct` here. Replace the body with the actual plotting code.
fn draw_simulation(sim: &ConstSimptr) {
    if let Some(state) = sim.snapshot() {
        println!("drawing simulation: {state}");
    }
    if let Some(eventcounts) = sim.eventcounts() {
        tracing::debug!("eventcounts: {eventcounts:?}");
    }
    if let Some(molecules) = sim.molecules()
        && !molecules.is_empty()
    {
        tracing::debug!(
            "{} molecules, first {:?} ({:?})",
            molecules.len(),
            molecules.position(0),
            molecules.species_name(0),
        );
    }
}
