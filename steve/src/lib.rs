use std::ffi::CStr;
use libc::c_char;

#[no_mangle]
pub extern "C" fn strisnumber(ptxt: *const c_char) -> i32 {
    let txt = unsafe { CStr::from_ptr(ptxt) };
    txt.to_string_lossy().parse::<f64>().is_ok().into()
}

#[no_mangle]
pub extern "C" fn strhasname(ptxt: *const c_char, pname: *const c_char) -> i32 {
    let txt = unsafe { CStr::from_ptr(ptxt) };
    let name = unsafe { CStr::from_ptr(pname) };
    txt.to_string_lossy()
        .as_ref()
        .contains(name.to_string_lossy().as_ref())
        .into()
}
