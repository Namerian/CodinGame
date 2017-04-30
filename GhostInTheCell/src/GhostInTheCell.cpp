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

//################################################################################
//################################################################################
// NAMESPACE UTILITIES
//################################################################################
//################################################################################
namespace utilities
{

unsigned long long ComputeNumberCombinations(unsigned long long n, unsigned long long r)
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

double elapsed(timespec& begin)
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

private:
	priority_queue<element, vector<element>, PriorityCompare> elements;
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
	int _id;

	int _owner = 0;
	int _numCyborgs = 0;
	int _production = 0;

public:
	Factory()
	{
		_id = -1;
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

	void Update(int owner, int numCyborgs, int production)
	{
		_owner = owner;
		_numCyborgs = numCyborgs;
		_production = production;
	}

	int GetId() const
	{
		return _id;
	}

	int GetOwner() const
	{
		return _owner;
	}

	int GetNumCyborgs() const
	{
		return _numCyborgs;
	}

	int GetProduction() const
	{
		return _production;
	}
};

class Troop
{
private:
	int _owner;
	int _originId;
	int _destinationId;
	int _numCyborgs;
	int _timeRemaining;

public:
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

	int GetOwner() const
	{
		return _owner;
	}

	int GetOriginId() const
	{
		return _originId;
	}

	int GetDestinationId() const
	{
		return _destinationId;
	}

	int GetNumCyborgs() const
	{
		return _numCyborgs;
	}

	int GetTimeRemaining() const
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

	int GetNumFactories() const
	{
		return _numFactories;
	}

	void SetDistance(int factory1, int factory2, int distance)
	{
		_distanceMatrix.at(factory1).at(factory2) = distance;
		_distanceMatrix.at(factory2).at(factory1) = distance;
	}

	int GetDistance(int factory1, int factory2) const
			{
		return _distanceMatrix.at(factory1).at(factory2);
	}

	float GetAverageDistance(int targetFactoryId, int owner, int minProduction = 0) const
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

		return totalDistance / (float) max(1, amountDistances);
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

	void CleanUp()
	{
		_troops.clear();
	}

	void UpdateFactory(int factoryId, int owner, int numCyborgs, int production)
	{
		_factories.at(factoryId).Update(owner, numCyborgs, production);
	}

	Factory GetFactory(int id) const
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

	void AddTroop(int owner, int originId, int destinationId, int numCyborgs, int timeRemaining)
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

	int GetId() const
	{
		return _id;
	}

	int GetTargetFactory() const
	{
		return _targetFactory;
	}

	int GetAmountCyborgsNeeded() const
	{
		return _amountCyborgsNeeded;
	}

	int GetPriority() const
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

	int GetObjectiveId() const
	{
		return _objectiveId;
	}

	int GetOriginFactoryId() const
	{
		return _originFactoryId;
	}

	int GetAmountCyborgs() const
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
	vector<float> _previousAttackPriorities;

//public methods
public:
	string ComputeMoves(const Model* model)
	{
		_currentTurn++;

		if (_previousAttackPriorities.size() == 0)
		{
			_previousAttackPriorities = vector<float>(model->GetNumFactories());
		}

		vector<Objective> objectives;
		vector<int> availableCyborgs = vector<int>(model->GetNumFactories());

		EvaluateFactories(model, objectives, availableCyborgs);

		string bombCommand = LaunchBombs(model);
		string moveCommands = AssignTroops(model, objectives, availableCyborgs);

		if (bombCommand != "" && moveCommands != "")
		{
			bombCommand += ";";
		}

		return bombCommand + moveCommands;
	}

//private methods
private:
	/**
	 * Loops through all Factories and creates Objectives.
	 *
	 */
	void EvaluateFactories(const Model* model, vector<Objective>& objectives, vector<int>& availableCyborgs)
	{
		if (objectives.size() > 0)
		{
			objectives.clear();
		}

		timespec beginEvaluating;
		clock_gettime(CLOCK_REALTIME, &beginEvaluating);
		cerr << "start evaluating factories" << endl;

		PriorityQueueMax<Objective, float> queue;
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

		if (availableFactories > 0)
		{
			do
			{
				objectives.emplace_back(queue.Pop());
				numSelectedObjectives++;
			} while (!queue.Empty()
					&& ComputeNumberCombinations((numSelectedObjectives + 1) * availableFactories, availableFactories) < 10000);
		}

		double time1 = elapsed(beginEvaluating);
		cerr << "finished evaluating factories (" << time1 << ") :" << endl;
		cerr << "- numSelectedObjectives=" << objectives.size() << endl;
	}

