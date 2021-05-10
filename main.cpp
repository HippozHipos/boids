#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

static constexpr float PI = 3.14159f;

float wrap(float value, float max)
{
	value /= max;
	value -= std::floor(value);
	value *= max;
	return value;

}

struct TwoFovLines
{
	olc::vf2d minusFovLine{};
	olc::vf2d plusFovLine{};
};

struct Arrow
{

	Arrow()
	{
	}

	Arrow(olc::vf2d _position, float _rotationAngle) :
		position(_position), rotationAngle(_rotationAngle)
	{
	}

	olc::vf2d position = { 300.0f, 150.0f };
	const olc::vi2d centre = { 170 / 2, 207 / 2 };
	const olc::vf2d scale = { 0.05f, 0.05f };
	float rotationAngle = 0.0f;
	float sensoryRadius = 65.0f;
	float fov = 4.782f / 2.0f;
	std::vector<Arrow*> withinSensoryRange;
	std::vector<Arrow*> withinFovRange;

	int numWithinFov = 0;

	float GetDirectionAngle()
	{
		return rotationAngle - (PI / 2);
	}

	TwoFovLines GetFovLines()
	{
		olc::vf2d line1;
		olc::vf2d line2;

		line1.x = (std::cos(GetDirectionAngle() - fov) * sensoryRadius) + position.x;
		line1.y = (std::sin(GetDirectionAngle() - fov) * sensoryRadius) + position.y;

		line2.x = (std::cos(GetDirectionAngle() + fov) * sensoryRadius) + position.x;
		line2.y = (std::sin(GetDirectionAngle() + fov) * sensoryRadius) + position.y;

		return { line1, line2 };
	}

	olc::vf2d GetForwardVector()
	{
		return
		{
			std::cos(GetDirectionAngle()),
			std::sin(GetDirectionAngle())
		};
	}
};

class SteerAway : public olc::PixelGameEngine
{
public:

	SteerAway()
	{
		sAppName = "Steer Awayyy";
	}

private:

	bool showDebugInfo = false;
	bool seperationEnabled = false;
	bool cohestionEnabled = false;
	bool allignmentEnabled = false;

private:

	olc::Pixel greenish{ 100, 200, 100 };

private:

	std::vector<std::unique_ptr<Arrow>> arrows;
	std::unique_ptr<olc::Sprite> arrowSprite;
	std::unique_ptr<olc::Decal> arrowDecal;

private:

	int nArrows = 80;
	float arrowSpeed = 80.0f;
	float rotationMobility = (1.0f / arrowSpeed) * 0.1f;

private:

	float GetDistance(olc::vf2d p1, olc::vf2d p2)
	{
		return std::hypot(p2.x - p1.x, p2.y - p1.y);
	};

private:

	void LimitValues()
	{
		for (auto& arrow : arrows)
		{
			arrowSpeed = std::max(std::min(arrowSpeed, 200.0f), 10.0f);
			arrow->fov = std::max(std::min(arrow->fov, PI), 0.0f);
			arrow->position.x = wrap(arrow->position.x, 600.0f);
			arrow->position.y = wrap(arrow->position.y, 300.0f);
			arrow->rotationAngle = wrap(arrow->rotationAngle, PI * 2.0f);
			arrow->sensoryRadius = std::max(std::min(arrow->sensoryRadius, 300.0f), 10.0f);
		}
	}

