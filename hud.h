#pragma once

#include "dungeon.h"
#include "graphics.h"
#include "player.h"
#include "spritemap.h"
#include "text.h"

class HUD {
 public:
  HUD();
  void draw(Graphics& graphics, const Player& player,
            const Dungeon& dungeon) const;

 private:
  static constexpr int kLine1 = 4 * Config::kHalfTile;
  static constexpr int kLine2 = kLine1 + Config::kHalfTile;
  static constexpr int kLine3 = kLine2 + Config::kHalfTile;
  static constexpr int kLine4 = kLine3 + Config::kHalfTile;

  static constexpr int kLeftSide = 5 * Config::kHalfTile;
  static constexpr int kPanelX = 13 * Config::kHalfTile;
  static constexpr int kPanelTextWide = kPanelX + Config::kHalfTile;
  static constexpr int kPanelTextNarrow = kPanelTextWide + Config::kQuarterTile;

  SpriteMap ui_;
  Text text_;

  void draw_hearts(Graphics& graphics, int x, int y, int full, int total) const;
  void draw_orb_count(Graphics& graphics, int x, int y, int count) const;
  void draw_panel(Graphics& graphics, int x, int y) const;
  void draw_panel_row(Graphics& graphics, int x, int y, int first) const;
};
