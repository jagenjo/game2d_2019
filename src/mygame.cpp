#include "mygame.h"

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

#include <algorithm>    // std::max
#include "includes.h"
#include "framework.h"
#include "input.h"

Vector2 campos;
World world;

World* World::instance = NULL;

Image* font = NULL;
Image* minifont = NULL;

const sUpgrade upgrade_table[] = {
	{ 0, 0, 0, 0, 0, 0, "nothing" },
	{ ITEM_FOUNTAIN, ITEM_WELL, 5, -1, -1, 0, "build well" },
	{ ITEM_TREE, ITEM_NOTHING, 5, 1, 0, 0, "chop tree" },
	{ ITEM_TREE2, ITEM_NOTHING, 5, 1, 0, 0, "chop tree" },
	{ ITEM_ROCKS, ITEM_NOTHING, 5, 0, 1, 0, "collect rocks" },
	{ ITEM_LOGS, ITEM_FOUNDATIONS, 5, -1, 0, 0, "foundations" },
	{ ITEM_FOUNDATIONS, 129, 10, -5, 0, 0, "build hut" },
	{ ITEM_PORT_WATER, ITEM_HARBOUR, 10, -5, 0, 0, "build harbour" },
	{ ITEM_PORT_SAND, ITEM_WAREHOUSE, 10, -5, -5, 0, "build warehouse" },
	{ 128, 132, 10, -5, -5, -5, "build church" },
	{ 129, 133, 10, -5, -5, -2, "upgrade house" },
	{ 130, 134, 10, -5, -5, -2, "upgrade house" },
	{ 131, 135, 10, -5, -5, -2, "upgrade house" },
	{ 132, 136, 10, -5, -5, -5, "upgrade church" },
	{ 255, 255, 0, 0, 0, 0, "LAST" }
};
const int upgrade_table_size = 16;

bool blink( float freq ) { return int(getTime()*0.001*freq) % 2 == 0; }

World::World()
{
	font = Image::Get("data/bitmap-font-white.tga");
	minifont = Image::Get("data/bitmap-font-yellow.tga");

	alive_players = 0;
	map_fog = true;
	unlimited_movements = false;

	restart();
}

void World::restart()
{
	gamemap.resize(128, 128);
	generateMap();
	selected_player = 0;
	day = 0;
	souls_saved = 0;
	alive_players = 3;

	for (int i = 0; i < 3; ++i)
	{
		sCharacter& player = players[i];
		player.character = i;
		player.pos.set(16*20, int(gamemap.height * 0.5) * 16 - 16 * i);
		player.max_movements = 20;
		player.movements = player.max_movements;
		player.love = 100;
		player.water = 100;

		player.wood = 10;
		player.stone = 10;
		player.goods = 10;

		player.alive = true;
	}
	campos = players[0].pos;
}

sUpgrade World::getUpgradeInfo(int item)
{
	if(!item)
		return upgrade_table[0];
	for (int i = 0; i < upgrade_table_size; ++i)
	{
		const sUpgrade& row = upgrade_table[i];
		if (row.item != item)
			continue;
		return row;
	}
	return upgrade_table[0];
}

void World::upgradeCell(sCharacter* author, int x, int y)
{
	if (x < 0 || x >= world.gamemap.width || y < 0 || y >= world.gamemap.height)
		return;
	sCell& cell = world.gamemap.get(x, y);
	PlayStage* play = (PlayStage*)Stage::stages["play"];

	for (int i = 0; i < upgrade_table_size; ++i)
	{
		const sUpgrade& row = upgrade_table[i];
		if (row.item != cell.item)
			continue;
		//check if I fill all the requirements
		play->missing_resources.set(0, 0, 0, 0);
		if (author->movements < row.movements)
			play->missing_resources.x = row.movements;
		if (author->wood + row.wood < 0)
			play->missing_resources.y = row.wood;
		if (author->stone + row.stone < 0)
			play->missing_resources.z = row.stone;
		if (author->goods + row.goods < 0)
			play->missing_resources.w = row.goods;
		if ( !play->missing_resources.isZero() )
		{
			play->missing_time = getTime() + 2000; //2 seconds
			return;
		}
		cell.item = row.next_item;
		if(!unlimited_movements)
			author->movements -= row.movements;
		author->stone += row.stone;
		author->wood += row.wood;
		author->goods += row.goods;

		//update blessing area
		int bless_radius = 0;
		if (cell.item == 132) //church
			blessArea(x, y, 5);
		else if (cell.item == 136) //church
			blessArea(x, y, 10);
		break;
	}
}

void World::blessArea(int x, int y, int radius)
{
	Vector2 center(x, y);
	for (int cx = x - radius; cx <= x + radius; ++cx)
		for (int cy = y - radius; cy <= y + radius; ++cy)
		{
			if (cx < 0 || cx >= gamemap.width || cy < 0 || cy >= gamemap.height)
				continue;
			if (Vector2(cx, cy).distance(center) > radius)
				continue;
			sCell& bless_cell = gamemap.get(cx, cy);
			bless_cell.blessed = true;
		}
}

