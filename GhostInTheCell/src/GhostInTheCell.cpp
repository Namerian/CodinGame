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

const int GARRISON_MODIFIER = 2;

const int ATTACK_PRODUCTION_SCORE = 40;
const int ATTACK_ENEMIES_SCORE = 30;
const int ATTACK_NEUTRAL_SCORE = 30;
const int ATTACK_DISTANCE_SCORE = 0;

const int DEFENSE_PRODUCTION_SCORE = 80;
const int DEFENSE_URGENCY_SCORE = 20;
const int DEFENSE_DISTANCE_SCORE = 0;

const int MAX_NUMBER_COMBINATIONS = 10000;

//################################################################################
//################################################################################
// NAMESPACE UTILITIES
//################################################################################
//################################################################################
namespace utilities
{

inline unsigned long long ComputeNumberCombinations(unsigned long long n, unsigned long long r)
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

inline double elapsed(timespec& begin)
{
	timespec end;
	clock_gettime(CLOCK_REALTIME, &end);
	return (end.tv_nsec - begin.tv_nsec) / 1000000.;
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

}
using namespace utilities;

//################################################################################
//################################################################################
// NAMESPACE MODEL
//################################################################################
//################################################################################
namespace model
{

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
	{
		_id = id;
	}

	Factory(const Factory& factory)
	{
		_id = factory.GetId();
		_owner = factory.GetOwner();
		_numCyborgs = factory.GetNumCyborgs();
		_production = factory.GetProduction();
		_incapacitatedTimer = factory.GetIncapacitatedTimer();
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
	{
		_id = id;
		_owner = owner;
		_originId = originId;
		_destinationId = destinationId;
		_numCyborgs = numCyborgs;
		_timeRemaining = timeRemaining;
	}

	Troop(const Troop& troop)
	{
		_id = troop.GetId();
		_owner = troop.GetOwner();
		_originId = troop.GetOriginId();
		_destinationId = troop.GetDestinationId();
		_numCyborgs = troop.GetNumCyborgs();
		_timeRemaining = troop.GetTimeRemaining();
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

	Bomb(int id, int owner, int originFactoryId, int targetFactoryId, int timeRemaining) :
			_id(id), _owner(owner), _originFactoryId(originFactoryId), _targetFactoryId(targetFactoryId), _timeRemaining(
					timeRemaining)

	{
	}

//	Bomb(const Bomb& bomb)
//	{
//		_id = bomb.GetId();
//		_owner = bomb.GetOwner();
//		_originFactoryId = bomb.GetOriginFactoryId();
//		_targetFactoryId = bomb.GetTargetFactoryId();
//		_timeRemaining = bomb.GetTimeRemaining();
//	}

//	inline void Update(int id, int owner, int originFactoryId, int targetFactoryId, int timeRemaining)
//	{
//		_id = id;
//		_owner = owner;
//		_originFactoryId = originFactoryId;
//		_targetFactoryId = targetFactoryId;
//		_timeRemaining = timeRemaining;
//	}

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
	vector<vector<int>> _distanceMatrix;

	int _numFactories = 0;
	vector<Factory> _factories;

	unsigned int _numTroops = 0;
	vector<Troop> _troops;

	unsigned int _numBombs = 0;
	vector<Bomb> _bombs;

public:
	Model()
	{
	}

	Model(int numFactories)
	{
		_numFactories = numFactories;

		_distanceMatrix.resize(_numFactories);
		for (int i = 0; i < _numFactories; i++)
		{
			_distanceMatrix[i].resize(_numFactories);
			_factories.push_back(i);
		}
	}

	//=======================================================
	// SETTER

	inline void SetDistance(int factory1, int factory2, int distance)
	{
		_distanceMatrix[factory1][factory2] = distance;
		_distanceMatrix[factory2][factory1] = distance;
	}

	inline void SetFactory(int factoryId, int owner, int numCyborgs, int production, int incapacitatedTimer)
	{
		_factories[factoryId].Update(owner, numCyborgs, production, incapacitatedTimer);
	}

	inline void SetTroop(int id, int owner, int originId, int destinationId, int numCyborgs, int timeRemaining)
	{
		/*if (_numTroops < _troops.size())
		 {
		 _troops.at(_numTroops).Update(id, owner, originId, destinationId, numCyborgs, timeRemaining);
		 }
		 else
		 {*/
		_troops.emplace_back(id, owner, originId, destinationId, numCyborgs, timeRemaining);
		/*}

		 _numTroops++;*/
	}

	inline void SetBomb(int id, int owner, int originFactoryId, int targetFactoryId, int timeRemaining)
	{
		/*if (_numBombs < _bombs.size())
		 {
		 _bombs.at(_numBombs).Update(id, owner, originFactoryId, targetFactoryId, timeRemaining);
		 }
		 else
		 {*/
		_bombs.emplace_back(id, owner, originFactoryId, targetFactoryId, timeRemaining);
		/*}

		 _numBombs++;*/
	}

	//=======================================================
	// GETTER

	inline int GetNumFactories() const
	{
		return _numFactories;
	}

	inline int GetDistance(int factory1, int factory2) const
	{
		return _distanceMatrix[factory1][factory2];
	}

	double GetAverageDistance(int targetFactoryId, int owner, int minProduction = 0) const
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

	vector<int> GetFactoriesSortedByDistance(const int originFactoryId, const int owner) const
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

	inline Factory GetFactory(int id) const
	{
		return _factories[id];
	}

	vector<Factory> GetOwnedFactories(int owner) const
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

	vector<Troop> GetTroops(int targetFactoryId) const
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

	vector<Bomb> GetBombs(int targetFactoryId) const
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

	//=======================================================
	// OTHER

//	void FinishUpdate()
//	{
//		if (_numTroops < _troops.size())
//		{
//			_troops.erase(_troops.begin() + (_numTroops - 1), _troops.end());
//		}
//
//		_numTroops = 0;
//
//		if (_numBombs < _bombs.size())
//		{
//			_bombs.erase(_bombs.begin() + (_numBombs - 1), _bombs.end());
//		}
//
//		_numBombs = 0;
//	}

	inline void CleanUp()
	{
		_troops.clear();
		_bombs.clear();
	}
};

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
	int _id = -1;
	int _targetFactory = -1;
	int _amountCyborgsNeeded = 0;
	int _priority = -1000;

public:
	Objective()
	{
	}

	Objective(int id, int targetFactory, int amountCyborgsNeeded, int priority)
	{
		_id = id;
		_targetFactory = targetFactory;
		_amountCyborgsNeeded = amountCyborgsNeeded;
		_priority = priority;
	}

	Objective(const Objective& objective)
	{
		_id = objective.GetId();
		_targetFactory = objective.GetTargetFactory();
		_amountCyborgsNeeded = objective.GetAmountCyborgsNeeded();
		_priority = objective.GetPriority();
	}

	inline int GetId() const
	{
		return _id;
	}

	inline int GetTargetFactory() const
	{
		return _targetFactory;
	}

	inline int GetAmountCyborgsNeeded() const
	{
		return _amountCyborgsNeeded;
	}

	inline int GetPriority() const
	{
		return _priority;
	}
};

class Assignment
{
private:
	int _objectiveId = -1;
	int _originFactoryId = -1;
	int _amountCyborgs = 0;

public:
	Assignment()
	{
	}

