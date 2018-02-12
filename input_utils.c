void movement_logic(void) {
	if ((joypad1 & RIGHT) != 0) {
		state = Going_Right;
		++X_POS;
	}
	if ((joypad1 & LEFT) != 0) {
		state = Going_Left;
		--X_POS;
	}
	// check for collision going left right
	collision_left_right();

	if ((joypad1 & DOWN) != 0) {
		state = Going_Down;
		++Y_POS;
	}
	if ((joypad1 & UP) != 0) {
		state = Going_Up;
		--Y_POS;
	}
	// check for collision going up down
	collision_up_down();
}

void check_start(void) {
	// checks if player is pressing start button
	if (((joypad1 & START) != 0) && ((joypad1old & START) == 0)) {
		++new_background;
	}
}