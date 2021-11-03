#include <iostream>
#include <vector>
#include <math.h>
#include <cstdlib>
#include <string>
#include "raylib.h"

#define PI 3.14159265358979323846


void log(std::string msg)
{
	std::cout << msg << std::endl;
}



class PhysicsObject {
	public:
		void init(int cord_x, int cord_y, float vel_x, float vel_y, float acc_x, float acc_y)
		{
			cordinates.x = cord_x;
			cordinates.y = cord_y;
			
			velocity.x = vel_x;
			velocity.y = vel_y;

			acceleration.x = acc_x;
			acceleration.y = acc_y;
		}
		
		void updateValues(float deltaTime)
		{
			velocity.x += acceleration.x * deltaTime;
			velocity.y += acceleration.y * deltaTime;

			cordinates.x += velocity.x*deltaTime;
			cordinates.y += velocity.y*deltaTime;
		}
		
		void setCordinates(Vector2 newPosition)
		{
			cordinates = newPosition;
		}

		void setVelocity(Vector2 newVelocity)
		{
			velocity = newVelocity;
		}

		void setOldVelocity(Vector2 velocity)
		{
			oldVelocity = velocity;
		}

		Vector2 getCordinates()
		{
			return cordinates;
		}

		Vector2 getVelocity()
		{
			return velocity;
		}

		Vector2 getOldVelocity()
		{
			return oldVelocity;
		}

		void clampSpeed()
		{
			if (pow(velocity.x, 2) + pow(velocity.y, 2) < 0.01) { velocity.x = 0; velocity.y = 0; }
		}

		void setTimeRemaining(float time)
		{
			timeRemaining = time;
		}

		float getTimeRemaining()
		{
			return timeRemaining;
		}

		virtual int getRadius() = 0;
		virtual void getDrag(float g) = 0;
		virtual void drawObject() = 0;
		virtual void checkBounds(int screenWidth, int screenHeight) = 0;
		virtual void isClicked(Vector2 mousePosition) = 0;
		virtual void followMouse(Vector2 mousePosition) = 0;

		bool clicked;

	protected: 
		Vector2 cordinates, velocity, acceleration, oldVelocity;
		float timeRemaining;

};

class Circle : public PhysicsObject
{
	public:
		void setRadius(int r)
		{
			radius = r;
		}

		void drawObject() override
		{
			DrawCircleV(cordinates, radius, WHITE);
		}

		void getDrag(float g) override
		{	
			if (velocity.x > 0){ acceleration.x = -dragCoefficient * velocity.x * velocity.x * pow(radius,0.5); }
			else { acceleration.x = dragCoefficient * velocity.x * velocity.x * pow(radius, 0.5); }

			if (velocity.y > 0) { acceleration.y = g - dragCoefficient * velocity.y * velocity.y * pow(radius, 0.5); }
			else { acceleration.y = g + dragCoefficient * velocity.y * velocity.y * pow(radius, 0.5); }
		}

		void checkBounds(int screenWidth, int screenHeight) override
		{
			if (cordinates.x < radius) { cordinates.x = radius; velocity.x *= -0.8; }
			else if (cordinates.x + radius > screenWidth) { cordinates.x = screenWidth - radius; velocity.x *= -0.8; }
			else if (cordinates.y < radius) { cordinates.y = radius; velocity.y *= -0.8; }
			else if (cordinates.y + radius > screenHeight) { cordinates.y = screenHeight - radius; velocity.y *= -0.8; }
		}

		void isClicked(Vector2 mousePosition) override
		{	
			if (pow(cordinates.x - mousePosition.x ,2) + pow(cordinates.y - mousePosition.y, 2) < pow(radius,2))
			{	
				
				clicked =  true;
				velocity.x = 0;
				velocity.y = 0;
			}
			else 
			{	
				clicked = false;
			}
		}

		void followMouse(Vector2 mousePosition) override
		{
			velocity.x = 6*(mousePosition.x - cordinates.x);
			velocity.y = 6*(mousePosition.y - cordinates.y);
			DrawLineV(mousePosition, cordinates, RED);
		}

		int getRadius() override
		{
			return radius;
		}

	private:
		int radius;
		float dragCoefficient = 0.00008;


};

bool circleVScircle(PhysicsObject* c1, PhysicsObject* c2)
{
	Vector2 c1Cordinates = c1->getCordinates();
	Vector2 c2Cordinates = c2->getCordinates();
	int c1Radius = c1->getRadius();
	int c2Radius = c2->getRadius();

	return (pow(c1Cordinates.x - c2Cordinates.x, 2) + pow(c1Cordinates.y - c2Cordinates.y, 2) < pow(c1Radius + c2Radius, 2));
}

void DrawNumObjects(const std::vector<PhysicsObject*>& physicsObjectList)
{
	std::string s = std::to_string(physicsObjectList.size());
	char const* pchar = s.c_str();  //use char const* as target type
	DrawText(pchar, 0, 20, 20, LIGHTGRAY);
}


auto checkColisions(const std::vector<PhysicsObject*>& physicsObjectList)
{
	std::vector<std::pair<PhysicsObject*, PhysicsObject*>> colidingObjects;

	for (int i = 0; i < physicsObjectList.size(); i++) 
	{
		for (int q = i+1; q < physicsObjectList.size(); q++) 
		{
			if(circleVScircle(physicsObjectList[i], physicsObjectList[q]) == true)
			{	
				colidingObjects.push_back({ physicsObjectList[i], physicsObjectList[q] });
			}
		}
	}
	return colidingObjects;
}

