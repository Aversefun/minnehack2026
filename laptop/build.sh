#!/bin/bash

# Check if sccache is installed
if command -v sccache >/dev/null 2>&1; then
    echo "sccache found. Enabling compilation cache..."
    export RUSTC_WRAPPER=sccache
    # Ensure the background server is running
    sccache --start-server >/dev/null 2>&1
else
    echo "sccache not found. Proceeding with standard build..."
fi

cargo build --target=wasm32-unknown-unknown
wasm-bindgen --out-dir ./out/ --target web target/wasm32-unknown-unknown/debug/laptop.wasm 