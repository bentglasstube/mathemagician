#include "dungeon_screen.h"

#include "title_screen.h"
#include "util.h"

DungeonScreen::DungeonScreen()
    : text_("text.png"),
      camera_(),
      dungeon_(1024, 1024, Util::random_seed()),
      player_(0, 0),
      state_(State::FadeIn),
      hud_(),
      timer_(0) {
  player_.set_position(512 * 16 - 8, 1023 * 16);
}

bool DungeonScreen::update(const Input& input, Audio& audio,
                           unsigned int elapsed) {
  if (state_ == State::FadeIn) {
    timer_ += elapsed;
    if (timer_ > kFadeTimer) {
      state_ = State::Playing;
      timer_ = 0;
    }
  } else if (state_ == State::FadeOut) {
    timer_ += elapsed;
    if (timer_ > kFadeTimer) {
      if (player_.dead()) return false;

      timer_ = 0;
      state_ = State::FadeIn;

      return true;
    }
  } else {
    if (input.key_held(Input::Button::Left)) {
      player_.move(Player::Direction::West);
    } else if (input.key_held(Input::Button::Right)) {
      player_.move(Player::Direction::East);
    } else if (input.key_held(Input::Button::Up)) {
      player_.move(Player::Direction::North);
    } else if (input.key_held(Input::Button::Down)) {
      player_.move(Player::Direction::South);
    } else {
      player_.stop();
    }

    if (input.key_pressed(Input::Button::A)) {
      if (!player_.interact(dungeon_, audio)) player_.attack();
    }

    if (input.key_pressed(Input::Button::B)) {
      audio.play_sample("focus.wav");
      player_.focus();
    }

    if (player_.dead()) state_ = State::FadeOut;
  }

  player_.update(dungeon_, elapsed, audio);
  camera_.update(player_);

  return true;
}

void DungeonScreen::draw(Graphics& graphics) const {
  const int xo = camera_.xoffset();
  const int yo = camera_.yoffset();

  dungeon_.draw(graphics, kHudHeight, xo, yo);
  dungeon_.draw_overlay(graphics, kHudHeight);
  player_.draw(graphics, xo, yo);

  if (state_ == State::FadeIn || state_ == State::FadeOut) {
    const double pct = timer_ / (double)kFadeTimer;
    const int width = (int)((state_ == State::FadeOut ? pct : 1 - pct) *
                            graphics.width() / 2);

    graphics.draw_rect({0, 0}, {width, graphics.height()}, 0x000000ff, true);
    graphics.draw_rect({graphics.width() - width, 0},
                       {graphics.width(), graphics.height()}, 0x000000ff, true);
  }

  graphics.draw_rect({0, 0}, {graphics.width(), kHudHeight}, 0x000000ff, true);
  hud_.draw(graphics, player_, dungeon_);
}

Screen* DungeonScreen::next_screen() const { return new TitleScreen(); }

std::string DungeonScreen::get_music_track() const { return ""; }
