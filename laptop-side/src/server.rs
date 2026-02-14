//! Server-side code
use dioxus::prelude::*;
use dioxus_fullstack::WebSocketOptions;
use dioxus_fullstack::Websocket;

#[get("/api/phone_ws/:lobby")]
async fn phone_ws(options: WebSocketOptions, lobby: String) -> Result<Websocket> {
    Ok(options.on_upgrade(move |mut socket| async move {
        while let Ok(msg) = socket.recv().await {
            _ = socket.send(msg).await;
        }
    }))
}