void World::passTurn()
{
	day += 1;
	alive_players = 0;
	for (int i = 0; i < 3; ++i)
	{
		sCharacter& player = players[i];
		if (!player.alive)
			continue;

		if (player.love > 5)
			player.love -= 5;
		else
			player.love = 0;

		if (player.water > 5)
			player.water -= 5;
		else
			player.water = 0;

		if(!player.love || !player.water)
			player.max_movements--;

		player.movements = player.max_movements;
		player.alive = player.movements > 0;
		if(player.alive)
			alive_players++;
	}

	//compute map stuff
	souls_saved = 0;
	for(int x = 1; x < gamemap.width - 1; ++x)
		for (int y = 1; y < gamemap.height - 1; ++y)
		{
			sCell& cell = gamemap.get(x, y);
			if (cell.item == ITEM_WAREHOUSE && ( gamemap.get(x - 1, y).item == ITEM_HARBOUR || gamemap.get(x + 1, y).item == ITEM_HARBOUR) && cell.goods < 10 && random() > 0.8 )
				cell.goods += 1;
			if (cell.blessed)
			{
				if (cell.people)
					souls_saved += cell.people;
				if (cell.item >= 129 && cell.item <= 131)
					souls_saved += 5;
				if (cell.item >= 133 && cell.item <= 135)
					souls_saved += 10;
			}
			if ( (cell.item == 1 || cell.item == 2) && random() > 0.9) //tree reproducing
			{
				const Vector2 offsets[4] = { {-1.0f,0.0f }, {1.0f,0.0f },{ 0.0f, 1.0f }, {0.0f,-1.0f} };
				for (int i = 0; i < 4; ++i)
				{
					sCell& nextcell = gamemap.get(x + offsets[i].x, y + offsets[i].y);
					if (random() > 0.9 && !nextcell.item && nextcell.terrain == TILE_GRASS)
						nextcell.item = rand() % 2 + 1;
				}
			}
		}

	if (alive_players == 0 || day >= 365 )
		Stage::changeStage("ending");
}

void World::generateMap()
{
	int sea_margin = 10;
	memset( gamemap.data, 0, sizeof(sCell) * gamemap.width *gamemap.height); //set all to 0
	float seed = 0;
	seed = getTime();
	int num_islands = 16;
	int max_size = gamemap.width * 0.3;
	Vector3 islands[16];
	Vector2 center(gamemap.width * 0.5, gamemap.height * 0.5);
	Vector2 spawn( 3, gamemap.height * 0.5);

	//define islands
	for (int i = 0; i < num_islands; ++i)
	{
		float islandx = (random() * 0.6 + 0.2) * gamemap.width;
		float islandy = (random() * 0.8 + 0.1) * gamemap.height;
		float island_size = (random() * 0.6 + 0.2) * max_size;
		islands[i].set(islandx, islandy, island_size);
	}

	//create terrain
	for (int x = 0; x < gamemap.width; ++x)
		for (int y = 0; y < gamemap.height; ++y)
		{
			sCell& cell = gamemap.get(x, y);
			cell.item = 0;
			cell.goods = 0;
			cell.road = false;
			cell.discovered = false;
			cell.blessed = false;

			float dist = 0;
			float r = stb_perlin_noise3(x*0.1, y*0.15, 0, 0, 0, 0)*0.5 + 0.7;
			Vector2 point(x, y);
			for (int i = 0; i < num_islands; ++i)
				dist = max( dist, (islands[i].z - point.distance(islands[i].xy)) / islands[i].z);
			float dist_to_start = point.distance(spawn);
			if (dist_to_start < 10) //to avoid building islands on start position
				dist -= dist_to_start / 10.0;
			dist *= pow(max(0.0, 1 - Vector2(x / (float)gamemap.width, y / (float)gamemap.height).distance(Vector2(0.5,0.5)) * 2.0), 0.5); //avoids borders
			r = r * dist * 1.2;
			r = clamp(r, 0.0, 0.999);

			if(r < 0.15)
				cell.terrain = TILE_WATER;
			else if (r < 0.2)
				cell.terrain = TILE_SAND;
			else if (r < 0.6)
				cell.terrain = TILE_GRASS;
			else
				cell.terrain = TILE_ROCK;

			if (cell.terrain == TILE_GRASS )
			{
				if (random() > 0.9)
					cell.item = 5;
				else if (r > 0.5 || random() > 0.9)
					cell.item = uint8(rand() % 2) + 1;
			}
		}

	//plant stuff
	for (int x = 4; x <= gamemap.width - 4; ++x)
		for (int y = 4; y <= gamemap.height - 4; ++y)
		{
			sCell& cell = gamemap.get(x, y);
			sCell& cell_left = gamemap.get(x-1, y);
			sCell& cell_right = gamemap.get(x+1, y);
			if (cell.terrain == TILE_SAND && cell_left.terrain == TILE_WATER && random() > 0.9)
			{
				cell_left.item = ITEM_PORT_WATER;
				cell.item = ITEM_PORT_SAND;
			}
			else if (cell.terrain == TILE_SAND && cell_right.terrain == TILE_WATER && random() > 0.9)
			{
				cell_right.item = ITEM_PORT_WATER;
				cell.item = ITEM_PORT_SAND;
			}
		}
	//create villages
	int num_villages = 10;
	while (num_villages)
	{
		int x = rand() % gamemap.width;
		int y = rand() % gamemap.height;
		sCell& cell = gamemap.get(x, y);
		if (cell.terrain != TILE_GRASS )
			continue;
		cell.terrain = TILE_GRASS;
		cell.item = TILE_HOUSE; //monolith

		//clear radius
		for(int px = x - 5; px <= x + 5; ++px)
			for (int py = y - 5; py <= y + 5; ++py)
				if (Vector2(px, py).distance(Vector2(x, y)) <= 5)
					gamemap.getMirrored(px,py).terrain = TILE_GRASS;

		//build HOUSES
		for (int i = 0; i < 6; ++i)
		{
			sCell& housecell = gamemap.getMirrored(x + rand() % 7 - 3, y + rand() % 7 - 3);
			housecell.terrain = TILE_GRASS;
			if (housecell.item == TILE_HOUSE)
				continue;
			housecell.item = TILE_HOUSE + 1 + rand()%3;
		}

		//place FOUNTAIN
		sCell& watercell = gamemap.getMirrored(x + rand() % 11 - 5, y + rand() % 11 - 5);
		watercell.item = 3;

		//spawn PEOPLE
		for (int i = 0; i < 6; ++i)
		{
			sCell& peoplecell = gamemap.getMirrored(x + rand() % 7 - 3, y + rand() % 7 - 3);
			peoplecell.terrain = TILE_GRASS;
			if (peoplecell.item != 0)
				continue;
			peoplecell.people = rand() % 3;
		}

		num_villages--;
	}
}

