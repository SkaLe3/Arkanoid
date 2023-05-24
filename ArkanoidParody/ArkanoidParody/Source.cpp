#include "Framework.h"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <list>
#define PI 3.14159265358979323846264

void clamp(double& value, double minVal, double maxVal)
{
	value = std::min(maxVal, std::max(value, minVal));
}
void clamp(int& value, int minVal, int maxVal)
{
	value = std::min(maxVal, std::max(value, minVal));
}

class Object {
public:


	std::vector<double> vec;

	Object(std::string num, double  _x, double  _y, double _size, double _ratio, double _speed, double vx, double vy) :
		x(_x), y(_y), size(_size), ratio(_ratio), speed(_speed), vec{ vx, vy }, exist(false)
	{
		sprite = createSprite(("data\\" + num + "-Breakout-Tiles.png").c_str());
	};
	void Move()
	{
		if (exist) {
			x += vec[0] * speed;
			y += vec[1] * speed;
		}
	}
	double Right()
	{
		return x + size * ratio;
	}
	double Left()
	{
		return x;
	}
	double Top()
	{
		return y;
	}
	double Bottom()
	{
		return y + size;
	}
	double Xsize() 
	{
		return size * ratio;
	}
	double Ysize() {
		return size;
	}
	void SetSize(double _size, double Xaspect, double Yaspect) 
	{
		size = _size;
		setSpriteSize(sprite, Xsize() * Xaspect, Ysize() * Yaspect);
	}
	void SetPos(double _x, double _y)
	{
		x = _x;
		y = _y;
	}

	// Returns true if Game is over
	virtual bool ProcessScreenCollision() = 0;
	void Draw(double Xaspect, double Yaspect)
	{
		if (exist)
			drawSprite(sprite, x * Xaspect, y * Yaspect);
	}

	virtual bool IsExist()
	{
		return exist;
	}
	virtual void Spawn()
	{
		exist = true;
	}
	virtual void Destroy()
	{
		exist = false;
	}
protected:
	bool exist;
	Sprite* sprite;
	double x;
	double y;
	double size;
	double ratio;
	double speed;
	double length()
	{
		return sqrt(pow(vec[0], 2) + pow(vec[1], 2));
	}

};
class Ability : public Object
{
public:
	static std::chrono::system_clock::time_point SpawnTime;

	Ability(std::string num, double _ratio, double _speed) :
		Object(num, 0, 0, 30, _ratio, _speed, 0, 1) {};

	// Spawns random ability if seconds gone
	static void Spawn(Ability* first, Ability* second)
	{
		if (std::chrono::system_clock::now() - Ability::SpawnTime > std::chrono::seconds{ 15 }) {
			Ability* ab[2] = { first, second };
			ab[rand() % 2]->Spawn();
		}
	}
	// Activates ability and stores activation time
	void Activate()
	{

			Destroy();
			catchTime.push_back(std::chrono::system_clock::now());

	}
	// Deletes activation time
	void Disactivate()
	{
		catchTime.pop_front();
	}
	// Amount of activated abilities
	int Active()
	{
		return catchTime.size();
	}
	void SetPos()
	{
		double random = (rand() % int(800 - Xsize()));
		x = random;
		y = 0;
	}
	// Spawns ability and stores spawn time
	virtual void Spawn() override
	{
		exist = true;
		Ability::SpawnTime = std::chrono::system_clock::now();
		SetPos();
	}
	// Processing of ability collision with ball
	virtual bool ProcessScreenCollision() override
	{
		if (IsExist() && Bottom() >= 600)
		{
			Destroy();
		}
		return false;
	}
	// Processing activated ability 
	void Process()
	{
		for (int i = 0; i < catchTime.size(); i++)
		{
			if (std::chrono::system_clock::now() - catchTime.front() > std::chrono::seconds{ 20 })
				Disactivate();
		}
	}
private:
	std::list<std::chrono::system_clock::time_point> catchTime;

};

std::chrono::system_clock::time_point Ability::SpawnTime;

class TPlatform : public Object {
public:
	TPlatform(std::string num, double _size, double _ratio, double _speed) :
		Object(num, 0, 0, _size, _ratio, _speed, 0, 0)
	{
		x = (800.0 - Xsize()) / 2.0;
		y = 600 - 40;
	};
	// Returns true if Game is over
	virtual bool ProcessScreenCollision() override
	{
		clamp(x, 0.0, 800 - Xsize());
		return false;
		
	}
	// Processing collision with ability
	void ProcessAbility(Ability& ability)
	{
		if (ability.Bottom() >= Top() && ability.Right() >= Left() && ability.Left() <= Right() && ability.IsExist())
		{
			ability.Activate();
		}
		ability.Process();
	}
};

