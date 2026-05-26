//! Decodes a raw Bitcoin transaction hex string and prints its fields as pretty-printed JSON.
//!
//! Usage: `view-transactions <hex>`

use equity::transaction::Transaction;
use serde_json::{Value, json};
use std::process::ExitCode;

fn syntax(prog: &str) {
    eprintln!("syntax: {prog} <hash>");
}

fn build_json(tx: &Transaction) -> Value {
    let inputs: Vec<Value> = tx
        .inputs()
        .iter()
        .map(|i| {
            json!({
                "txid": i.txid.to_hex_be(),
                "outputIndex": i.output_index,
                "script": hex::encode(&i.script),
                "sequence": i.sequence,
            })
        })
        .collect();

    let outputs: Vec<Value> = tx
        .outputs()
        .iter()
        .map(|o| {
            json!({
                "value": o.value as f64,
                "script": hex::encode(&o.script),
            })
        })
        .collect();

    json!({
        "version": tx.version(),
        "inputs": inputs,
        "outputs": outputs,
        "locktime": tx.lock_time(),
    })
}

fn dump_pretty(value: &Value) -> String {
    use serde::Serialize;
    let mut buf = Vec::new();
    let formatter = serde_json::ser::PrettyFormatter::with_indent(b"    ");
    let mut ser = serde_json::Serializer::with_formatter(&mut buf, formatter);
    value.serialize(&mut ser).expect("serialize");
    String::from_utf8(buf).expect("utf8")
}

fn main() -> ExitCode {
    let args: Vec<String> = std::env::args().collect();
    let prog = args.first().map(String::as_str).unwrap_or("view-transactions");

    if args.len() < 2 {
        syntax(prog);
        return ExitCode::from(1);
    }

    let data = match hex::decode(&args[1]) {
        Ok(v) => v,
        Err(_) => {
            eprintln!("Invalid transaction.");
            return ExitCode::from(2);
        }
    };

    let tx = match Transaction::from_data(&data) {
        Ok(t) => t,
        Err(_) => {
            eprintln!("Invalid transaction.");
            return ExitCode::from(2);
        }
    };

    println!("{}", dump_pretty(&build_json(&tx)));
    ExitCode::SUCCESS
}
