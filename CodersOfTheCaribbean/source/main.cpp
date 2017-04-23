#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <time.h>
#include <queue>
#include <memory>

using namespace std;

//=============================================================
// CONSTANTS
//=============================================================

const int MAP_WIDTH = 23;
const int MAP_HEIGHT = 21;

const int MAX_FIRING_RANGE = 5;

//#############################################################
//#############################################################
// NAMESPACE COORDINATES
//#############################################################
//#############################################################

namespace coordinates
{

//=============================================================
// DECLARATIONS
//=============================================================

/*struct CubeCoord;
 struct OffsetCoord;

 const int PosToIndex(const int& x, const int& y);
 const CubeCoord OffsetToCube(const OffsetCoord& offset);
 const OffsetCoord CubeToOffset(const CubeCoord& cube);

 const bool operator==(const OffsetCoord& a, const OffsetCoord& b);
 const bool operator<(const OffsetCoord& a, const OffsetCoord& b);

 const int ComputeDistance(const CubeCoord& a, const CubeCoord& b);*/

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

//=============================================================
// FUNCTIONS
//=============================================================

const int PosToIndex(const int& x, const int& y)
{
	return x + y * MAP_WIDTH;
}

const int OffsetToIndex(const OffsetCoord& offset)
{
	return PosToIndex(offset.col, offset.row);
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

const int RoundDirection(int direction)
{
	while (direction < 0)
	{
		direction += 6;
	}

	return direction % 6;
}

const int GetOppositeDirection(int direction)
{
	return RoundDirection(direction + 6);
}

//=============================================================
// CONSTANTS
//=============================================================

const CubeCoord DIRECTIONS[6] =
{ CubeCoord(1, -1, 0), CubeCoord(1, 0, -1), CubeCoord(0, 1, -1), CubeCoord(-1, 1, 0), CubeCoord(-1, 0, 1), CubeCoord(0, -1, 1) };

}
using namespace coordinates;

//#############################################################
//#############################################################
// NAMESPACE GAMEDATA
//#############################################################
//#############################################################

namespace gamedata
{

//=============================================================
// DECLARATIONS
//=============================================================

struct Ship;
struct Barrel;

Ship* GetShip(vector<Ship>& ships, int entityId);

//=============================================================
// STRUCTS
//=============================================================

class Ship
{
	// VARIABLES
private:
	int entityId;
	int x;
	int y;
	int rotation;
	int speed;
	int rum;

	bool updated;
	int hasFired;
	OffsetCoord wanderTarget;

	// CONSTRUCTORS
public:
	Ship()
	{
		this->x = -1;
		this->y = -1;

		entityId = rotation = speed = rum = -1;
		hasFired = 0;
		updated = true;
	}

	Ship(int entityId, int x, int y, int rotation, int speed, int rum)
	{
		this->x = x;
		this->y = y;

		this->entityId = entityId;
		this->rotation = rotation;
		this->speed = speed;
		this->rum = rum;

		hasFired = 0;
		updated = true;
	}

	Ship(const Ship& ship)
	{
		this->x = ship.x;
		this->y = ship.y;

		this->entityId = ship.entityId;
		this->rotation = ship.rotation;
		this->speed = ship.speed;
		this->rum = ship.rum;

		this->hasFired = ship.hasFired;
		this->updated = ship.updated;
	}