void centerCamera(int x, int y, Image& framebuffer)
{
	int margin = 50; //margin in pixels
	int targetx = clamp(x, margin, world.gamemap.width * 16 - margin) - framebuffer.width * 0.5 + 8;
	int targety = clamp(y, margin, world.gamemap.height * 16 - margin) - framebuffer.height * 0.5 + 8;
	campos.x = lerp(campos.x, targetx, 0.1);
	campos.y = lerp(campos.y, targety, 0.1);
}

sCell& getCellWorld(int wx, int wy)
{
	int x = clamp(wx / 16, 0, world.gamemap.width - 1);
	int y = clamp(wy / 16, 0, world.gamemap.height - 1);
	return world.gamemap.get(x, y);
}

void World::computeFamilyLove()
{
	for (int i = 0; i < 3; ++i)
	{
		sCharacter* c = &players[i];
		for (int j = i + 1; j < 3; ++j)
		{
			sCharacter* c2 = &players[j];
			if (c->pos.distance(c2->pos) < 0.001)
				c->love = c2->love = 100;
		}
	}
}

Stage* Stage::current = NULL;
std::map<std::string,Stage*> Stage::stages;

Stage::Stage(const char* name)
{
	this->name = name;
	stages[name] = this;
}

void Stage::changeStage(const char* name)
{
	auto it = stages.find(name);
	if (it == stages.end())
		return;
	if (current == it->second)
		return;
	current = it->second;
	current->enter_time = getTime();
	current->onEnter();
}

PlayStage::PlayStage() : Stage("play")
{
	mode = WALK_MODE;
	selection = 0;
	missing_time = 0;
}

void PlayStage::render(Image& framebuffer)
{
	sCharacter& player = world.players[world.selected_player];
	centerCamera(player.pos.x, player.pos.y, framebuffer); //center camera in player
	framebuffer.fill(Color(77, 127, 161));
	renderMap(framebuffer);
	renderHUD(framebuffer);
}

