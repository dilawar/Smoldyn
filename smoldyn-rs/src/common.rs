//! Common functions

use std::path::Path;

use crate::ffi;

unsafe extern "C" {

    /// `enum ErrorCode smolFreeSim(simptr sim)`.
    fn smolFreeSim(sim: *mut ffi::simstruct) -> i32;
}

// Thanks <https://stackoverflow.com/a/57667836/1805129>
#[cfg(unix)]
pub(crate) fn path_to_bytes<P: AsRef<Path>>(path: P) -> Vec<u8> {
    use std::os::unix::ffi::OsStrExt;
    path.as_ref().as_os_str().as_bytes().to_vec()
}

#[cfg(not(unix))]
pub(crate) fn path_to_bytes<P: AsRef<Path>>(path: P) -> Vec<u8> {
    // On Windows, could use std::os::windows::ffi::OsStrExt to encode_wide(),
    // but you end up with a Vec<u16> instead of a Vec<u8>, so that doesn't
    // really help.
    path.as_ref().to_string_lossy().to_string().into_bytes()
}

#[derive(Clone, Debug)]
pub struct ConstSimptr(pub *const ffi::simstruct);

unsafe impl Send for ConstSimptr {}
unsafe impl Sync for ConstSimptr {}

impl From<&MutSimptr> for ConstSimptr {
    fn from(value: &MutSimptr) -> Self {
        Self(value.0 as *const ffi::simstruct)
    }
}

/// A read-only snapshot of the scalar fields of a `simstruct`.
#[derive(Clone, Debug)]
pub struct SimState {
    pub dim: i32,
    pub condition: i32,
    pub time: f64,
    pub tmin: f64,
    pub tmax: f64,
    pub tbreak: f64,
    pub dt: f64,
    pub accur: f64,
    pub elapsedtime: f64,
    pub quitatend: bool,
    pub randseed: i64,
    pub nvar: i32,
    pub maxvar: i32,
}

impl std::fmt::Display for SimState {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "t={} dt={} range=[{}, {}] tbreak={} dim={} accur={} elapsed={}s \
             condition={} quitatend={} randseed={} vars={}/{}",
            self.time,
            self.dt,
            self.tmin,
            self.tmax,
            self.tbreak,
            self.dim,
            self.accur,
            self.elapsedtime,
            self.condition,
            self.quitatend,
            self.randseed,
            self.nvar,
            self.maxvar,
        )
    }
}

impl ConstSimptr {
    /// Returns a snapshot of the scalar `simstruct` fields, or `None` when the
    /// underlying pointer is null.
    ///
    /// The returned values are copied out one at a time; a snapshot taken while
    /// the simulation is running is therefore not guaranteed to be internally
    /// consistent.
    pub fn snapshot(&self) -> Option<SimState> {
        if self.0.is_null() {
            return None;
        }
        let sim = unsafe { &*self.0 };
        Some(SimState {
            dim: ffi::simstruct_dim(sim),
            condition: ffi::simstruct_condition(sim),
            time: ffi::simstruct_time(sim),
            tmin: ffi::simstruct_tmin(sim),
            tmax: ffi::simstruct_tmax(sim),
            tbreak: ffi::simstruct_tbreak(sim),
            dt: ffi::simstruct_dt(sim),
            accur: ffi::simstruct_accur(sim),
            elapsedtime: ffi::simstruct_elapsedtime(sim),
            quitatend: ffi::simstruct_quitatend(sim),
            randseed: ffi::simstruct_randseed(sim),
            nvar: ffi::simstruct_nvar(sim),
            maxvar: ffi::simstruct_maxvar(sim),
        })
    }

    /// Returns the per-event counters (`simstruct::eventcount`).
    pub fn eventcounts(&self) -> Option<Vec<i32>> {
        if self.0.is_null() {
            return None;
        }
        let sim = unsafe { &*self.0 };
        let count = ffi::simstruct_eventcount_max();
        Some(
            (0..count)
                .map(|i| ffi::simstruct_eventcount(sim, i))
                .collect(),
        )
    }

    /// Returns a snapshot of every live molecule, or `None` when the
    /// underlying pointer is null.
    ///
    /// Like [`snapshot`](Self::snapshot) this should be called from a safe
    /// point (not concurrently with the simulation stepping) to get a
    /// consistent view.
    pub fn molecules(&self) -> Option<ffi::MoleculeSnapshot> {
        if self.0.is_null() {
            return None;
        }
        let sim = unsafe { &*self.0 };
        Some(ffi::simstruct_molecules(sim))
    }
}

impl ffi::MoleculeSnapshot {
    /// Number of molecules in the snapshot.
    pub fn len(&self) -> usize {
        self.serno.len()
    }

    /// Whether the snapshot contains no molecules.
    pub fn is_empty(&self) -> bool {
        self.serno.is_empty()
    }

    /// Position `[x, y, z, ..]` of molecule `i`, or `None` if out of range.
    pub fn position(&self, i: usize) -> Option<&[f64]> {
        let dim = self.dim.max(0) as usize;
        let start = i.checked_mul(dim)?;
        self.positions.get(start..start + dim)
    }

    /// Species name of molecule `i`, or `None` if out of range.
    pub fn species_name(&self, i: usize) -> Option<&str> {
        let id = *self.species.get(i)?;
        self.species_names.get(id as usize).map(String::as_str)
    }
}

#[derive(Clone, Debug)]
pub struct MutSimptr(pub *mut ffi::simstruct);

impl MutSimptr {
    pub fn is_null(&self) -> bool {
        self.0.is_null()
    }
}

impl Drop for MutSimptr {
    // cleanup
    fn drop(&mut self) {
        unsafe { smolFreeSim(self.0) };
    }
}
