//! Game mechanics

use std::time::Duration;

use bevy::prelude::*;

////////////////////////////////////////////////////////////////////////////////////
// Resources & Components
////////////////////////////////////////////////////////////////////////////////////

#[derive(Resource)]
pub struct GameStats {
    pub score: u32,
    pub level_timer: Timer, // Use Bevy's built-in Timer
    pub is_paused: bool,
}

impl Default for GameStats {
    fn default() -> Self {
        Self {
            score: 0,
            level_timer: Timer::from_seconds(60.0, TimerMode::Once),
            is_paused: false,
        }
    }
}

// pub struct GameState {
//     pub camera_pos: Vector3,
//     pub player_pos: Vector3,
//     pub rotation: Vector2,
//     pub score: u32,
//     pub level_timer: Duration,
//     pub is_paused: bool,
// }

// impl GameState {
//     pub fn new() -> Self {
//         Self {
//             camera_pos: Vector3::zero(),
//             player_pos: Vector3::zero(),
//             rotation: Vector2::zero(),
//             score: 0,
//             level_timer: Duration::ZERO,
//             is_paused: false,
//         }
//     }
//     pub fn update(&mut self, handle: &mut raylib::RaylibHandle, thread:
// &raylib::RaylibThread) {}     pub fn process_inputs(&mut self, input_device:
// &InputDevice) {} }

// #[derive(Component)]
// struct FlightPhysics {
//     airspeed: f32,
//     lift_coefficient: f32,
// }

// struct Player {
//     pub position: Vector3,
//     pub velocity: Vector3,
//     pub orientation: Quaternion, //Avoid gimbal lock.
//     pub throttle: f32,
//     model: Model,
// }

#[derive(Component)]
struct Map {}

#[derive(Component)]
struct Health {
    current: f32,
    max: f32,
}

#[derive(Resource, Default)]
pub struct InputDevice {
    pub roll: f32,
    pub pitch: f32,
    pub yaw: f32,
    pub timestamp: f64,
    pub firing: bool,
}

////////////////////////////////////////////////////////////////////////////////////
// Entities
////////////////////////////////////////////////////////////////////////////////////

#[derive(Component)]
struct MainCamera;

#[derive(Component)]
struct Player;

fn spawn_player(
    mut commands: &mut Commands,
    asset_server: &Res<AssetServer>,
    mut materials: &mut ResMut<Assets<StandardMaterial>>,
) {
    let mesh_handle: Handle<Mesh> = asset_server.load("plane.obj");
    let texture_handle = asset_server.load("texture.png");

    let material_handle = materials.add(StandardMaterial {
        base_color_texture: Some(texture_handle),
        ..default()
    });

    // 2. Spawn a single entity with all required components
    commands.spawn((
        Player,
        Health {
            current: 100.0,
            max: 100.0,
        },
        // Rendering components
        Mesh3d(mesh_handle),
        MeshMaterial3d(material_handle),
        // Spatial components
        Transform {
            translation: Vec3::new(0.0, 100.0, 0.0),
            rotation: Quat::from_rotation_x(0.5),
            ..default()
        },
        // GlobalTransform::default(),
        // Visibility::default(),
        // InheritedVisibility::default(),
    ));
}

////////////////////////////////////////////////////////////////////////////////////
// Systems
////////////////////////////////////////////////////////////////////////////////////

fn move_player_fixed(mut query: Query<&mut Transform, With<Player>>) {
    for mut transform in &mut query {
        // Retrieve the forward vector based on current rotation
        let forward = transform.forward();

        // Move by a fixed constant (e.g., 0.1 meters per tick)
        // No delta_time multiplication is required for consistency here
        transform.translation += forward * 0.1;
    }
}

fn spawn_default_scene(commands: &mut Commands, asset_server: &mut Res<AssetServer>) {
    commands.spawn((
        Camera3d::default(),
        Transform::from_xyz(0.0, 10.0, 20.0).looking_at(Vec3::ZERO, Vec3::Y),
        MainCamera,
    ));
}

////////////////////////////////////////////////////////////////////////////////////
// Main Callbacks
////////////////////////////////////////////////////////////////////////////////////

/// set up a simple 3D scene
pub fn setup(
    mut commands: Commands,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
    asset_server: Res<AssetServer>,
) {
    spawn_default_scene(&mut commands, &mut asset_server);
    spawn_player(&mut commands, &mut asset_server, &mut materials);
    // circular base
    commands.spawn((
        Mesh3d(meshes.add(Circle::new(4.0))),
        MeshMaterial3d(materials.add(Color::WHITE)),
        Transform::from_rotation(Quat::from_rotation_x(-std::f32::consts::FRAC_PI_2)),
    ));
    // cube
    commands.spawn((
        Mesh3d(meshes.add(Cuboid::new(1.0, 1.0, 1.0))),
        MeshMaterial3d(materials.add(Color::srgb_u8(124, 144, 255))),
        Transform::from_xyz(0.0, 0.5, 0.0),
    ));
    // light
    commands.spawn((
        PointLight {
            shadows_enabled: true,
            ..default()
        },
        Transform::from_xyz(4.0, 8.0, 4.0),
    ));
    // // camera
    // commands.spawn((
    //     Camera3d::default(),
    //     Transform::from_xyz(-2.5, 4.5, 9.0).looking_at(Vec3::ZERO, Vec3::Y),
    // ));
}

//update everything based on the current frame
pub fn update() {
    // updateInputs();
    // updateGame();
    // draw();
}

//update everything on a fixed timescale
pub fn update_fixed() {
    move_player_fixed();
}
