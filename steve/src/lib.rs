#[no_mangle]
pub extern "C" fn hello_from_rust() -> i32 {
    println!("Rust said hi...");
    42
}
