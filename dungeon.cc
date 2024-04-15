#include "dungeon.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <map>
#include <stack>
#include <unordered_set>

#include "log.h"
#include "ui.h"
#include "util.h"

Dungeon::Dungeon(int width, int height, unsigned int seed)
    : width_(width),
      height_(height),
      rng_(seed),
      tiles_("tiles.png", 4, Config::kTileSize, Config::kTileSize),
      ui_("ui.png", 10, Config::kHalfTile, Config::kHalfTile),
      doors_("doors.png", 8, Config::kTileSize, Config::kTileSize),
      wall_overlay_("room-overlay.png", 0, 0, 256, 176) {
  load_room_data("content/rooms.txt");
  while (!generate(seed)) {
    ++seed;
  }
}

bool Dungeon::generate(unsigned int seed) {
  DEBUG_LOG << "Generating dungeon with seed " << seed << "\n";
  rng_.seed(seed);

  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      cells_[y][x] = {Dungeon::Tile::Wall, 0, 0, false};
    }
  }

  int rx = width_ / 2 - 7;
  int ry = height_ - 9;
  int room = 0;

  place_room(rx, ry, room, RoomType::Entrance);
  set_tile(rx + 6, ry + 8, Tile::DoorOpen);

  std::uniform_int_distribution<int> rand_dir(0, 3);

  for (int i = 0; i < 15; ++i) {
    const Tile door_tile = room > 0 ? Tile::DoorLocked : Tile::DoorOpen;
    int tries = 10;
    while (tries > 0) {
      const int dir = rand_dir(rng_);
      if (dir == 0) {
        if (try_place_door(rx + 6, ry, rx + 6, ry - 1, door_tile)) {
          ry -= 8;
          break;
        }
      } else if (dir == 1) {
        if (try_place_door(rx + 6, ry + 8, rx + 6, ry + 1, door_tile)) {
          ry += 8;
          break;
        }
      } else if (dir == 2) {
        if (try_place_door(rx + 12, ry + 4, rx + 13, ry + 4, door_tile)) {
          rx += 12;
          break;
        }
      } else if (dir == 3) {
        if (try_place_door(rx, ry + 4, rx - 1, ry + 4, door_tile)) {
          rx -= 12;
          break;
        }
      }
      --tries;
      if (tries == 0) {
        return false;
      }
    }
    place_room(rx, ry, ++room, RoomType::Normal);
  }

  DEBUG_LOG << "Done placing rooms.\n";
  return true;
}

void Dungeon::apply_template(int x, int y, int n) {
  DEBUG_LOG << "Applying template " << n << "\n";
  for (int ty = 0; ty < 7; ++ty) {
    for (int tx = 0; tx < 11; ++tx) {
      set_tile(x + tx + 1, y + ty + 1, room_templates_[n][ty * 11 + tx]);
    }
  }
}

void Dungeon::tile_room(int x, int y, RoomType type) {
  if (type == RoomType::Entrance) {
    apply_template(x, y, 0);
  } else if (type == RoomType::Normal) {
    std::uniform_int_distribution<int> rand_template(
        1, room_templates_.size() - 1);
    apply_template(x, y, rand_template(rng_));
  }
}

Dungeon::Position Dungeon::grid_coords(double px, double py) const {
  return {(int)(px / Config::kTileSize), (int)(py / Config::kTileSize)};
}

Dungeon::Position Dungeon::find_tile(Tile tile) const {
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      if (cells_[y][x].tile == tile) return {x, y};
    }
  }

  return {-1, -1};
}

bool Dungeon::Cell::is_door() const {
  switch (tile) {
    case Tile::DoorOpen:
    case Tile::DoorLocked:
    case Tile::DoorClosed:
      return true;
    default:
      return false;
  }
}

