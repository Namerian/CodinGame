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

using namespace std;

const int MAX_DISTANCE = 20;

const int GARRISON_MODIFIER = 2;

const int ATTACK_PRODUCTION_SCORE = 20;
const int ATTACK_ENEMIES_SCORE = 10;
const int ATTACK_NEUTRAL_SCORE = 30;
const int ATTACK_DISTANCE_SCORE = 40;

const int DEFENSE_PRODUCTION_SCORE = 40;
const int DEFENSE_URGENCY_SCORE = 40;
const int DEFENSE_DISTANCE_SCORE = 20;

const int MAX_NUMBER_COMBINATIONS = 11000;

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
	}

	inline void Update(int owner, int numCyborgs, int production)
	{
		_owner = owner;
		_numCyborgs = numCyborgs;
		_production = production;
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
};

class Troop
{
private:
	int _owner = 0;
	int _originId = -1;
	int _destinationId = -1;
	int _numCyborgs = 0;
	int _timeRemaining = 0;

public:
	Troop()
	{
	}

	Troop(int owner, int originId, int destinationId, int numCyborgs, int timeRemaining)
	{
		_owner = owner;
		_originId = originId;
		_destinationId = destinationId;
		_numCyborgs = numCyborgs;
		_timeRemaining = timeRemaining;
	}

