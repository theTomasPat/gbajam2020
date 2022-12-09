[x] Build environment
	[x] Vim
	[x] DevkitPro
	[x] Makefile
	[x] Git
	[x] script to quickly setup dev environment
	[x] `intercept-build` (part of scan-build) to generate compile_commands

[x] Draw a sprite to the screen
	[x] create bat sprite
	[x] find color palette
	[x] use `grit` to convert sprite bmp to .c and .h files
	[x] assign sprite data to OAM tile

[x] Move sprite around the screen
[x] Check for collision with screen boundary
[x] Define routines for button just-pressed, down, and up
[x] Apply downward velocity to the player
[x] Apply upward force to the player when button pressed
[x] Keep player within screen bounds


[ ] Setup colors
	[ ] convert palette to 16-bit color (b=5-bit, g=5-bit, r=5-bit)
	[ ] (optional) brighten colors in the low-end. "0-14 are practically all black"
	[ ] export to C file (GIMP exports to C natively)




## NOTES

Flappy bird clone ideas

- witch flying through obstacles
	- holes in trees
	- mushrooms
- drone (or paper plane) flying through targets

Infinite runner instead of Flappy Bird

- similar to Chrome's dino game
- witch flying on vroom
	- hopping over obstacles
	- ducking under bats