void Dungeon::draw(Graphics& graphics, int hud_height, int xo, int yo) const {
  door_tiles_[0] = Tile::Wall;
  door_tiles_[1] = Tile::Wall;
  door_tiles_[2] = Tile::Wall;
  door_tiles_[3] = Tile::Wall;

  for (int y = 0; y < height_; ++y) {
    const int gy = Config::kTileSize * y - yo;
    if (gy < hud_height - Config::kTileSize) continue;
    if (gy > graphics.height()) break;

    for (int x = 0; x < width_; ++x) {
      const int gx = Config::kTileSize * x - xo;
      if (gx < -Config::kTileSize) continue;
      if (gx > graphics.width()) break;

      auto cell = cells_[y][x];
      if (cell.is_door()) {
        if (gy == 96) {
          doors_.draw(graphics, 32, gx, gy);
          door_tiles_[0] = cell.tile;
        } else if (gy == 224) {
          doors_.draw(graphics, 40, gx, gy);
          door_tiles_[1] = cell.tile;
        } else if (gx == 24) {
          doors_.draw(graphics, 48, gx, gy);
          door_tiles_[2] = cell.tile;
        } else if (gx == 216) {
          doors_.draw(graphics, 56, gx, gy);
          door_tiles_[3] = cell.tile;
        }
      } else if (cell.tile == Tile::Wall) {
        if (gy == 96) doors_.draw(graphics, 27, gx, gy);
        if (gy == 224) doors_.draw(graphics, 28, gx, gy);
        if (gx == 24) doors_.draw(graphics, 29, gx, gy);
        if (gx == 216) doors_.draw(graphics, 30, gx, gy);
      } else {
        tiles_.draw(graphics, static_cast<int>(cell.tile), gx, gy);
        if (cell.value > 0) {
          const int vx = cell.value > 9 ? gx : gx + Config::kQuarterTile;
          const int vy = gy + Config::kQuarterTile;
          const UI::Color vc = cell.active ? UI::Color::Cyan : UI::Color::Black;
          UI::draw_small_number(graphics, ui_, vx, vy, cell.value, vc);
        }
      }
    }
  }
}

#define DRAW_DOOR_TILE(n, ox, oy)                          \
  doors_.draw(graphics, (n), x + (ox) * Config::kTileSize, \
              y + (oy) * Config::kTileSize)

void Dungeon::draw_door_frame(Graphics& graphics, Tile tile, int x,
                              int y) const {
  if (tile == Tile::Wall) return;

  if (y == 96) {  // north door
    DRAW_DOOR_TILE(0, -1, -1);
    DRAW_DOOR_TILE(1, 0, -1);
    DRAW_DOOR_TILE(2, 1, -1);
    DRAW_DOOR_TILE(8, -1, 0);
    if (tile == Tile::DoorLocked) DRAW_DOOR_TILE(33, 0, 0);
    if (tile == Tile::DoorClosed) DRAW_DOOR_TILE(36, 0, 0);
    DRAW_DOOR_TILE(10, 1, 0);
  } else if (y == 224) {  // south door
    DRAW_DOOR_TILE(16, -1, 0);
    if (tile == Tile::DoorLocked) DRAW_DOOR_TILE(41, 0, 0);
    if (tile == Tile::DoorClosed) DRAW_DOOR_TILE(44, 0, 0);
    DRAW_DOOR_TILE(18, 1, 0);
    DRAW_DOOR_TILE(24, -1, 1);
    DRAW_DOOR_TILE(25, 0, 1);
    DRAW_DOOR_TILE(26, 1, 1);
  } else if (x == 24) {  // west door
    DRAW_DOOR_TILE(3, -1, -1);
    DRAW_DOOR_TILE(4, 0, -1);
    DRAW_DOOR_TILE(11, -1, 0);
    if (tile == Tile::DoorLocked) DRAW_DOOR_TILE(49, 0, 0);
    if (tile == Tile::DoorClosed) DRAW_DOOR_TILE(52, 0, 0);
    DRAW_DOOR_TILE(19, -1, 1);
    DRAW_DOOR_TILE(20, 0, 1);
  } else if (x == 216) {  // east door
    DRAW_DOOR_TILE(5, 0, -1);
    DRAW_DOOR_TILE(6, 1, -1);
    if (tile == Tile::DoorLocked) DRAW_DOOR_TILE(57, 0, 0);
    if (tile == Tile::DoorClosed) DRAW_DOOR_TILE(60, 0, 0);
    DRAW_DOOR_TILE(14, 1, 0);
    DRAW_DOOR_TILE(21, 0, 1);
    DRAW_DOOR_TILE(22, 1, 1);
  }
}

