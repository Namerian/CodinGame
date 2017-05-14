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

const std::string MOLECULE_TYPES[5] { "A", "B", "C", "D", "E" };

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

	inline int GetTotalCost() const
	{
		return _cost[0] + _cost[1] + _cost[2] + _cost[3] + _cost[4];
	}
};

class Model
{
private:
	Robot _bot;
	Robot _enemy;
	int _availableMolecules[5] { 0, 0, 0, 0, 0 };
	std::vector<SampleData> _sampleData;

public:
	//===============================================
	//SETTER

	void UpdateRobot(int robotId, std::string target, int eta, int score, int storageA, int storageB, int storageC, int storageD,
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

	void SetAvailableMolecules(int availableA, int availableB, int availableC, int availableD, int availableE)
	{
		_availableMolecules[0] = availableA;
		_availableMolecules[1] = availableB;
		_availableMolecules[2] = availableC;
		_availableMolecules[3] = availableD;
		_availableMolecules[4] = availableE;
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

	inline int GetAvailableMolecules(int index) const
	{
		return _availableMolecules[index];
	}

	std::vector<int> GetAvailableMolecules() const
	{
		std::vector<int> result;
		result.push_back(_availableMolecules[0]);
		result.push_back(_availableMolecules[1]);
		result.push_back(_availableMolecules[2]);
		result.push_back(_availableMolecules[3]);
		result.push_back(_availableMolecules[4]);
		return result;
	}

	/**
	 * Returns an array with the owned samples.
	 * -1 = cloud
	 *  0 = bot
	 *  1 = enemy
	 */
	std::vector<SampleData> GetSamples(int owner) const
	{
		std::vector<SampleData> result;
		auto pos = _sampleData.begin();

		while (pos != _sampleData.end())
		{
			pos = find_if(pos, _sampleData.end(), [&owner](const SampleData& sample)
			{	return sample.GetCarrier()==owner;});

			if (pos != _sampleData.end())
			{
				int index = distance(_sampleData.begin(), pos);
				result.emplace_back(_sampleData[index]);
				pos++;
			}
		}

		return result;
	}

	/**
	 *
	 */
	SampleData GetSample(int sampleId) const
	{
		auto pos = find_if(_sampleData.begin(), _sampleData.end(), [&sampleId](const SampleData& sample)
		{	return sample.GetSampleId()==sampleId;});
		int index = std::distance(_sampleData.begin(), pos);
		return _sampleData[index];
	}
};

}
using namespace model;

namespace bot
{

class Bot
{
private:
	enum BotState
	{
		SAMPLES, DIAGNOSIS, MOLECULES, LABORATORY
	};

	BotState _currentState = BotState::SAMPLES;

public:

	std::string Run(const Model& model)
	{
		Robot bot = model.GetBot();
		std::string command = "WAIT";

		if (bot.GetETA() > 0)
		{
			return command;
		}

		while (true)
		{
			bool commandIssued = false;

			switch (_currentState)
			{
				case BotState::SAMPLES:
					commandIssued = HandleSampleState(model, command);
					break;
				case BotState::DIAGNOSIS:
					commandIssued = HandleDiagnosisState(model, command);
					break;
				case BotState::MOLECULES:
					commandIssued = HandleMoleculeState(model, command);
					break;
				case BotState::LABORATORY:
					commandIssued = HandleLaboratoryState(model, command);
					break;
			}

			if (commandIssued)
			{
				break;
			}
		}

		return command;
	}

private:

	bool HandleSampleState(const Model& model, std::string& outCommand)
	{
		Robot bot = model.GetBot();
		std::vector<SampleData> botSamples = model.GetSamples(0);

		//======================================================
		//move to samples
		if (bot.GetTarget() != "SAMPLES")
		{
			outCommand = "GOTO SAMPLES";
			return true;
		}

		//======================================================
		//take a sample
		if (botSamples.size() < 1)
		{
			outCommand = "CONNECT 2";
			return true;
		}

		//======================================================
		this->_currentState = BotState::DIAGNOSIS;
		return false;
	}

	bool HandleDiagnosisState(const Model& model, std::string& outCommand)
	{
		Robot bot = model.GetBot();
		std::vector<SampleData> botSamples = model.GetSamples(0);

		//======================================================
		//move to diagnosis
		if (bot.GetTarget() != "DIAGNOSIS")
		{
			outCommand = "GOTO DIAGNOSIS";
			return true;
		}

		//======================================================
		//diagnose all undiagnosed samples
		for (unsigned int botSamplesIndex = 0; botSamplesIndex < botSamples.size(); botSamplesIndex++)
		{
			if (botSamples[botSamplesIndex].GetTotalCost() == -5)
			{
				outCommand = "CONNECT " + std::to_string(botSamples[botSamplesIndex].GetSampleId());
				return true;
			}
		}

		//======================================================
		//print diagnosed samples to console
		for (unsigned int botSamplesIndex = 0; botSamplesIndex < botSamples.size(); botSamplesIndex++)
		{
			SampleData sample = botSamples[botSamplesIndex];
			std::cerr << "SampleData " << sample.GetSampleId() << std::endl;
			std::cerr << " -cost: " << sample.GetCost(0) << "," << sample.GetCost(1) << "," << sample.GetCost(2) << "," << sample.GetCost(3)
					<< "," << sample.GetCost(4) << std::endl;
			std::cerr << " -health: " << sample.GetHealth() << std::endl;
		}

		//======================================================
		this->_currentState = BotState::MOLECULES;
		return false;
	}