	Assignment(int objectiveId, int originFactoryId, int amountCyborgs)
	{
		_objectiveId = objectiveId;
		_originFactoryId = originFactoryId;
		_amountCyborgs = amountCyborgs;
	}

	Assignment(const Assignment& assignment)
	{
		_objectiveId = assignment.GetObjectiveId();
		_originFactoryId = assignment.GetOriginFactoryId();
		_amountCyborgs = assignment.GetAmountCyborgs();
	}

	inline int GetObjectiveId() const
	{
		return _objectiveId;
	}

	inline int GetOriginFactoryId() const
	{
		return _originFactoryId;
	}

	inline int GetAmountCyborgs() const
	{
		return _amountCyborgs;
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
	string
	ComputeMoves(const Model& model);

//private methods
private:
	/**
	 * Loops through all Factories and creates Objectives.
	 *
	 */
	static void EvaluateFactories(const Model& model, vector<Objective>& objectives, vector<int>& availableCyborgs);

	/**
	 * Evaluates an owned Factory:
	 * - creates reinforcement Objectives if necessary
	 * - computes the amount of available Cyborgs for attacks
	 *
	 */
	static bool CreateReinforceObjective(const Model& model, Objective& objective, const Factory& targetFactory,
			const vector<Troop>& incomingTroops/*, vector<int>& availableCyborgs*/);

	/**
	 *
	 */
	static void ComputeAvailableCyborgs(const Model& model, const vector<Objective>& objectives,
			vector<int>& availableCyborgs);

	/**
	 * Evaluates an enemy Factory and creates an attack objective.
	 *
	 */
	static bool CreateAttackObjective(const Model& model, Objective& objective, const Factory& targetFactory,
			const vector<Troop>& incomingTroops);

	/**
	 * Assigns available Cyborgs to the Objectives.
	 *
	 */
	static string AssignTroops(const Model& model, const vector<Objective>& objectives,
			const vector<int>& availableCyborgs);

	/**
	 *
	 */
	static string LaunchBombs(const Model& model, int numAvailBombs, vector<int>& bombTargets);

	/**
	 *
	 */
	static double ComputeDistanceScore(const Model& model, const Factory& targetFactory, int scoreMultiplier);

	/**
	 *
	 */
	static double ComputeProductionScore(const Factory& targetFactory, int scoreMultiplier);

	/**
	 *
	 */
	inline static double ComputeEnemiesScore(const Factory& targetFactory, int scoreMultiplier)
	{
		return scoreMultiplier * ((1 - min(targetFactory.GetNumCyborgs() / 30.0, 1.0)));
	}

	/**
	 *
	 */
	static double
	ComputeNeutralScore(const Factory& targetFactory, int scoreMultiplier);

	/**
	 *
	 */
	inline static double ComputeUrgencyScore(int timeToEmergency, int scoreMultiplier)
	{
		return scoreMultiplier * (1 - (timeToEmergency / (double) MAX_DISTANCE));
	}

};

string Bot::ComputeMoves(const Model& model)
{
	_currentTurn++;

	vector<Objective> objectives;
	vector<int> availableCyborgs = vector<int>(model.GetNumFactories());
	fill(availableCyborgs.begin(), availableCyborgs.end(), 0);

	EvaluateFactories(model, objectives, availableCyborgs);

	string incCommand = "";
	if (_currentTurn > 1)
	{
		vector<Factory> factories = model.GetOwnedFactories(1);
		for (unsigned int i = 0; i < factories.size(); i++)
		{
			Factory factory = factories[i];

			if (factory.GetId() != _startFactoryId && factory.GetProduction() < 3
					&& availableCyborgs[factory.GetId()] > 15)
			{
				incCommand = "INC " + to_string(factory.GetId());
				availableCyborgs[factory.GetId()] -= 10;
				break;
			}
		}
	}
	else
	{
		vector<Factory> factories = model.GetOwnedFactories(1);
		_startFactoryId = factories[0].GetId();
	}

	string bombCommand = LaunchBombs(model, _numAvailBombs, _bombTargets);
	string moveCommands = AssignTroops(model, objectives, availableCyborgs);

	if (incCommand != "" && (bombCommand != "" || moveCommands != ""))
	{
		incCommand += ";";
	}

	if (bombCommand != "" && moveCommands != "")
	{
		bombCommand += ";";
	}

	return incCommand + bombCommand + moveCommands;
}

void Bot::EvaluateFactories(const Model& model, vector<Objective>& objectives, vector<int>& availableCyborgs)
{
	if (objectives.size() > 0)
	{
		objectives.clear();
	}

	timespec beginEvaluating;
	clock_gettime(CLOCK_REALTIME, &beginEvaluating);
	//cerr << "start evaluating factories" << endl;

	PriorityQueueMax<Objective, double> queue;
	int numCreatedObjectives = 0;

	for (int currentFactoryId = 0; currentFactoryId < model.GetNumFactories(); currentFactoryId++)
	{
		Factory currentFactory = model.GetFactory(currentFactoryId);
		vector<Troop> incomingTroops = model.GetTroops(currentFactoryId);
		Objective objective;

		if (currentFactory.GetOwner() == 1) //owned factory
		{
			if (CreateReinforceObjective(model, objective, currentFactory, incomingTroops/*, availableCyborgs*/))
			{
				queue.Push(objective, objective.GetPriority());
				numCreatedObjectives++;
			}
		}
		else //factory controlled by neutral or enemy
		{
			if (CreateAttackObjective(model, objective, currentFactory, incomingTroops))
			{
				queue.Push(objective, objective.GetPriority());
				numCreatedObjectives++;
			}
		}
	}

	ComputeAvailableCyborgs(model, objectives, availableCyborgs);

	int availableFactories = 0;

	for (unsigned int factoryId = 0; factoryId < availableCyborgs.size(); factoryId++)
	{
		if (availableCyborgs[factoryId] > 0)
		{
			availableFactories++;
		}
	}

	int numSelectedObjectives = 0;

	if (availableFactories > 0 && !queue.Empty())
	{
		do
		{
			objectives.emplace_back(queue.Pop());
			numSelectedObjectives++;
		} while (!queue.Empty()
				&& ComputeNumberCombinations((numSelectedObjectives + 1) * availableFactories, availableFactories)
						< MAX_NUMBER_COMBINATIONS);
	}

	double time1 = elapsed(beginEvaluating);
	cerr << "- finished evaluating factories (" << time1 << ") :" << endl;
	cerr << "  * availableFact=" << availableFactories;
	cerr << "; createdObj=" << numCreatedObjectives;
	cerr << "; selectedObj=" << objectives.size() << endl;
}

bool Bot::CreateReinforceObjective(const Model& model, Objective& objective, const Factory& targetFactory,
		const vector<Troop>& incomingTroops/*, vector<int>& availableCyborgs*/)
{
	vector<int> incomingDeltas = vector<int>(MAX_DISTANCE);

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

	int firstEmergency = MAX_DISTANCE;
	int neededReinforcements = 0;

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

		if (amountCyborgs <= GARRISON_MODIFIER * targetFactory.GetProduction())
		{
			if (turn < firstEmergency)
			{
				firstEmergency = turn;
			}

			if (turn < firstEmergency + 5)
			{
				neededReinforcements += GARRISON_MODIFIER * targetFactory.GetProduction() - amountCyborgs + 1;
			}
		}
	}

