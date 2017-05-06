//============================================================================
// Name        : GhostInTheCell.cpp
// Author      : Patrick Monville
// Version     :
// Copyright   : The world is my oister!
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <queue>
#include <ctime>
#include <math.h>

using namespace std;

const int MAX_DISTANCE = 20;
const int MAX_NUMBER_COMBINATIONS = 15000;

//################################################################################
//################################################################################
// NAMESPACE UTILITIES
//################################################################################
//################################################################################
namespace utilities
{

inline static unsigned long long ComputeNumberCombinations(unsigned long long n, unsigned long long r)
{
	//cerr << "ComputeNumberCombinations(" << n << ", " << r << ") = ";

	if (r > n)
	{
		return 0;
	}

	unsigned long long result = 1;

	for (unsigned long long d = 1; d <= r; ++d)
	{
		result *= n--;
		result /= d;
	}

	//cerr << result << endl;
	return result;
}

inline static unsigned long long ComputeTotalNumberCombinations(unsigned long long n, unsigned long long r)
{
	unsigned long long result = 0;

	for (unsigned int i = 1; i <= r; i++)
	{
		result += ComputeNumberCombinations(n, i);
	}

	return result;
}

inline static double Elapsed(timespec& begin)
{
	timespec end;
	clock_gettime(CLOCK_REALTIME, &end);
	return end.tv_sec - begin.tv_sec + (end.tv_nsec - begin.tv_nsec) / 1000000.;
}

template<typename T, typename priority_t>
class PriorityQueueMax
{
	typedef pair<priority_t, T> element;

	struct PriorityCompare
	{
		bool operator()(const element& lhs, const element& rhs)
		{
			return lhs.first < rhs.first;
		}
	};

private:
	priority_queue<element, vector<element>, PriorityCompare> elements;

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
		element e = elements.top();
		elements.pop();
		return e.second;
	}

	inline void Clear()
	{
		while (!elements.empty())
		{
			elements.pop();
		}
	}
};

template<typename T, typename priority_t>
class PriorityQueueMin
{
	typedef pair<priority_t, T> element;

	struct PriorityCompare
	{
		bool operator()(const element& lhs, const element& rhs)
		{
			return lhs.first > rhs.first;
		}
	};

private:
	priority_queue<element, vector<element>, PriorityCompare> elements;

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
		element e = elements.top();
		elements.pop();
		return e.second;
	}

	inline void Clear()
	{
		while (!elements.empty())
		{
			elements.pop();
		}
	}
};

}
using namespace utilities;

//################################################################################
//################################################################################
// NAMESPACE MODEL
//################################################################################
//################################################################################
namespace model
{

class PathNode
{
private:
	int _factoryId = -1;
	int _previousFactoryId = -1;
	int _timeCost = 0;
	int _waypoints = 0;

public:
	PathNode(int factoryId, int previousFactoryId, int timeCost, int waypoints)
			: _factoryId(factoryId), _previousFactoryId(previousFactoryId), _timeCost(timeCost), _waypoints(waypoints)
	{
	}

	inline void UpdateNode(int previousFactoryId, int timeCost, int waypoints)
	{
		_previousFactoryId = previousFactoryId;
		_timeCost = timeCost;
		_waypoints = waypoints;
	}

	inline int GetFactoryId() const
	{
		return _factoryId;
	}

	inline int GetPreviousFactoryId() const
	{
		return _previousFactoryId;
	}

	inline int GetTimeCost() const
	{
		return _timeCost;
	}

	inline int GetWaypoints() const
	{
		return _waypoints;
	}
};

class Factory
{
private:
	int _id = -1;

	int _owner = 0;
	int _numCyborgs = 0;
	int _production = 0;
	int _incapacitatedTimer = 0;

public:
	Factory()
	{
	}

	Factory(int id)
			: _id(id)
	{
	}

	inline void Update(int owner, int numCyborgs, int production, int incapacitatedTimer)
	{
		_owner = owner;
		_numCyborgs = numCyborgs;
		_production = production;
		_incapacitatedTimer = incapacitatedTimer;
	}

	inline int GetId() const
	{
		return _id;
	}

	inline int GetOwner() const
	{
		return _owner;
	}

	inline int GetNumCyborgs() const
	{
		return _numCyborgs;
	}

	inline int GetProduction() const
	{
		return _production;
	}

	inline int GetIncapacitatedTimer() const
	{
		return _incapacitatedTimer;
	}
};

class Troop
{
private:
	int _id = -1;

	int _owner = 0;
	int _originId = -1;
	int _destinationId = -1;
	int _numCyborgs = 0;
	int _timeRemaining = 0;

public:
	Troop()
	{
	}

	Troop(int id, int owner, int originId, int destinationId, int numCyborgs, int timeRemaining)
			: _id(id), _owner(owner), _originId(originId), _destinationId(destinationId), _numCyborgs(numCyborgs), _timeRemaining(
					timeRemaining)
	{
	}

	inline void Update(int id, int owner, int originId, int destinationId, int numCyborgs, int timeRemaining)
	{
		_id = id;
		_owner = owner;
		_originId = originId;
		_destinationId = destinationId;
		_numCyborgs = numCyborgs;
		_timeRemaining = timeRemaining;
	}

	inline int GetId() const
	{
		return _id;
	}

	inline int GetOwner() const
	{
		return _owner;
	}

	inline int GetOriginId() const
	{
		return _originId;
	}

	inline int GetDestinationId() const
	{
		return _destinationId;
	}

	inline int GetNumCyborgs() const
	{
		return _numCyborgs;
	}

	inline int GetTimeRemaining() const
	{
		return _timeRemaining;
	}
};

class Bomb
{
private:
	int _id = -1;
	int _owner = 0;
	int _originFactoryId = -1;
	int _targetFactoryId = -1;
	int _timeRemaining = -1;

public:
	Bomb()
	{
	}

