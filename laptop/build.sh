#!/bin/bash

cargo build --target=wasm32-unknown-unknown --debug
wasm-bindgen --out-dir ./out/ --target web target/wasm32-unknown-unknown/debug/laptop.wasm 