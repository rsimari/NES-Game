
void update_sprites(void) {
	index4 = 0;
	move4 = state << 2;
	for (index = 0; index < 4; ++index) {
		SPRITES[index4] = MetaSprite_Y[index] + Y_POS; // relative y + master y
		++index4;
		SPRITES[index4] = MetaSprite_Tile[index + move4]; // tile number switch
		++index4;
		SPRITES[index4] = MetaSprite_Attr[index]; // change attributes
		++index4;
		SPRITES[index4] = MetaSprite_X[index] + X_POS; // relative x + master x
		++index4;
	}
}