use cmake::Config;

fn main() {
    // build libsmodlyn_static.a
    let dst = Config::new("..")
        .define("OPTION_PYTHON", "OFF")
        .define("OPTION_STATIC", "ON")
        .define("OPTION_USE_LIBTIFF", "OFF")
        .build();

    println!("cargo:rustc-link-search=native={}/build", dst.display());
    println!("cargo:rustc-link-lib=static=smoldyn_static");

    cxx_build::bridge("src/lib.rs")
        .file("../source/python/Simulation.cpp")
        .include("../source")
        .include(format!("{}/build", dst.display()))
        .compile("simulation");
}
