#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <time.h>

using namespace std;

//=============================================================
// DECLARATIONS
//=============================================================

struct CubeCoord;
struct OffsetCoord;

struct Ship;
struct Barrel;

//******************************

const int PosToIndex(const int& x, const int& y);
const CubeCoord OffsetToCube(const OffsetCoord& offset);
const OffsetCoord CubeToOffset(const CubeCoord& cube);

const bool operator==(const OffsetCoord& a, const OffsetCoord& b);
const bool operator<(const OffsetCoord& a, const OffsetCoord& b);

const int ComputeDistance(const CubeCoord& a, const CubeCoord& b);

Ship* GetShip(vector<Ship>& ships, int entityId);

bool CommandGoToBarrel(Ship* ship, const vector<Barrel>& barrels);
bool CommandWander(Ship* ship, const vector<char>& objectMap);
bool CommandFire(Ship* ship, const vector<Ship>& enemyShips);

//=============================================================
// STRUCTS
//=============================================================

struct CubeCoord
{
	int x, y, z;

	CubeCoord()
	{
		x = y = z = -1;
	}

	CubeCoord(int x, int y, int z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	CubeCoord operator+(const CubeCoord& o) const
	{
		return CubeCoord(this->x + o.x, this->y + o.y, this->z + o.z);
	}

	CubeCoord operator*(const int& m) const
	{
		return CubeCoord(this->x * m, this->y * m, this->z * m);
	}
};

struct OffsetCoord
{
	int col, row;

	OffsetCoord()
	{
		col = row = -1;
	}

	OffsetCoord(int col, int row)
	{
		this->col = col;
		this->row = row;
	}

	bool IsNull() const
	{
		if (col == -1 && row == -1)
			return true;

		return false;
	}

	OffsetCoord operator+(const OffsetCoord& o) const
	{
		return OffsetCoord(this->col + o.col, this->row + o.row);
	}
};

struct Ship
{
	int entityId;
	OffsetCoord offsetPos;
	CubeCoord cubePos;
	int rotation;
	int speed;
	int rum;

	bool updated;
	int hasFired;
	OffsetCoord wanderTarget;

	Ship()
	{
		offsetPos = OffsetCoord(-1, -1);
		cubePos = OffsetToCube(offsetPos);

		entityId = rotation = speed = rum = -1;
		hasFired = 0;
		updated = true;
	}

	Ship(int entityId, int x, int y, int rotation, int speed, int rum)
	{
		offsetPos = OffsetCoord(x, y);
		cubePos = OffsetToCube(offsetPos);

		this->entityId = entityId;
		this->rotation = rotation;
		this->speed = speed;
		this->rum = rum;

		hasFired = 0;
		updated = true;
	}

	void Update(int x, int y, int rotation, int speed, int rum)
	{
		offsetPos = OffsetCoord(x, y);
		cubePos = OffsetToCube(offsetPos);
		this->rotation = rotation;
		this->speed = speed;
		this->rum = rum;

		if (hasFired == 1)
			hasFired = 2;
		else if (hasFired == 2)
			hasFired = 0;

		updated = true;
	}
};

struct Barrel
{
	OffsetCoord offsetPos;
	CubeCoord cubePos;
	int amount;

	Barrel()
	{
		offsetPos = OffsetCoord(-1, -1);
		cubePos = OffsetToCube(offsetPos);

		amount = -1;
	}

