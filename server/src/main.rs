//! Server-side code.

use std::{collections::HashMap, sync::Arc};

use axum::{
    Router,
    extract::{
        Path,
        ws::{Message, WebSocket, WebSocketUpgrade},
    },
    routing::any,
};
use futures_util::{SinkExt, StreamExt, stream::SplitSink};
use tokio::sync::RwLock;

#[tokio::main]
async fn main() {
    let lobbies_real: Arc<
        RwLock<
            HashMap<
                String,
                (
                    Option<SplitSink<WebSocket, Message>>,
                    Option<SplitSink<WebSocket, Message>>,
                ),
            >,
        >,
    > = Arc::new(RwLock::new(HashMap::new()));

    let lobbies = lobbies_real.clone();

    let app = Router::new()
        .route(
            "/api/phone_ws/{lobby}",
            any(
                move |ws: WebSocketUpgrade, Path(lobby): Path<String>| async move {
                    ws.on_upgrade(async move |socket| {
                        let (sender, mut receiver) = socket.split();

                        match lobbies.write().await.entry(lobby.clone()) {
                            std::collections::hash_map::Entry::Occupied(mut entry) => {
                                entry.get_mut().0 = Some(sender)
                            }
                            std::collections::hash_map::Entry::Vacant(entry) => {
                                _ = entry.insert((Some(sender), None))
                            }
                        }
                        tokio::spawn(async move {
                            let lobby = lobby;
                            while let Some(msg) = receiver.next().await {
                                if let Ok(msg) = msg {
<<<<<<< HEAD
                                    print!("{}\n", msg.to_text().unwrap());
=======
>>>>>>> origin/main
                                    let mut lobbies = lobbies.write().await;
                                    let sink = lobbies.get_mut(&lobby).unwrap().1.as_mut();
                                    if let Some(sink) = sink {
                                        sink.send(msg).await.unwrap();
                                    }
                                } else {
                                    break;
                                };
                            }
                            let mut lobbies = lobbies.write().await;
                            if lobbies.get(&lobby).unwrap().1.is_none() {
                                lobbies.remove(&lobby);
                            } else {
                                lobbies.get_mut(&lobby).unwrap().0.take();
                            }
                        });
                    })
                },
            ),
        )
        .route(
            "/api/laptop_ws/{lobby}",
            any(
                move |ws: WebSocketUpgrade, Path(lobby): Path<String>| async move {
                    let lobbies = lobbies_real;
                    ws.on_upgrade(async move |socket| {
                        let (sender, mut receiver) = socket.split();

                        match lobbies.write().await.entry(lobby.clone()) {
                            std::collections::hash_map::Entry::Occupied(mut entry) => {
                                entry.get_mut().1 = Some(sender)
                            }
                            std::collections::hash_map::Entry::Vacant(entry) => {
                                _ = entry.insert((None, Some(sender)))
                            }
                        }
                        tokio::spawn(async move {
                            let lobby = lobby;
                            while let Some(msg) = receiver.next().await {
                                if let Ok(msg) = msg {
                                    let mut lobbies = lobbies.write().await;
                                    let sink = lobbies.get_mut(&lobby).unwrap().0.as_mut();
                                    if let Some(sink) = sink {
                                        sink.send(msg).await.unwrap();
                                    }
                                } else {
                                    break;
                                };
                            }
                            let mut lobbies = lobbies.write().await;
                            if lobbies.get(&lobby).unwrap().0.is_none() {
                                lobbies.remove(&lobby);
                            } else {
                                lobbies.get_mut(&lobby).unwrap().1.take();
                            }
                        });
                    })
                },
            ),
        );

    let listener = tokio::net::TcpListener::bind("0.0.0.0:3000").await.unwrap();
    axum::serve(listener, app).await.unwrap();
}
