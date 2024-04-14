#pragma once

#include <functional>
#include <memory>
#include <random>
#include <vector>

#include "config.h"
#include "graphics.h"
#include "rect.h"
#include "spritemap.h"

class Dungeon {
 public:
  enum class Tile {
    OutOfBounds,
    Wall,
    Room,
    DoorLocked,
    DoorClosed,
    DoorOpen,
  };

  enum class Result { None, Overload, Perfect };

  struct Cell {
    Tile tile;
    int room;
    int value;
    bool active;
  };

  struct Position {
    int x, y;
  };

  struct Room {
    int target;
    int running_total;
    int number;

    bool overloaded() const { return running_total > target; }
    bool done() const { return running_total >= target; }
    void add(int amount) { running_total += amount; }
    void clear() { running_total = 0; }
    operator bool() const { return target > 0; }
  };

  Dungeon(int width, int height, unsigned int seed);

  Position grid_coords(double px, double py) const;

  const Cell& get_cell(int x, int y) const;
  Position find_tile(Tile tile) const;

  void draw(Graphics& graphics, int hud_height, int xo, int yo) const;

  bool walkable(int x, int y) const;
  bool box_walkable(const Rect& r) const;

  void open_door(int x, int y);
  Result activate(int x, int y);

  Room& get_room(int x, int y);
  const Room& get_room(int x, int y) const;

 private:
  static constexpr int kMaxVisibility = 9;
  static constexpr Cell kBadCell = {Tile::OutOfBounds, 0, 0, false};

  enum class Direction { North, South, East, West };

  int width_, height_;
  std::default_random_engine rng_;
  Cell cells_[1024][1024];
  Room rooms_[16];

  SpriteMap tiles_, ui_;

  bool generate(unsigned int seed);

  void set_tile(int x, int y, Tile tile);
  Tile get_tile(int x, int y);

  void place_room(int x, int y, int room);
  void place_room_value(int x, int y, int value);
  bool try_place_door(int x, int y, int cx, int cy);
  std::vector<int> divide(int target, size_t max_count);
  int random_in_range(int min, int max);
  void clear_active_cells(int room);
  void unlock_doors(int room);
};