void Dungeon::draw_overlay(Graphics& graphics, int hud_height) const {
  wall_overlay_.draw(graphics, 0, hud_height);
  draw_door_frame(graphics, door_tiles_[0], 120, 96);
  draw_door_frame(graphics, door_tiles_[1], 120, 224);
  draw_door_frame(graphics, door_tiles_[2], 24, 160);
  draw_door_frame(graphics, door_tiles_[3], 216, 160);
}

bool Dungeon::walkable(int x, int y) const {
  switch (get_cell(x, y).tile) {
    case Dungeon::Tile::Room:
    case Dungeon::Tile::DoorOpen:
    case Dungeon::Tile::Sand:
      return true;
    default:
      return false;
  }
}

void Dungeon::set_tile(int x, int y, Dungeon::Tile tile) {
  if (x < 0 || x >= width_) return;
  if (y < 0 || y >= height_) return;
  cells_[y][x].tile = tile;
}

Dungeon::Tile Dungeon::get_tile(int x, int y) {
  if (x < 0 || x >= width_) return Tile::OutOfBounds;
  if (y < 0 || y >= height_) return Tile::OutOfBounds;
  return cells_[y][x].tile;
}

const Dungeon::Cell& Dungeon::get_cell(int x, int y) const {
  if (x < 0 || x >= width_) return kBadCell;
  if (y < 0 || y >= height_) return kBadCell;
  return cells_[y][x];
}

namespace {
int lerp(int a, int b, float t) {
  return a + static_cast<int>(std::round(t * (b - a)));
}
}  // namespace

void Dungeon::place_room(int x, int y, int room, RoomType type) {
  DEBUG_LOG << "Placing room at " << x << ", " << y << "\n";
  for (int ty = 0; ty < 7; ++ty) {
    for (int tx = 0; tx < 11; ++tx) {
      Cell& cell = cells_[ty + y + 1][tx + x + 1];
      cell.tile = Tile::Room;
      cell.room = room;
    }
  }
  tile_room(x, y, type);
  if (type != RoomType::Normal) return;

  DEBUG_LOG << "Configuring room\n";

  float t = (room - 1) / 14.f;
  const int min_target = lerp(10, 100, t);
  const int max_target = lerp(25, 300, t);
  const int target =
      std::uniform_int_distribution<int>(min_target, max_target)(rng_);
  rooms_[room].number = room;
  rooms_[room].target = target;

  const int rows = 3 + (room - 1) / 6;
  const int cols = 3 + (room + 2) / 6;
  int tiles_to_value = rows * cols;

  const int max_group_size = std::min(rows + 1, cols + 1);

  while (tiles_to_value > 2) {
    auto values = divide(target, std::min(tiles_to_value - 1, max_group_size));
    assert(values.size() > 1);
    DEBUG_LOG << "  Set ";
    for (auto value : values) {
      DEBUG_LOG << value << ", ";
      place_room_value(x, y, value);
    }
    DEBUG_LOG << "\n";
    tiles_to_value -= values.size();
  }
  while (tiles_to_value > 0) {
    int value = random_in_range(target / 4, 3 * target / 4);
    place_room_value(x, y, std::min(value, 99));
    DEBUG_LOG << "  Extra " << value << "\n";
    --tiles_to_value;
  }
}

void Dungeon::place_room_value(int x, int y, int value) {
  assert(value < 100);

  std::uniform_int_distribution<int> rx(x + 2, x + 10);
  std::uniform_int_distribution<int> ry(y + 2, y + 6);

  while (true) {
    const int tx = rx(rng_);
    const int ty = ry(rng_);
    auto& cell = cells_[ty][tx];
    if (cell.value == 0 && cell.tile == Tile::Room) {
      cell.value = value;
      return;
    }
  }
}

int Dungeon::random_in_range(int min, int max) {
  std::uniform_int_distribution<int> dist(min, max);
  return dist(rng_);
}

std::vector<int> Dungeon::divide(int value, size_t max_count) {
  assert(value / max_count < 99);
  std::vector<int> results;
  DEBUG_LOG << "Dividing " << value << " into " << max_count << " parts\n";
  while (value > 5) {
    const int part = random_in_range(2, std::min(value - 1, 99));
    value -= part;
    results.push_back(part);
    if (results.size() == max_count) {
      DEBUG_LOG << "Hit max size, adding remainder.\n";
      for (size_t i = 0; i < results.size(); ++i) {
        if (results[i] + value > 99) {
          value -= 99 - results[i];
          results[i] = 99;
        } else {
          results[i] += value;
          value = 0;
        }
      }
    }
  }
  if (value > 0) {
    results.push_back(value);
  }
  return results;
}

