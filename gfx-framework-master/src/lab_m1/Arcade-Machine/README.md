# Arcade-Machine

## Assignment Overview
- **Title:** Assignment 1 – Arcade Machine
- **Mentors:** Robert Caragicu, Cristian Lambru, Andrei Voicu, Ioana Chiper, Vlad Novetschi
- **Release Date:** 26 October 2025
- **Submission Deadline:** 16 November 2025, 23:59
- **Rules:** Follow the general regulations
- **Important:** Anything not stated here is left to your judgment

## Project Scope
- Build a spaceship construction editor.
- Use the assembled ship in one of three mini-games: Pong, Breakout, or Space Invaders.
- Maximum of ten blocks per ship while honoring all editor constraints.

## Spaceship Editor
- Layout includes a block palette on the left, a construction grid (minimum 5×5) on the lower right, a remaining-block counter (green squares) on the upper right, and a Start button in the top-right corner.
- Ships consist of 1×1 blocks placed on the grid; the Start button is green when the layout satisfies all constraints and red otherwise.
- Clicking the green Start button launches the selected game using the current ship layout.

### Block Catalogue
| Block | Description |
| - | - |
| Solid Block | Simple 1×1 square. |
| Cannon | Footprint of 3 rows × 1 column with a base square plus semicircle and a 2×1 barrel on top. |
| Engine | 1×1 square with exhaust geometry beneath. |
| Bumper | 1×1 square capped with a 3-column × 1-row semicircle. |

- Omit blocks that are irrelevant to your chosen game.
- Choose vertex colors freely; keep distinct elements visually differentiated.

### Validation Constraints
- At least one block must be present.
- Layout must contain the middle cell of the grid.
- Layout must be 4-neighborhood connected (horizontal/vertical adjacency only).
- No block may be placed below an engine.
- No block may be placed above a cannon.
- No block may be placed above a bumper across its three-column span.
- Cannons cannot sit exactly one column left or right of a bumper.
- Bumpers cannot share edges with other bumpers.
- Total block count must not exceed ten.

### Editor Interaction
- Drag-and-drop construction:
	- Left-click a palette cell to attach that block to the cursor while the button is held.
	- Release inside the grid to place the block; release outside to cancel it.
- Right-clicking an occupied grid cell removes the block.
- The Start button reflects validity: green for a playable layout, red otherwise.

## Pong Game Reference
- Classic two-player table-tennis arcade experience; players control vertical paddles to keep the ball in play.
- A point is awarded when the opponent misses; first to the configured score wins.
- After every goal, the ball restarts from the center toward the player who conceded.

### Core Requirements
- **Scene Rendering:** Two paddles, the ball, field boundary lines, dotted midline, and player scores (icons or text).
- **Controls:** Player 1 uses `W/S`; Player 2 uses `↑/↓`. Paddles clamp to the screen bounds.
- **Collisions:** Ball is circular, paddles are AABBs. Left/right wall hits award points; top/bottom hits invert vertical velocity; paddle hits adjust direction using impact offset:
	```text
	angle = (ball_y - paddle_y) / (paddle_height / 2)
	ball_vx = ball_speed * cos(angle)
	ball_vy = ball_speed * sin(angle)
	```
- **Ball Reset:** On a goal, increment the scorer, reset the ball to center, and launch toward the player who conceded.

### Advanced Requirements
- **Speed Boost:** Increase ball speed on every bumper collision (bumper approximated as a rectangle).
- **Paddle Animation:** Trigger a brief scale animation (~1 second) on paddle-ball impact.
- **Cannon Launch:** Replace basic reset by spawning new balls from every cannon at round start; the round ends once all balls reach a goal.

## Run

In `main.cpp`
```
// Set this
World *world = new m1::Editor();
```