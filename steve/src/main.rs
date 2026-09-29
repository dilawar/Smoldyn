use autocxx::prelude::*;

include_cpp! {
    safety!(unsafe)
    generate!("DeepThought")
}

fn main() {
    println!("main..");
}
