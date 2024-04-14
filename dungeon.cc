#include "dungeon.h"

#include <algorithm>
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
      ui_("ui.png", 10, Config::kHalfTile, Config::kHalfTile) {
  while (!generate(seed)) {
    ++seed;
  }
}

bool Dungeon::generate(unsigned int seed) {
  DEBUG_LOG << "Generating dungeon with seed " << seed << std::endl;
  rng_.seed(seed);

  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      cells_[y][x] = {Dungeon::Tile::Wall, 0, 0, false};
    }
  }

  int rx = width_ / 2 - 7;
  int ry = height_ - 9;
  int room = 0;

  place_room(rx, ry, room);
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
    place_room(rx, ry, ++room);
  }

  DEBUG_LOG << "Done placing rooms." << std::endl;
  return true;
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

void Dungeon::draw(Graphics& graphics, int hud_height, int xo, int yo) const {
  for (int y = 0; y < height_; ++y) {
    const int gy = Config::kTileSize * y - yo;
    if (gy < hud_height - Config::kTileSize) continue;
    if (gy > graphics.height()) break;

    for (int x = 0; x < width_; ++x) {
      const int gx = Config::kTileSize * x - xo;
      if (gx < -Config::kTileSize) continue;
      if (gx > graphics.width()) break;
      auto cell = cells_[y][x];
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

void Dungeon::place_room(int x, int y, int room) {
  DEBUG_LOG << "Placing room at " << x << ", " << y << std::endl;
  for (int ty = 0; ty < 7; ++ty) {
    for (int tx = 0; tx < 11; ++tx) {
      Cell& cell = cells_[ty + y + 1][tx + x + 1];
      cell.tile = Tile::Room;
      cell.room = room;
    }
  }

  // don't place values in rooms without numbers
  if (room == 0) return;

  DEBUG_LOG << "Configuring room" << std::endl;
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

  while (tiles_to_value > 1) {
    auto values = divide(target, std::min(tiles_to_value - 1, max_group_size));
    DEBUG_LOG << "  Set ";
    for (auto value : values) {
      DEBUG_LOG << value << ", ";
      place_room_value(x, y, value);
    }
    DEBUG_LOG << std::endl;
    tiles_to_value -= values.size();
  }
  while (tiles_to_value > 0) {
    int value = random_in_range(target / 4, 3 * target / 4);
    place_room_value(x, y, value);
    DEBUG_LOG << "  Extra " << value << std::endl;
    --tiles_to_value;
  }
}

void Dungeon::place_room_value(int x, int y, int value) {
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
  std::vector<int> results;
  while (value > 2) {
    const int part = random_in_range(2, std::min(value - 1, 99));
    value -= part;
    results.push_back(part);
    if (results.size() == max_count) {
      const int i = random_in_range(0, max_count - 1);
      results[i] += value;
      value = 0;
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
        DEBUG_LOG << "Clearing cell " << x << ", " << y << std::endl;
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

Dungeon::Result Dungeon::activate(int x, int y) {
  if (x < 0 || x >= width_) return Result::None;
  if (y < 0 || y >= height_) return Result::None;
  auto& cell = cells_[y][x];
  if (cell.value == 0 || cell.active) return Result::None;
  auto& room = get_room(x, y);
  cell.active = true;
  room.add(cell.value);
  DEBUG_LOG << "Activated tile!  Room is now " << room.running_total << " of "
            << room.target << std::endl;
  if (room.done()) {
    DEBUG_LOG << "Clearing active cells." << std::endl;
    clear_active_cells(room.number);
    if (room.overloaded()) {
      DEBUG_LOG << "Room overloaded, OUCH!" << std::endl;
      room.clear();
      return Result::Overload;
    }
    DEBUG_LOG << "ORB" << std::endl;
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

constexpr Dungeon::Cell Dungeon::kBadCell;