class TBlock : public Object {

public:
	static int BlockCounter;
	TBlock() :
		Object("01", 0, 0, 1, 1, 0.003, 0.0, 1.0) {};

	void SetTexture(std::string num, double _ratio)
	{
		sprite = createSprite(("data\\" + num + "-Breakout-Tiles.png").c_str());
		ratio = _ratio;
	}

	virtual bool IsExist() override
	{
		return exist;
	}
	virtual void Spawn() override
	{
		exist = true;
		BlockCounter++;
	}
	virtual void Destroy() override
	{
		exist = false;
		--BlockCounter;
	}
	// Returns true if Game is over
	virtual bool ProcessScreenCollision() override
	{
		if (IsExist() && Bottom() >= 600)
			return true;
		return false;
	}
	// Returns true if Game is over
	bool ProcessPlatformCollision(TPlatform& platform)
	{
		if (Bottom() >= platform.Top() && Right() >= platform.Left() && Left() <= platform.Right())
			return true;
		return false;
	}

};

int TBlock::BlockCounter;

class TBall : public Object {
public:
	static int DefaultSize;
	TBall(std::string num, double _size, double _speed) :
		Object(num, 0, 0, _size, 1, _speed, 0, 0)
	{
		x = (800.0 - size) / 2.0;
		y = 600 - 80;
	};
	double Radius() const {
		return size / 2.0;
	}
	double Xcenter()const {
		return x + size / 2.0;
	}
	double Ycenter()const {
		return y + size / 2.0;
	}
	// Vector normalization
	void Normalize()
	{
		double len = length() == 0 ? 1 : length();
		vec[0] = vec[0] / len;
		vec[1] = vec[1] / len;
	}
	// Returns true if Game is over
	virtual bool ProcessScreenCollision() override
	{
		// Bounce from edges of the screen
		if (Right() >= 800 || Left() <= 0) 
		{
			vec[0] *= -1;
		}
		// Bounce from celling
		if (Top() <= 0)
		{
			vec[1] *= -1;

		}
		// Ball on the floor
		if (Bottom() >= 600)
			return true;
		return false;
	}
	bool ProcessPlatformCollision(TPlatform& platform)
	{
		// Bounce from platform
		if (Bottom() >= platform.Top() && Xcenter() >= platform.Left() &&
			Xcenter() <= platform.Right())
		{
			// Changing angle of ball trajectory depending on the place of contact with the platform
			double deviation = (Xcenter() - (platform.Left() + platform.Xsize() / 2.0)) / platform.Xsize();
			double angle = acos(vec[0]) - 2 * deviation;
			clamp(angle, 0.15, 3);

			// Finding new ball vector
			vec[0] = std::cos(angle);
			vec[1] = -std::sin(angle);
			if (TBlock::BlockCounter == 0)
				return true;
			std::cout << size <<  "\t" << speed << std::endl;
		}
		// Bounce from platform sides
		if (Left() <= platform.Right() && Right() >= platform.Left() &&
			Ycenter() >= platform.Top() && Ycenter() <= platform.Bottom())
		{
			vec[1] *= -1;
		}
		return false;
	}
	bool ProcessBlockCollision(TBlock& block)
	{
		// Sides
		if ((Left() <= block.Right() && Right() >= block.Left()) &&
			Ycenter() >= block.Top() && Ycenter() <= block.Bottom())
		{
			vec[0] *= -1;
			block.Destroy();
		}
		// Top and down
		if (Top() <= block.Bottom() && Bottom() >= block.Top() &&
			(Xcenter() >= block.Left() && Xcenter() <= block.Right()))
		{
			vec[1] *= -1;
			block.Destroy();
		}
		// Top Left angle
		AngleBounce(block.Left(), block.Top(), block);
		// Down Left angle
		AngleBounce(block.Left(), block.Bottom(), block);
		// Top Right angle
		AngleBounce(block.Right(), block.Top(), block);
		// Down Right angle
		AngleBounce(block.Right(), block.Bottom(), block);
		return false;
	}
	// Finds new trajectory vector if ball collides angle of block
	void AngleBounce(double x, double y, TBlock& block)
	{
		if (pow(x - Xcenter(), 2) + pow(y - Ycenter(), 2) <= pow(Radius(), 2))
		{
			// Finding reflected vector from angle 
			std::vector<double> n = { x - Xcenter(), y - Ycenter() };
			double len = sqrt(n[0] * n[0] + n[1] * n[1]);
			n[0] = n[0] / len;
			n[1] = n[1] / len;
			double k = (n[0] * vec[0] + n[1] * vec[1]) / (n[0] * n[0] + n[1] * n[1]);
			vec[0] = vec[0] - 2 * k * n[0];
			vec[1] = vec[1] - 2 * k * n[1];
			Normalize();
			block.Destroy();
		}
	}
	// Changes the size of the ball depending on the number of activated abilities
	void ProcessAbility(Ability& in, Ability& de, double Xaspect, double Yaspect)
	{
		int diff = in.Active() - de.Active();
		while (in.Active() != 0 && de.Active() != 0)
		{
			in.Disactivate();
			de.Disactivate();
		}
		double s;
		if (diff != 0)
			s = pow(1 + (diff) / abs(diff) * 0.4, abs(diff)) * TBall::DefaultSize;
		else
			s = TBall::DefaultSize;
		clamp(s, TBall::DefaultSize / 2.0, TBall::DefaultSize * 2.0);
		SetSize(s, Xaspect, Yaspect);
	}

};