void PlayStage::renderMap(Image& framebuffer)
{
	Matrix<sCell>& gamemap = world.gamemap;
	Image* tileset = Image::Get("data/tileset.tga");

	int startx = max(1, (campos.x / 16.0));
	int starty = max(1, (campos.y / 16.0));
	int endx = min(gamemap.width - 2, startx + framebuffer.width / 16 + 1);
	int endy = min(gamemap.height - 2, starty + framebuffer.height / 16 + 1);
	for ( int x = startx; x < endx; ++x )
	{
		for (int y = starty; y < endy; ++y)
		{
			//floor
			sCell& cell = gamemap.get(x, y);
			sCell& cell_left = gamemap.get(x - 1, y);
			sCell& cell_top = gamemap.get(x, y - 1);
			sCell& cell_right = gamemap.get(x + 1, y);
			sCell& cell_bottom = gamemap.get(x, y + 1);
			uint8 tile = cell.terrain;
			uint8 tile_left = cell_left.terrain;
			uint8 tile_up = cell_top.terrain;
			uint8 col = tile_left << 2 | tile_up;
			framebuffer.drawImage(*tileset, x * 16 - campos.x, y * 16 - campos.y, Area(16 * col, 16 * tile, 16, 16));
			cell.discovered = true;

			//road
			if (cell.road)
			{
				if (cell_left.road)
					framebuffer.drawImage(*tileset, x * 16 - campos.x, y * 16 - campos.y, Area(16 * 4, 16 * 11, 16, 16));
				if (cell_right.road)
					framebuffer.drawImage(*tileset, x * 16 - campos.x, y * 16 - campos.y, Area(16 * 2, 16 * 11, 16, 16));
				if (cell_top.road)
					framebuffer.drawImage(*tileset, x * 16 - campos.x, y * 16 - campos.y, Area(16 * 3, 16 * 11, 16, 16));
				if (cell_bottom.road)
					framebuffer.drawImage(*tileset, x * 16 - campos.x, y * 16 - campos.y, Area(16 * 1, 16 * 11, 16, 16));
				if (!cell_bottom.road && !cell_top.road && !cell_left.road && !cell_right.road)
					framebuffer.drawImage(*tileset, x * 16 - campos.x, y * 16 - campos.y, Area(0, 16 * 11, 16, 16));
			}

			//item
			if (cell.item != 0)
			{
				if (cell.item >= 128) //houses
				{
					framebuffer.drawImage(*tileset, x * 16 - campos.x, y * 16 - campos.y, Area(16 * (cell.item - 128), 16 * 10, 16, 16));
					if( cell.item == ITEM_WAREHOUSE && cell.goods && blink(2) )
						framebuffer.drawImage(*tileset, x * 16 - campos.x, y * 16 - campos.y - 16, Area( 6 * 16, 12 * 16, 16, 16) );
				}
				else
					framebuffer.drawImage(*tileset, x * 16 - campos.x, y * 16 - campos.y, Area(16 * cell.item, 16 * 4, 16, 16));
			}

			//people
			if (cell.people != 0)
				framebuffer.drawImage(*tileset, x * 16 - campos.x, y * 16 - campos.y, Area(16 * (4 + cell.people), 16 * 5, 16, 16));
		}
	}

	//render players
	for (int i2 = world.selected_player + 1; i2 < world.selected_player + 4; ++i2)
	{
		int i = i2 % 3;//hack to force player is the last one
		sCharacter& player = world.players[i];
		player.draw_pos = lerp(player.draw_pos, player.pos, 0.5);

		sCell& player_cell = gamemap.get(player.pos.x / 16, player.pos.y / 16);
		if (player_cell.terrain == TILE_WATER)
			framebuffer.drawImage(*tileset, player.draw_pos.x - campos.x, player.draw_pos.y - campos.y, Area(12 * 16, 5 * 16 + (player.alive ? 0 : 16), 16, 16));
		else
			framebuffer.drawImage(*tileset, player.draw_pos.x - campos.x, player.draw_pos.y - campos.y, Area(i*16, 16 * 5 + (player.alive ? 0 : 16), 16, 16));
		if ( i == world.selected_player && blink(2) && player.alive)
			framebuffer.drawImage(*tileset, player.draw_pos.x - campos.x, player.draw_pos.y - campos.y - 16, Area(1 * 16, 7 * 16, 16, 16));
	}
}

