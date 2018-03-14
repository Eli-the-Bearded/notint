NOTINT
------

"Easy" Tetris (take two). This is a fork of "TINT is not Tetris"
[tint-0.04+nmu1](https://packages.debian.org/jessie/tint) to
continue the game piece distribution experiment I started with
eastris-bsd. 

Since the reader probably does not know what that is: I modified
the BSD games version of Tetris to drop the same piece about 30
times in a row (there's a small chance of getting something
different). This changes gameplay significantly. In the end, I
found that the controls in that version of Tetris to be
unsatisfactory, hence moving to a new code base.

As of release 1.1, there are four game play modes with their
own high score lists:

* _traditional_
  This should be just the same as TINT, except that a scoring
  rounding bug when using 'draw grid' and 'show next' at once
  has been fixed.

* _easytris_
  This is the less random game piece selection. For intervals
  of ten to thirty pieces, you have an 85% chance to get a
  particular piece, then the interval resets to a new piece.

* _zen_
  This just scores based on lines cleared. The game play
  never speeds up, it's just a steady stream of blocks.

* _challenge_
  Clear the board of the initial pieces, trying not to leave
  the board full of additional pieces when solved.

March 2018
