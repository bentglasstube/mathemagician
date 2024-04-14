#include "player.h"

Player::Player(int x, int y)
    : Entity("player.png", 4, x, y, 3),
      weapons_("weapons.png", 2, Config::kTileSize, Config::kTileSize),
      text_("text.png"),
      attack_cooldown_(0),
      orbs_(0) {}

void Player::move(Player::Direction direction) {
  if (state_ == State::Attacking) return;
  if (state_ == State::Dying) return;

  facing_ = direction;
  state_ = State::Walking;
}

void Player::stop() {
  if (state_ == State::Walking) state_ = State::Waiting;
}

bool Player::interact(Dungeon& dungeon) {
  if (state_ == State::Dying) return true;

  auto p = dungeon.grid_coords(x_, y_);
  switch (facing_) {
    case Direction::North:
      --p.y;
      break;
    case Direction::South:
      ++p.y;
      break;
    case Direction::West:
      --p.x;
      break;
    case Direction::East:
      ++p.x;
      break;
  }

  auto cell = dungeon.get_cell(p.x, p.y);
  if (cell.tile == Dungeon::Tile::DoorClosed) {
    if (orbs_ > 0) {
      dungeon.open_door(p.x, p.y);
      --orbs_;
      return true;
    }
  }
  return false;
}

void Player::activate(Dungeon& dungeon) {
  if (state_ == State::Dying) return;
  if (state_ == State::Attacking) return;

  auto p = dungeon.grid_coords(x_, y_);
  auto result = dungeon.activate(p.x, p.y);
  switch (result) {
    case Dungeon::Result::Overload:
      hurt(1);
      break;
    case Dungeon::Result::Perfect:
      ++orbs_;
      break;
    default:
      // do nothing
      break;
  }
}

void Player::attack() {
  if (state_ == State::Attacking) return;
  if (attack_cooldown_ > 0) return;
  state_transition(State::Attacking);
}

// Helper to lock walking to half-tile grid
std::pair<double, double> grid_walk(double delta, double minor, int grid) {
  const double dmin = minor - 8 * (int)(minor / 8);
  if (dmin > grid) {
    if (delta > 8 - dmin) {
      return {delta - 8 + dmin, 8 - dmin};
    } else {
      return {0, delta};
    }
  } else {
    if (delta > dmin) {
      return {delta - dmin, -dmin};
    } else {
      return {0, -delta};
    }
  }
}

void Player::hit(Entity& source) { Entity::hit(source); }

void Player::update(Dungeon& dungeon, unsigned int elapsed) {
  Entity::update_generic(dungeon, elapsed);

  if (attack_cooldown_ > 0) attack_cooldown_ -= elapsed;

  if (state_ == State::Walking && kbtimer_ == 0) {
    const double delta = kSpeed * elapsed;
    std::pair<double, double> d;
    double dx = 0, dy = 0;

    switch (facing_) {
      case Direction::North:
      case Direction::South:
        d = grid_walk(delta, x_, Config::kQuarterTile);
        dx = d.second;
        dy = d.first * (facing_ == Direction::North ? -1 : 1);
        break;

      case Direction::West:
      case Direction::East:
        d = grid_walk(delta, y_, Config::kQuarterTile);
        dx = d.first * (facing_ == Direction::West ? -1 : 1);
        dy = d.second;
        break;
    }

    if (move_if_possible(dungeon, dx, dy)) {
      timer_ = (timer_ + elapsed) % (kAnimationTime * 4);
    }

  } else if (state_ == State::Attacking) {
    timer_ += elapsed;
    if (timer_ > kAttackTime) {
      attack_cooldown_ = kAttackCooldown;
      state_transition(State::Waiting);
    }
  } else if (state_ == State::Dying) {
    timer_ += elapsed;
    facing_ = static_cast<Direction>((timer_ / kSpinTime) % 4);
    dead_ = timer_ > kDeathTimer;
  }
}

void Player::draw(Graphics& graphics, int xo, int yo) const {
  if (iframes_ > 0 && (iframes_ / 32) % 2 == 0) return;

  const int x = (int)x_ - Config::kHalfTile - xo;
  const int y = (int)y_ - Config::kHalfTile - yo;

  if (facing_ == Direction::North) draw_weapon(graphics, xo, yo);
  sprites_.draw_flip(graphics, sprite_number(), x, y,
                     facing_ == Direction::West, false);
  if (facing_ != Direction::North) draw_weapon(graphics, xo, yo);

#ifndef NDEBUG
  hit_box().draw(graphics, 0xff0000ff, false, xo, yo);
#endif
}

int Player::sprite_number() const {
  int d = 0;

  if (state_ == State::Dying && timer_ > kDeathTimer) return 15;

  switch (facing_) {
    case Direction::North:
      d = 0;
      break;
    case Direction::South:
      d = 2;
      break;
    default:
      d = 1;
      break;
  }

  if (state_ == State::Attacking && timer_ > kAttackTime / 4) return 12 + d;

  return d * 4 + (state_ == State::Walking ? timer_ / kAnimationTime : 0);
}

Rect Player::collision_box() const {
  return {x_ - Config::kHalfTile, y_, x_ + Config::kHalfTile - 1,
          y_ + Config::kHalfTile - 1};
}

Rect Player::hit_box() const {
  return {x_ - Config::kHalfTile + 2, y_ + 2, x_ + Config::kHalfTile - 2,
          y_ + Config::kHalfTile - 2};
}

Rect Player::attack_box() const {
  if (state_ == State::Attacking) {
    double sx = x_;
    double sy = y_;
    double w = Config::kTileSize;
    double h = Config::kTileSize;

    const double pct = timer_ / (double)kAttackTime;
    const double offset = (pct <= 0.5 ? pct : 1 - pct) * 2 * Config::kTileSize;

    switch (facing_) {
      case Direction::North:
        sx -= Config::kHalfTile;
        sy -= Config::kHalfTile + offset;
        w = Config::kHalfTile;
        break;

      case Direction::South:
        sy += offset / 2;
        w = Config::kHalfTile;
        break;

      case Direction::West:
        sx -= Config::kHalfTile + offset;
        h = Config::kHalfTile;
        break;

      case Direction::East:
        sx -= Config::kHalfTile - offset;
        h = Config::kHalfTile;
        break;
    }

    return {sx, sy, sx + w, sy + h};
  } else {
    return {0, 0, 0, 0};
  }
}

void Player::draw_weapon(Graphics& graphics, int xo, int yo) const {
  const Rect weapon = attack_box();
  int wx = (int)weapon.left - xo;
  int wy = (int)weapon.top - yo;

  if (weapon.height() != 0) {
    int weapon_sprite = 0;
    switch (facing_) {
      case Direction::North:
        weapon_sprite = 0;
        wx -= 4;
        break;
      case Direction::South:
        weapon_sprite = 1;
        wx -= 4;
        break;
      case Direction::West:
        weapon_sprite = 2;
        wy -= 4;
        break;
      case Direction::East:
        weapon_sprite = 3;
        wy -= 4;
        break;
    }

    // TODO better handling of weapon sprite positioning
    weapons_.draw(graphics, weapon_sprite, wx, wy);
#ifndef NDEBUG
    weapon.draw(graphics, 0x0000ffff, false, xo, yo);
#endif
  }
}