	if (neededReinforcements > 0)
	{
		double priority = 0;

		//points for the distance, closer = more score
		//priority += ComputeDistanceScore(model, targetFactory, DEFENSE_DISTANCE_SCORE);

		//points if the target has production
		priority += ComputeProductionScore(targetFactory, DEFENSE_PRODUCTION_SCORE);

		//points for the urgency
		priority += ComputeUrgencyScore(firstEmergency, DEFENSE_URGENCY_SCORE);

		if (priority >= 0)
		{
			int objectiveId = targetFactory.GetId() * 100;

			objective = Objective(objectiveId, targetFactory.GetId(), neededReinforcements, priority);

			return true;
		}
	}

	return false;
}

void Bot::ComputeAvailableCyborgs(const Model& model, const vector<Objective>& objectives,
		vector<int>& availableCyborgs)
{
	vector<Factory> ownedFactories = model.GetOwnedFactories(1);

	for (unsigned int factoryIndex = 0; factoryIndex < ownedFactories.size(); factoryIndex++)
	{
		Factory currentFactory = ownedFactories[factoryIndex];
		vector<Troop> incomingTroops = model.GetTroops(currentFactory.GetId());
		int currentFactoryId = currentFactory.GetId();

		auto pos = find_if(objectives.begin(), objectives.end(), [&currentFactoryId](const Objective& obj)
		{	return obj.GetTargetFactory()==currentFactoryId;});

		if (pos != objectives.end())
		{
			continue;
		}

		int incomingEnemies = 0;
		int firstArrival = MAX_DISTANCE;
		for (unsigned int i = 0; i < incomingTroops.size(); i++)
		{
			if (incomingTroops[i].GetOwner() == -1)
			{
				incomingEnemies += incomingTroops[i].GetNumCyborgs();

				if (incomingTroops[i].GetTimeRemaining() < firstArrival)
				{
					firstArrival = incomingTroops[i].GetTimeRemaining();
				}
			}
		}

		vector<Factory> enemyFactories = model.GetOwnedFactories(-1);
		for (unsigned int i = 0; i < enemyFactories.size(); i++)
		{
			if (model.GetDistance(currentFactory.GetId(), enemyFactories[i].GetId()) < 4)
			{
				incomingEnemies += enemyFactories[i].GetNumCyborgs();
			}
		}

		int production = 0;
		for (int turn = 0; turn <= firstArrival; turn++)
		{
			if (currentFactory.GetIncapacitatedTimer() - turn < 1)
			{
				production += currentFactory.GetProduction();
			}
		}

		int num = currentFactory.GetNumCyborgs() + min(0, production - incomingEnemies)
				- GARRISON_MODIFIER * currentFactory.GetProduction();

		if (currentFactory.GetIncapacitatedTimer() < 1 && num > 0)
		{
			availableCyborgs[currentFactory.GetId()] = num;
			//cerr << "(" << currentFactory.GetId() << ", " << num << ") ";
		}
	}
	//cerr << endl;
}

bool Bot::CreateAttackObjective(const Model& model, Objective& objective, const Factory& targetFactory,
		const vector<Troop>& incomingTroops)
{
	int amountCyborgs = -targetFactory.GetNumCyborgs();

	for (unsigned int troopIndex = 0; troopIndex < incomingTroops.size(); troopIndex++)
	{
		Troop troop = incomingTroops[troopIndex];

		if (troop.GetOwner() == 1 && troop.GetTimeRemaining() < (MAX_DISTANCE / 2))
		{
			amountCyborgs += troop.GetNumCyborgs();
		}
		else if (troop.GetOwner() == -1)
		{
			amountCyborgs -= troop.GetNumCyborgs();
		}
	}

	vector<int> sortedFactories = model.GetFactoriesSortedByDistance(targetFactory.GetId(), 1);

	if (sortedFactories.size() > 0 && targetFactory.GetOwner() == -1)
	{
		amountCyborgs -= targetFactory.GetProduction() * 2
				* model.GetDistance(targetFactory.GetId(), sortedFactories[sortedFactories.size() - 1]);
	}

	if (amountCyborgs <= GARRISON_MODIFIER * targetFactory.GetProduction())
	{
		//points for the distance, closer = more score
		//double distance = ComputeDistanceScore(model, targetFactory, 1);

		//points for target production
		double prodScore = ComputeProductionScore(targetFactory, ATTACK_PRODUCTION_SCORE);

		//points for the amount of enemies present
		double enemyScore = ComputeEnemiesScore(targetFactory, ATTACK_ENEMIES_SCORE);

		//points if the target is neutral
		double neutralScore = ComputeNeutralScore(targetFactory, ATTACK_NEUTRAL_SCORE);

		double bombScore = 0;
		if (!model.GetBombs(targetFactory.GetId()).empty())
		{
			bombScore = ATTACK_NEUTRAL_SCORE;
		}

		double priority = prodScore + enemyScore + neutralScore + bombScore;

		if (priority >= 0)
		{
			int cyborgsNeeded = GARRISON_MODIFIER * targetFactory.GetProduction() - amountCyborgs + 1;
			int objectiveId = targetFactory.GetId() * 100;

			objective = Objective(objectiveId, targetFactory.GetId(), cyborgsNeeded, priority);

			return true;
		}
	}

	return false;
}

string Bot::AssignTroops(const Model& model, const vector<Objective>& objectives, const vector<int>& availableCyborgs)
{
	timespec beginAssigning;
	clock_gettime(CLOCK_REALTIME, &beginAssigning);
	//cerr << "start assigning troops" << endl;

	string commands = "";
	vector<Factory> ownedFactories = model.GetOwnedFactories(1);
	int numAvailableFactories = 0;

	//***********************************************************************
	//create assignments
	vector<Assignment> assignments;

	//cerr << "- owned factories: ";
	for (unsigned int i = 0; i < ownedFactories.size(); i++)
	{
		int factoryId = ownedFactories[i].GetId();
		//cerr << factoryId << " ";

		if (availableCyborgs[factoryId] > 0)
		{
			numAvailableFactories++;

			for (unsigned int j = 0; j < objectives.size(); j++)
			{
				Objective objective = objectives[j];

				if (objective.GetTargetFactory() != factoryId)
				{
					int amountCyborgs = min(availableCyborgs[factoryId], objective.GetAmountCyborgsNeeded());

					assignments.emplace_back(objective.GetId(), factoryId, amountCyborgs);
				}
			}
		}
	}
	//cerr << endl;

	cerr << "- finished creating assignments (" << elapsed(beginAssigning) << ") :" << endl;
	cerr << "  * availableFact=" << numAvailableFactories;
	cerr << "; numAssignments=" << assignments.size() << endl;

	//***********************************************************************
	//create pair groups

	timespec beginCreatingPairs;
	clock_gettime(CLOCK_REALTIME, &beginCreatingPairs);

	vector<vector<int>> assignmentGroups;

	if (assignments.size() == 0)
	{
		return commands;
	}

	int n = assignments.size();
	int r = numAvailableFactories;

	vector<bool> combination(n);
	fill(combination.begin(), combination.begin() + r, true);

	vector<int> assignedCyborgs = vector<int>(model.GetNumFactories());
	;

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

		//################################
		//check for validity
		fill(assignedCyborgs.begin(), assignedCyborgs.end(), 0);

		for (unsigned int assignmentIndex = 0; assignmentIndex < currentGroup.size(); assignmentIndex++)
		{
			Assignment currentAssignment = assignments[currentGroup[assignmentIndex]];

			assignedCyborgs[currentAssignment.GetOriginFactoryId()] += currentAssignment.GetAmountCyborgs();
		}

		bool valid = true;

		for (int factoryId = 0; factoryId < model.GetNumFactories(); factoryId++)
		{
			if (assignedCyborgs[factoryId] > availableCyborgs[factoryId])
			{
				valid = false;
				break;
			}
		}

		if (valid)
		{
			assignmentGroups.emplace_back(currentGroup);
		}
	} while (prev_permutation(combination.begin(), combination.end()));

