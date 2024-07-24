fn main() {
    let dst = cmake::build("../");
    println!("cargo:rustc-link-search=native={}/build", dst.display());
    println!("cargo:rustc-link-lib=static=smoldyn_static");
}
