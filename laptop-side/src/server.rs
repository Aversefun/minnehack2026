//! Server-side code
use dioxus::prelude::*;
use dioxus_fullstack::WebSocketOptions;
use dioxus_fullstack::Websocket;

#[get("/api/uppercase_ws")]
async fn uppercase_ws(options: WebSocketOptions) -> Result<Websocket> {
    Ok(options.on_upgrade(move |mut socket| async move {
        // send back a greeting message
        _ = socket
            .send("Hello!".to_string())
            .await;

        // Loop and echo back uppercase messages
        while let Ok(msg) = socket.recv().await {
            _ = socket.send(msg.to_ascii_uppercase()).await;
        }
    }))
}