int TBall::DefaultSize;


// Stores pressed keys and mouse coordinates
struct Control {
	bool left;
	bool right;
	bool MouseClick;
	double MouseX;
	double MouseY;

};


class MyFramework : public Framework {
public:
	int Width;
	int Height;
	bool Fullscreen;
	double Xaspect;
	double Yaspect;

	bool GameOver;
	bool GameStarted;
	
	TBlock** Field;
	TPlatform* Platform;
	TBall* Ball;
	Ability* Increase;
	Ability* Decrease;
	Control control;

	MyFramework(int argc, char* argv[]) : 
		Width(800), Height(600), Fullscreen(false), GameOver(true), GameStarted(false),
		Field(nullptr), Platform(nullptr), Ball(nullptr), Increase(nullptr), Decrease(nullptr), control{ false, false, false, 0, 0 }
	{
		int window[2] = {Width, Height};
		std::string values[2];
		for (int i = 0; i < argc; i++)
		{
			if (!strcmp(argv[i], "-window"))
			{
				int str = 0;
				int index = 0;
				++i;
				for (int j = 0; j <= 9; j++)
				{
					if (isdigit(argv[i][j]))
					{
						std::cout << argv[i][j] << std::endl;
						values[str]+= argv[i][j];
						++index;

					}
					else
					{
						window[str] = atoi(values[str].c_str());
						if (argv[i][j] != 'x') break;
						++str;
						index = 0;
					}
				}
			}
			if (!strcmp(argv[i], "-fullscreen"))
				Fullscreen = true;
		}
		Width = window[0];
		Height = window[1];
		Xaspect = Width / 800.0;
		Yaspect = Height / 600.0;

	}
	// Creates All objects
	void CreateGame()
	{
		Field = new TBlock *[6];
		for (int i = 0; i < 6; i++)
		{
			Field[i] = new TBlock[14];
			for (int j = 0; j < 14; j++)
			{
				TBlock& block = Field[i][j];
				std::string num = "";
				if (17 - i * 2 < 10) 
					num += '0';
				num += std::to_string(17 - i * 2);
				block.SetTexture(num, 3);
				block.SetPos(2 + j * 57, 50 + i * 20);
				block.SetSize(18, Xaspect, Yaspect);
				block.Spawn();
			}
		}

		Platform = new TPlatform{ "56" , 32, 693.0 / 128.0, 2.5};
		Platform->SetSize(32, Xaspect, Yaspect);
		Platform->Spawn();
		

		Ball = new TBall{ "62",  26, 2};
		Ball->SetSize(26,Xaspect, Yaspect);
		Ball->Spawn();
		TBall::DefaultSize = Ball->Xsize();

		Decrease = new Ability{ "46", 485.0 / 128.0, 0.3 };
		Decrease->SetPos();
		Decrease->SetSize(24, Xaspect, Yaspect);

		Increase = new Ability{ "47", 485.0 / 128.0, 0.3 };
		Increase->SetPos();
		Increase->SetSize(24, Xaspect, Yaspect);
		Ability::SpawnTime = std::chrono::system_clock::now();


	}
	// Draws All objects
	void DrawFrame()
	{
		for (int i = 0; i < 6; i++)
			for (int j = 0; j < 14; j++) 
				Field[i][j].Draw(Xaspect, Yaspect);

		Platform->Draw(Xaspect, Yaspect);
		Ball->Draw(Xaspect, Yaspect);
		Decrease->Draw(Xaspect, Yaspect);
		Increase->Draw(Xaspect, Yaspect);

	}
	// Processing all abilities events
	void ProcessAbilities()
	{
		Ability::Spawn(Increase, Decrease);

		Platform->ProcessAbility(*Increase);
		Platform->ProcessAbility(*Decrease);
		Ball->ProcessAbility(*Increase, *Decrease, Xaspect, Yaspect);
		Increase->ProcessScreenCollision();
		Decrease->ProcessScreenCollision();
		Decrease->Move();
		Increase->Move();
	}
	// Ball collision and movement 
	void MoveBall()
	{
		// Bounce from every block
		for (int i = 0; i < 6; i++)
			for (int j = 0; j < 14; j++)
				if (Field[i][j].IsExist())
				{
					Ball->ProcessBlockCollision(Field[i][j]);
				}

		if (Ball->ProcessScreenCollision() || Ball->ProcessPlatformCollision(*Platform))
		{
			GameOver = true;
			return;
		}
		Ball->Move();

	}
	void MoveBlocks()
	{
		for (int i = 0; i < 6; i++)
			for (int j = 0; j < 14; j++)
			{
				TBlock& block = Field[i][j];
				if (block.ProcessScreenCollision() || block.ProcessPlatformCollision(*Platform))
				{
					GameOver = true;
					return;
				}
				block.Move();
			}


	}
	// Platform movement 
	void MovePlatform()
	{
		Platform->ProcessScreenCollision();
		Platform->Move();
	}
	void Start()
	{
			Ball->vec[0] = control.MouseX - Ball->Xcenter();
			Ball->vec[1] = control.MouseY - Ball->Ycenter();
			if (control.MouseClick) {
				Ball->Normalize();
				GameStarted = true;

			}
			Ability::SpawnTime = std::chrono::system_clock::now();
			
	}
	// Memory cleaning
	void TheEnd()
	{
		for (int i = 0; i < 6; i++)
			delete[] Field[i];
		delete[] Field;
		delete Platform;
		delete Ball;
	}

