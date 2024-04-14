#include "hud.h"

#include "config.h"
#include "ui.h"

HUD::HUD()
    : ui_("ui.png", 10, Config::kHalfTile, Config::kHalfTile),
      text_("text.png") {}

void HUD::draw(Graphics& graphics, const Player& player,
               const Dungeon& dungeon) const {
  draw_hearts(graphics, kLeftSide, kLine3, player.health(),
              player.max_health());
  draw_orb_count(graphics, kLeftSide, kLine4, player.orbs());
  draw_panel(graphics, kPanelX, kLine1);
  const auto p = dungeon.grid_coords(player.x(), player.y());
  const auto room = dungeon.get_room(p.x, p.y);
  if (room) {
    text_.draw(graphics, "ROOM " + std::to_string(room.number), kLeftSide,
               kLine1);
    UI::draw_large_number(graphics, ui_,
                          room.target > 99 ? kPanelTextWide : kPanelTextNarrow,
                          kLine2, room.target);
  }
}

void HUD::draw_hearts(Graphics& graphics, int x, int y, int full,
                      int total) const {
  for (int i = 0; i < total; ++i) {
    ui_.draw(graphics, i < full ? 30 : 20, x + Config::kHalfTile * i, y);
  }
}

void HUD::draw_orb_count(Graphics& graphics, int x, int y, int count) const {
  ui_.draw(graphics, 40, x, y);
  ui_.draw(graphics, 80, x + Config::kHalfTile, y);
  UI::draw_small_number(graphics, ui_, x + 2 * Config::kHalfTile, y, count,
                        UI::Color::White);
}

void HUD::draw_panel_row(Graphics& graphics, int x, int y, int first) const {
  ui_.draw(graphics, first + 0, x + 0 * Config::kHalfTile, y);
  ui_.draw(graphics, first + 1, x + 1 * Config::kHalfTile, y);
  ui_.draw(graphics, first + 1, x + 2 * Config::kHalfTile, y);
  ui_.draw(graphics, first + 1, x + 3 * Config::kHalfTile, y);
  ui_.draw(graphics, first + 2, x + 4 * Config::kHalfTile, y);
}

void HUD::draw_panel(Graphics& graphics, int x, int y) const {
  draw_panel_row(graphics, x, y + 0 * Config::kHalfTile, 21);
  draw_panel_row(graphics, x, y + 1 * Config::kHalfTile, 31);
  draw_panel_row(graphics, x, y + 2 * Config::kHalfTile, 31);
  draw_panel_row(graphics, x, y + 3 * Config::kHalfTile, 41);
}

