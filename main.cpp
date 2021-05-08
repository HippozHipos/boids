#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "stdlib.h"

#define PI 3.14159f

struct Triangle
{
	Triangle()
	{
	}

	Triangle(olc::vf2d _p1, olc::vf2d _p2, olc::vf2d _p3) :
		p1(_p1), p2(_p2), p3(_p3)
	{
	}


	olc::vf2d p1 = { 0.0f,   -7.0f };
	olc::vf2d p2 = { -5.0f,   5.0f };
	olc::vf2d p3 = { 5.0f,    5.0f };

	Triangle Rotate(const float rotationAngle)
	{
		Triangle tri;
		tri.p1.x = cosf(rotationAngle) * p1.x - sinf(rotationAngle) * p1.y;
		tri.p1.x = sinf(rotationAngle) * p1.x + cosf(rotationAngle) * p1.y;
		tri.p2.x = cosf(rotationAngle) * p2.x - sinf(rotationAngle) * p2.y;
		tri.p2.x = sinf(rotationAngle) * p2.x + cosf(rotationAngle) * p2.y;
		tri.p3.x = cosf(rotationAngle) * p3.x - sinf(rotationAngle) * p3.y;
		tri.p3.x = sinf(rotationAngle) * p3.x + cosf(rotationAngle) * p3.y;
		return tri;
	}

	Triangle Translate(olc::vf2d offset)
	{
		Triangle tri;
		tri.p1 += offset;
		tri.p2 += offset;
		tri.p3 += offset;
		return tri;
	}

	Triangle TranslateAndRotate(const float rotationAngle, olc::vf2d offset)
	{
		Triangle tri;
		tri.p1.x = cosf(rotationAngle) * p1.x - sinf(rotationAngle) * p1.y + offset.x;
		tri.p1.y = sinf(rotationAngle) * p1.x + cosf(rotationAngle) * p1.y + offset.y; 
		tri.p2.x = cosf(rotationAngle) * p2.x - sinf(rotationAngle) * p2.y + offset.x;
		tri.p2.y = sinf(rotationAngle) * p2.x + cosf(rotationAngle) * p2.y + offset.y;
		tri.p3.x = cosf(rotationAngle) * p3.x - sinf(rotationAngle) * p3.y + offset.x;
		tri.p3.y = sinf(rotationAngle) * p3.x + cosf(rotationAngle) * p3.y + offset.y;
		return tri;
	}
};

struct Boid
{
	Boid()
	{
	}

	Boid(olc::vf2d _position, float _angle, olc::Pixel _color) : position(_position), rotationAngle(_angle), color(_color)
	{
	};

	float rotationAngle = 0.0f;
	float sensoryRadius = 50.0f;
	std::vector<Boid*> withinSensoryRange;
	Triangle TriModel;
	olc::Pixel color;
	olc::vf2d position = { 0.0f, 0.0f };

	void MoveTowardsDirectionAngle(float elapsedTime, float speedFactor)
	{
		position.x += cosf(GetDirectionAngle()) * elapsedTime * speedFactor;
		position.y += sinf(GetDirectionAngle()) * elapsedTime * speedFactor;
	}

	void LimitBoundry()
	{
		if (position.x < 0)
			position.x = 600;
		if (position.x > 600)
			position.x = 0;
		if (position.y < 0)
			position.y = 300;
		if (position.y > 300)
			position.y = 0;
	}

	void ModerateAngles()
	{
		if (rotationAngle > PI * 2.0f)
			rotationAngle = 0;
		if (rotationAngle < 0)
			rotationAngle = PI * 2.0f;
	}

	float GetDirectionAngle()
	{
		return rotationAngle - (PI / 2.0f);
	}
};

class App : public olc::PixelGameEngine
{
public:

	App()
	{
		sAppName = "Boids";
	}

private:

	std::vector<std::unique_ptr<Boid>> boids;
	int nInitialBoids = 100;
	float boidSpeed = 100.0f;

private:

	bool OnUserCreate() override
	{
		srand(time(0));
		for (int i = 0; i < nInitialBoids; i++)
		{
			boids.push_back(std::make_unique<Boid>
				(
				olc::vf2d(rand() % 600 * 1.0f, rand() % 300 * 1.0f),
				rand() % 6 * 1.0f,
				olc::Pixel(0, 0, (rand() % 150) + 100)
			    ));
		}

		return true;
	}

	bool OnUserUpdate(float elapsedTime) override
	{

		Clear(olc::BLACK);

		for (auto& boid : boids)
		{
		    boid->ModerateAngles();
			boid->LimitBoundry();
			boid->MoveTowardsDirectionAngle(elapsedTime, boidSpeed);
			Triangle transformedTriangleModel = boid->TriModel.TranslateAndRotate(boid->rotationAngle, boid->position);

			FillTriangle(
				(int)transformedTriangleModel.p1.x, (int)transformedTriangleModel.p1.y,
				(int)transformedTriangleModel.p2.x, (int)transformedTriangleModel.p2.y,
				(int)transformedTriangleModel.p3.x, (int)transformedTriangleModel.p3.y,
				boid->color);
		}

		for (auto& boid : boids)
		{
			boid->withinSensoryRange.clear();
		}

		for (int i = 0; i <  boids.size() - 1; i++)
			for (int j = i + 1; j < boids.size(); j++)
			{
				float distance = sqrtf((boids[i]->position.x - boids[j]->position.x) * (boids[i]->position.x - boids[j]->position.x) +
					(boids[i]->position.y - boids[j]->position.y) * (boids[i]->position.y - boids[j]->position.y));

				if (distance < boids[i]->sensoryRadius)
				{
					boids[i]->withinSensoryRange.push_back(boids[j].get());
					boids[j]->withinSensoryRange.push_back(boids[i].get());;
				}
			}

		//debugging and observation
		boids[0]->color = olc::Pixel(255, 0, 0);
		DrawCircle((int)boids[0]->position.x, (int)boids[0]->position.y, boids[0]->sensoryRadius, olc::Pixel(255, 0, 0));
		for (auto& boid : boids[0]->withinSensoryRange)
		{
			DrawLine((int)boid->position.x, (int)boid->position.y, boids[0]->position.x, boids[0]->position.y, olc::Pixel(255, 0, 0));
		}

		for (auto& boid : boids)
		{
			if (!boid->withinSensoryRange.empty())
			{
				Boid* closest = boid->withinSensoryRange[0];
				float lowest = INFINITY;
				for (auto& boidwithinRange : boid->withinSensoryRange)
				{
					float distance = sqrtf((boid->position.x - boidwithinRange->position.x) * (boid->position.x - boidwithinRange->position.x) +
						(boid->position.y - boidwithinRange->position.y) * (boid->position.y - boidwithinRange->position.y));

					if (distance < lowest)
					{
						lowest = distance;
						closest = boidwithinRange;
					}
				}

				float difference = boid->GetDirectionAngle() - closest->GetDirectionAngle();
				if (difference != 0)
					boid->rotationAngle += difference * 0.05f;
			}
				
		}

		return true;
	}
};

int main(int argc, char* argv[])
{
	App app;
	if (app.Construct(600, 300, 2, 2))
		app.Start();
}