	/**
	 * Evaluates an owned Factory:
	 * - creates reinforcement Objectives if necessary
	 * - computes the amount of available Cyborgs for attacks
	 *
	 */
	bool CreateReinforceObjective(const Model* model, Objective& objective, const Factory& targetFactory,
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
			//10 points for base priority
			float priority = 10;

			//30 points for the distance, closer = more score
			float averageDistance = model->GetAverageDistance(targetFactory.GetId(), 1, 1);

			if (averageDistance == 0)
			{
				averageDistance = model->GetAverageDistance(targetFactory.GetId(), 1, 0);
			}

			float distanceScore = 30 * (1.0f - (averageDistance / (float) MAX_DISTANCE));
			priority += distanceScore;

			//up to 20 points if the target has production
			switch (targetFactory.GetProduction())
			{
			case 1:
				priority += 10;
				break;
			case 2:
				priority += 15;
				break;
			case 3:
				priority += 20;
				break;
			}

			//40 points for the urgency
			priority += 40 * (1 - ((float) firstEmergency / (float) MAX_DISTANCE));

			int objectiveId = targetFactory.GetId() * 100;

			objective = Objective(objectiveId, targetFactory.GetId(), neededReinforcements, priority);

			return true;
		}
		else //this code is only called if no objective is created
		{
			availableCyborgs.at(targetFactory.GetId()) = min(
					targetFactory.GetNumCyborgs(),
					max(0, amountCyborgs - (1 + targetFactory.GetProduction() * GARRISON_MODIFIER)));

			return false;
		}
	}

	/**
	 * Evaluates an enemy Factory and creates an attack objective.
	 *
	 */
	bool CreateAttackObjective(const Model* model, Objective& objective, const Factory& targetFactory,
			const vector<Troop>& incomingTroops)
	{
		int amountCyborgs = -targetFactory.GetNumCyborgs();

		for (unsigned int troopIndex = 0; troopIndex < incomingTroops.size(); troopIndex++)
		{
			Troop troop = incomingTroops.at(troopIndex);

			if (troop.GetOwner() == 1)
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
			float priority = 0;

			//30 points for the distance, closer = more score
			float averageDistance = model->GetAverageDistance(targetFactory.GetId(), 1, 1);

			if (averageDistance == 0)
			{
				averageDistance = model->GetAverageDistance(targetFactory.GetId(), 1, 0);
			}

			float distanceScore = 30 * (1.0f - (averageDistance / (float) MAX_DISTANCE));
			//cerr << "attack distance score: targetId=" << currentFactoryId << " score=" << distanceScore << endl;
			priority += distanceScore;

			//up to 20 points for target production
			switch (targetFactory.GetProduction())
			{
			case 0:
				priority -= 10;
				break;
			case 1:
				priority += 10;
				break;
			case 2:
				priority += 15;
				break;
			case 3:
				priority += 20;
				break;
			}

			//+-30 points for the amount of enemies present
			priority += 60 * ((1 - min(targetFactory.GetNumCyborgs() / 30.0f, 1.0f)) - 0.5f);

			//up to 20 points if the target is neutral
			if (targetFactory.GetOwner() == 0)
			{
				float neutralScore = 0;

				switch (targetFactory.GetProduction())
				{
				case 1:
					neutralScore = 10;
					break;
				case 2:
					neutralScore = 15;
					break;
				case 3:
					neutralScore = 20;
					break;
				}

				if (_currentTurn > 0)
				{
					neutralScore *= 1.0f - (_currentTurn / 20.0f);
				}

				priority += neutralScore;
			}

			//priority *= 0.8f;
			//priority += 0.2f * _previousAttackPriorities.at(currentFactoryId);

			int cyborgsNeeded = GARRISON_MODIFIER * targetFactory.GetProduction() - amountCyborgs + 1;
			int objectiveId = targetFactory.GetId() * 100;

			objective = Objective(objectiveId, targetFactory.GetId(), cyborgsNeeded, priority);

			return true;

			//_previousAttackPriorities.at(currentFactoryId) = priority;
		}

		return false;
	}

