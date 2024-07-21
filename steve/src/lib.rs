use std::ffi::CStr;

#[no_mangle]
pub extern "C" fn strisnumber(txt: &CStr) -> i32 {
    txt.to_str().map(|x| x.parse::<f64>()).is_ok().into()
}
