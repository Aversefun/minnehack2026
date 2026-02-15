//! Laptop-side code.

use bevy::prelude::*;
use bevy_obj::ObjPlugin;

mod game;

fn main() {
    App::new()
        .add_plugins(DefaultPlugins)
        .insert_resource(GameStats::default())
        .add_plugins(ObjPlugin)
        .add_systems(Startup, game::setup)
        .add_systems(Update, game::update)
        .add_systems(FixedUpdate, game::update_fixed)
        .run();
}
