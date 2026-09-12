use cmake::Config;

fn main() {
    // build libsmodlyn_static.a
    let dst = Config::new("..")
        .define("OPTION_PYTHON", "OFF")
        .define("OPTION_STATIC", "ON")
        .define("OPTION_USE_OPENGL", "OFF")
        .define("OPTION_USE_LIBTIFF", "OFF")
        .build();

    println!("cargo:rustc-link-search=native={}/build", dst.display());
    println!("cargo:rustc-link-lib=static=smoldyn_static");

    // Transitive dependencies of the static Smoldyn library (OpenGL/GLUT).
    println!("cargo:rustc-link-lib=GL");
    println!("cargo:rustc-link-lib=GLU");
    println!("cargo:rustc-link-lib=glut");
    println!("cargo:rustc-link-lib=Xmu");
    println!("cargo:rustc-link-lib=Xi");

    cxx_build::bridge("src/lib.rs")
        .include("src")
        .include("../source")
        .include(format!("{}/build", dst.display()))
        .compile("simulation");
}
