#include "ClickableTile.h"
#include "Texture.h"
#include "TextureFactory.h"
#include "Board.h"
#include "Window.h"
#include "ScreenWriter.h"
#include "Game.h"
#include "MinesweeperState.h"
#include "WinLoseState.h"
#include "SoundFactory.h"
#include <sstream>
#include <iostream>
#include <cmath>

ClickableTile::ClickableTile(MinesweeperState& s, Board& board,
	int x, int y, unsigned width, unsigned height,
  unsigned boardX, unsigned boardY)
  : state_(s), board_(board), x_(x), y_(y), width_(width), height_(height),
  boardX_(boardX), boardY_(boardY),
  texture_(*TextureFactory::Inst().GetTexture("tile.png")),
  doFadeInEffect_(false), fadeColor_({255,255,255,0}),
  currentAlpha_(0.0f), clickedInTile_(false)
{
}

void ClickableTile::HandleInput(const SDL_Event& ev) {
  if (ev.type == SDL_MOUSEBUTTONUP &&
      ev.button.x >= x_ && ev.button.x < x_ + static_cast<int>(width_) &&
      ev.button.y >= y_ && ev.button.y < y_ + static_cast<int>(height_) &&
      clickedInTile_)
  {
    clickedInTile_ = false;
    fadeColor_ = {255,255,255,static_cast<Uint8>(currentAlpha_)};

    if (ev.button.button == SDL_BUTTON_LEFT) {
			OnLeftClick(ev);
    }
    else if (ev.button.button == SDL_BUTTON_RIGHT) {
			OnRightClick(ev);
    }
  }
  else if (ev.type == SDL_MOUSEBUTTONUP) {
    clickedInTile_ = false;
  }
  else if (ev.type == SDL_MOUSEMOTION) {
    Sint32 oldX = ev.motion.x - ev.motion.xrel;
    Sint32 oldY = ev.motion.y - ev.motion.yrel;

    if (ev.motion.x >= x_ && ev.motion.x < x_ + static_cast<int>(width_) &&
        ev.motion.y >= y_ && ev.motion.y < y_ + static_cast<int>(height_))
    {
      // if we're moving inside the tile, trigger fade-in
      if ((oldX < x_ || oldX >= x_ + static_cast<int>(width_) ||
          oldY < y_ || oldY >= y_ + static_cast<int>(height_)))
      {
				OnMouseEnter(ev);
      }
    }
    else if (ev.motion.x < x_ || ev.motion.x >= x_ + static_cast<int>(width_) ||
            ev.motion.y < y_ || ev.motion.y >= y_ + static_cast<int>(height_))
    {
			// if we're moving outside the tile, trigger fade-out
			doFadeInEffect_ = false;
			currentAlpha_ = fadeColor_.a;
    }
  }
  else if (ev.type == SDL_MOUSEBUTTONDOWN &&
          ev.button.x >= x_ && ev.button.x < x_ + static_cast<int>(width_) &&
          ev.button.y >= y_ && ev.button.y < y_ + static_cast<int>(height_))
  {
    // if we're pressing it down, change the hover color so it is darker
    currentAlpha_ = maxAlpha_;
    fadeColor_ = {0, 0, 0, static_cast<Uint8>(currentAlpha_)};

    // If the user presses the mbutton INSIDE of the tile, then we allow it
    // to be "clicked" when the mouse button is released. This prevents the user
    // from releasing the mouse button over other tiles and having them count
    // as a click.
    clickedInTile_ = true;
  }
}

void ClickableTile::Update(Uint32 ticks) {
  if (doFadeInEffect_) {
    currentAlpha_ += fadeSpeed_ * ticks * 0.001f;
    fadeColor_.a = std::min(currentAlpha_, maxAlpha_);
  }
  else {
    currentAlpha_ -= fadeSpeed_ * ticks * 0.001f;
    fadeColor_.a = std::max(currentAlpha_, 0.0f);
  }
}

