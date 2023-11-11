// build.rs
use std::io::Write;

fn main() -> anyhow::Result<()> {
    const LIBSTEVE_SRC: &str = "source/libSteve";
    const SMOLDYN_SRC: &str = "source/Smoldyn";

    let mut src_files = vec![];
    for src_file in glob::glob(&format!("{LIBSTEVE_SRC}/*.c"))?
        .into_iter()
        .flatten()
    {
        println!("cargo:rerun-if-changed={}", src_file.display());
        src_files.push(src_file);
    }

    for src_file in glob::glob(&format!("{SMOLDYN_SRC}/*.c"))?
        .into_iter()
        .flatten()
    {
        println!("cargo:rerun-if-changed={}", src_file.display());
        src_files.push(src_file);
    }

    assert!(src_files.len() > 1);
    println!("Total {} source files found", src_files.len());

    // write smoldynconfigure.h file
    let mut f_smol_config = std::fs::File::create("smoldynconfigure.h")?;
    const VERSION: &str = env!("CARGO_PKG_VERSION");
    f_smol_config.write(format!("#define VERSION \"{VERSION}\"").as_bytes())?;

    let mut build = cxx_build::bridge("src/main.rs");
    // returns a cc::Build
    build
        .flag("-std=c++14")
        .flag_if_supported("-std=c++17")
        .cpp(true)
        // .flag_if_supported("-Wall")
        .files(src_files)
        .define("BNG2_PATH", bng2_path().as_deref())
        .define("OPTION_VCELL", "ON")
        .include(&format!("{LIBSTEVE_SRC}"))
        .include(&format!("{SMOLDYN_SRC}"))
        .include(".");
    build.compile("smoldyn");

    Ok(())
}

/// Return bng2 path (post installation)
fn bng2_path() -> Option<String> {
    Some("\"/usr/local/bin/BioNetGen/bng2.pl\"".to_string())
}