void PlayStage::renderHUD(Image& framebuffer)
{
	Image* tileset = Image::Get("data/tileset.tga");
	Image* minifont = Image::Get("data/mini-font-white-4x6.tga"); //load bitmap-font image

	int height = 12;
	if (mode == MENU_MODE)
		height = 30;
	int y = framebuffer.height - height;
	sCharacter& player = world.players[world.selected_player];

	framebuffer.drawRectangle(0, y, framebuffer.width, height, Color(69, 46, 22));
	framebuffer.drawLine(0, y, framebuffer.width, y, Color(80, 80, 80));

	if(player.alive)
		framebuffer.drawImage(*tileset, 0, y - 32, Area( player.character * 16, 128, 16, 32) ); //face
	else
		framebuffer.drawImage(*tileset, 0, y - 32, Area(3 * 16, 128, 16, 32)); //face

	sCell& cell = world.gamemap.get(player.pos.x / 16, player.pos.y / 16);
	long now = getTime();
	sUpgrade upgrade = world.getUpgradeInfo( cell.item );

	//action
	if(upgrade.item)
		framebuffer.drawText( upgrade.str, 20, y - 15, *minifont, 4, 6 );
	else if(cell.people)//empty
		framebuffer.drawText("talk", 20, y - 15, *minifont, 4, 6);

	//movements counter
	for (int i = 0; i < player.max_movements; ++i)
	{
		framebuffer.drawRectangle(20 + i * 5, y - 6, 4, 4, Color(180,180,180) );
		framebuffer.drawRectangle(20 + i * 5 + 1, y - 5, 2, 2, i < player.movements ? Color::GREEN : Color::BLACK );
		if( (missing_time - now) > 0 && missing_resources.x > i && blink(5) )
			framebuffer.drawRectangle(20 + i * 5, y - 6, 4, 4, Color::RED);
	}
	if (upgrade.item)
		framebuffer.drawLine(20, y - 8, 20 + upgrade.movements * 5 - 2, y - 8, Color::GREEN );

	//player stats
	float f = 1 - (player.love / 100.0);
	if (player.love || blink(5))
	{
		framebuffer.drawImage(*tileset, 0, y - 2, Area(1 * 16, 12 * 16, 16, 16));
		framebuffer.drawImage(*tileset, 0, y - 2, Area(1 * 16, 13 * 16, 16, 16 * f));
	}
	f = 1 - (player.water / 100.0);
	if (player.water || blink(5))
	{
		framebuffer.drawImage(*tileset, 10, y - 2, Area(0 * 16, 12 * 16, 16, 16));
		framebuffer.drawImage(*tileset, 10, y - 2, Area(0 * 16, 13 * 16, 16, 16 * f));
	}

	//resources
	if ((missing_time - now) < 0 || !missing_resources.y || blink(5))
		framebuffer.drawImage(*tileset, 28, y - 1, Area(3 * 16, 12 * 16, 16, 16));
	if ((missing_time - now) < 0 || !missing_resources.z || blink(5))
		framebuffer.drawImage(*tileset, 60, y - 2, Area(4 * 16, 12 * 16, 16, 16));
	if ((missing_time - now) < 0 || !missing_resources.w || blink(5))
		framebuffer.drawImage(*tileset, 92, y - 2, Area(6 * 16, 12 * 16, 16, 16));
	std::string wood_extra = "";
	if (upgrade.item && upgrade.wood)
		wood_extra = std::string(upgrade.wood > 0 ? "+" : "") + std::to_string(upgrade.wood);
	framebuffer.drawText(std::to_string(player.wood) + wood_extra, 41, y + 4, *minifont, 4, 6);
	std::string stone_extra = "";
	if (upgrade.item && upgrade.stone)
		stone_extra = std::string(upgrade.stone > 0 ? "+" : "") + std::to_string(upgrade.stone);
	framebuffer.drawText(std::to_string(player.stone) + stone_extra, 76, y + 4, *minifont, 4, 6);
	std::string goods_extra = "";
	if (upgrade.item && upgrade.goods)
		goods_extra = std::string(upgrade.goods > 0 ? "+" : "") + std::to_string(upgrade.goods);
	framebuffer.drawText(std::to_string(player.goods) + goods_extra, 106, y + 4, *minifont, 4, 6);

	//menu
	if (mode == MENU_MODE)
	{
		for (int i = 0; i < 2; ++i)
		{
			sCharacter& character = world.players[((world.selected_player + 1 + i) % 3)];
			framebuffer.drawImage(*tileset, 16*i, framebuffer.height - 16, Area(character.alive ? character.character * 16 : 3*16, 138, 16, 16)); //face2
		}
		framebuffer.drawImage(*tileset, 16*2, framebuffer.height - 16, Area(4*16,112,16*2,16)); //icons
		if( int(getTime() * 0.005) % 2 == 0 )
			framebuffer.drawImage(*tileset, selection * 16, framebuffer.height - 16, Area(0, 112, 16, 16)); //icons
	}

	framebuffer.drawRectangle( 0, 0, 30, 8, Color(0, 0, 0, 100) );
	framebuffer.drawText( std::string("Day: ") + std::to_string(world.day + 1), 1, 1, *minifont, 4, 6 );
}

