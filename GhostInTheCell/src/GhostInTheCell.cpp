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

using namespace std;

const int MAX_DISTANCE = 20;

//################################################################################
//################################################################################
// NAMESPACE UTILITIES
//################################################################################
//################################################################################
namespace utilities
{

template<typename T, typename priority_t>
class PriorityQueue
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

	float GetAverageDistance(int targetFactoryId, int owner) const
			{
		int totalDistance = 0;
		int amountDistances = 0;

		for (int factoryId = 0; factoryId < _numFactories; factoryId++)
		{
			Factory factory = _factories.at(factoryId);

			if (factory.GetId() != targetFactoryId && factory.GetOwner() == owner)
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

		for (int i = 0; i < _numFactories; i++)
		{
			if (i != originFactoryId && _factories.at(i).GetOwner() == owner)
			{
				result.push_back(i);
			}
		}

		if (result.size() == 1)
		{
			return result;
		}

		bool switched = true;
		while (switched)
		{
			switched = false;

			for (unsigned int i = 0; i < result.size() - 2; i++)
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
	int _targetFactory;
	int _amountCyborgsNeeded;
	int _priority;

public:
	Objective(int targetFactory, int amountCyborgsNeeded, int priority)
	{
		_targetFactory = targetFactory;
		_amountCyborgsNeeded = amountCyborgsNeeded;
		_priority = priority;
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

class Bot
{
//variables
private:

//public methods
public:
	string ComputeMoves(const Model* model)
	{
		PriorityQueue<Objective, float> objectives;
		vector<int> availableCyborgs = vector<int>(model->GetNumFactories());

		EvaluateFactories(model, objectives, availableCyborgs);

		return AssignTroops(model, objectives, availableCyborgs);
	}

//private methods
private:
	void EvaluateFactories(const Model* model, PriorityQueue<Objective, float>& objectives, vector<int>& availableCyborgs)
	{
		for (int factoryId = 0; factoryId < model->GetNumFactories(); factoryId++)
		{
			Factory currentFactory = model->GetFactory(factoryId);
			vector<Troop> incomingTroops = model->GetTroops(factoryId);

			if (currentFactory.GetOwner() == 1) //owned factory
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

				int amountCyborgs = currentFactory.GetNumCyborgs();
				bool createdCommand = false;

				for (int turn = 0; turn < MAX_DISTANCE; turn++)
				{
					if (amountCyborgs >= 0)
					{
						amountCyborgs += currentFactory.GetProduction();
					}
					else
					{
						amountCyborgs += currentFactory.GetProduction();
					}

					amountCyborgs += incomingDeltas.at(turn);

					if (amountCyborgs < 0)
					{
						float priority = 10;

						priority += 30 * (1.0f - ((float) max(1.0f, model->GetAverageDistance(factoryId, 1)) / MAX_DISTANCE));

						if (currentFactory.GetProduction() > 0)
						{
							priority += 30;
						}

						priority += 30 * ((float) max(1, turn) / MAX_DISTANCE);

						int reinforcementsNeeded = abs(amountCyborgs) + 1;

						objectives.Push(Objective(factoryId, reinforcementsNeeded, priority), priority);
						createdCommand = true;

						break;
					}
				}

				//this code is only called if no objective is created
				if (!createdCommand)
				{
					availableCyborgs.at(factoryId) = max(0, amountCyborgs - (1 + currentFactory.GetProduction() * 3));
				}
			}
			else //factory controlled by neutral or enemy
			{
				int amountCyborgs = -currentFactory.GetNumCyborgs();

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

				if (amountCyborgs <= 2 * currentFactory.GetProduction())
				{
					float priority = 30 * (1.0f - ((float) max(1.0f, model->GetAverageDistance(factoryId, 1)) / MAX_DISTANCE));
					//cerr << "tt " << ((float) max(1.0f, model->GetAverageDistance(factoryId, 1)) / MAX_DISTANCE) << endl;
					//cerr << "attack distance score = " << priority << endl;

					if (currentFactory.GetProduction() > 0)
					{
						priority += 20;
					}

					priority += 20 * (min(currentFactory.GetNumCyborgs() / 50.0f, 1.0f) - 0.5f);

					if (currentFactory.GetOwner() == 0)
					{
						priority += 20;
					}

					int cyborgsNeeded = abs(amountCyborgs) + 1;

					objectives.Push(Objective(factoryId, cyborgsNeeded, priority), priority);
				}
			}
		}
	}

	string AssignTroops(const Model* model, PriorityQueue<Objective, float>& objectives, vector<int>& availableCyborgs)
	{
		string commands = "";
		int totalAvailableTroops = 0;

		for (unsigned int i = 0; i < availableCyborgs.size(); i++)
		{
			totalAvailableTroops += availableCyborgs.at(i);
		}

		while (totalAvailableTroops > 0)
		{
			if (objectives.Empty())
			{
				break;
			}

			Objective currentObjective = objectives.Pop();
			vector<int> sortedFactories = model->SortFactoriesByDistance(currentObjective.GetTargetFactory(), 1);

			int sentCyborgs = 0;

			for (unsigned int i = 0; i < sortedFactories.size(); i++)
			{
				int factoryId = sortedFactories.at(i);

				if (availableCyborgs.at(factoryId) > 0)
				{
					int amountCyborgs = min(availableCyborgs.at(factoryId), currentObjective.GetAmountCyborgsNeeded() - sentCyborgs);

					availableCyborgs.at(factoryId) -= amountCyborgs;
					sentCyborgs += amountCyborgs;
					totalAvailableTroops -= amountCyborgs;

					if (commands != "")
					{
						commands += ';';
					}

					commands.append(
							"MOVE " + to_string(factoryId) + " " + to_string(currentObjective.GetTargetFactory()) + " "
									+ to_string(amountCyborgs));

					if (sentCyborgs == currentObjective.GetAmountCyborgsNeeded() || totalAvailableTroops == 0)
					{
						break;
					}
				}
			}
		}

		return commands;
	}
};

}
using namespace decisions;

//################################################################################
//################################################################################
// HELPER METHODS
//################################################################################
//################################################################################

int SelectHighProductionFactory(int originFactoryId, Model* model, int targetOwner)
{
	//int bestProduction = 0;
	int shortestDistance = 21;
	int targetFactoryId = -1;

	for (int id = 0; id < model->GetNumFactories(); id++)
	{
		if (id != originFactoryId)
		{
			Factory currentFactory = model->GetFactory(id);

			if (currentFactory.GetOwner() == targetOwner
					&& currentFactory.GetProduction() > 0
					&& model->GetDistance(originFactoryId, currentFactory.GetId()) < shortestDistance)
			{
				shortestDistance = model->GetDistance(originFactoryId, currentFactory.GetId());
				targetFactoryId = currentFactory.GetId();

				/*if (currentFactory->GetProduction() > bestProduction
				 || (currentFactory->GetProduction() == bestProduction
				 && model->GetDistance(originFactoryId, currentFactory->GetId()) < shortestDistance))
				 {
				 bestProduction = currentFactory->GetProduction();
				 shortestDistance = model->GetDistance(originFactoryId, currentFactory->GetId());
				 targetFactoryId = currentFactory->GetId();
				 }*/
			}
		}
	}

	return targetFactoryId;
}

int SelectedNearestFactory(int originFactoryId, Model* model)
{
	int shortestDistance = 21;
	int targetFactoryId = -1;

	for (int id = 0; id < model->GetNumFactories(); id++)
	{
		if (id != originFactoryId)
		{
			Factory currentFactory = model->GetFactory(id);

			if (currentFactory.GetOwner() != 1)
			{
				if (model->GetDistance(originFactoryId, currentFactory.GetId()) < shortestDistance)
				{
					shortestDistance = model->GetDistance(originFactoryId, currentFactory.GetId());
					targetFactoryId = currentFactory.GetId();
				}
			}
		}
	}

	return targetFactoryId;
}

int ComputeAvailableCyborgs(Factory factory)
{
	int garrison = factory.GetProduction() * 5;
	return max(0, factory.GetNumCyborgs() - garrison);
}

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

		/*vector<Factory> myFactories = _model->GetOwnedFactories(1);
		 bool gaveOrder = false;

		 for (unsigned int i = 0; i < myFactories.size(); i++)
		 {
		 Factory selectedFactory;

		 if (ComputeAvailableCyborgs(myFactories.at(i)) > 0)
		 {
		 selectedFactory = myFactories.at(i);
		 }

		 if (selectedFactory.GetId() >= 0)
		 {
		 int targetFactoryId = SelectHighProductionFactory(selectedFactory.GetId(), _model, 0);

		 if (targetFactoryId == -1)
		 {
		 SelectHighProductionFactory(selectedFactory.GetId(), _model, -1);
		 }

		 if (targetFactoryId == -1)
		 {
		 targetFactoryId = SelectedNearestFactory(selectedFactory.GetId(), _model);
		 }

		 if (targetFactoryId >= 0)
		 {
		 Factory targetFactory = _model->GetFactory(targetFactoryId);

		 if (gaveOrder)
		 cout << ";";

		 cout << "MOVE " << selectedFactory.GetId() << " " << targetFactory.GetId() << " "
		 << ComputeAvailableCyborgs(selectedFactory);
		 gaveOrder = true;
		 }
		 }
		 }

		 if (!gaveOrder)
		 cout << "WAIT" << endl;
		 else
		 cout << endl;*/

		//************************************************************
		// cleaning up
		_model->CleanUp();
	}

//************************************************************
// final cleaning up

	delete (_model);
	delete (_bot);
}