	virtual void PreInit(int& width, int& height, bool& fullscreen)
	{
		width = Width;
		height = Height;
		fullscreen = Fullscreen;
	}

	virtual bool Init() {

		return true;
	}

	virtual void Close() {
	
		TheEnd();
	} 
	
	virtual bool Tick() {

		if (GameOver)
		{
			GameStarted = false;
			CreateGame();
			GameOver = false;
		}

		DrawFrame();
		
		if (GameStarted)
		{
			ProcessAbilities();
			MovePlatform();
			MoveBall();
			MoveBlocks();
		}
		else Start();
		// Clean memory
		if (GameOver) TheEnd();
		std::cout << sizeof(void*) << std::endl;

		return false;
	}

	virtual void onMouseMove(int x, int y, int xrelative, int yrelative) {

		control.MouseX = x / Xaspect;
		control.MouseY = y / Yaspect;
	}

	virtual void onMouseButtonClick(FRMouseButton button, bool isReleased) {
		if (button == FRMouseButton::LEFT && !isReleased)
			control.MouseClick = true;
		if (button == FRMouseButton::LEFT && isReleased)
			control.MouseClick = false;
	}

	virtual void onKeyPressed(FRKey k) {
		if (k == FRKey::RIGHT) {
			control.right = true;
			Platform->vec[0] = 1;
		};
		if (k == FRKey::LEFT) {
			control.left = true;
			Platform->vec[0] = -1;
		};

	}

	virtual void onKeyReleased(FRKey k) {

		if (k == FRKey::RIGHT) 
		{
			control.right = false;
			if (control.left)
				Platform->vec[0] = -1;
			else
				Platform->vec[0] = 0;
		}

		if (k == FRKey::LEFT)
		{
			control.left = false;
			if (control.right)
				Platform->vec[0] = 1;
			else
				Platform->vec[0] = 0;
		}
	}

	virtual const char* GetTitle() override
	{
		return "Arkanoid Parody";
	}
};
int main(int argc, char* argv[])
{
	srand(time(0));
	return run(new MyFramework(argc, argv));
}