void PlayStage::update(float dt)
{
	sCharacter& player = world.players[ world.selected_player ];
	uint8 action = NO_ACTION;
	uint8 param = 0;

	if (mode == WALK_MODE)
	{
		if (Input::wasKeyPressed(SDL_SCANCODE_UP)) //if key up
		{
			action = WALK;
			param = NORTH;
		}
		if (Input::wasKeyPressed(SDL_SCANCODE_DOWN)) //if key down
		{
			action = WALK;
			param = SOUTH;
		}
		if (Input::wasKeyPressed(SDL_SCANCODE_LEFT)) //if key up
		{
			action = WALK;
			param = WEST;
		}
		if (Input::wasKeyPressed(SDL_SCANCODE_RIGHT)) //if key down
		{
			action = WALK;
			param = EAST;
		}
		if (Input::wasKeyPressed(SDL_SCANCODE_A) || Input::gamepads[0].wasButtonPressed(A_BUTTON)) //open menu
		{
			action = INTERACT;
		}
		if (Input::wasKeyPressed(SDL_SCANCODE_Z) || Input::gamepads[0].wasButtonPressed(B_BUTTON)) //next player
		{
			//world.phase = (world.phase + 1) % 3;
			mode = MENU_MODE;
		}
	}
	else if (mode == MENU_MODE)
	{
		if (Input::wasKeyPressed(SDL_SCANCODE_LEFT)) //if key up
		{
			if(selection > 0)
				selection -= 1;
		}
		if (Input::wasKeyPressed(SDL_SCANCODE_RIGHT)) //if key down
		{
			if(selection < 3)
				selection += 1;
		}
		if (Input::wasKeyPressed(SDL_SCANCODE_Z)) //back
		{
			mode = WALK_MODE;
			selection = 0;
		}
		if (Input::wasKeyPressed(SDL_SCANCODE_A) || Input::gamepads[0].wasButtonPressed(A_BUTTON)) //select
		{
			//do action
			if (selection == 0 || selection == 1) //change character
			{
				world.selected_player = (world.selected_player + 1 + selection) % 3;
				mode = WALK_MODE;
				selection = 0;
			}
			else if (selection == 2)
			{
				Stage::changeStage("map");
			}
			else if (selection == 3)
			{
				mode = WALK_MODE;
				selection = 0;
				Stage::changeStage("turn");
			}
		}
	}

	if (action && player.movements == 0 && !world.unlimited_movements )
		action = NO_ACTION;

	if (action == WALK)
	{
		player.prev_pos = player.pos;
		switch (param)
		{
			case NORTH: player.pos.y -= 16; break;
			case SOUTH: player.pos.y += 16; break;
			case EAST: player.pos.x += 16; break;
			case WEST: player.pos.x -= 16; break;
		}

		//adjust
		player.pos.x = clamp(player.pos.x, 0, (world.gamemap.width - 1) * 16);
		player.pos.y = clamp(player.pos.y, 0, (world.gamemap.height - 1) * 16);
		sCell& targetcell = getCellWorld(player.pos.x, player.pos.y);
		if (targetcell.terrain == TILE_ROCK)
			player.pos = player.prev_pos;
		else if(!world.unlimited_movements)
			player.movements--;

		//passive actions
		sCell& finalcell = getCellWorld(player.pos.x, player.pos.y);
		if (finalcell.item == ITEM_FOUNTAIN || finalcell.item == ITEM_WELL)
			player.water = 100;
		if (finalcell.goods)
		{
			player.goods = min(10, player.goods + finalcell.goods);
			finalcell.goods = 0;
		}
		world.computeFamilyLove();
	}

	if (action == INTERACT)
	{
		sCell& finalcell = getCellWorld(player.pos.x, player.pos.y);
		if(finalcell.item)
			world.upgradeCell( &player, player.pos.x / 16, player.pos.y / 16);
		else if (finalcell.people)
			Stage::changeStage("talk");
	}

	//DEBUG STUFF
	sCell& cell = getCellWorld( player.pos.x, player.pos.y );
	if (Input::wasKeyPressed(SDL_SCANCODE_1))
		world.selected_player = 0;
	if (Input::wasKeyPressed(SDL_SCANCODE_2))
		world.selected_player = 1;
	if (Input::wasKeyPressed(SDL_SCANCODE_3))
		world.selected_player = 2;
	if (Input::wasKeyPressed(SDL_SCANCODE_C))
		cell.item = 12;
	if (Input::wasKeyPressed(SDL_SCANCODE_R))
		cell.road = !cell.road;
	if (Input::wasKeyPressed(SDL_SCANCODE_T))
		world.restart();
	if (Input::wasKeyPressed(SDL_SCANCODE_N))
		Stage::changeStage("turn");
	if (Input::wasKeyPressed(SDL_SCANCODE_M))
		Stage::changeStage("map");
	if (Input::wasKeyPressed(SDL_SCANCODE_I))
		world.unlimited_movements = !world.unlimited_movements;
	if (Input::wasKeyPressed(SDL_SCANCODE_7)) 
		cell.terrain = 0;
	if (Input::wasKeyPressed(SDL_SCANCODE_8)) 
		cell.terrain = 1;
	if (Input::wasKeyPressed(SDL_SCANCODE_9)) 
		cell.terrain = 2;
	if (Input::wasKeyPressed(SDL_SCANCODE_0)) 
		cell.terrain = 3;
	if (Input::wasKeyPressed(SDL_SCANCODE_PAGEDOWN))
		world.passTurn();
	if (Input::wasKeyPressed(SDL_SCANCODE_INSERT))
		player.wood += 1;
	if (Input::wasKeyPressed(SDL_SCANCODE_HOME))
		player.stone += 1;
	if (Input::wasKeyPressed(SDL_SCANCODE_PAGEUP))
		player.goods += 1;
	if (Input::wasKeyPressed(SDL_SCANCODE_DELETE))
	{
		player.max_movements = player.movements = 0;
		player.alive = false;
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_END))
		Stage::changeStage("ending");
}


TalkStage::TalkStage() : Stage("talk")
{
	agreement = 1;
	sentence = 0;
}

