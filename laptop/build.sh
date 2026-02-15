#!/bin/bash
set -euo pipefail

cargo build --target=wasm32-unknown-unknown
wasm-bindgen --out-dir ./out/ --target web target/wasm32-unknown-unknown/debug/laptop.wasm 