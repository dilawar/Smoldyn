use std::ffi::CStr;

#[no_mangle]
pub extern "C" fn strisnumber(txt: &CStr) -> i32 {
    txt.to_string_lossy().parse::<f64>().is_ok().into()
}

#[no_mangle]
pub extern "C" fn strhasname(txt: &CStr, name: &CStr) -> i32 {
    txt.to_string_lossy()
        .as_ref()
        .contains(name.to_string_lossy().as_ref())
        .into()
}