void TalkStage::onEnter()
{
	sentence = 0;
}

void TalkStage::render(Image& framebuffer)
{
	stages["play"]->render(framebuffer);

	Image* tileset = Image::Get("data/tileset.tga");
	int y = framebuffer.height - 48;

	framebuffer.drawRectangle(0, y, 128, framebuffer.height - y,Color(80,80,80));
	framebuffer.drawLine(0, y, 128, y, Color(70, 70, 70));

	framebuffer.drawImage(*tileset, 0, y - 32, Area(world.selected_player * 16, 128, 16, 32)); //you
	framebuffer.drawImage(*tileset, 128 - 16, y - 32, Area((5 + agreement) * 16, 128, 16, 32)); //him

	const char* sentences[] = { 
		"Hello friend,\nHave you heard\nabout our lord\nand savior?",
		"Sure, and you\nabout our mother\nEarth?",
		"Oh, I guess...\nBut is your\nsoul saved?",
		"Can you elaborate?",
		"Pray my Lord",
		"I will then."
	};

	if (sentence < 6)
		framebuffer.drawText( sentences[sentence], 2, y + 4, sentence %2 == 0 ? *font : *minifont);
	else
		Stage::changeStage("play");
}

void TalkStage::update(float dt)
{
	if (Input::wasKeyPressed(SDL_SCANCODE_A)) 
		sentence++;
	if (Input::wasKeyPressed(SDL_SCANCODE_Z))
		Stage::changeStage("play");
}

NextTurnStage::NextTurnStage() : Stage("turn")
{
}

void NextTurnStage::onEnter()
{
	world.passTurn();
}

void NextTurnStage::render(Image& framebuffer)
{
	framebuffer.fill(Color(0, 0, 0));

	Image* tileset = Image::Get("data/tileset.tga");
	float elapsed = (getTime() - enter_time) * 0.001;

	framebuffer.drawText("Next turn", 32, 44, *font);

	std::string str = std::string("Day ") + std::to_string( world.day + 1 );
	framebuffer.drawText(str, 43, 58, *font);

	framebuffer.drawText("Souls saved", 23, 81, *font);
	framebuffer.drawText(std::to_string(world.souls_saved), 56, 94, *font);
	framebuffer.drawImage(*tileset, 40, 90, Area(2 * 16, 12 * 16, 16, 16));

	if (elapsed > 4) //autopass
		Stage::changeStage("play");
}

void NextTurnStage::update(float dt)
{
	if (Input::wasKeyPressed(SDL_SCANCODE_A)) 
	{
		world.passTurn();
		Stage::changeStage("play");
	}
}

MapStage::MapStage() : Stage("map")
{
	show_blessing = false;
}

void MapStage::render(Image& framebuffer)
{
	framebuffer.fill(Color(209,208,190));
	static const Color colors[4] = { {94,125,159,255}, {197,191,154,255 },{ 116,140,98,255 },{ 125,125,125,255 } };
	for(int x = 0; x < framebuffer.width; ++x)
		for (int y = 0; y < framebuffer.height; ++y)
		{
			sCell& cell = world.gamemap.get(clamp(x, 0, world.gamemap.width), clamp(y, 0, world.gamemap.height));
			if (!cell.discovered && world.map_fog)
				continue;
			Color c = colors[cell.terrain % 4];
			if (cell.item)
			{
				if (cell.item < 3) //tree
					c *= 0.8;
				else if (cell.item == 5) //rock
					c.set(125, 125, 125);
				else if (cell.item == ITEM_FOUNTAIN || cell.item == ITEM_WELL) //water
					c.set(50, 255, 255);
				else if (cell.item == 128 ) //church
					c.set(255,255,100);
				else if (cell.item > 128) //house
					c.set(200, 100, 50);
			}

			if (show_blessing && cell.terrain != TILE_WATER)
				c = cell.blessed ? Color(255, 255, 0) : Color(50, 50, 50);
			framebuffer.setPixel(x,y,c);
		}

	for (int i = 0; i < 3; ++i)
	{
		sCharacter& player = world.players[i];
		framebuffer.setPixelSafe(player.pos.x / 16, player.pos.y / 16, Color(rand(),rand(),rand()));
	}
}

void MapStage::update(float dt)
{
	if (Input::wasKeyPressed(SDL_SCANCODE_Z) || Input::wasKeyPressed(SDL_SCANCODE_M))
		Stage::changeStage("play");
	if (Input::wasKeyPressed(SDL_SCANCODE_R)) //regenerates the map
		world.restart();
	if (Input::wasKeyPressed(SDL_SCANCODE_F)) //shows all map
		world.map_fog = !world.map_fog;
	if (Input::wasKeyPressed(SDL_SCANCODE_B) || Input::wasKeyPressed(SDL_SCANCODE_A))
		show_blessing = !show_blessing;
}

Vector2ub wave_points[10]; //used to show moving waves in random positions

