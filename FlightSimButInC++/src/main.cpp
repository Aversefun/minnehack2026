#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <variant>
#define WSPP_USE_OPENSSL
#include "raygui.h"
#include "wspp.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <raylib.h>
#include <raymath.h>

int main(int argc, char *argv[]) {
  srand(time(NULL));
  auto rng = std::default_random_engine{};

  if (argc != 2) {
    printf("USAGE: %s [lobby code]\n", argv[0]);
    return 1;
  }

  InitWindow(1024, 1024, "MinneFlight");
  SetTargetFPS(60);

  Camera3D camera = {};
  camera.position = (Vector3){-10.0f, 1.0f, 0.0f};
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

  float health = 100;
  float progression = 0;

  float cool_down_time = 0;
  float show_time = 0;
  float fire_time = 0;
  float hurt_count_down = 0;
  float hurt_object = 0;
  float manuevering_speed = 0;
  float health_dec = 0;

  enum GunFireState {
    FIRING_COOL_DOWN,
    FIRING_SHOWN,
    FIRING_ACTIVE
  } fire_state = FIRING_COOL_DOWN;

  enum MenuState {
    MAIN_MENU,
    GAME_RUNNING,
    UPGRADE_SCREEN,
    GAME_OVER,
  } menu_state = MAIN_MENU;

  std::vector<Vector3> ring_positions;
  std::vector<Vector3> hurt_spheres;

  struct Upgrade {
    std::string name;
    enum UpgradeType {
      LESS_BARRAGES,
      FASTER_MANUEVERING,
      MORE_HEALTH,
      PROTECTION,
      INCREASE_SHOOTER_COOLDOWN,
      MAGNITISM
    } type;
    bool one_use = false;
    bool used = false;
  };

  std::vector<Upgrade> upgrades = {
      {"Less Barrages", Upgrade::LESS_BARRAGES},
      {"Faster Manuevering", Upgrade::FASTER_MANUEVERING},
      {"More Health", Upgrade::MORE_HEALTH},
      {"Protection", Upgrade::PROTECTION},
      {"Increase Barrage Cooldown", Upgrade::INCREASE_SHOOTER_COOLDOWN},
      {"Magnitisim", Upgrade::INCREASE_SHOOTER_COOLDOWN, true},
  };

  std::vector<Upgrade> choosable_upgrades = {};

  size_t frame_ticks = 0;

  c.on_tick([&](std::optional<wspp::message_view> msg) {
    frame_ticks++;
    if (WindowShouldClose()) {
      c.close();
    }
    if (msg.has_value()) {
      nlohmann::json data = nlohmann::json::parse(msg->text());

      if (data["type"] == "gyro_update") {
        x += (float)data["y"];
        z += (float)data["x"];
      }
      if (data["type"] == "button_down" &&
          (menu_state == MAIN_MENU || menu_state == GAME_OVER)) {
        // init game state
        menu_state = GAME_RUNNING;
        health = 100;

        ring_positions.clear();
        for (size_t i = 0; i < 100; i++) {
          ring_positions.push_back({40.0f + (i * 20.0f),
                                    (float)(std::rand() % 256) / 256 * 10 - 5,
                                    (float)(std::rand() % 256) / 256 * 10 - 5});
        }

        hurt_spheres.clear();

        health = 100;
        progression = 0;

        cool_down_time = 10;
        show_time = 5;
        fire_time = 5;
        hurt_count_down = cool_down_time;
        hurt_object = 1;
        manuevering_speed = 0.1;
        health_dec = 1;

        for (size_t i = 0; i < upgrades.size(); i++) {
          upgrades[i].used = false;
        }
        std::shuffle(upgrades.begin(), upgrades.end(), rng);

        fire_state = FIRING_COOL_DOWN;
      }
    }

    x = x * 0.9;
    target_x = x * 0.1 + target_x * 0.9;

    z = z * 0.9;
    target_z = z * 0.1 + target_z * 0.9;

    BeginDrawing();

    ClearBackground(Color{201, 209, 211, 255});

    BeginMode3D(camera);

    Vector3 euler_rot{target_x * 90, 0, target_z * 90};
    Vector3 normalized = Vector3Normalize(euler_rot);
    float scale = Vector3Length(euler_rot);

    float up = std::sin((target_z * 90) * DEG2RAD) * 1;
    float horizontal = std::sin((target_x * 90) * DEG2RAD) * 1;

    DrawModelEx(plane_model, {0, 0, 0}, normalized, scale, {1, 1, 1}, WHITE);

    if (menu_state == GAME_RUNNING) {
      hurt_count_down -= (float)1 / 60;
      if (hurt_count_down < 0) {
        switch (fire_state) {
        case FIRING_COOL_DOWN:
          fire_state = FIRING_SHOWN;
          hurt_count_down = show_time;

          for (size_t i = 0; i < hurt_object; i++) {
            hurt_spheres.push_back(
                {0,
                 static_cast<float>(((float)(std::rand() % 256) / 256 * 5) -
                                    2.5),
                 static_cast<float>(((float)(std::rand() % 256) / 256 * 5) -
                                    2.5)});
          }

          break;
        case FIRING_SHOWN:
          fire_state = FIRING_ACTIVE;
          hurt_count_down = fire_time;
          break;
        case FIRING_ACTIVE:
          fire_state = FIRING_COOL_DOWN;
          cool_down_time -= 1;
          cool_down_time = std::max(cool_down_time, 2.0f);
          hurt_count_down = cool_down_time;
          hurt_spheres.clear();
          hurt_object++;
          break;
        }
      }

      for (size_t i = 0; i < hurt_spheres.size(); i++) {
        hurt_spheres[i] -= Vector3{0, up, horizontal} * manuevering_speed;
        if (fire_state == FIRING_SHOWN && frame_ticks % 100 < 50) {
          DrawSphere(hurt_spheres[i], 1, Color{230, 41, 55, 100});
        } else if (fire_state == FIRING_ACTIVE) {
          DrawSphere(hurt_spheres[i], 1, Color{230, 41, 55, 255});
          if (Vector3Distance(Vector3Zero(), hurt_spheres[i]) < 1.0) {
            health -= health_dec;
          }
        }
      }

      for (size_t i = 0; i < ring_positions.size(); i++) {
        DrawCube(ring_positions[i], 1, 1, 1, RED);
        ring_positions[i] -= Vector3{1.0, up, horizontal} * manuevering_speed;

        if (Vector3Distance(Vector3Zero(), ring_positions[i]) < 1.0) {
          progression += 5;
        }
      }
    }

    EndMode3D();

    DrawFPS(10, 10);

    if (menu_state == GAME_RUNNING || menu_state == UPGRADE_SCREEN) {
      GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
      GuiProgressBar({1024 / 5, 1024 / 16, 1024 / 8 * 6, 30}, "Progression",
                     NULL, &progression, 0, 100);

      GuiProgressBar({1024 / 4, 1024 / 8 * 5, 1024 / 2, 20}, "Health", NULL,
                     &health, 0, 100);

      std::string firing_state_str;
      switch (fire_state) {
      case FIRING_COOL_DOWN:
        firing_state_str = "Next barrage in ";
        break;
      case FIRING_SHOWN:
        firing_state_str = "Firing shown for the next ";
        break;
      case FIRING_ACTIVE:
        firing_state_str = "Firing active for the next ";
        break;
      }

      DrawText(TextFormat("%s%.02f", firing_state_str.c_str(), hurt_count_down),
               1024 / 5, 1024 / 5 + 100, 40, BLACK);

      if (health <= 0) {
        menu_state = GAME_OVER;
      }

      if (progression > 100) {
        menu_state = UPGRADE_SCREEN;
        std::shuffle(upgrades.begin(), upgrades.end(), rng);
        choosable_upgrades.clear();
        auto upgrade_iter = upgrades.begin();

        while (choosable_upgrades.size() != 4) {
          if (upgrade_iter->one_use && upgrade_iter->used) {
            upgrade_iter += 1;
            continue;
          }

          choosable_upgrades.push_back(*upgrade_iter.base());
          upgrade_iter += 1;
        }
      }

    } else if (menu_state == MAIN_MENU) {
      DrawTextPro(GetFontDefault(), "MinneFlight", {1024 / 2, 1024 / 5},
                  {300, 50}, std::sin(GetTime() * 2) * 5, 100, 2, BLACK);
      DrawText("Press any button to start", 1024 / 5, 1024 / 5 + 100, 40,
               BLACK);
    } else if (menu_state == GAME_OVER) {
      DrawTextPro(GetFontDefault(), "You died!", {1024 / 2, 1024 / 5},
                  {300, 50}, std::sin(GetTime() * 2) * 5, 100, 2, BLACK);
      DrawText("Press any button to restart", 1024 / 5, 1024 / 5 + 100, 40,
               BLACK);
    } else if (menu_state == UPGRADE_SCREEN) {
      DrawTextPro(GetFontDefault(), "Upgrades!", {1024 / 2, 1024 / 9},
                  {300, 50}, std::sin(GetTime() * 2) * 5, 100, 2, BLACK);
      DrawText(TextFormat("A: %s", choosable_upgrades[0].name.c_str()),
               1024 / 8, 1024 / 8, 40, BLACK);
      DrawText(TextFormat("B: %s", choosable_upgrades[1].name.c_str()),
               1024 / 8, 1024 / 2, 40, BLACK);
      DrawText(TextFormat("A: %s", choosable_upgrades[0].name.c_str()),
               1024 / 8 * 6, 1024 / 8, 40, BLACK);
      DrawText(TextFormat("B: %s", choosable_upgrades[1].name.c_str()),
               1024 / 8 * 6, 1024 / 2, 40, BLACK);
    }

    EndDrawing();
  });

  c.on_close([](auto) { std::cout << "ws closed\n"; });

  c.run();
}
