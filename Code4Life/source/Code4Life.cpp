/*
 * Code4Life.cpp
 *
 *  Created on: May 12, 2017
 *      Author: patrick
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

namespace model
{

class Robot
{
private:
	std::string _target; //module where the robot is
	int _eta;
	int _score; //the robots's number of health points
	int _storage[5]; //number of molecules held by the robot for each molecule type
	int _expertise[5];

public:
	void Update(std::string target, int eta, int score, int storageA, int storageB, int storageC, int storageD, int storageE,
			int expertiseA, int expertiseB, int expertiseC, int expertiseD, int expertiseE)
	{
		_target = target;
		_eta = eta;
		_score = score;
		_storage[0] = storageA;
		_storage[1] = storageB;
		_storage[2] = storageC;
		_storage[3] = storageD;
		_storage[4] = storageE;
		_expertise[0] = expertiseA;
		_expertise[1] = expertiseB;
		_expertise[2] = expertiseC;
		_expertise[3] = expertiseD;
		_expertise[4] = expertiseE;
	}

	inline std::string GetTarget() const
	{
		return _target;
	}

	inline int GetETA() const
	{
		return _eta;
	}

	inline int GetScore() const
	{
		return _score;
	}

	inline int GetStorage(int index) const
	{
		return _storage[index];
	}

	inline int GetExpertise(int index) const
	{
		return _expertise[index];
	}
};

class SampleData
{
private:
	int _sampleId;
	int _carrier;
	int _rank;
	std::string _gain;
	int _health;
	int _cost[5];

public:
	SampleData(int sampleId, int carrier, int rank, std::string gain, int health, int costA, int costB, int costC, int costD, int costE)
			: _sampleId(sampleId), _carrier(carrier), _rank(rank), _gain(gain), _health(health), _cost { costA, costB, costC, costD, costE }
	{
	}

	inline int GetSampleId() const
	{
		return _sampleId;
	}

	inline int GetCarrier() const
	{
		return _carrier;
	}

	inline int GetRank() const
	{
		return _rank;
	}

	inline std::string GetGain() const
	{
		return _gain;
	}

	inline int GetHealth() const
	{
		return _health;
	}

	inline int GetCost(int index) const
	{
		return _cost[index];
	}
};

class Model
{
private:
	Robot _bot;
	Robot _enemy;
	std::vector<SampleData> _sampleData;

public:
	//===============================================
	//SETTER

	inline void UpdateRobot(int robotId, std::string target, int eta, int score, int storageA, int storageB, int storageC, int storageD,
			int storageE, int expertiseA, int expertiseB, int expertiseC, int expertiseD, int expertiseE)
	{
		if (robotId == 0)
		{
			_bot.Update(target, eta, score, storageA, storageB, storageC, storageD, storageE, expertiseA, expertiseB, expertiseC,
					expertiseD, expertiseE);
		}
		else if (robotId == 1)
		{
			_enemy.Update(target, eta, score, storageA, storageB, storageC, storageD, storageE, expertiseA, expertiseB, expertiseC,
					expertiseD, expertiseE);
		}
	}

	inline void CleanUp()
	{
		_sampleData.clear();
	}

	inline void AddSampleData(int sampleId, int carrier, int rank, std::string gain, int health, int costA, int costB, int costC, int costD,
			int costE)
	{
		_sampleData.emplace_back(sampleId, carrier, rank, gain, health, costA, costB, costC, costD, costE);
	}

	//===============================================
	//GETTER
	inline Robot GetBot() const
	{
		return _bot;
	}

	inline Robot GetEnemy() const
	{
		return _enemy;
	}
};

}
using namespace model;

/**
 * Bring data on patient samples from the diagnosis machine to the laboratory with enough molecules to produce medicine!
 **/
int main()
{
	Model _model;

	int projectCount;
	std::cin >> projectCount;
	std::cin.ignore();
	for (int i = 0; i < projectCount; i++)
	{
		int a;
		int b;
		int c;
		int d;
		int e;
		std::cin >> a >> b >> c >> d >> e;
		std::cin.ignore();
	}

	// game loop
	while (1)
	{
		//reading robot data
		for (int i = 0; i < 2; i++)
		{
			std::string target;
			int eta;
			int score;
			int storageA;
			int storageB;
			int storageC;
			int storageD;
			int storageE;
			int expertiseA;
			int expertiseB;
			int expertiseC;
			int expertiseD;
			int expertiseE;
			std::cin >> target >> eta >> score >> storageA >> storageB >> storageC >> storageD >> storageE >> expertiseA >> expertiseB
					>> expertiseC >> expertiseD >> expertiseE;
			std::cin.ignore();

			_model.UpdateRobot(i, target, eta, score, storageA, storageB, storageC, storageD, storageE, expertiseA, expertiseB, expertiseC,
					expertiseD, expertiseE);
		}

		int availableA;
		int availableB;
		int availableC;
		int availableD;
		int availableE;
		std::cin >> availableA >> availableB >> availableC >> availableD >> availableE;
		std::cin.ignore();

		int sampleCount;
		std::cin >> sampleCount;
		std::cin.ignore();
		for (int i = 0; i < sampleCount; i++)
		{
			int sampleId;
			int carriedBy;
			int rank;
			std::string expertiseGain;
			int health;
			int costA;
			int costB;
			int costC;
			int costD;
			int costE;
			std::cin >> sampleId >> carriedBy >> rank >> expertiseGain >> health >> costA >> costB >> costC >> costD >> costE;
			std::cin.ignore();

			_model.AddSampleData(sampleId, carriedBy, rank, expertiseGain, health, costA, costB, costC, costD, costE);
		}

		// Write an action using cout. DON'T FORGET THE "<< endl"
		// To debug: cerr << "Debug messages..." << endl;

		std::cout << "GOTO DIAGNOSIS" << std::endl;
	}
}
