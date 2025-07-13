use clap::Parser;

#[derive(Parser)]
#[command(name = "list-prefixes")]
#[command(about = "A tool for listing network prefixes")]
struct Cli {
    #[arg(short, long)]
    format: Option<String>,
}

fn main() -> anyhow::Result<()> {
    let cli = Cli::parse();
    
    println!("Listing prefixes...");
    if let Some(format) = cli.format {
        println!("Using format: {}", format);
    }
    
    Ok(())
}
