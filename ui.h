#pragma once

#include "graphics.h"
#include "spritemap.h"

class UI {
 public:
  enum class Color { Black, White, Cyan };
  static void draw_small_number(Graphics& graphics, const SpriteMap& sprites,
                                int x, int y, int number, Color color);
  static void draw_large_number(Graphics& graphics, const SpriteMap& sprites,
                                int x, int y, int number);

 private:
  static void draw_digit_string(Graphics& graphics, const SpriteMap& sprites,
                                int x, int y, int number, int offset);
  static int base_for_color(Color color);
};