	Bomb(int id, int owner, int originFactoryId, int targetFactoryId, int timeRemaining)
			: _id(id), _owner(owner), _originFactoryId(originFactoryId), _targetFactoryId(targetFactoryId), _timeRemaining(timeRemaining)

	{
	}

	inline int GetId() const
	{
		return _id;
	}

	inline int GetOwner() const
	{
		return _owner;
	}

	inline int GetOriginFactoryId() const
	{
		return _originFactoryId;
	}

	inline int GetTargetFactoryId() const
	{
		return _targetFactoryId;
	}

	inline int GetTimeRemaining() const
	{
		return _timeRemaining;
	}
};

class Model
{
private:
	int _numFactories = 0;

	vector<int> _distancesVector;
	std::vector<vector<int>> _pathsVector;
	std::vector<int> _waypointValue; //index = factoryIndex, value = number of paths passing through a factory

	vector<Factory> _factories;
	vector<Troop> _troops;
	vector<Bomb> _bombs;

public:
	Model()
	{
	}

	Model(int numFactories)
	{
		_numFactories = numFactories;

		_distancesVector.resize(_numFactories * _numFactories);
		_pathsVector.resize(_numFactories * _numFactories);
		_waypointValue.resize(_numFactories);

		for (int i = 0; i < _numFactories; i++)
		{
			_factories.emplace_back(i);
		}
	}

	//=======================================================
	// SETTER

	inline void SetDistance(int factory1, int factory2, int distance)
	{
		_distancesVector[GetConnectionId(factory1, factory2)] = distance;
		_distancesVector[GetConnectionId(factory2, factory1)] = distance;
	}

	inline void SetFactory(int factoryId, int owner, int numCyborgs, int production, int incapacitatedTimer)
	{
		_factories[factoryId].Update(owner, numCyborgs, production, incapacitatedTimer);
	}

	inline void SetTroop(int id, int owner, int originId, int destinationId, int numCyborgs, int timeRemaining)
	{
		_troops.emplace_back(id, owner, originId, destinationId, numCyborgs, timeRemaining);
	}

	inline void SetBomb(int id, int owner, int originFactoryId, int targetFactoryId, int timeRemaining)
	{
		_bombs.emplace_back(id, owner, originFactoryId, targetFactoryId, timeRemaining);
	}

	//=======================================================
	// GETTER

	/**
	 *
	 */
	inline int GetNumFactories() const
	{
		return _numFactories;
	}

	/**
	 *
	 */
	inline int GetDistance(int fromFactoryId, int toFactoryId) const
	{
		return _distancesVector[GetConnectionId(fromFactoryId, toFactoryId)];
	}

	/**
	 *
	 */
	double GetAverageDistance(int targetFactoryId, int owner, int minProduction) const;

	/**
	 *
	 */
	vector<int> GetFactoriesSortedByDistance(const int originFactoryId, const int owner) const;

	/**
	 *
	 */
	inline Factory GetFactory(int id) const
	{
		return _factories[id];
	}

	/**
	 *
	 */
	vector<Factory> GetOwnedFactories(int owner) const;

	/**
	 *
	 */
	vector<Troop> GetTroops(int targetFactoryId) const;

	/**
	 *
	 */
	vector<Bomb> GetBombs(int targetFactoryId) const;

	/**
	 *
	 */
	inline vector<int> GetPath(int fromFactoryId, int toFactoryId) const
	{
		return _pathsVector[GetConnectionId(fromFactoryId, toFactoryId)];
	}

	/**
	 *
	 */
	inline int GetWaypointValue(int factoryIndex) const
	{
		return _waypointValue[factoryIndex];
	}

	//=======================================================
	// OTHER

	/**
	 *
	 */
	inline void CleanUp()
	{
		_troops.clear();
		_bombs.clear();
	}

	/**
	 *
	 */
	void ComputePaths();

private:

	/**
	 *
	 */
	inline int GetConnectionId(int fromFactoryId, int toFactoryId) const
	{
		return fromFactoryId * _numFactories + toFactoryId;
	}

