#pragma once

#include "config.h"
#include "entity.h"
#include "graphics.h"
#include "rect.h"
#include "spritemap.h"
#include "text.h"

class Player : public Entity {
 public:
  Player(int x, int y);

  void move(Direction direction);
  void stop();
  bool interact(Dungeon& dungeon);
  void activate(Dungeon& dungeon);
  void attack();

  void hit(Entity& source) override;
  void update(Dungeon& dungeon, unsigned int elapsed) override;
  void draw(Graphics& graphics, int xo, int yo) const override;

  Rect collision_box() const override;
  Rect hit_box() const override;
  Rect attack_box() const override;

  int orbs() const { return orbs_; }

 private:
  static constexpr double kSpeed = 0.1;
  static constexpr int kAttackTime = 250;
  static constexpr int kAttackCooldown = 100;
  static constexpr int kDeathTimer = 2500;
  static constexpr int kAnimationTime = 250;
  static constexpr int kSpinTime = kAnimationTime / 2;

  SpriteMap weapons_;
  Text text_;
  int attack_cooldown_, orbs_;

  int sprite_number() const override;

  void draw_weapon(Graphics& graphics, int xo, int yo) const;
};
