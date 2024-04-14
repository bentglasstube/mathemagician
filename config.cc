#include "config.h"

Config::Config() : Game::Config() {
  graphics.title = "Mathemagician";
  graphics.width = 256;
  graphics.height = 240;
  graphics.scale = 3;
  graphics.fullscreen = false;
}
