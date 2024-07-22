use libc::c_char;
use std::ffi::CStr;

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

/// Returns 1 if name is a valid identifier.
#[no_mangle]
pub extern "C" fn strokname(pname: *const c_char) -> i32 {
    let name = unsafe { CStr::from_ptr(pname) };
    syn::parse_str::<syn::Ident>(name.to_str().unwrap_or(""))
        .is_ok()
        .into()
}

/// Returns 1 if `plong` begins with `pshort`
#[no_mangle]
pub extern "C" fn strbegin(pshort: *const c_char, plong: *const c_char, casesensitive: i32) -> i32 {
    let short = unsafe { CStr::from_ptr(pshort) };
    let long = unsafe { CStr::from_ptr(plong) };
    if let Ok(short) = short.to_str() {
        if let Ok(long) = long.to_str() {
            if casesensitive != 0 {
                return long
                    .to_lowercase()
                    .starts_with(&short.to_lowercase())
                    .into();
            } else {
                return long.starts_with(short).into();
            };
        }
    }
    0
}

/// Count character in given string.
#[no_mangle]
pub extern "C" fn strsymbolcount(s: *const c_char, c: c_char) -> i32 {
    let s = unsafe { CStr::from_ptr(s) };
    s.to_bytes().iter().filter(|x| **x == c as u8).count() as u32 as i32
}
