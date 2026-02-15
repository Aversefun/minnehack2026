#include <cmath>
#include <cstdio>
#include <variant>
#include <vector>
#define WSPP_USE_OPENSSL
#include "wspp.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

void handleInputs(float &x, float &y, float &z,
                  float sensitivity = 0.5f) { // bbbbbroken!
  // Quaternion q = QuaternionFromEuler(target_z * 45.0f * DEG2RAD, 0,
  //                                    target_x * 45.0f * DEG2RAD);
  // Emulate y-axis gyro (horizontal movement/roll)
  if (IsKeyDown(KEY_RIGHT))
    x += sensitivity;
  if (IsKeyDown(KEY_LEFT))
    x -= sensitivity;

  // Emulate x-axis gyro (vertical movement/pitch)
  if (IsKeyDown(KEY_UP))
    z -= sensitivity;
  if (IsKeyDown(KEY_DOWN))
    z += sensitivity;
}

class Entity {
public:
  Vector3 position;
  float rotation;
  bool active;

  Color color = WHITE;

  Entity(Vector3 pos) : position(pos), rotation(0.0f), active(true) {}

  void Update(Vector3 velocity) {
    if (!active)
      return;

    rotation += 2.0f;
    if (rotation >= 360.0f)
      rotation -= 360.0f;

    // Move relative to the plane's apparent motion
    position = Vector3Subtract(position, velocity);
  }

  void draw(Model &model) {
    if (!active)
      return;
    // Draw centered at position, rotating around the Y-axis
    DrawModelEx(model, position, {0, 1, 0}, rotation, {1, 1, 1}, color);
  }

  void draw() {
    if (model == nullptr || !active)
      return;
    // Draw centered at position, rotating around the Y-axis
    DrawModelEx(*model, position, {0, 1, 0}, rotation, {1, 1, 1}, color);
  }

  void setModel(Model &model) { this->model = &model; }

private:
  Model *model = nullptr;
};

int main(int argc, char *argv[]) {

  if (argc != 2) {
    printf("USAGE: %s [lobby code]\n", argv[0]);
    return 1;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Window Initialization
  /////////////////////////////////////////////////////////////////////////////

  InitWindow(1024, 1024, "MinneFlight");
  SetTargetFPS(60);

  /////////////////////////////////////////////////////////////////////////////
  // World Initialization
  /////////////////////////////////////////////////////////////////////////////
  Camera3D camera = {};
  camera.position = (Vector3){-10.0f, 1.0f, 0.0f};
  camera.target = (Vector3){0.0f, 0.0f, 0.0f};
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 45.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  Model gasCan, truck, propeller, collectibleGear, collectibleBoard,
      minneapolisModel;
  gasCan = LoadModel("../assets/GasCan.glb");
  truck = LoadModel("../assets/truck.glb");
  propeller = LoadModel("../assets/Propeller.glb");
  collectibleGear = LoadModel("../assets/CollectibleGear.glb");
  collectibleBoard = LoadModel("../assets/CollectibleBoard.glb");
  minneapolisModel = LoadModel("../assets/minneapolis.stl");

  Model plane_model = LoadModel("../resources/PUSHILIN_Plane.obj");
  Texture2D texture = LoadTexture("../resources/PUSHILIN_PLANE.png");
  plane_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

  Vector3 planePosition = {0.0f, 0.0f, 0.0f};
  Quaternion currentOrientation = {0.0f, 0.0f, 0.0f, 1.0f};

  float x = 0.0;
  float target_x = 0.0;

  float z = 0.0;
  float target_z = 0.0;

  float y = 0.0;
  float target_y = 0.0;

  Vector3 cityPosition = planePosition;
  {
    float altitude = 50.0f;
    Vector3 cityPosition = planePosition;
    cityPosition.y -= altitude;
  }

  Entity minneapolis(cityPosition);
  minneapolis.setModel(minneapolisModel);
  minneapolis.color = BLUE;

  // Skybox code
  Mesh sphere = GenMeshSphere(500.0f, 32, 32);
  Model sky = LoadModelFromMesh(sphere);
  Texture2D skyboxTex = LoadTexture("../assets/skybox.jpg");
  // Texture2D skyboxPanorama = LoadTexture("resources/skybox.hdr");
  sky.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = skyboxTex;
  sky.materials[0].shader = LoadShader(0, 0);

  /////////////////////////////////////////////////////////////////////////////
  // Thread client Initialization
  /////////////////////////////////////////////////////////////////////////////
  wspp::ws_client c;
  c.connect("ws://127.0.0.1:3000/api/laptop_ws/" + std::string(argv[1]));

  std::vector<Vector3> ring_positions;
  for (size_t i = 0; i < 100; i++) {
    ring_positions.push_back({40.0f + (i * 20.0f), 0, 0});
  }

  std::vector<Entity> collectibles;
  for (size_t i = 0; i < 100; i++) {
    collectibles.emplace_back(Vector3{40.0f + (i * 20.0f), 0, 0});
    // collectibles.push_back(new Collectible({40.0f + (i * 20.0f), 0, 0}));
  }

  c.on_tick([&](std::optional<wspp::message_view> msg) {
    float dt = 1.0f / 60.0f; // Assume we are running 60 fps
    if (msg.has_value()) {
      nlohmann::json data = nlohmann::json::parse(msg->text());

      if (data["type"] == "gyro_update") {
        x += (float)data["x"];
        y += (float)data["y"];
        z += (float)data["z"];
      }
      // x += (float)data["y"];
      // z += (float)data["x"];
    }

    x = x * 0.9;
    target_x = x * 0.1 + target_x * 0.9;

    z = z * 0.9;
    target_z = z * 0.1 + target_z * 0.9;

    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode3D(camera);

    rlDisableBackfaceCulling();
    rlDisableDepthMask();
    DrawModel(sky, camera.position, 1.0f, WHITE);
    rlEnableDepthMask();
    rlEnableBackfaceCulling();

    Vector3 euler_rot{target_x * 90, 0, target_z * 90};
    Vector3 normalized = Vector3Normalize(euler_rot);
    float scale = Vector3Length(euler_rot);

    float up = std::sin((target_z * 90) * DEG2RAD) * 1;
    float horizontal = std::sin((target_x * 90) * DEG2RAD) * 1;

    DrawModelEx(plane_model, {0, 0, 0}, normalized, scale, {1, 1, 1}, WHITE);

    // for (size_t i = 0; i < 100; i++) {
    //   DrawModel(collectibleGear, ring_positions[i], 1.0f, WHITE);
    //   ring_positions[i] -= Vector3{1.0, up, horizontal} * 0.1;
    // }

    Vector3 planeVelocity = Vector3Scale(Vector3{1.0f, up, horizontal}, 0.1f);

    for (size_t i = 0; i < 100; i++) {
      // DrawModel(collectibleGear, ring_positions[i], 1.0f, WHITE);
      collectibles[i].Update(planeVelocity);
      collectibles[i].draw(collectibleGear);
    }

    minneapolis.Update(planeVelocity);
    minneapolis.draw(); // doesn't draw anything idk.

    EndMode3D();

    DrawFPS(10, 10);

    handleInputs(x, y, y);

    EndDrawing();
    if (WindowShouldClose()) {
      c.close();
    }
  });

  c.on_close([](auto) { std::cout << "ws closed\n"; });

  c.run();
}