	/**
	 *
	 */
	static vector<int> FindBestPath(const Model& model, int fromFactoryId, int toFactoryId);
};

//==================================================================================
//
//==================================================================================
double Model::GetAverageDistance(int targetFactoryId, int owner, int minProduction) const
{
	int totalDistance = 0;
	int amountDistances = 0;
	vector<Factory> ownedFactories = GetOwnedFactories(owner);

	for (unsigned int i = 0; i < ownedFactories.size(); i++)
	{
		Factory factory = ownedFactories[i];

		if (factory.GetId() != targetFactoryId && factory.GetProduction() >= minProduction)
		{
			totalDistance += GetDistance(targetFactoryId, factory.GetId());
			amountDistances++;
		}
	}

	return totalDistance / (double) max(1, amountDistances);
}

//==================================================================================
//
//==================================================================================
vector<int> Model::GetFactoriesSortedByDistance(const int originFactoryId, const int owner) const
{
	vector<int> result;
	vector<Factory> ownedFactories = GetOwnedFactories(owner);

	for (unsigned int i = 0; i < ownedFactories.size(); i++)
	{
		if (ownedFactories[i].GetId() != originFactoryId)
		{
			result.push_back(ownedFactories[i].GetId());
		}
	}

	if (result.size() <= 1)
	{
		return result;
	}

	bool switched = true;
	while (switched)
	{
		switched = false;

		for (unsigned int i = 0; i < result.size() - 1; i++)
		{
			if (GetDistance(originFactoryId, result[i]) > GetDistance(originFactoryId, result[i + 1]))
			{
				switched = true;
				int tmp = result[i];
				result[i] = result[i + 1];
				result[i + 1] = tmp;
			}
		}
	}

	return result;
}

//==================================================================================
//
//==================================================================================
vector<Factory> Model::GetOwnedFactories(int owner) const
{
	vector<Factory> result;
	auto pos = _factories.begin();

	while (pos != _factories.end())
	{
		pos = find_if(pos, _factories.end(), [&owner](const Factory& factory)
		{	return factory.GetOwner()==owner;});

		if (pos != _factories.end())
		{
			result.emplace_back(_factories[distance(_factories.begin(), pos)]);
			pos++;
		}
	}

	return result;
}

//==================================================================================
//
//==================================================================================
vector<Troop> Model::GetTroops(int targetFactoryId) const
{
	vector<Troop> result;
	auto pos = _troops.begin();

	while (pos != _troops.end())
	{
		pos = find_if(pos, _troops.end(), [&targetFactoryId](const Troop& troop)
		{	return troop.GetDestinationId()==targetFactoryId;});

		if (pos != _troops.end())
		{
			result.emplace_back(_troops[distance(_troops.begin(), pos)]);
			pos++;
		}
	}

	return result;
}

//==================================================================================
//
//==================================================================================
vector<Bomb> Model::GetBombs(int targetFactoryId) const
{
	vector<Bomb> result;
	auto pos = _bombs.begin();

	while (pos != _bombs.end())
	{
		pos = find_if(pos, _bombs.end(), [&targetFactoryId](const Bomb& bomb)
		{	return bomb.GetTargetFactoryId()==targetFactoryId;});

		if (pos != _bombs.end())
		{
			result.emplace_back(_bombs[distance(_bombs.begin(), pos)]);
			pos++;
		}
	}

	return result;
}

void Model::ComputePaths()
{
	for (int fromFactoryId = 0; fromFactoryId < _numFactories; fromFactoryId++)
	{
		for (int toFactoryId = 0; toFactoryId < _numFactories; toFactoryId++)
		{
			if (fromFactoryId == toFactoryId)
			{
				continue;
			}

			vector<int> path = FindBestPath(*this, fromFactoryId, toFactoryId);

			for (unsigned int i = 0; i < path.size() - 1; i++)
			{
				_waypointValue[path[i]] += 1;
			}

			_pathsVector[GetConnectionId(fromFactoryId, toFactoryId)] = path;
		}
	}
}

vector<int> Model::FindBestPath(const Model& model, int fromFactoryId, int toFactoryId)
{
	vector<PathNode> nodes;
	PriorityQueueMin<int, int> openNodes;

	//creating the nodes
	for (int factoryId = 0; factoryId < model.GetNumFactories(); factoryId++)
	{
		nodes.emplace_back(factoryId, -1, 0, 0);
	}

	openNodes.Push(fromFactoryId, 0);

	while (!openNodes.Empty())
	{
		int currentFactoryId = openNodes.Pop();

		if (currentFactoryId == toFactoryId)
		{
			continue;
		}

		for (int factoryId = 0; factoryId < model.GetNumFactories(); factoryId++)
		{
			if (factoryId == currentFactoryId || factoryId == fromFactoryId)
			{
				continue;
			}

			int newCost = nodes[currentFactoryId].GetTimeCost() + model.GetDistance(currentFactoryId, factoryId);
			int newWaypoints = nodes[currentFactoryId].GetWaypoints();

			//if the factory being evaluated is not the destination:
			//-increase the time cost because the units will stay there for a turn
			//-increase the waypoint count
			//the waypoint count of a factory includes that factory,except if the factory is the destination or the origin
			if (factoryId != toFactoryId)
			{
				newCost += 1;
				newWaypoints += 1;
			}

			if (nodes[factoryId].GetPreviousFactoryId() == -1
					|| (nodes[factoryId].GetTimeCost() >= newCost && nodes[factoryId].GetWaypoints() <= newWaypoints))
			{
				nodes[factoryId].UpdateNode(currentFactoryId, newCost, newWaypoints);
				openNodes.Push(factoryId, newCost);
			}
		}
	}

	vector<int> path;
	int currentNode = toFactoryId;
	while (currentNode != fromFactoryId)
	{
		path.push_back(currentNode);
		currentNode = nodes[currentNode].GetPreviousFactoryId();
	}

	std::reverse(path.begin(), path.end());

	return path;
}

}
using namespace model;

//################################################################################
//################################################################################
// "DECISION MAKING"
//################################################################################
//################################################################################
namespace decisions
{

class Objective
{
private:
	int _targetFactoryId = -1;
	int _neededCyborgs = 0;
	double _score = 0;

public:
	Objective()
	{
	}

	Objective(int targetFactoryId, int neededCyborgs, double score)
			: _targetFactoryId(targetFactoryId), _neededCyborgs(neededCyborgs), _score(score)
	{
	}

	inline int GetTargetFactoryId() const
	{
		return _targetFactoryId;
	}

	inline int GetNeededCyborgs() const
	{
		return _neededCyborgs;
	}

	inline double GetScore() const
	{
		return _score;
	}
};

class Assignment
{
private:
	int _targetFactoryId = -1;
	int _originFactoryId = -1;
	int _amountCyborgs = 0;
	double _score = 0;

public:
	Assignment()
	{
	}

	Assignment(int targetFactoryId, int originFactoryId, int amountCyborgs, double score)
			: _targetFactoryId(targetFactoryId), _originFactoryId(originFactoryId), _amountCyborgs(amountCyborgs), _score(score)
	{
	}

	inline int GetTargetFactoryId() const
	{
		return _targetFactoryId;
	}

	inline int GetOriginFactoryId() const
	{
		return _originFactoryId;
	}

	inline int GetAmountCyborgs() const
	{
		return _amountCyborgs;
	}

