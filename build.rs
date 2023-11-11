// build.rs
use std::io::Write;

fn main() -> anyhow::Result<()> {
    const LIBSTEVE_SRC: &str = "source/libSteve";

    let libsteve_files: Vec<_> = glob::glob(&format!("{LIBSTEVE_SRC}/*.c"))
        .unwrap()
        .into_iter()
        .flatten()
        .collect();

    assert!(libsteve_files.len() > 1);
    println!(
        "Total {} files found in source/libSteve",
        libsteve_files.len()
    );

    // write smoldynconfigure.h file
    let mut f_smol_config = std::fs::File::create("smoldynconfigure.h")?;
    const VERSION : &str = env!("CARGO_PKG_VERSION");
    f_smol_config.write(format!("#define VERSION \"{VERSION}\"").as_bytes())?; 


    cxx_build::bridge("src/main.rs") // returns a cc::Build
        .files(libsteve_files)
        .flag("-std=c++14")
        .flag_if_supported("-std=c++17")
        .flag_if_supported("-Wall")
        .include(&format!("{LIBSTEVE_SRC}"))
        .include("source/Smoldyn")
        .include(".")
        .compile("smoldyn");

    Ok(())
}
