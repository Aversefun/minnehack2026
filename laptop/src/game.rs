//! Game mechanics

use std::time::Duration;

use bevy::prelude::*;

pub struct GameState {
    pub camera_pos: Vector3,
    pub player_pos: Vector3,
    pub rotation: Vector2,
    pub score: u32,
    pub level_timer: Duration,
    pub is_paused: bool,
}

impl GameState {
    pub fn new() -> Self {
        Self {
            camera_pos: Vector3::zero(),
            player_pos: Vector3::zero(),
            rotation: Vector2::zero(),
            score: 0,
            level_timer: Duration::ZERO,
            is_paused: false,
        }
    }
    pub fn update(&mut self, handle: &mut raylib::RaylibHandle, thread: &raylib::RaylibThread) {}
    pub fn process_inputs(&mut self, input_device: &InputDevice) {}
}

pub struct InputDevice {
    pub x: f32,
    pub y: f32,
    pub z: f32,
    pub timestamp: f64,
}

struct Player {
    pub position: Vector3,
    pub velocity: Vector3,
    pub orientation: Quaternion, //Avoid gimbal lock.
    pub throttle: f32,
    model: Model,
}

struct Map {}
