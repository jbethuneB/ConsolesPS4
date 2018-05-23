#pragma once
#include <cinttypes>
#include <vector>

namespace GameAAA {
	struct GameData;

	struct Input
	{
		int aim = 0;
		int force = 0;
		int shoot = 0;
		float dt = 0;
	};

	struct DrawableElement
	{
		int whatKind; //0 is Circle // 1 is Rect
		float x, y, x2, y2, radi;
	};

	struct RenderCommands
	{
		/*
		struct Sprite
		{
			float x, y, width, height, rotation;
			enum {
				BALL,POINT_1,POINT_2,PLAYER
			} image;
		};*/

		
		/*
		uint32_t orthoWidth, orthoHeight;
		std::vector<Sprite> sprites;*/

		std::vector<DrawableElement> elements;
	};

	

	GameData *CreateGameData();
	RenderCommands Update(Input const &input, GameData &gameData);
	void DestroyGameData(GameData* gameData);
	
	//functions that access Game
	void InitializeBalls_Game(GameData * gameData);
	RenderCommands Update_Game(GameData * gameData, Input &input);
	
}