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

const int MAX_DISTANCE = 21;

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

	inline priority_t Pop(T& item)
	{
		element e = elements.top();
		priority_t p = e.first;
		item = e.second;
		elements.pop();
		return p;
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
	Factory(int id)
	{
		_id = id;
	}

	void Update(int owner, int numCyborgs, int production)
	{
		_owner = owner;
		_numCyborgs = numCyborgs;
		_production = production;
	}

	int GetId()
	{
		return _id;
	}

	int GetOwner() const
	{
		return _owner;
	}

	int GetNumCyborgs()
	{
		return _numCyborgs;
	}

	int GetProduction()
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

	void Update(int owner, int originId, int destinationId, int numCyborgs, int timeRemaining)
	{
		_owner = owner;
		_originId = originId;
		_destinationId = destinationId;
		_numCyborgs = numCyborgs;
		_timeRemaining = timeRemaining;
	}

	int GetOwner()
	{
		return _owner;
	}

	int GetOriginId()
	{
		return _originId;
	}

	int GetDestinationId()
	{
		return _destinationId;
	}

	int GetNumCyborgs()
	{
		return _numCyborgs;
	}

	int GetTimeRemaining()
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

	void CleanUp()
	{
		_troops.clear();
	}

	Factory* GetFactory(int id)
	{
		return &_factories.at(id);
	}

	vector<Factory*> GetOwnedFactories(int owner)
	{
		vector<Factory*> result;

		for (int i = 0; i < _numFactories; i++)
		{
			if (_factories.at(i).GetOwner() == owner)
			{
				result.push_back(&_factories.at(i));
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

	int GetTargetFactory()
	{
		return _targetFactory;
	}

	int GetAmountCyborgsNeeded()
	{
		return _amountCyborgsNeeded;
	}

	int GetPriority()
	{
		return _priority;
	}
};

class Bot
{
//variables
private:
	PriorityQueue<Objective, float> _objectives;
	vector<int> _availableCyborgs;

//public methods
public:
	string ComputeMoves(Model* model)
	{
		_objectives.Clear();
		_availableCyborgs = vector<int>(model->GetNumFactories());

		EvaluateFactories(model);

		return nullptr;
	}

//private methods
private:
	void EvaluateFactories(Model* model)
	{
		for (int factoryId = 0; factoryId < model->GetNumFactories(); factoryId++)
		{
			Factory* currentFactory = model->GetFactory(factoryId);
			vector<Troop> incomingTroops = model->GetTroops(factoryId);

			if (currentFactory->GetOwner() == 1) //owned factory
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

				int amountCyborgs = currentFactory->GetNumCyborgs();

				for (int turn = 0; turn < MAX_DISTANCE; turn++)
				{
					if (amountCyborgs >= 0)
					{
						amountCyborgs += currentFactory->GetProduction();
					}
					else
					{
						amountCyborgs += currentFactory->GetProduction();
					}

					amountCyborgs += incomingDeltas.at(turn);

					if (amountCyborgs < 0)
					{
						float priority = 10;

						priority += 30 * (1 - (MAX_DISTANCE / (float) max(1.0f, model->GetAverageDistance(factoryId, 1))));

						if (currentFactory->GetProduction() > 0)
						{
							priority += 30;
						}

						priority += 30 * (MAX_DISTANCE / (float) max(1, turn));

						int reinforcementsNeeded = abs(amountCyborgs) + 1;

						_objectives.Push(Objective(factoryId, reinforcementsNeeded, priority), priority);

						break;
					}
				}
			}
			else //factory controlled by neutral or enemy
			{
				int amountCyborgs = -currentFactory->GetNumCyborgs();

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

				if (amountCyborgs < 0)
				{
					float priority = 30 * (1 - (MAX_DISTANCE / (float) max(1.0f, model->GetAverageDistance(factoryId, 1))));

					if (currentFactory->GetProduction() > 0)
					{
						priority += 30;
					}

					int cyborgsNeeded = abs(amountCyborgs) + 1;

					_objectives.Push(Objective(factoryId, cyborgsNeeded, priority), priority);
				}
			}
		}
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
	int bestProduction = 0;
	int shortestDistance = 21;
	int targetFactoryId = -1;

	for (int id = 0; id < model->GetNumFactories(); id++)
	{
		if (id != originFactoryId)
		{
			Factory* currentFactory = model->GetFactory(id);

			if (currentFactory->GetOwner() == targetOwner
					&& currentFactory->GetProduction() > 0
					&& model->GetDistance(originFactoryId, currentFactory->GetId()) < shortestDistance)
			{
				shortestDistance = model->GetDistance(originFactoryId, currentFactory->GetId());
				targetFactoryId = currentFactory->GetId();

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
			Factory* currentFactory = model->GetFactory(id);

			if (currentFactory->GetOwner() != 1)
			{
				if (model->GetDistance(originFactoryId, currentFactory->GetId()) < shortestDistance)
				{
					shortestDistance = model->GetDistance(originFactoryId, currentFactory->GetId());
					targetFactoryId = currentFactory->GetId();
				}
			}
		}
	}

	return targetFactoryId;
}

int ComputeAvailableCyborgs(Factory* factory)
{
	int garrison = factory->GetProduction() * 5;
	return max(0, factory->GetNumCyborgs() - garrison);
}

//################################################################################
//################################################################################
// MAIN
//################################################################################
//################################################################################
int main()
{
	Model* _model;

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
				Factory* factory = _model->GetFactory(entityId);
				factory->Update(arg1, arg2, arg3);
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

		vector<Factory*> myFactories = _model->GetOwnedFactories(1);
		bool gaveOrder = false;

		for (unsigned int i = 0; i < myFactories.size(); i++)
		{
			Factory* selectedFactory = nullptr;

			if (ComputeAvailableCyborgs(myFactories.at(i)) > 0)
			{
				selectedFactory = myFactories.at(i);
			}

			if (selectedFactory != nullptr)
			{
				int targetFactoryId = SelectHighProductionFactory(selectedFactory->GetId(), _model, 0);

				if (targetFactoryId == -1)
				{
					SelectHighProductionFactory(selectedFactory->GetId(), _model, -1);
				}

				if (targetFactoryId == -1)
				{
					targetFactoryId = SelectedNearestFactory(selectedFactory->GetId(), _model);
				}

				if (targetFactoryId >= 0)
				{
					Factory* targetFactory = _model->GetFactory(targetFactoryId);

					if (gaveOrder)
						cout << ";";

					cout << "MOVE " << selectedFactory->GetId() << " " << targetFactory->GetId() << " "
							<< ComputeAvailableCyborgs(selectedFactory);
					gaveOrder = true;
				}
			}
		}

		if (!gaveOrder)
			cout << "WAIT" << endl;
		else
			cout << endl;

		//************************************************************
		// cleaning up

		_model->CleanUp();
	}

//************************************************************
// final cleaning up

	delete (_model);
}
