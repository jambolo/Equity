use clap::Parser;

#[derive(Parser)]
#[command(name = "view-transactions")]
#[command(about = "A tool for viewing equity transactions")]
struct Cli {
    #[arg(short, long)]
    transaction_id: Option<String>,
    
    #[arg(short, long)]
    format: Option<String>,
}

fn main() -> anyhow::Result<()> {
    let cli = Cli::parse();
    
    println!("Viewing transactions...");
    if let Some(id) = cli.transaction_id {
        println!("Transaction ID: {}", id);
    }
    
    Ok(())
}
