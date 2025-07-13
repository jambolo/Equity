use clap::Parser;

#[derive(Parser)]
#[command(name = "bits")]
#[command(about = "A tool for working with bits")]
struct Cli {
    #[arg(short, long)]
    verbose: bool,
}

fn main() -> anyhow::Result<()> {
    let cli = Cli::parse();
    
    if cli.verbose {
        println!("Bits application starting...");
    }
    
    println!("Hello from bits!");
    Ok(())
}