	/**
	 * Assigns available Cyborgs to the Objectives.
	 *
	 */
//	string AssignTroops(const Model* model, PriorityQueueMax<Objective, float>& objectives, vector<int>& availableCyborgs)
//	{
//		//TODO
//		/*cerr << "available cyborgs: ";
//		 for (unsigned int i = 0; i < availableCyborgs.size(); i++)
//		 {
//		 cerr << to_string(i) << "=" << to_string(availableCyborgs.at(i)) << " ";
//		 }
//		 cerr << endl;*/
//
//		string commands = "";
//		int totalAvailableTroops = 0;
//
//		for (unsigned int i = 0; i < availableCyborgs.size(); i++)
//		{
//			totalAvailableTroops += availableCyborgs.at(i);
//		}
//
//		while (totalAvailableTroops > 0)
//		{
//			if (objectives.Empty())
//			{
//				break;
//			}
//
//			Objective currentObjective = objectives.Pop();
//			vector<int> sortedFactories = model->SortFactoriesByDistance(currentObjective.GetTargetFactory(), 1);
//
//			//TODO
//			/*cerr << "sorted factories: ";
//			 for (unsigned int i = 0; i < sortedFactories.size(); i++)
//			 {
//			 cerr << to_string(sortedFactories.at(i)) << " ";
//			 }
//			 cerr << "origin: " << to_string(currentObjective.GetTargetFactory()) << endl;*/
//
//			int sentCyborgs = 0;
//
//			for (unsigned int i = 0; i < sortedFactories.size(); i++)
//			{
//				int factoryId = sortedFactories.at(i);
//
//				if (availableCyborgs.at(factoryId) > 0)
//				{
//					int amountCyborgs = 0;
//
//					if (_currentTurn == 1 /*|| objectives.Empty()*/)
//					{
//						amountCyborgs = availableCyborgs.at(factoryId);
//					}
//					else if (!objectives.Empty())
//					{
//						amountCyborgs = min(availableCyborgs.at(factoryId), currentObjective.GetAmountCyborgsNeeded() - sentCyborgs);
//					}
//
//					availableCyborgs.at(factoryId) -= amountCyborgs;
//					sentCyborgs += amountCyborgs;
//					totalAvailableTroops -= amountCyborgs;
//
//					if (commands != "")
//					{
//						commands += ';';
//					}
//
//					commands.append(
//							"MOVE " + to_string(factoryId) + " " + to_string(currentObjective.GetTargetFactory()) + " "
//									+ to_string(amountCyborgs));
//
//					if (!objectives.Empty() && (sentCyborgs == currentObjective.GetAmountCyborgsNeeded() || totalAvailableTroops == 0))
//					{
//						break;
//					}
//				}
//			}
//		}
//
//		return commands;
//	}
	/**
	 * Assigns available Cyborgs to the Objectives.
	 *
	 */
	string AssignTroops(const Model* model, const vector<Objective>& objectives, const vector<int>& availableCyborgs)
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

		vector<int> assignedCyborgs;

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
			assignedCyborgs = vector<int>(model->GetNumFactories());

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
		//reject invalid pair groups
//		vector<vector<int>> validAssignmentGroups;
//
//		for (unsigned int assignmentGroupIndex = 0; assignmentGroupIndex < assignmentGroups.size(); assignmentGroupIndex++)
//		{
//			vector<int> currentAssignmentGroup = assignmentGroups.at(assignmentGroupIndex);
//
//			vector<int> assignedCyborgs = vector<int>(model->GetNumFactories());
//
//			for (unsigned int assignmentIndex = 0; assignmentIndex < currentAssignmentGroup.size(); assignmentIndex++)
//			{
//				Assignment currentAssignment = assignments.at(currentAssignmentGroup.at(assignmentIndex));
//
//				assignedCyborgs.at(currentAssignment.GetOriginFactoryId()) += currentAssignment.GetAmountCyborgs();
//			}
//
//			bool valid = true;
//
//			for (int factoryId = 0; factoryId < model->GetNumFactories(); factoryId++)
//			{
//				if (assignedCyborgs.at(factoryId) > availableCyborgs.at(factoryId))
//				{
//					valid = false;
//					break;
//				}
//			}
//
//			if (valid)
//			{
//				validAssignmentGroups.emplace_back(currentAssignmentGroup);
//			}
//		}
//
//		double time3 = elapsed(beginAssigning) - time2;
//		cerr << "finished rejecting invalid assignment groups (" << time3 << ") :" << endl;
//		//cerr << "- valid pair groups=" << validAssignmentGroups.size() << endl;