	cerr << "- finished creating assignment groups (" << elapsed(beginCreatingPairs) << ") :" << endl;
	cerr << "  * numPairGroups=" << assignmentGroups.size() << endl;

	//***********************************************************************
	//evaluate pairGroups

	timespec beginEvaluatingGroups;
	clock_gettime(CLOCK_REALTIME, &beginEvaluatingGroups);

	PriorityQueueMax<int, double> pairGroupPrioQueue;
	int numEvaluatedGroups = 0;

	for (unsigned int pairGroupIndex = 0; pairGroupIndex < assignmentGroups.size(); pairGroupIndex++)
	{
		vector<int> currentPairGroup = assignmentGroups[pairGroupIndex];

		//################################
		// identifying the objectives
		vector<int> objectiveIds;

		for (unsigned int pairIndex = 0; pairIndex < currentPairGroup.size(); pairIndex++)
		{
			int objectiveId = assignments[currentPairGroup[pairIndex]].GetObjectiveId();

			if (objectiveIds.size() == 0
					|| find(objectiveIds.begin(), objectiveIds.end(), objectiveId) == objectiveIds.end())
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

			int objectiveId = assignment.GetObjectiveId();

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

			int objectiveId = assignment.GetObjectiveId();
			int originFactoryId = assignment.GetOriginFactoryId();

			auto posObj = find_if(objectives.begin(), objectives.end(), [&objectiveId](const Objective& obj)
			{	return obj.GetId() == objectiveId;});
			int objectiveIndex = distance(objectives.begin(), posObj);
			Objective objective = objectives[objectiveIndex];

			auto pos = find(objectiveIds.begin(), objectiveIds.end(), objectiveId);
			int objectiveIdIndex = distance(objectiveIds.begin(), pos);

			if (pos != objectiveIds.end())
			{
				double objectiveFulfilment = assignedCyborgs[objectiveIdIndex]
						/ (double) objective.GetAmountCyborgsNeeded();

				double troopParticipation = assignment.GetAmountCyborgs() / (double) objective.GetAmountCyborgsNeeded();

				double revisedObjectiveScore = objective.GetPriority() * objectiveFulfilment * troopParticipation;
				int distance = model.GetDistance(originFactoryId, objective.GetTargetFactory());

				double score = revisedObjectiveScore / (double) pow(distance, 3);
				double scoreMultiplier = 1;

//				if (objectiveFulfilment < 0.75)
//				{
//					scoreMultiplier = 0;
//				}
//				else if (objectiveFulfilment < 1.25)
//				{
//					scoreMultiplier = 0.5;
//				}
//				else if (objectiveFulfilment < 1.75)
//				{
//					scoreMultiplier = 0.75;
//				}
//				else
//				{
//					scoreMultiplier = 1.5;
//				}

				pairGroupScore += score * scoreMultiplier;
			}
		}

		pairGroupPrioQueue.Push(pairGroupIndex, pairGroupScore);
		//numEvaluatedGroups++;
	}

