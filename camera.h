#pragma once

#include "config.h"
#include "player.h"

class Camera {
 public:
  Camera();

  void update(const Player& player);
  int xoffset() const;
  int yoffset() const;

 private:
  static constexpr int kLockX = 12 * Config::kTileSize;
  static constexpr int kLockY = 8 * Config::kTileSize;
  static constexpr int kOffsetY = 6 * Config::kTileSize;

  double ox_, oy_;
};
