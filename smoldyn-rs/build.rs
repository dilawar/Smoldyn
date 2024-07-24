fn main() {
    // build libsmodlyn_static.a
    let dst = cmake::build("../");
    println!("cargo:rustc-link-search=native={}/build", dst.display());
    println!("cargo:rustc-link-lib=static=smoldyn_static");

    cxx_build::bridge("src/lib.rs")
        .file("../source/python/Simulation.cpp")
        .compile("simulation");
}
