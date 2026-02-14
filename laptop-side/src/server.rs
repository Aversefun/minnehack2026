//! Server-side code
use dioxus::prelude::*;
use dioxus_fullstack::WebSocketOptions;
use dioxus_fullstack::Websocket;
use dioxus_fullstack::JsonEncoding;

#[get("/api/phone_ws/:lobby")]
async fn phone_ws(lobby: String, options: WebSocketOptions) -> Result<Websocket<String, String, JsonEncoding>> {
    Ok(options.on_upgrade(move |mut socket| async move {
        // send back a greeting message
        _ = socket
            .send("Hello!".to_string())
            .await;

        // Loop and echo back uppercase messages
        while let Ok(msg) = socket.recv().await {
            _ = socket.send("hey!".to_string()).await;
        }
    }))
}
