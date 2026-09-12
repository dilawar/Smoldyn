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