	cerr << "- finished evaluating assignment groups (" << elapsed(beginEvaluatingGroups) << ") :" << endl;
	//cerr << "- numEvaluatedGroups=" << numEvaluatedGroups << endl;

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
		int targetFactoryId = currentAssignment.GetObjectiveId() / 100;
		int amountCyborgs = currentAssignment.GetAmountCyborgs();

		if (commands != "")
		{
			commands.append(";");
		}

		commands.append(
				"MOVE " + to_string(originFactoryId) + " " + to_string(targetFactoryId) + " "
						+ to_string(amountCyborgs));
	}

	//cerr << "finished creating commands" << endl;
	return commands;
}

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

		if (target.GetProduction() >= 2
				&& find(bombTargets.begin(), bombTargets.end(), target.GetId()) == bombTargets.end())
		{
			vector<int> sourceFactories = model.GetFactoriesSortedByDistance(target.GetId(), 1);

			if (!sourceFactories.empty())
			{
				int distance = model.GetDistance(sourceFactories[0], target.GetId());

				if (distance < targetDistance)
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

double Bot::ComputeDistanceScore(const Model& model, const Factory& targetFactory, int scoreMultiplier)
{
//	vector<int> sortedFactories = model->SortFactoriesByDistance(targetFactory.GetId(), 1);
//	int distance = MAX_DISTANCE;
//
//	if (sortedFactories.size() > 0)
//	{
//		for (unsigned int i = 0; i < sortedFactories.size(); i++)
//		{
//			if (model->GetFactory(sortedFactories.at(i)).GetProduction() > 0)
//			{
//				distance = model->GetDistance(targetFactory.GetId(), sortedFactories.at(0));
//				break;
//			}
//		}
//	}
//
//	return scoreMultiplier * (1.0 - (distance / (double) MAX_DISTANCE));

	double averageDistance = model.GetAverageDistance(targetFactory.GetId(), 1, 0);

	if (averageDistance == 0)
	{
		averageDistance = MAX_DISTANCE;
	}

	return scoreMultiplier * (1.0 - (averageDistance / (double) MAX_DISTANCE));
}

double Bot::ComputeProductionScore(const Factory& targetFactory, int scoreMultiplier)
{
	double score = 0;

	switch (targetFactory.GetProduction())
	{
	case 0:
		score = 0;
		break;
	case 1:
		score = scoreMultiplier * 0.4;
		break;
	case 2:
		score = scoreMultiplier * 0.7;
		break;
	case 3:
		score = scoreMultiplier;
		break;
	}

	return score;
}

double Bot::ComputeNeutralScore(const Factory& targetFactory, int scoreMultiplier)
{
	double score = 0;

	if (targetFactory.GetOwner() == 0)
	{
		switch (targetFactory.GetProduction())
		{
		case 0:
			score = 0;
			break;
		case 1:
			score = scoreMultiplier * 0.4;
			break;
		case 2:
			score = scoreMultiplier * 0.7;
			break;
		case 3:
			score = scoreMultiplier;
			break;
		}
	}

	return score;
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
			//cerr<<entity<<endl;

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

			//cin >> entityId >> entityType >> arg1 >> arg2 >> arg3 >> arg4 >> arg5;
			//cin.ignore();

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

		//_model.FinishUpdate();

		cerr << "Finished updating Model (" << elapsed(beginUpdate) << ")" << endl;
		cerr << "* entityCount=" << entityCount << endl;

		//************************************************************
		//

		timespec beginBot;
		clock_gettime(CLOCK_REALTIME, &beginBot);

		string commands = _bot.ComputeMoves(_model);

		cerr << "Finished creating Commands (total: " << elapsed(beginBot) << ")" << endl;

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