void ClickableTile::Draw(const Window& w) const {
  Tile::Status s = board_.At(boardX_, boardY_).status;
  Rectangle src;
  Rectangle dst(x_, y_, width_, height_);

  switch (s) {
    case Tile::HIDDEN: src = {0, 0, 32, 32}; break;
    case Tile::REVEALED:
      if (!board_.At(boardX_, boardY_).hasMine)
        src = {32, 0, 32, 32};
      else
        src = {128, 0, 32, 32};
      break;
    case Tile::MARKED: src =  {64, 0, 32, 32}; break;
    case Tile::QMARK: src = {96, 0, 32, 32}; break;
  }

  w.Draw(texture_, &src, &dst);

  // draw fade effect
  if (s != Tile::REVEALED)
    w.DrawFilledRect(&dst, fadeColor_);

  // Draw adjacent mine count
  if (s == Tile::REVEALED && board_.At(boardX_, boardY_).adjacentMines > 0 &&
      !board_.At(boardX_, boardY_).hasMine)
  {
    std::stringstream ss;
    ss << board_.At(boardX_, boardY_).adjacentMines;

		ScreenWriter& sw = ScreenWriter::Inst();

		if (sw.GetCurrentFont() && ss.str().length() > 0) {
			unsigned ptSize = state_.GetTileHeight()*0.75;
			Font* f = sw.GetFont(sw.GetCurrentFont()->Name(), ptSize);

			if (f) {
				// draw the number in the center of the tile
				int w, h;
				f->SizeText(ss.str().at(0), &w, &h);
				int tx = x_ + state_.GetTileWidth() / 2 - w / 2;
				int ty = y_ + state_.GetTileHeight() / 2 - h / 2;

				sw.Write(f, sw.GetColor(), tx, ty, ss.str(), true);
			}
		}
  }
}

void ClickableTile::OnLeftClick(const SDL_Event& ev) {
	if (!board_.IsInitialized()) {
		// if clicking for the first time, initialize the board_and
		// start the game timer
		//
		board_.Initialize(boardX_, boardY_, state_.GetMineCount());
		state_.SpawnClearEffects(boardX_, boardY_, texture_,
			{0, 0, 32, 32});

		if (board_.RevealFrom(boardX_, boardY_) == 1)
			SoundFactory::Inst().PlaySound("single.wav");
		else
			SoundFactory::Inst().PlaySound("clear.wav");

		state_.SetFlagsUsed(0);
		state_.GetHUD().StartTimer();

	}
	else {
		if (!board_.At(boardX_, boardY_).hasMine &&
				board_.At(boardX_, boardY_).status != Tile::REVEALED)
		{
			state_.SpawnClearEffects(boardX_, boardY_, texture_,
				{0, 0, 32, 32});

			board_.RevealFrom(boardX_, boardY_);
			SoundFactory::Inst().PlaySound("clear.wav");
		}
		else if (board_.At(boardX_, boardY_).hasMine) {
			// player hit a mine: reveal all mine tiles and lose the game
			SoundFactory::Inst().PlaySound("lose.wav");

			for (int y = 0; y < board_.Height(); ++y) {
				for (int x = 0; x < board_.Width(); ++x) {
					if (board_.At(x, y).hasMine)
						board_.At(x, y).status = Tile::REVEALED;
				}
			}
			Game::Inst().PushState(new WinLoseState(false));
			return;
		}
	}

	// Check to see if we won
	int bw = board_.Width();
	int bh = board_.Height();
	if (board_.RevealedTiles() == bw*bh - board_.MineCount()) {

		Game::Inst().PushState(new WinLoseState(true));
	}
}

void ClickableTile::OnRightClick(const SDL_Event& ev) {
	// toggle flag or question mark
	//

	Tile::Status s = board_.At(boardX_, boardY_).status;

	switch (s) {
		case Tile::HIDDEN:
			board_.At(boardX_, boardY_).status = Tile::MARKED;
			state_.IncrementFlagsUsed();
			SoundFactory::Inst().PlaySound("bip.wav");
			break;

		case Tile::MARKED:
			board_.At(boardX_, boardY_).status = Tile::QMARK;
			state_.DecrementFlagsUsed();
			SoundFactory::Inst().PlaySound("bip.wav");
			break;

		case Tile::QMARK:
			board_.At(boardX_, boardY_).status = Tile::HIDDEN;
			SoundFactory::Inst().PlaySound("bip.wav");
			break;

		default: break; // silence warnings
	}
}

void ClickableTile::OnMouseEnter(const SDL_Event& ev) {
	if (board_.At(boardX_, boardY_).status != Tile::REVEALED)
		SoundFactory::Inst().PlaySound("SFX_ButtonHover.ogg");

	doFadeInEffect_ = true;
	if (!clickedInTile_)
		fadeColor_ = {255,255,255,0};
	else
		fadeColor_ = {0,0,0,0};

	currentAlpha_ = 50.0f;
}

void ClickableTile::OnMouseLeave(const SDL_Event& ev) {

}
