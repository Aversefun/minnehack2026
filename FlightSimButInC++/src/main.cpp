#include <cstdio>
#include <variant>
#define WSPP_USE_OPENSSL
#include "wspp.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <raylib.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("USAGE: %s [lobby code]\n", argv[0]);
    return 1;
  }

  InitWindow(1024, 1024, "MinneFlight");
  SetTargetFPS(60);

  Camera3D camera = {};
  camera.position = (Vector3){0.0f, 10.0f, 10.0f};
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

  c.on_tick([&](std::optional<wspp::message_view> msg) {
    if (msg.has_value()) {
      nlohmann::json data = nlohmann::json::parse(msg->text());

      if (data["type"] == "gyro_update") {
        x += (float)data["x"];
      }
    }

    BeginDrawing();

    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    DrawGrid(10, 1.0f);

    DrawCube({x, 0, 0}, 1, 1, 1, RED);

    DrawModel(plane_model, {0, 0, 0}, 1.0f, WHITE);

    EndMode3D();

    DrawFPS(10, 10);

    EndDrawing();
  });

  c.on_close([](auto) { std::cout << "ws closed\n"; });

  c.run();
}
