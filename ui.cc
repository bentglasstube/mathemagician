#include "ui.h"

#include "config.h"

void UI::draw_small_number(Graphics& graphics, const SpriteMap& sprites, int x,
                           int y, int number, Color color) {
  draw_digit_string(graphics, sprites, x, y, number, base_for_color(color));
}

void UI::draw_large_number(Graphics& graphics, const SpriteMap& sprites, int x,
                           int y, int number) {
  draw_digit_string(graphics, sprites, x, y, number, 0);
  draw_digit_string(graphics, sprites, x, y + Config::kHalfTile, number, 10);
}

void UI::draw_digit_string(Graphics& graphics, const SpriteMap& sprites, int x,
                           int y, int number, int base) {
  const std::string digits = std::to_string(number);
  int dx = x + 1;
  for (const char& c : digits) {
    sprites.draw(graphics, base + c - '0', dx, y);
    dx += Config::kHalfTile - 1;
  }
}

int UI::base_for_color(Color color) {
  switch (color) {
    case Color::Black:
      return 50;
    case Color::White:
      return 60;
    case Color::Cyan:
      return 70;
    default:
      return 0;
  }
}
