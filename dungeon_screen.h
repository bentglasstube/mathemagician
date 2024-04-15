#pragma once

#include "audio.h"
#include "backdrop.h"
#include "camera.h"
#include "config.h"
#include "graphics.h"
#include "hud.h"
#include "input.h"
#include "player.h"
#include "screen.h"
#include "text.h"

class DungeonScreen : public Screen {
 public:
  DungeonScreen();

  bool update(const Input& input, Audio& audio, unsigned int elapsed) override;
  void draw(Graphics& graphics) const override;

  Screen* next_screen() const override;
  std::string get_music_track() const override { return "music.ogg"; }

 private:
  enum class State { FadeIn, Playing, Pause, FadeOut };

  static constexpr int kHudHeight = 5 * Config::kTileSize;
  static constexpr int kFadeTimer = 1000;

  Text text_;
  Camera camera_;
  Dungeon dungeon_;
  Player player_;
  State state_;
  HUD hud_;
  int timer_;
};