	inline double GetScore() const
	{
		return _score;
	}
};

class Bot
{
//variables
private:
	int _numAvailBombs = 2;
	vector<int> _bombTargets;
	int _currentTurn = 0;
	int _startFactoryId = -1;

//public methods
public:
	string ComputeMoves(const Model& model);

//private methods
private:

	//=====================================================================
	// COMMAND METHODS

	/**
	 * Loops through all Factories and creates Objectives.
	 *
	 */
	static void EvaluateFactories(const Model& model, vector<Objective>& objectives, vector<int>& availableCyborgs);

	/**
	 * Evaluates an owned Factory:
	 * - creates reinforcement Objectives if necessary
	 *
	 */
	static bool EvaluateOwnedFactory(const Model& model, const Factory& targetFactory, Objective& objective, vector<int>& availableCyborgs);

	/**
	 * Evaluates an enemy Factory and creates an attack objective.
	 *
	 */
	static bool EvaluateEnemyFactory(const Model& model, const Factory& targetFactory, Objective& objective);

	/**
	 *
	 */
	static string LaunchBombs(const Model& model, int numAvailBombs, vector<int>& bombTargets);

	/**
	 *
	 */
	inline static double ComputeFactoryValue(const Model& model, int factoryId)
	{
		return model.GetFactory(factoryId).GetProduction() + 0.01 * pow(model.GetWaypointValue(factoryId), 0.5) + 0.1;
	}

	//=====================================================================
	//

	/**
	 * Assigns available Cyborgs to the Objectives.
	 *
	 */
	static string AssignTroops(const Model& model, const vector<Objective>& objectives, vector<int>& availableCyborgs);

	/**
	 *
	 *	@return: the size of the combinations that can be created
	 */
	static int CreateAssignments(const Model& model, const vector<Objective>& objectives, const vector<int>& availableCyborgs,
			vector<Assignment>& assignments);

	/**
	 *
	 */
	static void CreateAssignmentGroups(const Model& model, const vector<int>& availableCyborgs, const vector<Assignment>& assignments,
			int maxCombinationSize, vector<vector<int>>& assignmentGroups);

	//=====================================================================
	//