	// FUNCTIONS
public:
	void Update(int x, int y, int rotation, int speed, int rum)
	{
		this->x = x;
		this->y = y;

		this->rotation = rotation;
		this->speed = speed;
		this->rum = rum;

		if (hasFired == 1)
			hasFired = 2;
		else if (hasFired == 2)
			hasFired = 0;

		updated = true;
	}
	OffsetCoord GetCenterPosOffset() const
	{
		return OffsetCoord(x, y);
	}
	CubeCoord GetCenterPosCube() const
	{
		return OffsetToCube(GetCenterPosOffset());
	}
	OffsetCoord GetFrontPos() const
	{
		return CubeToOffset(GetCenterPosCube() + DIRECTIONS[this->rotation]);
	}
	OffsetCoord GetBackPos() const
	{
		return CubeToOffset(GetCenterPosCube() + DIRECTIONS[GetOppositeDirection(this->rotation)]);
	}
	int GetEntityId() const
	{
		return entityId;
	}
	int GetX() const
	{
		return x;
	}
	int GetY() const
	{
		return y;
	}
	void SetPos(int x, int y)
	{
		this->x = x;
		this->y = y;
	}
	void SetPos(const OffsetCoord& value)
	{
		this->x = value.col;
		this->y = value.row;
	}
	int GetRotation() const
	{
		return rotation;
	}
	void SetRotation(int value)
	{
		rotation = value;
	}
	int GetSpeed() const
	{
		return speed;
	}
	void SetSpeed(int value)
	{
		speed = value;
	}
	int GetRum() const
	{
		return rum;
	}
	bool GetUpdated() const
	{
		return updated;
	}
	void SetUpdated(bool value)
	{
		updated = value;
	}
	int GetHasFired() const
	{
		return hasFired;
	}
	void SetHasFired(int value)
	{
		hasFired = value;
	}
	OffsetCoord GetWanderTarget() const
	{
		return wanderTarget;
	}
	void SetWanderTarget(const OffsetCoord& value)
	{
		wanderTarget = value;
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
// FUNCTIONS
//=============================================================

Ship* GetShip(vector<Ship>& ships, int entityId)
{
	for (unsigned int shipIndex = 0; shipIndex < ships.size(); shipIndex++)
	{
		if (ships.at(shipIndex).GetEntityId() == entityId)
		{
			return &ships.at(shipIndex);
		}
	}

	return nullptr;
}

}
using namespace gamedata;

//#############################################################
//#############################################################
// NAMESPACE PATHFINDER
//#############################################################
//#############################################################

namespace pathfinder
{

class Action
{
	// VARIABLES
private:
	int _turn = 0;
protected:
	string _firstCommand = "";
	Ship _shipState;
	int _heuristic = 0;

	// CONSTRUCTORS
protected:
	Action(shared_ptr<Action> previousAction)
	{
		if (previousAction != nullptr)
		{
			if (previousAction->GetFirstCommand() != "")
				_firstCommand = previousAction->GetFirstCommand();
			else
				_firstCommand = previousAction->GetCommand();

			_turn = previousAction->_turn + 1;
		}
	}
public:
	Action()
	{
	}
	Action(const Action& action)
	{
		_firstCommand = action._firstCommand;
		_turn = action._turn;
		_shipState = Ship(action._shipState);
		_heuristic = action._heuristic;
	}
	virtual ~Action()
	{
	}

