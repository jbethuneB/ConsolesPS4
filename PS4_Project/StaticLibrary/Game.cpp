#include "Common.hpp"
#include "Game.hpp"

using namespace GameAAA;

GameData *GameAAA::CreateGameData()
{
	return new GameData();
}

RenderCommands GameAAA::Update(Input const &input, GameData &gameData)
{
	RenderCommands result = {};

	// ....
	
	return result;
}

void GameAAA::DestroyGameData(GameData* gameData)
{
	delete gameData;
}

void GameAAA::InitializeBalls_Game(GameData* gameData)
{
	gameData->Initialize();
}

RenderCommands GameAAA::Update_Game(GameData* gameData, Input &input)
{
	gameData->UpdateManageInput(input);
	gameData->UpdateBalls(input);
	gameData->ManageCollisions();

	//retornar llista d'instruccions per pintar
	return gameData->ReturnDrawableElements();
}

glm::vec2 GameAAA::GameData::PosAimingCercle()
{
	float x = bolas[0].x + (glm::cos(currentAimAngle) * distanceToCenterBall);
	float y = bolas[0].y + (glm::sin(currentAimAngle) * distanceToCenterBall);

	return glm::vec2(x, y);
}

void GameAAA::GameData::Initialize()
{
	bola b;
	bolas.push_back(b);
	bolas.push_back(b);
	bolas.push_back(b);
	bolas.push_back(b);
	bolas.push_back(b);

	float helper = -0.25;
	for (int i = 0; i < 5; i++)
	{
		bolas[i].radius = 0.1;
		bolas[i].x = helper;
		bolas[i].y = 0;
		bolas[i].vel = glm::vec2(0, 0);
		bolas[i].force = glm::vec2(20, 20);

		helper += 0.25;
	}

	//initialize player's ball
	bolas[0].x = -0.75;
	bolas[0].y = 0;
	bolas[0].force = glm::vec2(0, 0);
}

/*void GameAAA::GameData::InitializeBolas()
{
}*/

void GameAAA::GameData::UpdateBalls(Input & input)
{
	for (auto &bola : bolas)
	{
		glm::vec2 oldPos(bola.x, bola.y);
		glm::vec2 oldVel(bola.vel);

		bola.force *= 0.98;

		glm::vec2 newVel;
		newVel = bola.force * input.dt;

		glm::vec2 newPos;
		newPos = oldPos + newVel * input.dt + bola.force * (0.5f * input.dt * input.dt);


		//ha colisionat amb paret?
		if (newPos.x > 1 || newPos.x < -1)
		{
			bola.force.x *= -1;
		}
		if (newPos.y > 1 || newPos.y < -1)
		{
			bola.force.y *= -1;
		}


		bola.vel = newVel;

		bola.x = newPos.x;
		bola.y = newPos.y;

	}
}

void GameAAA::GameData::UpdateManageInput(Input & input)
{
	switch (input.aim)
	{
	case -1:
		currentAimAngle -= 0.1;
		break;
	case 1:
		currentAimAngle += 0.1;
		break;
	}

	switch (input.force)
	{
	case -1:
		currentForceRects -= 1;
		break;
	case 1:
		currentForceRects += 1;
		break;
	}

	if (input.shoot == 1)
	{
		ShootBall();
	}
}

void GameAAA::GameData::ShootBall()
{
	//cuanto le corresponde a la X y a la Y
	float xDif = PosAimingCercle().x - bolas[0].x;
	float yDif = PosAimingCercle().y - bolas[0].y;

	float percentageX = xDif / distanceToCenterBall;
	float percentageY = yDif / distanceToCenterBall;

	//multiplicamos por cantidad de fuerza tambien
	bolas[0].force.x = shootForce * percentageX * currentForceRects;
	bolas[0].force.y = shootForce * percentageY * currentForceRects;

}

void GameAAA::GameData::ManageCollisions()
{
	struct Extreme {
		char minOrMax;// i = min    a = max
		int whosIsIt;
		float xPosition;
	};

	std::vector <bola> bolasConPosibleColision; //las ordenaremos de 2 en 2. La bola 0 y 1 de este arrey tendran una colision. La bola 2 y 3 también.
	std::vector <bool> hasThisBallHadACollision(bolas.size(), false); //es un array de bools. A cada pelota le pertoca un bool. Nos ayudará al hacer el loop después.
	std::vector <Extreme> extremes;

	int i = 0;

	//we fill the "extremes" array
	for (auto &bola : bolas)
	{
		Extreme e;
		e.minOrMax = 'i';
		e.whosIsIt = i;
		e.xPosition = bola.GetMin();

		extremes.push_back(e);

		e.minOrMax = 'a';
		e.whosIsIt = i;
		e.xPosition = bola.GetMax();

		extremes.push_back(e);

		i++;
	}

	//we sort the "extremes" vector
	for (int i = (extremes.size() - 1); i >= 0; i--)
	{
		for (int j = 1; j <= i; j++)
		{
			if (extremes[j - 1].xPosition > extremes[j].xPosition)
			{
				Extreme temp = extremes[j - 1];
				extremes[j - 1] = extremes[j];
				extremes[j] = temp;
			}
		}
	}
}

RenderCommands GameAAA::GameData::ReturnDrawableElements()
{
	//se ejecuta cada Update
	RenderCommands renderCommands;
	for (auto bola : bolas)
	{
		DrawableElement de;
		de.whatKind = 0;
		de.x = bola.x;
		de.y = bola.y;
		de.radi = bola.radius;

		renderCommands.elements.push_back(de);
	}

	//AIM CIRCLE
	DrawableElement de;
	de.whatKind = 0; //es un circulo
	de.x = PosAimingCercle().x;
	de.y = PosAimingCercle().y;
	de.radi = radiusAimBall;
	renderCommands.elements.push_back(de);

	//FORCE RECTANGLES
	float helperYvalue = -1.0;
	float heightForEachRect = 2.0 / forceRects;
	float leftVertexX = -0.9;
	float rightVertexX = -0.8;
	for (int i = 0; i < currentForceRects; i++)
	{
		float yvalueForBottomVertexOfRect = helperYvalue;
		helperYvalue += heightForEachRect;
		float yvalueForTopVertexOfRect = helperYvalue;

		de.whatKind = 1; //es un rectangulo
		de.x = leftVertexX;
		de.x2 = rightVertexX;
		de.y = yvalueForTopVertexOfRect;
		de.y2 = yvalueForBottomVertexOfRect;
		renderCommands.elements.push_back(de);
	}

	

	return renderCommands;
}
