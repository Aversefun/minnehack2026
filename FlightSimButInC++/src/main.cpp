#include <cmath>
#include <cstdio>
#include <variant>
#define WSPP_USE_OPENSSL
#include "wspp.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <raylib.h>
#include <raymath.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("USAGE: %s [lobby code]\n", argv[0]);
    return 1;
  }

  InitWindow(1024, 1024, "MinneFlight");
  SetTargetFPS(60);

  Camera3D camera = {};
  camera.position = (Vector3){-10.0f, 5.0f, 0.0f};
  camera.target = (Vector3){0.0f, 0.0f, 0.0f};
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 45.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  Model plane_model = LoadModel("../resources/PUSHILIN_Plane.obj");
  Texture2D texture = LoadTexture("../resources/PUSHILIN_PLANE.png");
  plane_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

  wspp::ws_client c;
  c.connect("ws://127.0.0.1:3000/api/laptop_ws/" + std::string(argv[1]));

  float x = 0.0;
  float target_x = 0.0;

  float z = 0.0;
  float target_z = 0.0;

  std::vector<Vector3> ring_positions;
  for (size_t i = 0; i < 10; i++) {
    ring_positions.push_back({0, 0, static_cast<float>(5.0 + (i * 1.0))});
  }

  c.on_tick([&](std::optional<wspp::message_view> msg) {
    if (msg.has_value()) {
      nlohmann::json data = nlohmann::json::parse(msg->text());

      if (data["type"] == "gyro_update") {
        x += (float)data["y"];
        z += (float)data["x"];
      }
    }

    x = x * 0.9;
    target_x = x * 0.1 + target_x * 0.9;

    z = z * 0.9;
    target_z = z * 0.1 + target_z * 0.9;

    BeginDrawing();

    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    DrawGrid(10, 1.0f);

    Vector3 euler_rot{target_x * 90, 0, target_z * 90};
    Vector3 normalized = Vector3Normalize(euler_rot);
    float scale = Vector3Length(euler_rot);

    float up = std::sin((target_z * 90) * DEG2RAD) * 1;
    float horizontal = std::sin((target_x * 90) * DEG2RAD) * 1;

    DrawModelEx(plane_model, {0, 0, 0}, normalized, scale, {1, 1, 1}, WHITE);
    DrawCube({1, up, horizontal}, 0.1, 0.1, 0.1, RED);

    EndMode3D();

    DrawFPS(10, 10);

    EndDrawing();
  });

  c.on_close([](auto) { std::cout << "ws closed\n"; });

  c.run();
}