bool Dungeon::try_place_door(int x, int y, int cx, int cy, Tile door_tile) {
  if (get_tile(cx, cy) == Tile::Wall) {
    set_tile(x, y, door_tile);
    return true;
  }
  return false;
}

bool Dungeon::box_walkable(const Rect& r) const {
  const auto a = grid_coords(r.left, r.top);
  const auto b = grid_coords(r.right, r.bottom);

  return walkable(a.x, a.y) && walkable(a.x, b.y) && walkable(b.x, a.y) &&
         walkable(b.x, b.y);
}

void Dungeon::open_door(int x, int y) {
  switch (get_cell(x, y).tile) {
    case Tile::DoorLocked:
    case Tile::DoorClosed:
      cells_[y][x].tile = Tile::DoorOpen;
      break;
    default:
      // do nothing
      break;
  }
}

void Dungeon::clear_active_cells(int room) {
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      auto& cell = cells_[y][x];
      if (cell.room == room && cell.active) {
        DEBUG_LOG << "Clearing cell " << x << ", " << y << "\n";
        cell.active = false;
        cell.value = 0;
      }
    }
  }
}

void Dungeon::unlock_doors(int room) {
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      auto& cell = cells_[y][x];
      if (cell.tile == Tile::DoorLocked) {
        if (get_cell(x - 1, y).room == room) cell.tile = Tile::DoorClosed;
        if (get_cell(x + 1, y).room == room) cell.tile = Tile::DoorClosed;
        if (get_cell(x, y - 1).room == room) cell.tile = Tile::DoorClosed;
        if (get_cell(x, y + 1).room == room) cell.tile = Tile::DoorClosed;
      }
    }
  }
}

Dungeon::Result Dungeon::activate(int x, int y, Audio& audio) {
  if (x < 0 || x >= width_) return Result::None;
  if (y < 0 || y >= height_) return Result::None;
  auto& cell = cells_[y][x];
  if (cell.value == 0 || cell.active) return Result::None;
  auto& room = get_room(x, y);
  cell.active = true;
  room.add(cell.value);
  audio.play_sample("activate.wav");
  DEBUG_LOG << "Activated tile!  Room is now " << room.running_total << " of "
            << room.target << "\n";
  if (room.done()) {
    DEBUG_LOG << "Clearing active cells."
              << "\n";
    clear_active_cells(room.number);
    if (room.overloaded()) {
      DEBUG_LOG << "Room overloaded, OUCH!"
                << "\n";
      room.clear();
      return Result::Overload;
    }
    DEBUG_LOG << "ORB"
              << "\n";
    audio.play_sample("orb.wav");
    unlock_doors(room.number);
    room.clear();
    return Result::Perfect;
  }
  return Result::None;
}

Dungeon::Room& Dungeon::get_room(int x, int y) {
  return rooms_[get_cell(x, y).room];
}

const Dungeon::Room& Dungeon::get_room(int x, int y) const {
  return rooms_[get_cell(x, y).room];
}

namespace {
Dungeon::Tile tile_for_char(char c) {
  switch (c) {
    case 'x':
      return Dungeon::Tile::Block;
    case 'o':
      return Dungeon::Tile::Pit;
    case 's':
      return Dungeon::Tile::Sand;
    case 'l':
      return Dungeon::Tile::StatueLeft;
    case 'r':
      return Dungeon::Tile::StatueRight;
    default:
      return Dungeon::Tile::Room;
  }
}
}  // namespace

void Dungeon::load_room_data(const std::string& filename) {
  std::array<Tile, 77> template_tiles;
  int index = 0;
  std::ifstream reader(filename);
  std::string row;
  while (std::getline(reader, row)) {
    for (char c : row) {
      if (c == '\n') continue;
      template_tiles[index++] = tile_for_char(c);
    }
    if (index == 77) {
      room_templates_.push_back(template_tiles);
      index = 0;
    }
  }
}

constexpr Dungeon::Cell Dungeon::kBadCell;
