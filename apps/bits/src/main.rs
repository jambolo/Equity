//! Decodes a Bitcoin compact-target value into its hash and difficulty.
//!
//! Usage: `bits <value in decimal>`

use equity::target::Target;
use std::process::ExitCode;

fn syntax(prog: &str) {
    eprintln!("syntax: {prog} <value in decimal>");
}

fn main() -> ExitCode {
    let args: Vec<String> = std::env::args().collect();
    let prog = args.first().map(String::as_str).unwrap_or("bits");

    if args.len() < 2 {
        syntax(prog);
        return ExitCode::from(1);
    }

    let bits: u32 = match args[1].parse() {
        Ok(v) => v,
        Err(_) => return ExitCode::from(2),
    };

    let t = Target::from_compact(bits);
    println!("Bits = {bits} (0x{bits:08x})");
    println!("Hash = {}", hex::encode(t.hash()));
    println!("Difficulty = {:.0}", t.difficulty());
    ExitCode::SUCCESS
}