	// FUNCTIONS
protected:
	void ComputeMove()
	{
		if (_shipState.GetSpeed() > 0)
		{
			OffsetCoord newPos = CubeToOffset(
					_shipState.GetCenterPosCube() + (DIRECTIONS[_shipState.GetRotation()] * _shipState.GetSpeed()));
			_shipState.SetPos(newPos);
		}
	}
	void ComputeHeuristic(const CubeCoord& destination)
	{
		//_heuristic = 0;
		_heuristic = ComputeDistance(destination, OffsetToCube(_shipState.GetFrontPos()));
	}
public:
	string GetFirstCommand() const
	{
		return _firstCommand;
	}
	int GetTotalCost()
	{
		return _turn + _heuristic;
	}
	Ship GetShipState() const
	{
		return _shipState;
	}
	bool IsLegal(const vector<string>& obstacleMap) const
	{
		//cerr << "IsLegal called, pos=" << _shipState.GetCenterPosOffset().col << "," << _shipState.GetCenterPosOffset().row << endl;

		OffsetCoord center = _shipState.GetCenterPosOffset();
		OffsetCoord front = _shipState.GetFrontPos();
		OffsetCoord back = _shipState.GetBackPos();

		if (center.col < 0 || front.col < 0 || back.col < 0 || center.col >= MAP_WIDTH || front.col >= MAP_WIDTH || back.col >= MAP_WIDTH)
			return false;

		if (center.row < 0 || front.row < 0 || back.row < 0 || center.row >= MAP_HEIGHT || front.row >= MAP_HEIGHT
				|| back.row >= MAP_HEIGHT)
			return false;

		if (obstacleMap[OffsetToIndex(_shipState.GetCenterPosOffset())] == "e"
				&& obstacleMap[OffsetToIndex(_shipState.GetFrontPos())] == "e"
				&& obstacleMap[OffsetToIndex(_shipState.GetBackPos())] == "e")
			return true;

		return false;
	}
	virtual string GetCommand()=0;
};

class WaitAction: public Action
{
public:
	WaitAction(const Ship& initialState, shared_ptr<Action> previousAction, const CubeCoord& destination) :
			Action(previousAction)
	{
		_shipState = Ship(initialState);

		if (_firstCommand == "")
			_firstCommand = "WAIT";

		ComputeMove();
		ComputeHeuristic(destination);
	}
	string GetCommand()
	{
		return "WAIT";
	}
};

class PortAction: public Action
{
public:
	PortAction(const Ship& initialState, shared_ptr<Action> previousAction, const CubeCoord& destination) :
			Action(previousAction)
	{
		_shipState = Ship(initialState);

		if (_firstCommand == "")
			_firstCommand = "PORT";

		ComputeMove();

		_shipState.SetRotation(RoundDirection(_shipState.GetRotation() + 1));

		ComputeHeuristic(destination);
	}
	string GetCommand()
	{
		return "PORT";
	}
};

class StarboardAction: public Action
{
public:
	StarboardAction(const Ship& initialState, shared_ptr<Action> previousAction, const CubeCoord& destination) :
			Action(previousAction)
	{
		_shipState = Ship(initialState);

		if (_firstCommand == "")
			_firstCommand = "STARBOARD";

		ComputeMove();

		_shipState.SetRotation(RoundDirection(_shipState.GetRotation() - 1));
		ComputeHeuristic(destination);
	}
	string GetCommand()
	{
		return "STARBOARD";
	}
};

class FasterAction: public Action
{
public:
	FasterAction(const Ship& initialState, shared_ptr<Action> previousAction, const CubeCoord& destination) :
			Action(previousAction)
	{
		_shipState = Ship(initialState);

		if (_firstCommand == "")
			_firstCommand = "FASTER";

		if (_shipState.GetSpeed() < 2)
			_shipState.SetSpeed(_shipState.GetSpeed() + 1);

		ComputeMove();
		ComputeHeuristic(destination);
	}
	string GetCommand()
	{
		return "FASTER";
	}
};

class SlowerAction: public Action
{
public:
	SlowerAction(const Ship& initialState, shared_ptr<Action> previousAction, const CubeCoord& destination) :
			Action(previousAction)
	{
		_shipState = Ship(initialState);

		if (_firstCommand == "")
			_firstCommand = "SLOWER";

		if (_shipState.GetSpeed() > 0)
			_shipState.SetSpeed(_shipState.GetSpeed() - 1);

		ComputeMove();
		ComputeHeuristic(destination);
	}
	string GetCommand()
	{
		return "SLOWER";
	}
};

template<typename T, typename priority_t>
class PriorityQueue
{
	typedef pair<priority_t, T> element;

	struct PriorityCompare
	{
		bool operator()(const element& lhs, const element& rhs)
		{
			return lhs.first > rhs.first;
		}
	};

public:
	inline bool Empty() const
	{
		return elements.empty();
	}

	inline void Push(T item, priority_t priority)
	{
		elements.emplace(priority, item);
	}

