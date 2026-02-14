//! laptop code

use std::time::Duration;

use dioxus::prelude::*;
use web_sys::wasm_bindgen::prelude::*;

#[derive(Debug, Clone, Routable, PartialEq)]
#[rustfmt::skip]
enum Route {
    #[layout(Navbar)]
    #[route("/")]
    Home {},
    #[route("/blog/:id")]
    Blog { id: i32 },
}

const FAVICON: Asset = asset!("/assets/favicon.ico");
const MAIN_CSS: Asset = asset!("/assets/main.css");
const TAILWIND_CSS: Asset = asset!("/assets/tailwind.css");

fn main() {
    dioxus::launch(App);
}

#[component]
fn App() -> Element {
    rsx! {
        document::Link { rel: "icon", href: FAVICON }
        document::Link { rel: "stylesheet", href: MAIN_CSS }
        document::Link { rel: "stylesheet", href: TAILWIND_CSS }
        Router::<Route> {}
    }
}

/// Home page
#[component]
fn Home() -> Element {
    rsx! {
        // Echo {}
        Raylib {}
    }
}

/// Raylib rendering code
#[component]
fn Raylib() -> Element {
    #[cfg(feature = "web")]
    use_future(move || async move {
        start_render_loop();
    });

    rsx! {
        canvas { id: "canvas", width: 1920, height: 1080 }
    }
}

#[cfg(feature = "web")]
fn start_render_loop() {
    let raylib_ctx = raylib::init()
        .title("minnehack yayy")
        .width(1920)
        .height(1080)
        .build();

    render_loop(raylib_ctx);
}

#[cfg(feature = "web")]
fn render_loop(
    // context: web_sys::CanvasRenderingContext2d,
    mut raylib: (raylib::RaylibHandle, raylib::RaylibThread),
) {
    raylib.0.draw(&raylib.1, |mut handle| {
        use raylib::{color::Color, prelude::RaylibDraw};

        handle.draw_circle(0, 0, 20.0, Color::new(0xFF, 0, 0, 0xFF));
    });

    let closure = Closure::once(move || render_loop(raylib));
    web_sys::window()
        .expect("global window does not exists")
        .request_animation_frame(closure.as_ref().unchecked_ref())
        .unwrap();
}

/// Blog page
#[component]
pub fn Blog(id: i32) -> Element {
    rsx! {
        div { id: "blog",

            // Content
            h1 { "This is blog #{id}!" }
            p {
                "In blog #{id}, we show how the Dioxus router works and how URL parameters can be passed as props to our route components."
            }

            // Navigation links
            Link { to: Route::Blog { id: id - 1 }, "Previous" }
            span { " <---> " }
            Link { to: Route::Blog { id: id + 1 }, "Next" }
        }
    }
}

/// Shared navbar component.
#[component]
fn Navbar() -> Element {
    rsx! {
        div { id: "navbar",
            Link { to: Route::Home {}, "Home" }
            Link { to: Route::Blog { id: 1 }, "Blog" }
        }

        Outlet::<Route> {}
    }
}

/// Echo component that demonstrates fullstack server functions.
#[component]
fn Echo() -> Element {
    let mut response = use_signal(|| String::new());

    rsx! {
        div { id: "echo",
            h4 { "ServerFn Echo" }
            input {
                placeholder: "Type here to echo...",
                oninput: move |event| async move {},
            }

            if !response().is_empty() {
                p {
                    "Server echoed: "
                    i { "{response}" }
                }
            }
        }
    }
}

mod game;
mod server;
