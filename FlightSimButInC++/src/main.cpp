#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <variant>
#include <vector>
#define WSPP_USE_OPENSSL
#include "raygui.h"
#include "wspp.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

// void handleInputs(float &x, float &y, float &z,
//                   float sensitivity = 0.5f) { // bbbbbroken!
//   // Quaternion q = QuaternionFromEuler(target_z * 45.0f * DEG2RAD, 0,
//   //                                    target_x * 45.0f * DEG2RAD);
//   // Emulate y-axis gyro (horizontal movement/roll)
//   if (IsKeyDown(KEY_RIGHT))
//     x += sensitivity;
//   if (IsKeyDown(KEY_LEFT))
//     x -= sensitivity;
//
//   // Emulate x-axis gyro (vertical movement/pitch)
//   if (IsKeyDown(KEY_UP))
//     z -= sensitivity;
//   if (IsKeyDown(KEY_DOWN))
//     z += sensitivity;
// }

class Entity {
public:
  Vector3 position;
  float rotation;
  bool active;

  Color color = WHITE;

  Entity(Vector3 pos) : position(pos), rotation(0.0f), active(true) {}

  void Update(Vector3 pos) {
    if (!active)
      return;

    rotation += 2.0f;
    if (rotation >= 360.0f)
      rotation -= 360.0f;

    // Move relative to the plane's apparent motion
    position = pos;
  }

  void draw(Model &model) {
    if (!active)
      return;
    // Draw centered at position, rotating around the Y-axis
    DrawModelEx(model, position, {0, 1, 0}, rotation, {2, 2, 2}, color);
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
  srand(time(NULL));
  auto rng = std::default_random_engine{};

  if (argc != 2) {
    printf("USAGE: %s [lobby code]\n", argv[0]);
    return 1;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Window Initialization
  /////////////////////////////////////////////////////////////////////////////

  InitWindow(1024, 1024, "MinneFlight");
  SetTargetFPS(60);

  InitAudioDevice();
  Sound gearSound = LoadSound("../assets/gear_collect.mp3");
  Music intenseMusic = LoadMusicStream("../assets/intenseMusic.mp3");
  intenseMusic.looping = true; // Enables perfect looping
  PlayMusicStream(intenseMusic);

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

  float x = 0.0;
  float target_x = 0.0;

  float y = 0.0;
  float target_y = 0.0;

  float z = 0.0;
  float target_z = 0.0;

  float health = 100;
  float progression = 0;

  float cool_down_time = 0;
  float show_time = 0;
  float fire_time = 0;
  float hurt_count_down = 0;
  int hurt_object = 0;
  float manuevering_speed = 0;
  float health_dec = 0;
  bool magnitisim = false;

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
    PAUSE_SCREEN,
  } menu_state = MAIN_MENU;

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
  c.connect("ws://foxmoss.com:9003/api/laptop_ws/" + std::string(argv[1]));

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
      {"Magnitisim", Upgrade::MAGNITISM, true},
  };

  std::vector<Upgrade> choosable_upgrades = {};

  size_t frame_ticks = 0;

  std::vector<Entity> collectibles;