	/**
	 *
	 */
	static string MoveRemainingUnits(const Model& model, vector<int>& availableCyborgs);
};

//==================================================================================
//
//==================================================================================
string Bot::ComputeMoves(const Model& model)
{
	_currentTurn++;

	vector<Objective> objectives;
	vector<int> availableCyborgs = vector<int>(model.GetNumFactories());
	fill(availableCyborgs.begin(), availableCyborgs.end(), 0);

	EvaluateFactories(model, objectives, availableCyborgs);

	string commands = "";

	//****************************************************************
	// BOMBS
	string bombCommand = "";

	if (_currentTurn > 9)
	{
		bombCommand = LaunchBombs(model, _numAvailBombs, _bombTargets);
	}

	if (commands != "" && bombCommand != "")
	{
		commands += ";";
	}

	commands += bombCommand;

	//****************************************************************
	// ATTACKS
	string attackCommands = AssignTroops(model, objectives, availableCyborgs);

	if (commands != "" && attackCommands != "")
	{
		commands += ";";
	}

	commands += attackCommands;

	//****************************************************************
	// INCREASES
	string incCommand = "";
	if (_currentTurn > 5)
	{
		vector<Factory> factories = model.GetOwnedFactories(1);
		for (unsigned int i = 0; i < factories.size(); i++)
		{
			Factory factory = factories[i];
			vector<int> sortedEnemyFactories = model.GetFactoriesSortedByDistance(factory.GetId(), -1);
			if (!sortedEnemyFactories.empty())
			{
				int distToEnemy = model.GetDistance(factory.GetId(), sortedEnemyFactories[0]);

				if (factory.GetProduction() < 3 && availableCyborgs[factory.GetId()] > 15 && distToEnemy > 7)
				{
					incCommand = "INC " + to_string(factory.GetId());
					availableCyborgs[factory.GetId()] -= 10;
					break;
				}
			}
		}
	}
	else if (_currentTurn == 1)
	{
		vector<Factory> factories = model.GetOwnedFactories(1);
		_startFactoryId = factories[0].GetId();
	}

	if (commands != "" && incCommand != "")
	{
		commands += ";";
	}

	commands += incCommand;

	//****************************************************************
	// MOVES
	string moveCommands = MoveRemainingUnits(model, availableCyborgs);

	if (commands != "" && moveCommands != "")
	{
		commands += ";";
	}

	commands += moveCommands;

	return commands;
}

//==================================================================================
//
//==================================================================================
void Bot::EvaluateFactories(const Model& model, vector<Objective>& objectives, vector<int>& availableCyborgs)
{
	if (objectives.size() > 0)
	{
		objectives.clear();
	}

	timespec beginEvaluating;
	clock_gettime(CLOCK_REALTIME, &beginEvaluating);
	cerr << "start evaluating factories" << endl;

	for (int currentFactoryId = 0; currentFactoryId < model.GetNumFactories(); currentFactoryId++)
	{
		Factory currentFactory = model.GetFactory(currentFactoryId);
		Objective objective;
		bool objectiveCreated = false;

		if (currentFactory.GetOwner() == 1) //owned factory
		{
			objectiveCreated = EvaluateOwnedFactory(model, currentFactory, objective, availableCyborgs);
		}
		else //factory controlled by neutral or enemy
		{
			objectiveCreated = EvaluateEnemyFactory(model, currentFactory, objective);
		}

		if (objectiveCreated)
		{
			objectives.emplace_back(objective);
		}
	}

	double time1 = Elapsed(beginEvaluating);
	cerr << "- finished evaluating factories (" << time1 << ") :" << endl;
	cerr << "  * created objectives=" << objectives.size() << endl;
}

//==================================================================================
//
//==================================================================================
bool Bot::EvaluateOwnedFactory(const Model& model, const Factory& targetFactory, Objective& objective, vector<int>& availableCyborgs)
{
	int targetFactoryId = targetFactory.GetId();
	vector<Troop> incomingTroops = model.GetTroops(targetFactoryId);
	vector<int> incomingDeltas = vector<int>(MAX_DISTANCE);
	std::fill(incomingDeltas.begin(), incomingDeltas.end(), 0);

	//counting incoming friendlies and enemies depending on their ETA
	for (unsigned int troopIndex = 0; troopIndex < incomingTroops.size(); troopIndex++)
	{
		Troop troop = incomingTroops[troopIndex];

		if (troop.GetOwner() == 1)
		{
			incomingDeltas[troop.GetTimeRemaining()] += troop.GetNumCyborgs();
		}
		else if (troop.GetOwner() == -1)
		{
			incomingDeltas[troop.GetTimeRemaining()] -= troop.GetNumCyborgs();
		}
	}

	int amountCyborgs = targetFactory.GetNumCyborgs();
	int firstEmergency = MAX_DISTANCE + 1;
	int neededReinforcements = 0;

	//run a simulation to see if reinforcements are needed
	for (int turn = 0; turn < MAX_DISTANCE; turn++)
	{
		if (amountCyborgs >= 0 && targetFactory.GetIncapacitatedTimer() - turn > 1)
		{
			amountCyborgs += targetFactory.GetProduction();
		}
		else if (amountCyborgs <= 0)
		{
			amountCyborgs -= targetFactory.GetProduction();
		}

		amountCyborgs += incomingDeltas[turn];

		if (amountCyborgs < 0)
		{
			if (turn < firstEmergency)
			{
				firstEmergency = turn;
				neededReinforcements = -amountCyborgs + 1;
			}
		}
	}

	/*if (targetFactory.GetIncapacitatedTimer() > 0)
	 {
	 firstEmergency = 1;
	 neededReinforcements += 2;
	 }*/

	if (neededReinforcements > 0)
	{
		//double priority = min((targetFactory.GetProduction() * 3.0) / (double) firstEmergency, 1.0);
		double priority = ComputeFactoryValue(model, targetFactoryId);

		objective = Objective(targetFactoryId, neededReinforcements, priority);
		return true;
	}
	else if (amountCyborgs > 0)
	{
		availableCyborgs[targetFactoryId] = amountCyborgs;
	}

	return false;
}

//==================================================================================
//
//==================================================================================
bool Bot::EvaluateEnemyFactory(const Model& model, const Factory& targetFactory, Objective& objective)
{
	int targetFactoryId = targetFactory.GetId();
	vector<Troop> incomingTroops = model.GetTroops(targetFactoryId);
	int friendlies = 0;
	int enemies = targetFactory.GetNumCyborgs();

	//looping through the incoming troops to count incoming friendlies and enemies
	for (unsigned int troopIndex = 0; troopIndex < incomingTroops.size(); troopIndex++)
	{
		Troop troop = incomingTroops[troopIndex];

		if (troop.GetOwner() == 1)
		{
			friendlies += troop.GetNumCyborgs();
		}
		else if (troop.GetOwner() == -1)
		{
			enemies += troop.GetNumCyborgs();
		}
	}

	//computing the amount of cyborgs needed to take the targeted factory
	int neededCyborgs = enemies - friendlies + 2;

	if (targetFactory.GetOwner() == -1)
	{
		neededCyborgs += model.GetAverageDistance(targetFactoryId, 1, 0) * targetFactory.GetProduction();
	}

	if (neededCyborgs > 0)
	{
		//double score = max(targetFactory.GetProduction() / 3.0, 0.25);
		double score = ComputeFactoryValue(model, targetFactoryId);

		objective = Objective(targetFactoryId, neededCyborgs, score);
		return true;
	}

	return false;
}

//==================================================================================
//
//==================================================================================
string Bot::LaunchBombs(const Model& model, int numAvailBombs, vector<int>& bombTargets)
{
	string command = "";

	if (numAvailBombs == 0)
	{
		return command;
	}

	vector<Factory> targets = model.GetOwnedFactories(-1);

	if (targets.empty())
	{
		return command;
	}

	int selectedTarget;
	int selectedOrigin;
	int targetDistance = MAX_DISTANCE + 1;

	for (unsigned int i = 0; i < targets.size(); i++)
	{
		Factory target = targets[i];

		if (target.GetProduction() >= 2 && find(bombTargets.begin(), bombTargets.end(), target.GetId()) == bombTargets.end())
		{
			vector<int> sourceFactories = model.GetFactoriesSortedByDistance(target.GetId(), 1);

			vector<Troop> incomingTroopsVector = model.GetTroops(target.GetId());
			vector<int> incomingFriendliesVector(MAX_DISTANCE);

			for (unsigned int incomingTroopsIndex = 0; incomingTroopsIndex < incomingTroopsVector.size(); incomingTroopsIndex++)
			{
				Troop troop = incomingTroopsVector[incomingTroopsIndex];

				if (troop.GetOwner() == 1)
				{
					incomingFriendliesVector[troop.GetTimeRemaining()] += troop.GetNumCyborgs();
				}
			}

			if (!sourceFactories.empty())
			{
				int distance = model.GetDistance(sourceFactories[0], target.GetId());

				int incomingFriendlies = 0;
				for (int incomingFriendliesIndex = 0; incomingFriendliesIndex <= distance; incomingFriendliesIndex++)
				{
					incomingFriendlies += incomingFriendliesVector[incomingFriendliesIndex];
				}

				if (distance < targetDistance && incomingFriendlies < 3)
				{
					selectedTarget = target.GetId();
					selectedOrigin = sourceFactories[0];
					targetDistance = distance;
				}
			}
		}
	}

	if (targetDistance < MAX_DISTANCE + 1)
	{
		numAvailBombs -= 1;
		bombTargets.push_back(selectedTarget);

		command = "BOMB " + to_string(selectedOrigin) + " " + to_string(selectedTarget);
	}

	return command;
}

//==================================================================================
//
//==================================================================================
string Bot::AssignTroops(const Model& model, const vector<Objective>& objectives, vector<int>& availableCyborgs)
{
	string commands = "";

	//***********************************************************************
	//create assignments
	vector<Assignment> assignments;

	int maxCombinationSize = CreateAssignments(model, objectives, availableCyborgs, assignments);

	//***********************************************************************
	//create assignment groups

	vector<vector<int>> assignmentGroups;

	CreateAssignmentGroups(model, availableCyborgs, assignments, maxCombinationSize, assignmentGroups);

	//***********************************************************************
	//evaluate pairGroups

	timespec beginEvaluatingGroups;
	clock_gettime(CLOCK_REALTIME, &beginEvaluatingGroups);

	PriorityQueueMax<int, double> pairGroupPrioQueue;

	for (unsigned int pairGroupIndex = 0; pairGroupIndex < assignmentGroups.size(); pairGroupIndex++)
	{
		vector<int> currentPairGroup = assignmentGroups[pairGroupIndex];

		//################################
		// identifying the objectives
		vector<int> objectiveIds;

		for (unsigned int pairIndex = 0; pairIndex < currentPairGroup.size(); pairIndex++)
		{
			int objectiveId = assignments[currentPairGroup[pairIndex]].GetTargetFactoryId();

			if (objectiveIds.size() == 0 || find(objectiveIds.begin(), objectiveIds.end(), objectiveId) == objectiveIds.end())
			{
				objectiveIds.push_back(objectiveId);
			}
		}

		//################################
		// counting the total amount of assigned Cyborgs for each objective
		vector<int> assignedCyborgs(objectiveIds.size());
		vector<int> assignedFactories(objectiveIds.size());

		fill(assignedCyborgs.begin(), assignedCyborgs.end(), 0);
		fill(assignedFactories.begin(), assignedFactories.end(), 0);

		for (unsigned int pairIndex = 0; pairIndex < currentPairGroup.size(); pairIndex++)
		{
			Assignment assignment = assignments[currentPairGroup[pairIndex]];

			int objectiveId = assignment.GetTargetFactoryId();

			auto pos = find(objectiveIds.begin(), objectiveIds.end(), objectiveId);
			int objectiveIdIndex = distance(objectiveIds.begin(), pos);

			if (pos != objectiveIds.end())
			{
				assignedCyborgs[objectiveIdIndex] += assignment.GetAmountCyborgs();
				assignedFactories[objectiveIdIndex] += 1;
			}
		}

		//################################
		// computing the score
		double pairGroupScore = 0;

		for (unsigned int pairIndex = 0; pairIndex < currentPairGroup.size(); pairIndex++)
		{
			Assignment assignment = assignments[currentPairGroup[pairIndex]];

			int objectiveId = assignment.GetTargetFactoryId();

			auto posObj = find_if(objectives.begin(), objectives.end(), [&objectiveId](const Objective& obj)
			{	return obj.GetTargetFactoryId() == objectiveId;});
			int objectiveIndex = distance(objectives.begin(), posObj);
			Objective objective = objectives[objectiveIndex];

			auto pos = find(objectiveIds.begin(), objectiveIds.end(), objectiveId);
			int objectiveIdIndex = distance(objectiveIds.begin(), pos);

			if (pos != objectiveIds.end())
			{
				double objectiveFulfilment = assignedCyborgs[objectiveIdIndex] / (double) objective.GetNeededCyborgs();

				double maxFullfilment = 4;
				if (model.GetFactory(objectiveId).GetOwner() == 0)
				{
					maxFullfilment = 1.4;
				}

				objectiveFulfilment = max(maxFullfilment - (objectiveFulfilment - maxFullfilment), 0.1);
				double score = assignment.GetScore() * objectiveFulfilment;

				pairGroupScore += score;
			}

		}

		pairGroupPrioQueue.Push(pairGroupIndex, pairGroupScore);
	}

	cerr << "- finished evaluating assignment groups (" << Elapsed(beginEvaluatingGroups) << ") :" << endl;

	//***********************************************************************
	//create commands
	if (pairGroupPrioQueue.Empty())
	{
		return commands;
	}

	int bestPairGroupIndex = pairGroupPrioQueue.Pop();
	vector<int> bestPairGroup = assignmentGroups[bestPairGroupIndex];
	vector<Assignment> bestPairs;

	for (unsigned int bestPairGroupIndex = 0; bestPairGroupIndex < bestPairGroup.size(); bestPairGroupIndex++)
	{
		bestPairs.emplace_back(assignments[bestPairGroup[bestPairGroupIndex]]);
	}

	for (unsigned int bestPairIndex = 0; bestPairIndex < bestPairs.size(); bestPairIndex++)
	{
		Assignment currentAssignment = bestPairs[bestPairIndex];

		int originFactoryId = currentAssignment.GetOriginFactoryId();
		int targetFactoryId = currentAssignment.GetTargetFactoryId();
		int amountCyborgs = currentAssignment.GetAmountCyborgs();
		vector<int> path = model.GetPath(originFactoryId, targetFactoryId);

		if (commands != "")
		{
			commands.append(";");
		}

		commands.append("MOVE " + to_string(originFactoryId) + " " + to_string(path[0]) + " " + to_string(amountCyborgs));

		availableCyborgs[originFactoryId] -= amountCyborgs;
	}

	return commands;
}

//==================================================================================
//
//==================================================================================
int Bot::CreateAssignments(const Model& model, const vector<Objective>& objectives, const vector<int>& availableCyborgs,
		vector<Assignment>& assignments)
{
	timespec beginAssigning;
	clock_gettime(CLOCK_REALTIME, &beginAssigning);

	vector<Factory> ownedFactories = model.GetOwnedFactories(1);
	vector<Factory> availableFactories;
	int combinationSize = 0;
	PriorityQueueMax<int, double> queue;
	vector<Assignment> createdAssignments;

	//identifying the factories with available cyborgs
	for (unsigned int ownedFactoriesIndex = 0; ownedFactoriesIndex < ownedFactories.size(); ownedFactoriesIndex++)
	{
		if (availableCyborgs[ownedFactories[ownedFactoriesIndex].GetId()] > 0)
		{
			availableFactories.emplace_back(ownedFactories[ownedFactoriesIndex]);
		}
	}

	//looping through the objectives to create assignments
	for (unsigned int objectiveIndex = 0; objectiveIndex < objectives.size(); objectiveIndex++)
	{
		Objective currentObjective = objectives[objectiveIndex];

		if (currentObjective.GetScore() == 0)
		{
			continue;
		}

		int targetFactoryId = currentObjective.GetTargetFactoryId();
		vector<Bomb> bombs = model.GetBombs(targetFactoryId);
		int timeToImpact = 0;

		//determine the time to impact of the bomb that is farthest away
		for (unsigned int bombIndex = 0; bombIndex < bombs.size(); bombIndex++)
		{
			if (bombs[bombIndex].GetTimeRemaining() > timeToImpact)
			{
				timeToImpact = bombs[bombIndex].GetTimeRemaining();
			}
		}

		for (unsigned int availableFactoriesIndex = 0; availableFactoriesIndex < availableFactories.size(); availableFactoriesIndex++)
		{
			int originFactoryId = availableFactories[availableFactoriesIndex].GetId();
			int distance = model.GetDistance(originFactoryId, targetFactoryId);
			double baseScore = currentObjective.GetScore() / pow(distance, 2);

			if (originFactoryId != targetFactoryId && distance > timeToImpact)
			{
				int cyborgs = availableCyborgs[originFactoryId];

				//double participation = cyborgs / (double) currentObjective.GetNeededCyborgs();
				//double score = min((currentObjective.GetScore() * participation) / distanceSqr, 1.0);
				double score = baseScore * cyborgs;

				createdAssignments.emplace_back(targetFactoryId, originFactoryId, cyborgs, score);
				queue.Push(createdAssignments.size() - 1, score);

				//
				combinationSize++;

				if (cyborgs > currentObjective.GetNeededCyborgs())
				{
					int cyborgs1 = currentObjective.GetNeededCyborgs();

					//double participation1 = cyborgs1 / (double) currentObjective.GetNeededCyborgs();
					//double score1 = min((currentObjective.GetScore() * participation1) / distanceSqr, 1.0);
					double score1 = baseScore * cyborgs1;

					createdAssignments.emplace_back(targetFactoryId, originFactoryId, cyborgs1, score1);
					queue.Push(createdAssignments.size() - 1, score1);

					//
					//combinationSize++;
				}

				if (cyborgs > 4)
				{
					int cyborgs2 = 2;

					//double participation2 = cyborgs2 / (double) currentObjective.GetNeededCyborgs();
					//double score2 = min((currentObjective.GetScore() * participation2) / baseScore, 1.0);
					double score2 = baseScore * cyborgs2;

					createdAssignments.emplace_back(targetFactoryId, originFactoryId, cyborgs2, score2);
					queue.Push(createdAssignments.size() - 1, score2);

					//
					//combinationSize++;
				}
			}
		}
	}

	if (queue.Empty())
	{
		return 0;
	}

	int numSelectedAssignments = 0;
	do
	{
		assignments.emplace_back(createdAssignments[queue.Pop()]);
		numSelectedAssignments++;
	} while (!queue.Empty() && ComputeTotalNumberCombinations((numSelectedAssignments + 1), combinationSize) < MAX_NUMBER_COMBINATIONS);

	combinationSize = min(combinationSize, (int) assignments.size());

	cerr << "- finished creating assignments (" << Elapsed(beginAssigning) << ") :" << endl;
	cerr << "  * numAssignments=" << assignments.size();
	cerr << "; combinationSize=" << combinationSize << endl;

	return combinationSize;
}

void Bot::CreateAssignmentGroups(const Model& model, const vector<int>& availableCyborgs, const vector<Assignment>& assignments,
		int maxCombinationSize, vector<vector<int>>& assignmentGroups)
{
	if (assignments.size() == 0)
	{
		return;
	}

	timespec beginCreatingGroups;
	clock_gettime(CLOCK_REALTIME, &beginCreatingGroups);

	int n = assignments.size();

	for (int combinationSize = 1; combinationSize <= maxCombinationSize; combinationSize++)
	{
		vector<bool> combination(n);
		fill(combination.begin(), combination.begin() + combinationSize, true);

		vector<int> assignedCyborgs = vector<int>(model.GetNumFactories());

		do
		{
			vector<int> currentGroup;

			for (int i = 0; i < n; ++i)
			{
				if (combination[i])
				{
					currentGroup.push_back(i);
				}
			}

			//check for validity
			fill(assignedCyborgs.begin(), assignedCyborgs.end(), 0);
			bool valid = true;

			for (unsigned int assignmentIndex = 0; assignmentIndex < currentGroup.size(); assignmentIndex++)
			{
				Assignment currentAssignment = assignments[currentGroup[assignmentIndex]];
				int originFactoryId = currentAssignment.GetOriginFactoryId();

				if (assignedCyborgs[originFactoryId] + currentAssignment.GetAmountCyborgs() > availableCyborgs[originFactoryId])
				{
					valid = false;
					break;
				}
				else
				{
					assignedCyborgs[originFactoryId] += currentAssignment.GetAmountCyborgs();
				}
			}

			if (valid)
			{
				assignmentGroups.emplace_back(currentGroup);
			}
		} while (prev_permutation(combination.begin(), combination.end()));
	}

	cerr << "- finished creating assignment groups (" << Elapsed(beginCreatingGroups) << ") :" << endl;
	cerr << "  * numPairGroups=" << assignmentGroups.size() << endl;
}

string Bot::MoveRemainingUnits(const Model& model, vector<int>& availableCyborgs)
{
	string commands = "";

	vector<Factory> enemyFactories = model.GetOwnedFactories(-1);

	if (enemyFactories.empty())
	{
		return commands;
	}

//	vector<Factory> ownedFactories = model.GetOwnedFactories(1);
//	vector<int> averageDistancesVector(model.GetNumFactories());
//
//	for (unsigned int ownedFactoriesIndex = 0; ownedFactoriesIndex < ownedFactories.size(); ownedFactoriesIndex++)
//	{
//		int ownedFactoryId = ownedFactories[ownedFactoriesIndex].GetId();
//		averageDistancesVector[ownedFactoryId] = model.GetAverageDistance(ownedFactoryId, -1, 0);
//	}

	for (int factoryId = 0; factoryId < model.GetNumFactories(); factoryId++)
	{
		int cyborgs = availableCyborgs[factoryId];

		if (cyborgs > 0)
		{
			vector<int> sortedEnemyFactories = model.GetFactoriesSortedByDistance(factoryId, -1);
			vector<int> path = model.GetPath(factoryId, sortedEnemyFactories[0]);

			if (commands != "")
			{
				commands.append(";");
			}

			commands.append("MOVE " + to_string(factoryId) + " " + to_string(path[0]) + " " + to_string(cyborgs));

//			vector<int> sortedOwnedFactories = model.GetFactoriesSortedByDistance(factoryId, 1);
//
//			for (unsigned int sortedOwnedFactoriesIndex = 0; sortedOwnedFactoriesIndex < sortedOwnedFactories.size();
//					sortedOwnedFactoriesIndex++)
//			{
//				int sortedOwnedFactoryId = sortedOwnedFactories[sortedOwnedFactoriesIndex];
//
//				if (averageDistancesVector[sortedOwnedFactoryId] < averageDistancesVector[factoryId])
//				{
//					if (commands != "")
//					{
//						commands += ";";
//					}
//
//					commands.append("MOVE " + to_string(factoryId) + " " + to_string(sortedOwnedFactoryId) + " " + to_string(cyborgs));
//
//					break;
//				}
//			}
		}
	}

	return commands;
}

}
using namespace decisions;

