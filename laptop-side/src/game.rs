//! Game mechanics

use raylib::prelude::*;

pub struct GameState {
    pub camera_pos: Vector3,
    pub player_pos: Vector3,
    // pub points:
    pub rotation: Vector2,
    pub paused: bool,
}

impl GameState {
    pub fn new() -> Self {
        Self {
            camera_pos: Vector3::zero(),
            player_pos: Vector3::zero(),
            rotation: Vector2::zero(),
            paused: false,
        }
    }
    pub fn update(&mut self, handle: &mut raylib::RaylibHandle, thread: &raylib::RaylibThread){
        
    }   
    pub fn process_inputs(&mut self, input_device: &InputDevice){

    }
}

pub struct InputDevice {
    pub x: f32, // Radians per second
    pub y: f32,
    pub z: f32,
    pub timestamp: f64,
}

struct Player{

}

struct Enemies{

}

struct Map {

}

//Rad/s

pub fn updateLogic(){

}



