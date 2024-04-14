#pragma once

#include "game.h"

struct Config : public Game::Config {
  Config();

  static constexpr int kTileSize = 16;
  static constexpr int kHalfTile = kTileSize / 2;
  static constexpr int kQuarterTile = kTileSize / 4;
};

static const Config kConfig;
