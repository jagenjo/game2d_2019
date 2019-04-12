#pragma once

#include "image.h"

enum { TILE_WATER = 0, TILE_SAND, TILE_GRASS, TILE_ROCK };
enum { TILE_HOUSE = 128 };

//items that could be found in a cell, sorted by sprite position in spritesheet
enum {  ITEM_NOTHING = 0,
		ITEM_TREE, ITEM_TREE2, 
		ITEM_FOUNTAIN, ITEM_WELL, 
		ITEM_ROCKS, ITEM_NOT_USED, 
		ITEM_LOGS, ITEM_FOUNDATIONS, 
		ITEM_CROPS, ITEM_WHEAT, 
		ITEM_BURIAL, ITEM_GRAVESTONE, 
		ITEM_PORT_WATER, ITEM_PORT_SAND,
		ITEM_HARBOUR = 137,
		ITEM_WAREHOUSE = 138
};

enum {
	BOTTOM = 1,
	RIGHT = 2,
	TOP = 4,
	LEFT = 8,
};

struct sCell {
	uint8 terrain;	//type of terrain (water,sand,grass,rock)
	uint8 item;		//item (could be trees, objects, houses)
	bool road;		//does it have a road?
	bool discovered; //has been discovered by the player?
	bool blessed;	//is inside a church radius
	uint8 people;	//people living here
	uint8 goods;	//how many goods stored (used in ports)
};

struct sCharacter {
	uint8 character;
	bool alive;

	Vector2 pos;
	Vector2 prev_pos;
	Vector2 draw_pos;

	uint8 movements;
	uint8 max_movements;

	uint8 love;
	uint8 water;

	int8 wood;
	int8 stone;
	int8 goods;
};

struct sUpgrade {
	short item;
	short next_item;
	short movements;
	short wood;
	short stone;
	short goods;
	const char* str;
};

class World {
public:
	static World* instance;
	sCharacter players[3];
	Matrix<sCell> gamemap;
	uint8 day;
	uint8 selected_player;
	uint8 alive_players;
	int souls_saved;

	//debug
	bool map_fog;
	bool unlimited_movements;

	World();
	void restart();
	void generateMap();
	void passTurn();

	void computeFamilyLove();
	sUpgrade getUpgradeInfo(int item);
	void upgradeCell(sCharacter* author, int x, int y);
	void blessArea(int x, int y, int radius); //increases bleassing area
};

class Stage {
public:
	static Stage* current;
	static std::map<std::string, Stage*> stages;

	std::string name;
	long enter_time;

	Stage(const char* name);
	virtual void render(Image& framebuffer) {}
	virtual void update(float dt) {}
	virtual void onEnter() {}

	static void changeStage(const char* name);
};


enum PLAY_MODES {
	WALK_MODE = 0,
	MENU_MODE
};

enum {
	NORTH = 1,
	EAST = 2,
	SOUTH = 3,
	WEST = 4,
};

enum ACTIONS {
	NO_ACTION = 0,
	WALK,
	INTERACT
};


class PlayStage : public Stage {
public:
	PlayStage();
	virtual void render(Image& framebuffer);
	virtual void update(float dt);

	int mode;
	int selection;

	long missing_time;
	Vector4 missing_resources; //movements,wood,stone,goods

	void renderMap(Image& framebuffer);
	void renderHUD(Image& framebuffer);
};

class NextTurnStage : public Stage {
public:
	NextTurnStage();

	virtual void onEnter();
	virtual void render(Image& framebuffer);
	virtual void update(float dt);
};

class TalkStage : public Stage {
public:
	TalkStage();
	uint8 agreement;
	uint8 sentence;
	virtual void onEnter();
	virtual void render(Image& framebuffer);
	virtual void update(float dt);
};

class MapStage : public Stage {
public:
	MapStage();
	bool show_blessing;

	virtual void render(Image& framebuffer);
	virtual void update(float dt);
};

class IntroStage : public Stage {
public:
	IntroStage();

	virtual void render(Image& framebuffer);
	virtual void update(float dt);
};

class TutorialStage : public Stage {
public:
	TutorialStage();

	virtual void render(Image& framebuffer);
	virtual void update(float dt);
};

class EndingStage : public Stage {
public:
	EndingStage();

	virtual void render(Image& framebuffer);
	virtual void update(float dt);
};