	void AddArrow()
	{
		olc::vf2d position = {
					(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 600.0f,
					(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 300.0f };

		std::unique_ptr<Arrow> arrow = std::make_unique<Arrow>(position,
			(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * PI * 2.0f);

		arrows.push_back(std::move(arrow));
	}

	void CollisionDetection()
	{

		for (auto& arrow : arrows)
		{
			arrow->withinSensoryRange.clear();
			arrow->withinFovRange.clear();
			arrow->numWithinFov = 0;
		}

		for (unsigned int i = 0; i < arrows.size(); i++)
			for (unsigned int j = 0; j < arrows.size(); j++)
				if (i != j)
				{
					if (GetDistance(arrows[i]->position, arrows[j]->position) < arrows[i]->sensoryRadius)
						arrows[i]->withinSensoryRange.push_back(arrows[j].get());
				}

		for (auto& arrow : arrows)
			for (auto& withinRange : arrow->withinSensoryRange)
				if (arrow.get() != withinRange)
				{
					olc::vf2d forwardVector = -arrow->GetForwardVector().norm();
					olc::vf2d vectorToAnotherArrow = (arrow->position - withinRange->position).norm();
					float dot = vectorToAnotherArrow.dot(forwardVector);

					if (dot >= std::cos(arrow->fov))
					{
						arrow->numWithinFov++;
						arrow->withinFovRange.push_back(withinRange);
					}
				}
	}

	void UserControls(float elapsedTime)
	{
		for (auto& arrow : arrows)
			if (GetKey(olc::UP).bHeld)
				arrow->fov += 0.005f * elapsedTime * 300.0f;

		for (auto& arrow : arrows)
			if (GetKey(olc::DOWN).bHeld)
				arrow->fov -= 0.005f * elapsedTime * 300.0f;

		for (auto& arrow : arrows)
			if (GetKey(olc::LEFT).bHeld)
				arrow->sensoryRadius -= 0.5f * elapsedTime * 300.0f;

		for (auto& arrow : arrows)
			if (GetKey(olc::RIGHT).bHeld)
				arrow->sensoryRadius += 0.5f * elapsedTime * 300.0f;

		if (GetKey(olc::S).bPressed)
			seperationEnabled = !seperationEnabled;

		if (GetKey(olc::I).bPressed)
		{
			AddArrow();
			nArrows++;
		}

		if (GetKey(olc::U).bPressed)
		{
			AddArrow();
			nArrows--;
		}

		if (GetKey(olc::Q).bHeld)
			arrowSpeed -= 0.5f * elapsedTime * 300.0f;

		if (GetKey(olc::W).bHeld)
			arrowSpeed += 0.5f * elapsedTime * 300.0f;

		if (GetKey(olc::C).bPressed)
			cohestionEnabled = !cohestionEnabled;

		if (GetKey(olc::D).bPressed)
			showDebugInfo = !showDebugInfo;

		if (GetKey(olc::A).bPressed)
			allignmentEnabled = !allignmentEnabled;

		if (GetKey(olc::R).bPressed)
		{
			arrows.clear();
			for (int i = 0; i < nArrows; i++)
				AddArrow();
		}
	}

	void DrawEverything()
	{
		Clear(olc::Pixel(20, 20, 20));
		for (auto& arrow : arrows)
			//draw the arrows
			DrawRotatedDecal(arrow->position, arrowDecal.get(), arrow->rotationAngle, arrow->centre, arrow->scale);

		if (showDebugInfo)
		{
			for (auto& inRange : arrows[0]->withinFovRange)
				DrawLine((int)arrows[0]->position.x, (int)arrows[0]->position.y,
					(int)inRange->position.x, (int)inRange->position.y, olc::RED);
			//draw the fov lines			
			DrawLine((int)arrows[0]->GetFovLines().minusFovLine.x, (int)arrows[0]->GetFovLines().minusFovLine.y,
				(int)arrows[0]->position.x, (int)arrows[0]->position.y,
				olc::RED);

			DrawLine((int)arrows[0]->GetFovLines().plusFovLine.x, (int)arrows[0]->GetFovLines().plusFovLine.y,
				(int)arrows[0]->position.x, (int)arrows[0]->position.y,
				olc::RED);

			//draw the fov arc
			float tempAngle = arrows[0]->GetDirectionAngle() - arrows[0]->fov;
			while (tempAngle < arrows[0]->GetDirectionAngle() + arrows[0]->fov)
			{
				tempAngle += 0.01f;
				Draw((int)(std::cos(tempAngle) * arrows[0]->sensoryRadius + arrows[0]->position.x),
					(int)(std::sin(tempAngle) * arrows[0]->sensoryRadius + arrows[0]->position.y),
					olc::RED);
			}

			std::ostringstream os1;
			std::ostringstream os2;

			std::string seperationOn = seperationEnabled ? "YES" : "NO";
			std::string cohestionOn = cohestionEnabled ? "YES" : "NO";
			std::string allignmentOn = allignmentEnabled ? "YES" : "NO";

			os1 << "Sensory Radius: " << arrows[0]->sensoryRadius << "\n\n" << "FOV: " << arrows[0]->fov * (180/PI) * 2
				<< "\n\n" << "Seperation Enabled: " << seperationOn << "\n\n" <<  "Cohestion Enabled: " << cohestionOn
				<< "\n\n" << "Allignment Enabled: " << allignmentOn;

			os2 << "Speed: " << arrowSpeed << "\n\n" << "Number of arrows: " << nArrows;

			DrawString(10, 10, os1.str());
			DrawString(300, 10, os2.str());
		}
	}

	void Transformations(float elapsedTime)
	{
		for (auto& arrow : arrows)
		{
			olc::vf2d forwardVector = arrow->GetForwardVector();

			forwardVector = forwardVector.norm();
			arrow->position += forwardVector * elapsedTime * arrowSpeed;
		}

		if (seperationEnabled)
			for (auto& arrow : arrows)
			{
				olc::vf2d totalInfluence;
				for (auto& inRange : arrow->withinFovRange)
				{
					float distance = GetDistance(arrow->position, inRange->position);
					olc::vf2d vecToBoidInRange = (arrow->position - inRange->position).norm();
					if (distance != 0)
						vecToBoidInRange *= (1.0f / distance);
					totalInfluence += vecToBoidInRange;
				}
				arrow->rotationAngle += std::atan2(totalInfluence.y, totalInfluence.x) * rotationMobility;
			}

		if (cohestionEnabled)
			for (auto& arrow : arrows)
				if (!arrow->withinFovRange.empty())
				{
					int count = 0;
					olc::vf2d localAveragePosition = arrow->withinFovRange[0]->position;
					for (unsigned int i = 0; i < arrows.size(); i++)
						if (i > 0)
						{
							count++;
							localAveragePosition += arrows[i]->position;
						}
					if (count != 0)
						localAveragePosition /= count;
					olc::vf2d directionTolocalAveragePosition = (arrow->position - localAveragePosition).norm();
					float angle = std::atan2(directionTolocalAveragePosition.y, directionTolocalAveragePosition.x);
					angle < arrow->rotationAngle ? arrow->rotationAngle -= angle * rotationMobility
						: arrow->rotationAngle += angle * rotationMobility;

		        }

		if (allignmentEnabled)
			for (auto& arrow : arrows)
			{
				float averageAngle = 0.0f;
				int count = 0;
				for (auto& inRange : arrow->withinFovRange)
				{
					count++;
					averageAngle += inRange->rotationAngle;
				}
				if (count != 0)
					averageAngle /= count;
				averageAngle < arrow->rotationAngle ? arrow->rotationAngle -= averageAngle * rotationMobility
					: arrow->rotationAngle += averageAngle * rotationMobility;
			}
	}

private:

	bool OnUserCreate() override
	{
		srand(time(0));
		arrowSprite = std::make_unique<olc::Sprite>("tri.png");
		arrowDecal = std::make_unique<olc::Decal>(arrowSprite.get());
		for (int i = 0; i < nArrows; i++)
			AddArrow();

		return true;
	}

	bool OnUserUpdate(float elapsedTime) override
	{
		LimitValues();
		CollisionDetection();
		DrawEverything();
		Transformations(elapsedTime);
		UserControls(elapsedTime);

		return true;
	}

};

int main()
{
	SteerAway steerAway;
	if (steerAway.Construct(600, 300, 2, 2))
		steerAway.Start();
}
