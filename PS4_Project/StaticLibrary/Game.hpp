#pragma once
#include "Common.hpp"
#include "glm\glm.hpp"
namespace GameAAA {

	struct GameData
	{
		struct bola
		{
			float x;
			float y;
			float radius;
			glm::vec2 vel;
			glm::vec2 force;

			float GetMin() {
				return x - radius;
			}
			float GetMax() {
				return x + radius;
			}
		};

		int forceRects = 10; //numero maximo de fuerza de shoot
		int currentForceRects = 0;

		//aim
		float currentAimAngle = 0;
		float distanceToCenterBall = 0.25;
		float radiusAimBall = 0.02;
		glm::vec2 PosAimingCercle();

		//game stuff
		void Initialize();
		const float shootForce = 10.0f;
		std::vector<bola> bolas;
		//void InitializeBolas();
		void UpdateBalls(Input &input);
		void UpdateManageInput(Input & input);
		void ShootBall();
		void ManageCollisions();

		//Convert GameData into DrawElements Function

		RenderCommands ReturnDrawableElements();

		float positionX, positionY;
		// ...
	};
}