		//***********************************************************************
		//evaluate pairGroups
		PriorityQueueMax<int, float> pairGroupPrioQueue;
		int numEvaluatedGroups = 0;

		for (unsigned int pairGroupIndex = 0; pairGroupIndex < assignmentGroups.size(); pairGroupIndex++)
		{
			//cerr << "A ";
			vector<int> currentPairGroup = assignmentGroups.at(pairGroupIndex);

			vector<int> objectiveIds;

			for (unsigned int pairIndex = 0; pairIndex < currentPairGroup.size(); pairIndex++)
			{
				int objectiveId = assignments.at(currentPairGroup.at(pairIndex)).GetObjectiveId();

				if (objectiveIds.size() == 0 || find(objectiveIds.begin(), objectiveIds.end(), objectiveId) == objectiveIds.end())
				{
					//cerr << "a ";
					objectiveIds.push_back(objectiveId);
				}
			}

			//cerr << "B ";

			vector<int> assignedCyborgs(objectiveIds.size());

			for (unsigned int pairIndex = 0; pairIndex < currentPairGroup.size(); pairIndex++)
			{
				Assignment assignment = assignments.at(currentPairGroup.at(pairIndex));

				int objectiveId = assignment.GetObjectiveId();

				auto pos = find(objectiveIds.begin(), objectiveIds.end(), objectiveId);
				int objectiveIdIndex = distance(objectiveIds.begin(), pos);

				if (pos != objectiveIds.end())
				{
					//cerr << "b ";
					assignedCyborgs.at(objectiveIdIndex) += assignment.GetAmountCyborgs();
				}
			}

			//cerr << "C ";

			float pairGroupScore = 0;

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
					//cerr << "c ";
					int amountCyborgs = assignedCyborgs.at(objectiveIdIndex);

					int distance = model->GetDistance(originFactoryId, objective.GetTargetFactory());

					float score = (objective.GetPriority() * min(1.0f, amountCyborgs / (float) objective.GetAmountCyborgsNeeded()))
							/ (float) distance;

					/*cerr << "pair score: originId=" << originFactoryId << " targetId=" << objective.GetTargetFactory() << " score=" << score
					 << endl;*/

					pairGroupScore += score;
				}
			}

			//cerr << "D ";

			pairGroupPrioQueue.Push(pairGroupIndex, pairGroupScore);
			numEvaluatedGroups++;

			//cerr << endl;
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

	/**
	 *
	 */
	string LaunchBombs(const Model* model)
	{
		string command = "";

		if (_numAvailBombs == 0)
		{
			return command;
		}

		vector<Factory> targets = model->GetOwnedFactories(-1);

		for (unsigned int i = 0; i < targets.size(); i++)
		{
			Factory target = targets.at(i);

			if (target.GetProduction() >= 2)
			{
				bool alreadyBombed = false;

				for (unsigned int j = 0; j < _bombTargets.size(); j++)
				{
					if (_bombTargets.at(j) == target.GetId())
					{
						alreadyBombed = true;
					}
				}

				vector<int> sourceFactories = model->SortFactoriesByDistance(target.GetId(), 1);

				if (!alreadyBombed)
				{
					_bombTargets.push_back(target.GetId());
					_numAvailBombs--;

					command = "BOMB " + to_string(sourceFactories.at(0)) + " " + to_string(target.GetId());

					break;
				}
			}
		}

		return command;
	}
}
;

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

		//TODO
		/*vector<Factory> fac = _model->GetOwnedFactories(1);
		 cerr << "owned factories: ";
		 for (unsigned int i = 0; i < fac.size(); i++)
		 {
		 cerr << to_string(fac.at(i).GetId()) + " ";
		 }
		 cerr << endl;*/

		//************************************************************
		//
		// Write an action using cout. DON'T FORGET THE "<< endl"
		// To debug: cerr << "Debug messages..." << endl;
		// Any valid action, such as "WAIT" or "MOVE source destination cyborgs"
		//cout << "WAIT" << endl;
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