IntroStage::IntroStage() : Stage("intro")
{
	for (int i = 0; i < 10; ++i)
		wave_points[i].set(rand() % 128, rand() % 40 + 5);
}

void IntroStage::render(Image& framebuffer)
{
	Image* tileset = Image::Get("data/tileset.tga");
	float elapsed = (getTime() - enter_time) * 0.001;
	int horizon = framebuffer.height*0.6;

	framebuffer.drawRectangle(0, 0,framebuffer.width, horizon, Color(160,175,191));
	framebuffer.drawRectangle(0, horizon, framebuffer.width, framebuffer.height, Color(94, 125, 159));

	int offset = min(0, elapsed * 10 - 40);
	framebuffer.drawImage( *tileset, offset + elapsed * 2, 48, Area(128, 208, 128, 16)); //clouds
	framebuffer.drawImage(*tileset, offset + elapsed * 2 - 128, 48, Area(128, 208, 128, 16)); //clouds
	framebuffer.drawImage( *tileset, offset, horizon - 16, Area(128,224,128,32)); //island

	framebuffer.drawImage(*tileset, 1, min(10,elapsed * 20 - 50), Area(0, 224, 128, 32)); //title

	if (elapsed > 3 && blink(3))
		framebuffer.drawText("Press a button", 10, framebuffer.height - 20, *font);

	for (int i = 0; i < 10; ++i)
	{
		float f = sin(elapsed*0.8 + i*10) * 5;
		if (f > 0)
			framebuffer.drawLine(offset + wave_points[i].x - f, horizon + wave_points[i].y, offset + wave_points[i].x + f, horizon + wave_points[i].y, Color(174, 187, 202));
	}
}

void IntroStage::update(float dt)
{
	if (Input::wasKeyPressed(SDL_SCANCODE_A) || Input::wasKeyPressed(SDL_SCANCODE_Z))
		Stage::changeStage("tutorial");
}

TutorialStage::TutorialStage() : Stage("tutorial")
{
}

void TutorialStage::render(Image& framebuffer)
{
	Image* tileset = Image::Get("data/tileset.tga");
	float elapsed = (getTime() - enter_time) * 0.001;

	framebuffer.fill(Color::BLACK);

	int x = min(elapsed * 50 - 30,10);
	int y = framebuffer.height * 0.5;
	framebuffer.drawImage(*tileset, x + 0, y, Area(0 * 16, 128, 16, 32));
	framebuffer.drawImage(*tileset, x + 16, y, Area(1 * 16, 128, 16, 32));
	framebuffer.drawImage(*tileset, x + 32, y, Area(2 * 16, 128, 16, 32));

	x = 110 - min((elapsed - 1) * 50 - 30, 10);
	framebuffer.drawImage(*tileset, x, y, Area( (14 + (int(getTime()*0.003)%2) )* 16, 128, 16, 32));

	const char* text[] = {
		"You must go to\nthe new world\nand save their\nsouls.",
		"Build churches\nconvert the\nnatives.\nBring glory\nto our Lord.",
		"You have one\nyear.\nGood luck.\n"
	};

	if(elapsed < 5 && elapsed > 2)
		framebuffer.drawText(text[0], 10, 10, *font);
	else if (elapsed < 9 && elapsed > 5.5)
		framebuffer.drawText(text[1], 10, 10, *font);
	else if (elapsed < 15 && elapsed > 10)
		framebuffer.drawText(text[2], 10, 10, *font);
	else if (elapsed > 16)
		Stage::changeStage("play");
}

void TutorialStage::update(float dt)
{
	if (Input::wasKeyPressed(SDL_SCANCODE_A) || Input::wasKeyPressed(SDL_SCANCODE_Z)) 
		Stage::changeStage("play");
}

EndingStage::EndingStage() : Stage("ending")
{
}

void EndingStage::render(Image& framebuffer)
{
	Image* tileset = Image::Get("data/tileset.tga");
	float elapsed = (getTime() - enter_time) * 0.001;

	framebuffer.fill(Color::BLACK);

	int x = 20;
	int y = framebuffer.height * 0.5;

	if (world.alive_players == 0)
		framebuffer.drawText("All your family\nmembers are\ndead.", 10, 10, *font);
	else if (world.day >= 365)
		framebuffer.drawText("The year has passed.", 10, 10, *font);
	else
		framebuffer.drawText("You resigned.", 10, 10, *font);

	std::string str = std::string("Souls saved\n") + std::to_string(world.souls_saved);
	framebuffer.drawText(str, 23, 81, *font);
	framebuffer.drawImage(*tileset, 2, 76, Area(2 * 16, 12 * 16, 16, 16));
}

void EndingStage::update(float dt)
{
	float elapsed = (getTime() - enter_time) * 0.001;

	if (elapsed > 2 && (Input::wasKeyPressed(SDL_SCANCODE_A) || Input::wasKeyPressed(SDL_SCANCODE_Z))) 
		Stage::changeStage("intro");
}