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
		std::string command = "WAIT";

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

//		Robot bot = model.GetBot();
//		std::vector<SampleData> botSamples = model.GetSamples(0);
//		std::vector<SampleData> cloudSamples = model.GetSamples(-1);
//
//		if (botSamples.size() == 0 && bot.GetTarget() != "DIAGNOSIS")
//		{
//			return "GOTO DIAGNOSIS";
//		}
//		else if (bot.GetTarget() == "DIAGNOSIS" && botSamples.size() < 3)
//		{
//			return "CONNECT 2";
////			int bestSampleId = SelectBestSample(cloudSamples);
////
////			if (bestSampleId != -1)
////			{
////				return "CONNECT " + std::to_string(bestSampleId);
////			}
//		}
//		else
//		{
//			SampleData currentSample = botSamples[0];
//			int neededMolecule = -1;
//			bool moleculesNeeded = false;
//
//			for (int moleculeIndex = 0; moleculeIndex < 5; moleculeIndex++)
//			{
//				int need = currentSample.GetCost(moleculeIndex) - bot.GetStorage(moleculeIndex);
//
//				if (need > 0)
//				{
//					neededMolecule = moleculeIndex;
//					moleculesNeeded = true;
//					break;
//				}
//			}
//
//			if (moleculesNeeded)
//			{
//				if (bot.GetTarget() != "MOLECULES")
//				{
//					return "GOTO MOLECULES";
//				}
//				else
//				{
//					return "CONNECT " + MOLECULE_TYPES[neededMolecule];
//				}
//			}
//			else
//			{
//				if (bot.GetTarget() != "LABORATORY")
//				{
//					return "GOTO LABORATORY";
//				}
//				else
//				{
//					return "CONNECT " + std::to_string(currentSample.GetSampleId());
//				}
//			}
//		}
//
//		return "WAIT";
	}

private:

	bool HandleSampleState(const Model& model, std::string& outCommand)
	{
		Robot bot = model.GetBot();
		std::vector<SampleData> botSamples = model.GetSamples(0);

		if (bot.GetTarget() != "SAMPLES")
		{
			outCommand = "GOTO SAMPLES";
			return true;
		}
		else if (botSamples.size() < 1)
		{
			outCommand = "CONNECT 2";
			return true;
		}

		this->_currentState = BotState::DIAGNOSIS;
		return false;
	}

	bool HandleDiagnosisState(const Model& model, std::string& outCommand)
	{
		Robot bot = model.GetBot();
		std::vector<SampleData> botSamples = model.GetSamples(0);

		if (bot.GetTarget() != "DIAGNOSIS")
		{
			outCommand = "GOTO DIAGNOSIS";
			return true;
		}
		else
		{
			for (unsigned int botSamplesIndex = 0; botSamplesIndex < botSamples.size(); botSamplesIndex++)
			{
				if (botSamples[botSamplesIndex].GetTotalCost() == -5)
				{
					outCommand = "CONNECT " + std::to_string(botSamples[botSamplesIndex].GetSampleId());
					return true;
				}
//				else
//				{
//					SampleData sample = botSamples[botSamplesIndex];
//					std::cerr << "SampleData " << sample.GetSampleId() << std::endl;
//					std::cerr << " -cost: " << sample.GetCost(0) << "," << sample.GetCost(1) << "," << sample.GetCost(2) << ","
//							<< sample.GetCost(3) << "," << sample.GetCost(4) << std::endl;
//					std::cerr << " -health: " << sample.GetHealth() << std::endl;
//				}
			}
		}

		this->_currentState = BotState::MOLECULES;
		return false;
	}

	bool HandleMoleculeState(const Model& model, std::string& outCommand)
	{
		Robot bot = model.GetBot();
		std::vector<SampleData> botSamples = model.GetSamples(0);

		if (bot.GetTarget() != "MOLECULES")
		{
			outCommand = "GOTO MOLECULES";
			return true;
		}
		else
		{
			int neededMolecules[5] { 0, 0, 0, 0, 0 };

			for (unsigned int botSamplesIndex = 0; botSamplesIndex < botSamples.size(); botSamplesIndex++)
			{
				for (int moleculeIndex = 0; moleculeIndex < 5; moleculeIndex++)
				{
					neededMolecules[moleculeIndex] += botSamples[botSamplesIndex].GetCost(moleculeIndex);

					if (bot.GetStorage(moleculeIndex) < neededMolecules[moleculeIndex])
					{
						outCommand = "CONNECT " + MOLECULE_TYPES[moleculeIndex];
						return true;
					}
				}
			}
		}

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
	 * Return the id of the best sample
	 */
	static int SelectBestSample(const std::vector<SampleData>& samples)
	{
		int bestSampleId = -1;
		double bestSampleScore = 0;
		int bestSampleHealth = 0;

		for (unsigned int sampleIndex = 0; sampleIndex < samples.size(); sampleIndex++)
		{
			SampleData currentSample = samples[sampleIndex];
			double currentSampleScore = currentSample.GetHealth() / (double) currentSample.GetTotalCost();

			if (bestSampleId == -1 || currentSampleScore > bestSampleScore)
			{
				bestSampleId = currentSample.GetSampleId();
				bestSampleScore = currentSampleScore;
				bestSampleHealth = currentSample.GetHealth();
			}
			else if (currentSampleScore == bestSampleScore && currentSample.GetHealth() > bestSampleHealth)
			{
				bestSampleId = currentSample.GetSampleId();
				bestSampleScore = currentSampleScore;
				bestSampleHealth = currentSample.GetHealth();
			}
		}

		return bestSampleId;
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