  c.on_tick([&](std::optional<wspp::message_view> msg) {
    UpdateMusicStream(intenseMusic);

    frame_ticks++;
    if (WindowShouldClose()) {
      c.close();
    }

    if (msg.has_value()) {
      nlohmann::json data = nlohmann::json::parse(msg->text());

      if (data["type"] == "gyro_update") {
        x += -(float)data["x"] / 10;
        y += (float)data["z"] / 10;
        z += (float)data["y"] / 10;
      }
      if (data["type"] == "button_down" &&
          (menu_state == MAIN_MENU || menu_state == GAME_OVER)) {
        // init game statej
        menu_state = GAME_RUNNING;
        health = 100;

        collectibles.clear();
        for (size_t i = 0; i < 20; i++) {
          collectibles.emplace_back(Vector3{
              40.0f + (i * 20.0f), (float)(std::rand() % 256) / 256 * 10 - 5,
              (float)(std::rand() % 256) / 256 * 10 - 5});
        }

        hurt_spheres.clear();

        health = 100;
        progression = 100;

        cool_down_time = 10;
        show_time = 5;
        fire_time = 5;
        hurt_count_down = cool_down_time;
        hurt_object = 1;
        manuevering_speed = 0.1;
        health_dec = 1;
        magnitisim = false;

        for (size_t i = 0; i < upgrades.size(); i++) {
          upgrades[i].used = false;
        }
        std::shuffle(upgrades.begin(), upgrades.end(), rng);

        fire_state = FIRING_COOL_DOWN;
      } else if (data["type"] == "button_down" &&
                 menu_state == UPGRADE_SCREEN) {
        std::string button_int = data["button"];
        int button_index = std::stoi(button_int);
        auto selected_upgrade = choosable_upgrades[button_index];

        printf("Upgrade selected %s\n", selected_upgrade.name.c_str());

        switch (selected_upgrade.type) {
        case Upgrade::LESS_BARRAGES:
          hurt_object -= 1;
          hurt_object = std::max(hurt_object, 1);
          break;
        case Upgrade::INCREASE_SHOOTER_COOLDOWN:
          cool_down_time++;
          break;
        case Upgrade::PROTECTION:
          health_dec /= 2;
          break;
        case Upgrade::MORE_HEALTH:
          health += 20;
          health = std::clamp(health, 0.0f, 100.0f);
          break;
        case Upgrade::FASTER_MANUEVERING:
          manuevering_speed += 0.1;
          break;
        case Upgrade::MAGNITISM:
          magnitisim = true;
          break;
        }

        menu_state = GAME_RUNNING;
        progression = 0;
      }
    }

    x = x * 0.9;
    target_x = x * 0.1 + target_x * 0.9;

    z = z * 0.9;
    target_z = z * 0.1 + target_z * 0.9;

    BeginDrawing();

    ClearBackground(Color{201, 209, 211, 255});

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

      for (size_t i = 0; i < collectibles.size(); i++) {
        collectibles[i].Update(collectibles[i].position -
                               Vector3{1.0, up, horizontal} *
                                   manuevering_speed);

        if (magnitisim) {
          float strength = (16000 - collectibles[i].position.x) / 10000000.0;

          collectibles[i].Update(Vector3Add(
              Vector3Scale(collectibles[i].position, 1 - strength),
              Vector3Scale({collectibles[i].position.x, 0, 0}, strength)));
        }
        collectibles[i].draw(collectibleGear);

        if (CheckCollisionSpheres(Vector3Zero(), 1.0f, collectibles[i].position,
                                  0.5f)) {
          collectibles[i].Update({40.0f + (i * 20.0f),
                                  (float)(std::rand() % 256) / 256 * 10 - 5,
                                  (float)(std::rand() % 256) / 256 * 10 - 5});
          PlaySound(gearSound);
          progression += 5.0f;
        }

        if (collectibles[i].position.x < camera.position.x) {
          collectibles[i].Update({40.0f + (i * 20.0f),
                                  (float)(std::rand() % 256) / 256 * 10 - 5,
                                  (float)(std::rand() % 256) / 256 * 10 - 5});
        }

        // if (Vector3Distance(Vector3Zero(), collectibles[i].position) < 1.0) {
        //   progression += 5;
        // }
      }

      minneapolis.Update(minneapolis.position -
                         Vector3{1.0, up, horizontal} * manuevering_speed);
      minneapolis.draw(); // doesn't draw anything idk.
    }

    EndMode3D();

    DrawFPS(10, 10);

    if (menu_state == GAME_RUNNING) {
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

      if (progression >= 100) {
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
      GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
      GuiProgressBar({1024 / 5, 1024 / 16, 1024 / 8 * 6, 30}, "Progression",
                     NULL, &progression, 0, 100);

      GuiProgressBar({1024 / 4, 1024 / 8 * 5, 1024 / 2, 20}, "Health", NULL,
                     &health, 0, 100);

      DrawRectangle(0, 0, 1024, 1024, Color{255, 255, 255, 100});

      DrawTextPro(GetFontDefault(), "Upgrades!", {1024 / 2, 1024 / 7},
                  {300, 50}, std::sin(GetTime() * 2) * 5, 100, 2, BLACK);
      DrawText(TextFormat("A: %s", choosable_upgrades[0].name.c_str()),
               1024 / 16, 1024 / 3, 20, BLACK);
      DrawText(TextFormat("B: %s", choosable_upgrades[1].name.c_str()),
               1024 / 16, 1024 / 2, 20, BLACK);
      DrawText(TextFormat("X: %s", choosable_upgrades[2].name.c_str()),
               1024 / 16 * 8, 1024 / 3, 20, BLACK);
      DrawText(TextFormat("Y: %s", choosable_upgrades[3].name.c_str()),
               1024 / 16 * 8, 1024 / 2, 20, BLACK);
    }

    EndDrawing();
  });

  c.on_close([&](auto) {
    std::cout << "ws closed\n";
    UnloadSound(gearSound);
    CloseAudioDevice();
  });

  c.run();
}