	Barrel(int x, int y, int amount)
	{
		offsetPos = OffsetCoord(x, y);
		cubePos = OffsetToCube(offsetPos);

		this->amount = amount;
	}
};

//=============================================================
// CONSTANTS
//=============================================================

const int MAP_WIDTH = 23;
const int MAP_HEIGHT = 21;

const CubeCoord DIRECTIONS[6] =
{ CubeCoord(1, -1, 0), CubeCoord(1, 0, -1), CubeCoord(0, 1, -1), CubeCoord(-1, 1, 0), CubeCoord(-1, 0, 1), CubeCoord(0, -1, 1) };

const int MAX_FIRING_RANGE = 5;

//=============================================================
// MAIN
//=============================================================
/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
int main()
{
	srand(time(NULL));

	// game loop variables
	vector<Ship> _myShips;
	vector<Ship> _enemyShips;
	vector<Barrel> _barrels;

	vector<char> _objectMap = vector<char>(MAP_WIDTH * MAP_HEIGHT);
	vector<int> _cannonBallMap = vector<int>(MAP_WIDTH * MAP_HEIGHT);

	for (int col = 0; col < MAP_WIDTH; col++)
	{
		for (int row = 0; row < MAP_HEIGHT; row++)
		{
			int index = PosToIndex(col, row);
			_objectMap[index] = ' ';
			_cannonBallMap[index] = 0;
		}
	}

	// game loop
	while (1)
	{
		int myShipCount; // the number of remaining ships
		cin >> myShipCount;
		cin.ignore();

		int entityCount; // the number of entities (e.g. ships, mines or cannonballs)
		cin >> entityCount;
		cin.ignore();

		for (int i = 0; i < entityCount; i++)
		{
			int entityId;
			string entityType;
			int x;
			int y;
			int arg1;
			int arg2;
			int arg3;
			int arg4;
			cin >> entityId >> entityType >> x >> y >> arg1 >> arg2 >> arg3 >> arg4;
			cin.ignore();

			if (entityType == "SHIP")
			{
				if (arg4 == 1)
				{
					Ship* ship = GetShip(_myShips, entityId);

					if (ship == nullptr)
					{
						_myShips.emplace_back(Ship(entityId, x, y, arg1, arg2, arg3));
					}
					else
					{
						//cerr << "rum1=" << arg3 << endl;
						ship->Update(x, y, arg1, arg2, arg3);
					}
				}
				else
				{
					_enemyShips.emplace_back(Ship(entityId, x, y, arg1, arg2, arg3));
				}
			}
			else if (entityType == "BARREL")
			{
				_barrels.emplace_back(Barrel(x, y, arg1));
				_objectMap[PosToIndex(x, y)] = 'B';
			}
			else if (entityType == "CANNONBALL")
			{
				_objectMap[PosToIndex(x, y)] = 'C';
			}
			else if (entityType == "MINE")
			{
				_objectMap[PosToIndex(x, y)] = 'M';
			}
		}

		for (int i = 0; i < _myShips.size(); i++)
		{
			if (!_myShips[i].updated)
			{
				_myShips.erase(_myShips.begin() + i);
				i--;
			}
		}

		for (int i = 0; i < myShipCount; i++)
		{
			// Write an action using cout. DON'T FORGET THE "<< endl"
			// To debug: cerr << "Debug messages..." << endl;
			//cout << "MOVE 11 10" << endl; // Any valid action, such as "WAIT" or "MOVE x y"

			Ship* currentShip = &_myShips[i];

			//cerr << "rum2=" << currentShip->rum << endl;

			if (CommandFire(currentShip, _enemyShips))
			{
				cerr << "Command: Fire!" << endl;
				continue;
			}
			else if (currentShip->rum <= 80 && CommandGoToBarrel(currentShip, _barrels))
			{
				cerr << "Command: Go To Barrel!" << endl;
				continue;
			}
			else if (CommandWander(currentShip, _objectMap))
			{
				cerr << "Command: Wander!" << endl;
				continue;
			}

			cout << "WAIT" << endl;
		}

		// cleaning up
		for (int i = 0; i < _myShips.size(); i++)
		{
			_myShips[i].updated = false;
		}

		_enemyShips.clear();
		_barrels.clear();

		for (int col = 0; col < MAP_WIDTH; col++)
		{
			for (int row = 0; row < MAP_HEIGHT; row++)
			{
				int index = PosToIndex(col, row);
				_objectMap[index] = ' ';
				_cannonBallMap[index] = 0;
			}
		}
	}
}

//=============================================================
//
//=============================================================

const int PosToIndex(const int& x, const int& y)
{
	return x + y * MAP_WIDTH;
}

const CubeCoord OffsetToCube(const OffsetCoord& offset)
{
	int x = offset.col - (offset.row - (offset.row & 1)) / 2;
	int z = offset.row;
	int y = -x - z;
	return CubeCoord(x, y, z);
}

const OffsetCoord CubeToOffset(const CubeCoord& cube)
{
	int col = cube.x + (cube.z - (cube.z & 1)) / 2;
	int row = cube.z;
	return OffsetCoord(col, row);
}

const bool operator==(const OffsetCoord& a, const OffsetCoord& b)
{
	return a.col == b.col && a.row == b.row;
}

const bool operator<(const OffsetCoord& a, const OffsetCoord& b)
{
	return a.col < b.col || (a.col == b.col && a.row < b.row);
}

const int ComputeDistance(const CubeCoord& a, const CubeCoord& b)
{
	/*int distance = max(max(abs(a.x - b.x), abs(a.y - b.y)), abs(a.z - b.z));
	 cerr << "ComputeDistance: a=" << a.x << "," << a.y << "," << a.z << " b=" << b.x << "," << b.y << "," << b.z << " distance=" << distance
	 << endl;
	 return distance;*/

	return max(max(abs(a.x - b.x), abs(a.y - b.y)), abs(a.z - b.z));
}

Ship* GetShip(vector<Ship>& ships, int entityId)
{
	for (unsigned int shipIndex = 0; shipIndex < ships.size(); shipIndex++)
	{
		if (ships.at(shipIndex).entityId == entityId)
		{
			return &ships.at(shipIndex);
		}
	}

	return nullptr;
}

bool CommandGoToBarrel(Ship* ship, const vector<Barrel>& barrels)
{
	int shortestDistance = 999;
	OffsetCoord nearestBarrelPos;
	for (unsigned int barrelIndex = 0; barrelIndex < barrels.size(); barrelIndex++)
	{
		Barrel currentBarrel = barrels[barrelIndex];

		int distance = ComputeDistance(ship->cubePos, currentBarrel.cubePos);

		if (distance < shortestDistance)
		{
			shortestDistance = distance;
			nearestBarrelPos = currentBarrel.offsetPos;
		}
	}

	if (!nearestBarrelPos.IsNull())
	{
		cerr << "CommandGoToBarrel: shortestDistance=" << shortestDistance << endl;
		cout << "MOVE " << nearestBarrelPos.col << " " << nearestBarrelPos.row << endl;
		return true;
	}

	return false;
}

bool CommandWander(Ship* ship, const vector<char>& objectMap)
{
	if (ship->offsetPos == ship->wanderTarget)
	{
		cerr << "CommandWander: target reached" << endl;
		ship->wanderTarget = OffsetCoord();
	}

	if (ship->wanderTarget.IsNull())
	{
		int x, y;

		do
		{
			x = 1 + (rand() % (int) (MAP_WIDTH - 1));
			y = 1 + (rand() % (int) (MAP_HEIGHT - 1));
			cerr << "CommandWander: newX=" << x << " newY=" << y << endl;
		} while (objectMap[PosToIndex(x, y)] != ' ');

		ship->wanderTarget = OffsetCoord(x, y);
		cout << "MOVE " << x << " " << y << endl;
		return true;
	}
	else
	{
		cout << "MOVE " << ship->wanderTarget.col << " " << ship->wanderTarget.row << endl;
		return true;
	}
}

bool CommandFire(Ship* ship, const vector<Ship>& enemyShips)
{
	if (ship->hasFired != 0)
		return false;

	int shortestShotDistance = MAX_FIRING_RANGE + 1;
	OffsetCoord shortestShotPos;

	for (unsigned int enemyShipIndex = 0; enemyShipIndex < enemyShips.size(); enemyShipIndex++)
	{
		Ship enemyShip = enemyShips[enemyShipIndex];

		for (int t = 1; t <= MAX_FIRING_RANGE; t++)
		{
			CubeCoord futureEnemyPos = enemyShip.cubePos + DIRECTIONS[enemyShip.rotation] * (t * enemyShip.speed);

			if (ComputeDistance(ship->cubePos, futureEnemyPos) == t && t < shortestShotDistance)
			{
				shortestShotDistance = t;
				shortestShotPos = CubeToOffset(futureEnemyPos);
			}
		}
	}

	if (shortestShotDistance < MAX_FIRING_RANGE + 1)
	{
		cout << "FIRE " << shortestShotPos.col << " " << shortestShotPos.row << endl;
		ship->hasFired = 1;
		return true;
	}

	return false;
}