	bool HandleMoleculeState(const Model& model, std::string& outCommand)
	{
		Robot bot = model.GetBot();
		std::vector<SampleData> botSamples = model.GetSamples(0);

		//======================================================
		//move to molecules
		if (bot.GetTarget() != "MOLECULES")
		{
			outCommand = "GOTO MOLECULES";
			return true;
		}

		//======================================================
		//select the sample(s) to collect molecules for
		std::vector<int> bestSampleCombination = SelectBestSamples(botSamples, bot, model.GetAvailableMolecules());

		if (bestSampleCombination.size() == 0)
		{
			outCommand = "WAIT";
			return true;
		}

		//======================================================
		//
		int neededMolecules[5] { 0, 0, 0, 0, 0 };

		for (unsigned int combinationIndex = 0; combinationIndex < bestSampleCombination.size(); combinationIndex++)
		{
			SampleData sample = model.GetSample(bestSampleCombination[combinationIndex]);
			std::vector<int> sampleCost = ComputeSampleCost(sample, bot);

			for (int moleculeIndex = 0; moleculeIndex < 5; moleculeIndex++)
			{
				neededMolecules[moleculeIndex] += sampleCost[moleculeIndex];

				if (bot.GetStorage(moleculeIndex) < neededMolecules[moleculeIndex])
				{
					outCommand = "CONNECT " + MOLECULE_TYPES[moleculeIndex];
					return true;
				}
			}
		}

		//======================================================
		this->_currentState = BotState::LABORATORY;
		return false;
	}

	bool HandleLaboratoryState(const Model& model, std::string& outCommand)
	{
		Robot bot = model.GetBot();
		std::vector<SampleData> botSamples = model.GetSamples(0);

		if (bot.GetTarget() != "LABORATORY")
		{
			outCommand = "GOTO LABORATORY";
			return true;
		}
		else if (botSamples.size() > 0)
		{
			outCommand = "CONNECT " + std::to_string(botSamples[0].GetSampleId());
			return true;
		}

		this->_currentState = BotState::SAMPLES;
		return false;
	}

	/**
	 *
	 */
	static std::vector<int> SelectBestSamples(const std::vector<SampleData>& samples, const Robot& bot,
			const std::vector<int>& availableMolecules)
	{
		std::vector<int> bestCombination;
		double bestCombinationScore;

		//TODO

		return bestCombination;
	}

	/**
	 *	Computes the cost of a sample, takes into consideration expertise
	 */
	static std::vector<int> ComputeSampleCost(const SampleData& sample, const Robot& bot)
	{
		std::vector<int> result(5);

		for (int moleculeIndex = 0; moleculeIndex < 5; moleculeIndex++)
		{
			result[moleculeIndex] = sample.GetCost(moleculeIndex) - bot.GetExpertise(moleculeIndex);
		}

		return result;
	}

	/**
	 *	Computes the sum of the cost of a sample, takes into consideration expertise
	 */
	static int ComputeTotalSampleCost(const SampleData& sample, const Robot& bot)
	{
		std::vector<int> cost = ComputeSampleCost(sample, bot);
		return cost[0] + cost[1] + cost[2] + cost[3] + cost[4];
	}

	/**
	 *
	 */
	static double ComputeSampleScore(const int& totalCost, int health)
	{
		return (double) health / (double) totalCost;
	}
};

}
using namespace bot;

/**
 * Bring data on patient samples from the diagnosis machine to the laboratory with enough molecules to produce medicine!
 **/
int main()
{
	Model _model;
	Bot _bot;

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

		_model.SetAvailableMolecules(availableA, availableB, availableC, availableD, availableE);

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

			//std::cerr << "SampleData " << sampleId << std::endl;
			//std::cerr << " -cost: " << costA << "," << costB << "," << costC << "," << costD << "," << costE << std::endl;
			//std::cerr << " -health: " << health << std::endl;
		}

		//================================================================================================
		//

		std::string command = _bot.Run(_model);

		std::cout << command << std::endl;

		//================================================================================================
		//

		_model.CleanUp();
	}
}