	inline T Pop()
	{
		T item = elements.top().second;
		elements.pop();
		return item;
		/*element e = elements.top();
		 priority_t p = e.first;
		 item = e.second;
		 elements.pop();
		 return p;*/
	}

private:
	priority_queue<element, vector<element>, PriorityCompare> elements;
};

void ExpandAction(const Ship& initialState, shared_ptr<Action> previousAction, const vector<string>& obstacleMap,
		const OffsetCoord& destination, vector<shared_ptr<Action>>& actions, PriorityQueue<shared_ptr<Action>, int>& queue)
{
	vector<shared_ptr<Action>> newActions;
	CubeCoord cubeDestination = OffsetToCube(destination);

	if (initialState.GetSpeed() > 0)
	{
		shared_ptr<Action> waitAction(new WaitAction(initialState, previousAction, cubeDestination));
		newActions.push_back(waitAction);
	}

	shared_ptr<Action> portAction(new PortAction(initialState, previousAction, cubeDestination));
	newActions.push_back(portAction);

	shared_ptr<Action> starboardAction(new StarboardAction(initialState, previousAction, cubeDestination));
	newActions.push_back(starboardAction);

	if (initialState.GetSpeed() < 2)
	{
		shared_ptr<Action> fasterAction(new FasterAction(initialState, previousAction, cubeDestination));
		newActions.push_back(fasterAction);
	}

	if (initialState.GetSpeed() > 0)
	{
		shared_ptr<Action> slowerAction(new SlowerAction(initialState, previousAction, cubeDestination));
		newActions.push_back(slowerAction);
	}

	for (unsigned int i = 0; i < newActions.size(); i++)
	{
		//cerr << "pre legal test, i=" << i << endl;
		if (newActions[i]->IsLegal(obstacleMap))
		{
			//cerr << "ExpandAction: new action is legal" << endl;
			int index = actions.size();
			actions.push_back(newActions[i]);
			shared_ptr<Action> action = actions.at(index);
			queue.Push(action, newActions[i]->GetTotalCost());
		}
	}
}

int FindPath(const Ship& ship, const vector<string>& objectMap, const OffsetCoord destination, string& command)
{
	vector<shared_ptr<Action>> actions;
	PriorityQueue<shared_ptr<Action>, int> queue;

	ExpandAction(ship, nullptr, objectMap, destination, actions, queue);

	shared_ptr<Action> lastAction = nullptr;

	while (!queue.Empty())
	{
		shared_ptr<Action> currentAction = queue.Pop();

		Ship currentShipState = Ship(currentAction->GetShipState());

		if (currentShipState.GetFrontPos() == destination)
		{
			lastAction = currentAction;
			break;
		}

		if (currentShipState.GetFrontPos().col == destination.col && currentShipState.GetFrontPos().row == destination.row)
		{
			cerr << "destination found!" << endl;
		}

		ExpandAction(currentShipState, currentAction, objectMap, destination, actions, queue);
	}

	int result = -1;
	if (lastAction != nullptr)
	{
		command = (*lastAction).GetFirstCommand();
		result = 1;
	}

	return result;
}

}
using namespace pathfinder;

//=============================================================
// DECLARATIONS
//=============================================================

bool CommandGoToBarrel(Ship* ship, const vector<Barrel>& barrels, const vector<string>& objectMap);
bool CommandWander(Ship* ship, const vector<string>& objectMap);
bool CommandFire(Ship* ship, const vector<Ship>& enemyShips);

//=============================================================
// MAIN
//=============================================================
int main()
{
	srand(time(NULL));

	// game loop variables
	vector<Ship> _myShips;
	vector<Ship> _enemyShips;
	vector<Barrel> _barrels;

	// Map with all the obstacles
	// "e" = no obstacle
	// "M" = Mine
	// "C" = Canonball
	vector<string> _objectMap = vector<string>(MAP_WIDTH * MAP_HEIGHT);

	for (int col = 0; col < MAP_WIDTH; col++)
	{
		for (int row = 0; row < MAP_HEIGHT; row++)
		{
			int index = PosToIndex(col, row);
			_objectMap[index] = "e";
		}
	}

	//*****************************
//	Ship ship = Ship(0, 5, 5, 0, 0, 100);
//	string command;
//	int result = FindPath(ship, _objectMap, OffsetCoord(10, 10), command);
//	cerr << "path test: result=" << result << " command=" << command << endl;
	//*****************************

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
				//_objectMap[PosToIndex(x, y)] = 'B';
			}
			else if (entityType == "CANNONBALL")
			{
				_objectMap[PosToIndex(x, y)] = "C" + arg2;
			}
			else if (entityType == "MINE")
			{
				_objectMap[PosToIndex(x, y)] = "M";
			}
		}

