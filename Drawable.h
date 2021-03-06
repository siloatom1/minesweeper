#ifndef IDRAWABLE_H
#define IDRAWABLE_H

#include <SDL.h>

class Window;
class Board;

class Drawable {
public:
  virtual ~Drawable() {}

  virtual void HandleInput(const SDL_Event& ev) = 0;
  virtual void Update(Uint32 ticks) = 0;
  virtual void Draw(const Window& w) const = 0;

  bool ShouldDelete() const {return shouldDelete_;}
  void Delete() {shouldDelete_ = true;}

private:
  bool shouldDelete_ = false;
};

#endif /* IDRAWABLE_H */