	Troop(const Troop& troop)
	{
		_owner = troop.GetOwner();
		_originId = troop.GetOriginId();
		_destinationId = troop.GetDestinationId();
		_numCyborgs = troop.GetNumCyborgs();
		_timeRemaining = troop.GetTimeRemaining();
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

class Model
{
private:
	int _numFactories;
	vector<vector<int>> _distanceMatrix;
	vector<Factory> _factories;
	vector<Troop> _troops;

public:
	Model(int numFactories)
	{
		_numFactories = numFactories;

		_distanceMatrix.resize(_numFactories);
		for (int i = 0; i < _numFactories; i++)
		{
			_distanceMatrix.at(i).resize(_numFactories);
			_factories.push_back(i);
		}
	}

	inline int GetNumFactories() const
	{
		return _numFactories;
	}

	inline void SetDistance(int factory1, int factory2, int distance)
	{
		_distanceMatrix.at(factory1).at(factory2) = distance;
		_distanceMatrix.at(factory2).at(factory1) = distance;
	}

	inline int GetDistance(int factory1, int factory2) const
			{
		return _distanceMatrix.at(factory1).at(factory2);
	}

	double GetAverageDistance(int targetFactoryId, int owner, int minProduction = 0) const
			{
		int totalDistance = 0;
		int amountDistances = 0;
		vector<Factory> ownedFactories = GetOwnedFactories(owner);

		for (unsigned int i = 0; i < ownedFactories.size(); i++)
		{
			Factory factory = ownedFactories.at(i);

			if (factory.GetId() != targetFactoryId && factory.GetProduction() >= minProduction)
			{
				totalDistance += GetDistance(targetFactoryId, factory.GetId());
				amountDistances++;
			}
		}

		return totalDistance / (double) max(1, amountDistances);
	}

	vector<int> SortFactoriesByDistance(const int originFactoryId, const int owner) const
			{
		vector<int> result;
		vector<Factory> ownedFactories = GetOwnedFactories(owner);

		for (unsigned int i = 0; i < ownedFactories.size(); i++)
		{
			if (ownedFactories.at(i).GetId() != originFactoryId)
			{
				result.push_back(ownedFactories.at(i).GetId());
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
				if (GetDistance(originFactoryId, result.at(i)) > GetDistance(originFactoryId, result.at(i + 1)))
				{
					switched = true;
					int tmp = result.at(i);
					result.at(i) = result.at(i + 1);
					result.at(i + 1) = tmp;
				}
			}
		}

		return result;
	}

	inline void CleanUp()
	{
		_troops.clear();
	}

	inline void UpdateFactory(int factoryId, int owner, int numCyborgs, int production)
	{
		_factories.at(factoryId).Update(owner, numCyborgs, production);
	}

	inline Factory GetFactory(int id) const
			{
		return _factories.at(id);
	}

	vector<Factory> GetOwnedFactories(int owner) const
			{
		vector<Factory> result;

		for (int i = 0; i < _numFactories; i++)
		{
			if (_factories.at(i).GetOwner() == owner)
			{
				result.emplace_back(_factories.at(i));
			}
		}

		return result;
	}

	inline void AddTroop(int owner, int originId, int destinationId, int numCyborgs, int timeRemaining)
	{
		_troops.emplace_back(owner, originId, destinationId, numCyborgs, timeRemaining);
	}

	vector<Troop> GetTroops(int targetFactoryId) const
			{
		vector<Troop> result;

		for (unsigned int troopIndex = 0; troopIndex < _troops.size(); troopIndex++)
		{
			Troop currentTroop = _troops.at(troopIndex);

			if (currentTroop.GetDestinationId() == targetFactoryId)
			{
				result.emplace_back(currentTroop);
			}
		}

		return result;
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
	string ComputeMoves(const Model* model);

//private methods
private:
	/**
	 * Loops through all Factories and creates Objectives.
	 *
	 */
	static void EvaluateFactories(const Model* model, vector<Objective>& objectives, vector<int>& availableCyborgs);

	/**
	 * Evaluates an owned Factory:
	 * - creates reinforcement Objectives if necessary
	 * - computes the amount of available Cyborgs for attacks
	 *
	 */
	static bool CreateReinforceObjective(const Model* model, Objective& objective, const Factory& targetFactory,
			const vector<Troop>& incomingTroops, vector<int>& availableCyborgs);

	/**
	 * Evaluates an enemy Factory and creates an attack objective.
	 *
	 */
	static bool CreateAttackObjective(const Model* model, Objective& objective, const Factory& targetFactory,
			const vector<Troop>& incomingTroops);

	/**
	 * Assigns available Cyborgs to the Objectives.
	 *
	 */
	static string AssignTroops(const Model* model, const vector<Objective>& objectives, const vector<int>& availableCyborgs);

	/**
	 *
	 */
	static string LaunchBombs(const Model* model, int& numAvailBombs, vector<int>& bombTargets);

	/**
	 *
	 */
	static double ComputeDistanceScore(const Model* model, const Factory& targetFactory, const int& scoreMultiplier);

	/**
	 *
	 */
	static double ComputeProductionScore(const Factory& targetFactory, const int& scoreMultiplier);

	/**
	 *
	 */
	inline static double ComputeEnemiesScore(const Factory& targetFactory, const int& scoreMultiplier)
	{
		return (2 * scoreMultiplier) * ((1 - min(targetFactory.GetNumCyborgs() / 30.0, 1.0)) - 0.5);
	}

	/**
	 *
	 */
	static double ComputeNeutralScore(const Factory& targetFactory, const int& scoreMultiplier);

	/**
	 *
	 */
	inline static double ComputeUrgencyScore(const int& timeToEmergency, const int& scoreMultiplier)
	{
		return scoreMultiplier * (1 - (timeToEmergency / (double) MAX_DISTANCE));
	}

};

string Bot::ComputeMoves(const Model* model)
{
	_currentTurn++;

	vector<Objective> objectives;
	vector<int> availableCyborgs = vector<int>(model->GetNumFactories());
	fill(availableCyborgs.begin(), availableCyborgs.end(), 0);

	EvaluateFactories(model, objectives, availableCyborgs);

	string incCommand = "";
	if (_currentTurn > 1)
	{
		vector<Factory> factories = model->GetOwnedFactories(1);
		for (unsigned int i = 0; i < factories.size(); i++)
		{
			Factory factory = factories.at(i);

			if (factory.GetId() != _startFactoryId && factory.GetProduction() < 3 && availableCyborgs.at(factory.GetId()) > 10)
			{
				incCommand = "INC " + to_string(factory.GetId());
				availableCyborgs.at(factory.GetId()) -= 10;
				break;
			}
		}
	}
	else
	{
		vector<Factory> factories = model->GetOwnedFactories(1);
		_startFactoryId = factories.at(0).GetId();
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

void Bot::EvaluateFactories(const Model* model, vector<Objective>& objectives, vector<int>& availableCyborgs)
{
	if (objectives.size() > 0)
	{
		objectives.clear();
	}

	timespec beginEvaluating;
	clock_gettime(CLOCK_REALTIME, &beginEvaluating);
	cerr << "start evaluating factories" << endl;

	PriorityQueueMax<Objective, double> queue;
	int numCreatedObjectives = 0;

	for (int currentFactoryId = 0; currentFactoryId < model->GetNumFactories(); currentFactoryId++)
	{
		Factory currentFactory = model->GetFactory(currentFactoryId);
		vector<Troop> incomingTroops = model->GetTroops(currentFactoryId);
		Objective objective;

		if (currentFactory.GetOwner() == 1) //owned factory
		{
			if (CreateReinforceObjective(model, objective, currentFactory, incomingTroops, availableCyborgs))
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

	int availableFactories = 0;

	for (unsigned int factoryId = 0; factoryId < availableCyborgs.size(); factoryId++)
	{
		if (availableCyborgs.at(factoryId) > 0)
		{
			availableFactories++;
		}
	}

	cerr << "- availableFactories=" << availableFactories << endl;
	cerr << "- numCreatedObjectives=" << numCreatedObjectives << endl;

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
	cerr << "finished evaluating factories (" << time1 << ") :" << endl;
	cerr << "- numSelectedObjectives=" << objectives.size() << endl;
}

bool Bot::CreateReinforceObjective(const Model* model, Objective& objective, const Factory& targetFactory,
		const vector<Troop>& incomingTroops, vector<int>& availableCyborgs)
{
	vector<int> incomingDeltas = vector<int>(MAX_DISTANCE);

	for (unsigned int troopIndex = 0; troopIndex < incomingTroops.size(); troopIndex++)
	{
		Troop troop = incomingTroops.at(troopIndex);

		if (troop.GetOwner() == 1)
		{
			incomingDeltas.at(troop.GetTimeRemaining()) += troop.GetNumCyborgs();
		}
		else if (troop.GetOwner() == -1)
		{
			incomingDeltas.at(troop.GetTimeRemaining()) -= troop.GetNumCyborgs();
		}
	}

	int amountCyborgs = targetFactory.GetNumCyborgs();

	int firstEmergency = MAX_DISTANCE;
	int neededReinforcements = 0;

	for (int turn = 0; turn < MAX_DISTANCE; turn++)
	{
		if (amountCyborgs >= 0)
		{
			amountCyborgs += targetFactory.GetProduction();
		}
		else
		{
			amountCyborgs -= targetFactory.GetProduction();
		}

		amountCyborgs += incomingDeltas.at(turn);

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
		priority += ComputeDistanceScore(model, targetFactory, DEFENSE_DISTANCE_SCORE);

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

	int incomingEnemies = 0;
	int firstIncomingTurn = MAX_DISTANCE;

	for (unsigned int i = 0; i < incomingTroops.size(); i++)
	{
		if (incomingTroops.at(i).GetOwner() == -1)
		{
			incomingEnemies += incomingTroops.at(i).GetNumCyborgs();

			if (incomingTroops.at(i).GetTimeRemaining() < firstIncomingTurn)
			{
				firstIncomingTurn = incomingTroops.at(i).GetTimeRemaining();
			}
		}
	}

	vector<Factory> enemyFactories = model->GetOwnedFactories(-1);

	for (unsigned int i = 0; i < enemyFactories.size(); i++)
	{
		if (model->GetDistance(targetFactory.GetId(), enemyFactories.at(i).GetId()) < 4)
		{
			incomingEnemies += enemyFactories.at(i).GetNumCyborgs();
		}
	}

	int num = targetFactory.GetNumCyborgs() + (targetFactory.GetProduction() * firstIncomingTurn) - incomingEnemies
			- (targetFactory.GetProduction() * GARRISON_MODIFIER);

	availableCyborgs.at(targetFactory.GetId()) = min(targetFactory.GetNumCyborgs(), max(0, num));

	return false;
}

bool Bot::CreateAttackObjective(const Model* model, Objective& objective, const Factory& targetFactory,
		const vector<Troop>& incomingTroops)
{
	int amountCyborgs = -targetFactory.GetNumCyborgs();

	for (unsigned int troopIndex = 0; troopIndex < incomingTroops.size(); troopIndex++)
	{
		Troop troop = incomingTroops.at(troopIndex);

		if (troop.GetOwner() == 1 && troop.GetTimeRemaining() < (MAX_DISTANCE / 2))
		{
			amountCyborgs += troop.GetNumCyborgs();
		}
		else if (troop.GetOwner() == -1)
		{
			amountCyborgs -= troop.GetNumCyborgs();
		}
	}

	vector<int> sortedFactories = model->SortFactoriesByDistance(targetFactory.GetId(), 1);

	if (sortedFactories.size() > 0 && targetFactory.GetOwner() == -1)
	{
		amountCyborgs -= targetFactory.GetProduction() * 2
				* model->GetDistance(targetFactory.GetId(), sortedFactories.at(sortedFactories.size() - 1));
	}

	if (amountCyborgs <= GARRISON_MODIFIER * targetFactory.GetProduction())
	{
		double priority = 0;

		//points for the distance, closer = more score
		priority += ComputeDistanceScore(model, targetFactory, ATTACK_DISTANCE_SCORE);

		//points for target production
		priority += ComputeProductionScore(targetFactory, ATTACK_PRODUCTION_SCORE);

		//points for the amount of enemies present
		priority += ComputeEnemiesScore(targetFactory, ATTACK_ENEMIES_SCORE);

		//points if the target is neutral
		priority += ComputeNeutralScore(targetFactory, ATTACK_NEUTRAL_SCORE);

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

string Bot::AssignTroops(const Model* model, const vector<Objective>& objectives, const vector<int>& availableCyborgs)
{
	timespec beginAssigning;
	clock_gettime(CLOCK_REALTIME, &beginAssigning);
	cerr << "start assigning troops" << endl;

	string commands = "";
	vector<Factory> ownedFactories = model->GetOwnedFactories(1);
	int numAvailableFactories = 0;

	//***********************************************************************
	//create assignments
	vector<Assignment> assignments;

	//cerr << "- owned factories: ";
	for (unsigned int i = 0; i < ownedFactories.size(); i++)
	{
		int factoryId = ownedFactories.at(i).GetId();
		//cerr << factoryId << " ";

		if (availableCyborgs.at(factoryId) > 0)
		{
			numAvailableFactories++;

			for (unsigned int j = 0; j < objectives.size(); j++)
			{
				Objective objective = objectives.at(j);
				int amountCyborgs = min(availableCyborgs.at(factoryId), objective.GetAmountCyborgsNeeded());

				assignments.emplace_back(objective.GetId(), factoryId, amountCyborgs);
			}
		}
	}
	//cerr << endl;

	double time1 = elapsed(beginAssigning);
	cerr << "finished creating assignments (" << time1 << ") :" << endl;
	cerr << "- numAvailableFactories=" << numAvailableFactories << endl;
	cerr << "- numPairs=" << assignments.size() << endl;

	//***********************************************************************
	//create pair groups
	vector<vector<int>> assignmentGroups;

	if (assignments.size() == 0)
	{
		return commands;
	}

	int n = assignments.size();
	int r = numAvailableFactories;

	vector<bool> combination(n);
	fill(combination.begin(), combination.begin() + r, true);

	vector<int> assignedCyborgs = vector<int>(model->GetNumFactories());
	;

	do
	{
		vector<int> currentGroup;

		for (int i = 0; i < n; ++i)
		{
			if (combination.at(i))
			{
				currentGroup.push_back(i);
			}
		}

		//################################
		//check for validity
		fill(assignedCyborgs.begin(), assignedCyborgs.end(), 0);

		for (unsigned int assignmentIndex = 0; assignmentIndex < currentGroup.size(); assignmentIndex++)
		{
			Assignment currentAssignment = assignments.at(currentGroup.at(assignmentIndex));

			assignedCyborgs.at(currentAssignment.GetOriginFactoryId()) += currentAssignment.GetAmountCyborgs();
		}

		bool valid = true;

		for (int factoryId = 0; factoryId < model->GetNumFactories(); factoryId++)
		{
			if (assignedCyborgs.at(factoryId) > availableCyborgs.at(factoryId))
			{
				valid = false;
				break;
			}
		}

		if (valid)
		{
			assignmentGroups.emplace_back(currentGroup);
		}
	}
	while (prev_permutation(combination.begin(), combination.end()));

	double time2 = elapsed(beginAssigning) - time1;
	cerr << "finished creating assignment groups (" << time2 << ") :" << endl;
	cerr << "- numPairGroups=" << assignmentGroups.size() << endl;

	//***********************************************************************
	//evaluate pairGroups
	PriorityQueueMax<int, double> pairGroupPrioQueue;
	int numEvaluatedGroups = 0;

	for (unsigned int pairGroupIndex = 0; pairGroupIndex < assignmentGroups.size(); pairGroupIndex++)
	{
		vector<int> currentPairGroup = assignmentGroups.at(pairGroupIndex);

		//################################
		// identifying the objectives
		vector<int> objectiveIds;

		for (unsigned int pairIndex = 0; pairIndex < currentPairGroup.size(); pairIndex++)
		{
			int objectiveId = assignments.at(currentPairGroup.at(pairIndex)).GetObjectiveId();

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
			Assignment assignment = assignments.at(currentPairGroup.at(pairIndex));

			int objectiveId = assignment.GetObjectiveId();

			auto pos = find(objectiveIds.begin(), objectiveIds.end(), objectiveId);
			int objectiveIdIndex = distance(objectiveIds.begin(), pos);

			if (pos != objectiveIds.end())
			{
				assignedCyborgs.at(objectiveIdIndex) += assignment.GetAmountCyborgs();
				assignedFactories.at(objectiveIdIndex) += 1;
			}
		}

		//################################
		// computing the score
		double pairGroupScore = 0;

		for (unsigned int pairIndex = 0; pairIndex < currentPairGroup.size(); pairIndex++)
		{
			Assignment assignment = assignments.at(currentPairGroup.at(pairIndex));

			int objectiveId = assignment.GetObjectiveId();
			int originFactoryId = assignment.GetOriginFactoryId();

			auto posObj = find_if(objectives.begin(), objectives.end(), [&objectiveId](const Objective& obj)
			{	return obj.GetId() == objectiveId;});
			int objectiveIndex = distance(objectives.begin(), posObj);
			Objective objective = objectives.at(objectiveIndex);

			auto pos = find(objectiveIds.begin(), objectiveIds.end(), objectiveId);
			int objectiveIdIndex = distance(objectiveIds.begin(), pos);

			if (pos != objectiveIds.end())
			{
				int amountCyborgs = assignedCyborgs.at(objectiveIdIndex);
				double objectiveFulfilment = amountCyborgs / (double) objective.GetAmountCyborgsNeeded();
				double troopParticipation = assignment.GetAmountCyborgs() / (min(1.0, objectiveFulfilment));

				int distance = model->GetDistance(originFactoryId, objective.GetTargetFactory());

				double score = (objective.GetPriority() * troopParticipation) / (double) distance;
				double scoreMultiplier = 0;

				if (objectiveFulfilment < 0.75)
				{
					scoreMultiplier += 0.25;
				}
				else if (objectiveFulfilment < 1.25)
				{
					scoreMultiplier += 0.5;
				}
				else if (objectiveFulfilment < 1.75)
				{
					scoreMultiplier += 1;
				}
				else
				{
					scoreMultiplier += 2;
				}

				pairGroupScore += score * scoreMultiplier;
			}
		}

		pairGroupPrioQueue.Push(pairGroupIndex, pairGroupScore);
		numEvaluatedGroups++;
	}

	cerr << "finished evaluating assignment groups:" << endl;
	//cerr << "- numEvaluatedGroups=" << numEvaluatedGroups << endl;

	//***********************************************************************
	//create commands
	if (pairGroupPrioQueue.Empty())
	{
		return commands;
	}

	int bestPairGroupIndex = pairGroupPrioQueue.Pop();
	vector<int> bestPairGroup = assignmentGroups.at(bestPairGroupIndex);
	vector<Assignment> bestPairs;

	for (unsigned int bestPairGroupIndex = 0; bestPairGroupIndex < bestPairGroup.size(); bestPairGroupIndex++)
	{
		bestPairs.emplace_back(assignments.at(bestPairGroup.at(bestPairGroupIndex)));
	}

	for (unsigned int bestPairIndex = 0; bestPairIndex < bestPairs.size(); bestPairIndex++)
	{
		Assignment currentAssignment = bestPairs.at(bestPairIndex);

		int originFactoryId = currentAssignment.GetOriginFactoryId();
		int targetFactoryId = currentAssignment.GetObjectiveId() / 100;
		int amountCyborgs = currentAssignment.GetAmountCyborgs();

		if (commands != "")
		{
			commands.append(";");
		}

		commands.append("MOVE " + to_string(originFactoryId) + " " + to_string(targetFactoryId) + " " + to_string(amountCyborgs));
	}

	cerr << "finished creating commands" << endl;
	return commands;
}

string Bot::LaunchBombs(const Model* model, int& numAvailBombs, vector<int>& bombTargets)
{
	string command = "";

	if (numAvailBombs == 0)
	{
		return command;
	}

	vector<Factory> targets = model->GetOwnedFactories(-1);

	if (targets.empty())
	{
		return command;
	}

	int selectedTarget;
	int selectedOrigin;
	int targetDistance = MAX_DISTANCE + 1;

	for (unsigned int i = 0; i < targets.size(); i++)
	{
		Factory target = targets.at(i);

		if (target.GetProduction() >= 2 && find(bombTargets.begin(), bombTargets.end(), target.GetId()) == bombTargets.end())
		{
			vector<int> sourceFactories = model->SortFactoriesByDistance(target.GetId(), 1);

			if (!sourceFactories.empty())
			{
				int distance = model->GetDistance(sourceFactories.at(0), target.GetId());

				if (distance < targetDistance)
				{
					selectedTarget = target.GetId();
					selectedOrigin = sourceFactories.at(0);
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

double Bot::ComputeDistanceScore(const Model* model, const Factory& targetFactory, const int& scoreMultiplier)
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

	double averageDistance = model->GetAverageDistance(targetFactory.GetId(), 1, 0);

	if (averageDistance == 0)
	{
		averageDistance = MAX_DISTANCE;
	}

	return scoreMultiplier * 2 * (1.0 - (averageDistance / (double) MAX_DISTANCE) - 0.5);
}

double Bot::ComputeProductionScore(const Factory& targetFactory, const int& scoreMultiplier)
{
	double score = 0;

	switch (targetFactory.GetProduction())
	{
	case 0:
		score = -scoreMultiplier;
		break;
	case 1:
		score = scoreMultiplier * 0.25;
		break;
	case 2:
		score = scoreMultiplier * 0.5;
		break;
	case 3:
		score = scoreMultiplier;
		break;
	}

	return score;
}

double Bot::ComputeNeutralScore(const Factory& targetFactory, const int& scoreMultiplier)
{
	double score = 0;

	if (targetFactory.GetOwner() == 0)
	{
		switch (targetFactory.GetProduction())
		{
		case 0:
			score = -scoreMultiplier;
			break;
		case 1:
			score = scoreMultiplier * 0.25;
			break;
		case 2:
			score = scoreMultiplier * 0.5;
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
	Model* _model;
	Bot* _bot = new Bot();

//************************************************************
// initialization

	int factoryCount; // the number of factories
	cin >> factoryCount;
	cin.ignore();

	_model = new Model(factoryCount);

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

		_model->SetDistance(factory1, factory2, distance);
	}

//************************************************************
// game loop

	while (1)
	{
		int entityCount; // the number of entities (e.g. factories and troops)
		cin >> entityCount;
		cin.ignore();
		for (int i = 0; i < entityCount; i++)
		{
			int entityId;
			string entityType;
			int arg1;
			int arg2;
			int arg3;
			int arg4;
			int arg5;
			cin >> entityId >> entityType >> arg1 >> arg2 >> arg3 >> arg4 >> arg5;
			cin.ignore();

			if (entityType == "FACTORY")
			{
				_model->UpdateFactory(entityId, arg1, arg2, arg3);
			}
			else if (entityType == "TROOP")
			{
				_model->AddTroop(arg1, arg2, arg3, arg4, arg5);
			}
		}

		//************************************************************
		//

		string commands = _bot->ComputeMoves(_model);

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
		_model->CleanUp();
	}

//************************************************************
// final cleaning up

	delete (_model);
	delete (_bot);
}