void resolveColisions(const std::vector<std::pair<PhysicsObject*, PhysicsObject*>>& colidingObjects)
{
	float distance, intersectDepth, m1, m2;
	Vector2 cordinates1, cordinates2, newCordinates1, newCordinates2, velocity1, velocity2, newVelocity1, newVelocity2;

	for(int i = 0; i < colidingObjects.size(); i++)
	{
		cordinates1 = colidingObjects[i].first->getCordinates();
		cordinates2 = colidingObjects[i].second->getCordinates();

		distance = sqrt(pow(cordinates1.x - cordinates2.x, 2) + pow(cordinates1.y - cordinates2.y, 2));

		intersectDepth = 0.5f * (colidingObjects[i].first->getRadius() + colidingObjects[i].second->getRadius() - distance);

		newCordinates1.x = cordinates1.x + intersectDepth * (cordinates1.x - cordinates2.x) / distance;
		newCordinates1.y = cordinates1.y + intersectDepth * (cordinates1.y - cordinates2.y) / distance;

		newCordinates2.x = cordinates2.x - intersectDepth * (cordinates1.x - cordinates2.x) / distance;
		newCordinates2.y = cordinates2.y - intersectDepth * (cordinates1.y - cordinates2.y) / distance;

		colidingObjects[i].first->setCordinates(newCordinates1);
		colidingObjects[i].second->setCordinates(newCordinates2);

		DrawLineV(newCordinates1, newCordinates2, BLUE);

		velocity1 = colidingObjects[i].first->getVelocity();
		velocity2 = colidingObjects[i].second->getVelocity();

		m1 = PI * pow(colidingObjects[i].first->getRadius(), 2);
		m2 = PI * pow(colidingObjects[i].second->getRadius(), 2);

		float nx = (cordinates2.x - cordinates1.x) / distance;
		float ny = (cordinates2.y - cordinates1.y) / distance;


		float kx = velocity1.x - velocity2.x;
		float ky = velocity1.y - velocity2.y;
		float p = 2.0 * (nx * kx + ny * ky) / (m1 + m2);

		//No energy loss as to prevent microbounces depleting it all.
		newVelocity1.x = velocity1.x - p * m2 * nx;
		newVelocity1.y = velocity1.y - p * m2 * ny;

		newVelocity2.x = velocity2.x + p * m1 * nx;
		newVelocity2.y = velocity2.y + p * m1 * ny;

		colidingObjects[i].first->setVelocity(newVelocity1);
		colidingObjects[i].second->setVelocity(newVelocity2);
	}
}

int main() {

	const int screenWidth = 1024;
	const int screenHeight = 768;

	const int SimulationUpdates = 4;
	const int maxSimulationSteps = 15;
	const float g = 9.82f*30;


	std::vector<PhysicsObject*> physicsObjectList;
	std::vector<std::pair<PhysicsObject*, PhysicsObject*>> colidingObjects;
	
	Vector2 mousePosition;

	InitWindow(screenWidth, screenHeight, "Physics engine v.1");
	SetTargetFPS(9999);

	while (WindowShouldClose() == false) {

		mousePosition = GetMousePosition();

		if (IsKeyPressed(KEY_SPACE))
		{
			Circle* ball = new Circle();
			ball->init(mousePosition.x, mousePosition.y, 0, 0, 0, g);
			ball->setRadius(132);
			physicsObjectList.push_back(ball);

		}

		if (IsKeyDown(KEY_P))
		{
			Circle* ball = new Circle();
			ball->init(mousePosition.x, mousePosition.y, 0, 0, 0, g);
			ball->setRadius(2);
			physicsObjectList.push_back(ball);

		}

		if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) 
		{
			physicsObjectList.clear();
		
		}

		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		{
			mousePosition = GetMousePosition();
			for (int i = 0; i <physicsObjectList.size(); i++)
			{
				if (physicsObjectList[i]->clicked == false) 
				{
				physicsObjectList[i]->isClicked(mousePosition);
				}
			}

		}
		else if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
		{
			for (int i = 0; i <physicsObjectList.size(); i++)
			{
				physicsObjectList[i]->clicked = false;
			}
		}

		for (int i = 0; i < SimulationUpdates; i++)
		{
			colidingObjects = checkColisions(physicsObjectList);
			resolveColisions(colidingObjects);

			for (int i = 0; i < physicsObjectList.size(); i++)
			{
				physicsObjectList[i]->getDrag(g);
				physicsObjectList[i]->checkBounds(screenWidth, screenHeight);
				physicsObjectList[i]->updateValues(GetFrameTime()/SimulationUpdates);
				physicsObjectList[i]->clampSpeed();

				if (physicsObjectList[i]->clicked == true)
				{
					physicsObjectList[i]->followMouse(mousePosition);
				}

			}
		}

		BeginDrawing();
		ClearBackground(BLACK);

		//colidingObjects = checkColisions(physicsObjectList);
		//resolveColisions(colidingObjects);

		for (int i = 0; i < physicsObjectList.size(); i++) 
		{	
			//physicsObjectList[i]->getDrag(g);
			//physicsObjectList[i]->checkBounds(screenWidth, screenHeight);
			//physicsObjectList[i]->updateValues(GetFrameTime());
			//physicsObjectList[i]->clampSpeed();

			physicsObjectList[i]->drawObject();

			//if (physicsObjectList[i]->clicked == true) 
			//{
			//	physicsObjectList[i]->followMouse(mousePosition);
			//}

		}

		DrawFPS(0, 0);
		DrawNumObjects(physicsObjectList);
		EndDrawing();
	}

	CloseWindow();

	return 0;
}