//################################################################################
//################################################################################
// MAIN
//################################################################################
//################################################################################
int main()
{
	Model _model;
	Bot _bot;

	//************************************************************
	// initialization

	int factoryCount; // the number of factories
	cin >> factoryCount;
	cin.ignore();

	_model = Model(factoryCount);

	int linkCount; // the number of links between factories
	cin >> linkCount;
	cin.ignore();
	for (int i = 0; i < linkCount; i++)
	{
		int factory1;
		int factory2;
		int distance;
		cin >> factory1 >> factory2 >> distance;
		cin.ignore();

		_model.SetDistance(factory1, factory2, distance);
	}

	_model.ComputePaths();

	//************************************************************
	// game loop

	while (1)
	{
		timespec beginUpdate;
		clock_gettime(CLOCK_REALTIME, &beginUpdate);

		int entityCount; // the number of entities (e.g. factories and troops)
		cin >> entityCount;
		cin.ignore();
		for (int i = 0; i < entityCount; i++)
		{
			string entity;
			getline(cin, entity);

			vector<string> arguments;
			string argument = "";

			for (unsigned int i = 0; i < entity.length(); i++)
			{
				if (entity[i] == ' ')
				{
					arguments.emplace_back(argument);
					argument = "";
				}
				else
				{
					argument += entity[i];
				}
			}
			arguments.emplace_back(argument);

			int entityId = atoi(arguments[0].c_str());
			string entityType = arguments[1];
			int arg1 = atoi(arguments[2].c_str());
			int arg2 = atoi(arguments[3].c_str());
			int arg3 = atoi(arguments[4].c_str());
			int arg4 = atoi(arguments[5].c_str());
			int arg5 = atoi(arguments[6].c_str());

			if (entityType == "FACTORY")
			{
				_model.SetFactory(entityId, arg1, arg2, arg3, arg4);
			}
			else if (entityType == "TROOP")
			{
				_model.SetTroop(entityId, arg1, arg2, arg3, arg4, arg5);
			}
			else if (entityType == "BOMB")
			{
				_model.SetBomb(entityId, arg1, arg2, arg3, arg4);
			}
		}

		cerr << "Finished updating Model (" << Elapsed(beginUpdate) << ")" << endl;
		cerr << "* entityCount=" << entityCount << endl;

		//************************************************************
		//

		timespec beginBot;
		clock_gettime(CLOCK_REALTIME, &beginBot);

		string commands = _bot.ComputeMoves(_model);

		cerr << "Finished creating Commands (total: " << Elapsed(beginBot) << ")" << endl;

		if (commands == "")
		{
			cout << "WAIT" << endl;
		}
		else
		{
			cout << commands << endl;
		}

		//************************************************************
		// cleaning up
		_model.CleanUp();
	}

	//************************************************************
	// final cleaning up

}