		for (unsigned int i = 0; i < _myShips.size(); i++)
		{
			if (!_myShips[i].GetUpdated())
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
			else if (currentShip->GetRum() <= 80 && CommandGoToBarrel(currentShip, _barrels, _objectMap))
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
		for (unsigned int i = 0; i < _myShips.size(); i++)
		{
			_myShips[i].SetUpdated(false);
		}

		_enemyShips.clear();
		_barrels.clear();

		for (int col = 0; col < MAP_WIDTH; col++)
		{
			for (int row = 0; row < MAP_HEIGHT; row++)
			{
				int index = PosToIndex(col, row);
				_objectMap[index] = "e";
			}
		}
	}
}

//=============================================================
//
//=============================================================

bool CommandGoToBarrel(Ship* ship, const vector<Barrel>& barrels, const vector<string>& objectMap)
{
	int shortestDistance = 999;
	OffsetCoord nearestBarrelPos;
	for (unsigned int barrelIndex = 0; barrelIndex < barrels.size(); barrelIndex++)
	{
		Barrel currentBarrel = barrels[barrelIndex];

		int distance = ComputeDistance(ship->GetCenterPosCube(), currentBarrel.cubePos);

		if (distance < shortestDistance)
		{
			shortestDistance = distance;
			nearestBarrelPos = currentBarrel.offsetPos;
		}
	}

	if (!nearestBarrelPos.IsNull())
	{
		string command;
		if (FindPath(*ship, objectMap, nearestBarrelPos, command) > 0)
		{
			cerr << "FindPath success, command=" << command << endl;
			cout << command << endl;
			return true;
		}
		else
		{
			cerr << "FindPath failed!" << endl;
			cout << "MOVE " << nearestBarrelPos.col << " " << nearestBarrelPos.row << endl;
			return true;
		}

//		cerr << "CommandGoToBarrel: shortestDistance=" << shortestDistance << endl;
//		cout << "MOVE " << nearestBarrelPos.col << " " << nearestBarrelPos.row << endl;
//		return true;
	}

	return false;
}

bool CommandWander(Ship* ship, const vector<string>& objectMap)
{
	if (ship->GetCenterPosOffset() == ship->GetWanderTarget())
	{
		cerr << "CommandWander: target reached" << endl;
		ship->SetWanderTarget(OffsetCoord());
	}

	if (ship->GetWanderTarget().IsNull())
	{
		int x, y;

		do
		{
			x = 1 + (rand() % (int) (MAP_WIDTH - 1));
			y = 1 + (rand() % (int) (MAP_HEIGHT - 1));
			cerr << "CommandWander: newX=" << x << " newY=" << y << endl;
		} while (objectMap[PosToIndex(x, y)] != "e");

		ship->SetWanderTarget(OffsetCoord(x, y));
	}

	string command;
	if (FindPath(*ship, objectMap, ship->GetWanderTarget(), command) > 0)
	{
		cerr << "FindPath success, command=" << command << endl;
		cout << command << endl;
		return true;
	}
	else
	{
		cerr << "FindPath failed!" << endl;
		cout << "MOVE " << ship->GetWanderTarget().col << " " << ship->GetWanderTarget().row << endl;
		return true;
	}

	return false;
}

bool CommandFire(Ship* ship, const vector<Ship>& enemyShips)
{
	if (ship->GetHasFired() != 0)
		return false;

	int shortestShotDistance = MAX_FIRING_RANGE + 1;
	OffsetCoord shortestShotPos;

	for (unsigned int enemyShipIndex = 0; enemyShipIndex < enemyShips.size(); enemyShipIndex++)
	{
		Ship enemyShip = enemyShips[enemyShipIndex];

		for (int t = 1; t <= MAX_FIRING_RANGE; t++)
		{
			CubeCoord futureEnemyPos = enemyShip.GetCenterPosCube() + DIRECTIONS[enemyShip.GetRotation()] * (t * enemyShip.GetSpeed());
			int timeToHit = round(1.0 + ComputeDistance(ship->GetCenterPosCube(), futureEnemyPos) / 3.0);

			if (timeToHit == t && t < shortestShotDistance)
			{
				shortestShotDistance = t;
				shortestShotPos = CubeToOffset(futureEnemyPos);
			}
		}
	}

	if (shortestShotDistance < MAX_FIRING_RANGE + 1)
	{
		cout << "FIRE " << shortestShotPos.col << " " << shortestShotPos.row << endl;
		ship->SetHasFired(1);
		return true;
	}

	return false;
}
