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

using namespace std;

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
			_factories.emplace_back(i);
		}
	}

	void SetDistance(int factory1, int factory2, int distance)
	{
		_distanceMatrix.at(factory1).at(factory2) = distance;
		_distanceMatrix.at(factory2).at(factory1) = distance;
	}

	void AddTroop(int owner, int originId, int destinationId, int numCyborgs, int timeRemaining)
	{
		_troops.emplace_back(owner, originId, destinationId, numCyborgs, timeRemaining);
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

	int GetDistance(int factory1, int factory2)
	{
		return _distanceMatrix.at(factory1).at(factory2);
	}

	int GetNumFactories()
	{
		return _numFactories;
	}
};

}
using namespace model;

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
