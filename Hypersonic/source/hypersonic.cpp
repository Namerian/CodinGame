/*
 * hypersonic.cpp
 *
 *  Created on: Apr 6, 2017
 *      Author: patrick
 */

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
//#include <queue>

using namespace std;

struct Player
{
	Player() :
			owner(-1), x(-1), y(-1), numAvailBombs(0), explosionRange(0)
	{
	}

	Player(int owner, int x, int y, int numAvailBombs, int explosionRange) :
			owner(owner), x(x), y(y), numAvailBombs(numAvailBombs), explosionRange(explosionRange)
	{
	}

	int owner;
	int x;
	int y;
	int numAvailBombs;
	int explosionRange;
};

struct Bomb
{
	Bomb(int owner, int x, int y, int roundsLeft, int explosionRange) :
			owner(owner), x(x), y(y), roundsLeft(roundsLeft), explosionRange(explosionRange)
	{
	}

	int owner;
	int x;
	int y;
	int roundsLeft;
	int explosionRange;
};

struct Item
{
	Item() :
			x(-1), y(-1), type(0)
	{
	}

	Item(int x, int y, int type) :
			x(x), y(y), type(type)
	{
	}

	int x;
	int y;
	int type; //1=extra range, 2=extra bomb
};

//==========================================================================================================
//==========================================================================================================
//
//==========================================================================================================
//==========================================================================================================

int ComputeIndex(const int posX, const int posY, const int mapWidth)
{
	return posX + posY * mapWidth;
}

void ComputePos(const int index, const int mapWidth, int &x, int &y)
{
	y = index / mapWidth;
	x = index % mapWidth;
}

float ComputeDistance(const int x1, const int y1, const int x2, const int y2)
{
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

bool FillBombRangeInfo(const int posIndex, const Bomb &bomb, int *map)
{
	switch (map[posIndex])
	{
	case -1:
		map[posIndex] = -2;
		return true;
	case 0:
		map[posIndex] = bomb.roundsLeft;
		return false;
	}

	return false;
}

vector<int> ComputeReachableTiles(const int x, const int y, const int* map, const int mapWidth, const int mapSize)
{
	vector<int> result;
	vector<int> openTiles;

	int startIndex = ComputeIndex(x, y, mapWidth);
	result.push_back(startIndex);
	openTiles.push_back(startIndex);

	while (!openTiles.empty())
	{
		int currentIndex = openTiles.front();
		openTiles.erase(openTiles.begin());

		int neighbourIndex = currentIndex + 1;
		if (neighbourIndex < mapSize && neighbourIndex % mapWidth != 0 && map[neighbourIndex] >= 0
				&& find(result.begin(), result.end(), neighbourIndex) == result.end())
		{
			result.push_back(neighbourIndex);
			openTiles.push_back(neighbourIndex);
		}

		neighbourIndex = currentIndex - 1;
		if (neighbourIndex >= 0 && neighbourIndex % mapWidth != mapWidth - 1 && map[neighbourIndex] >= 0
				&& find(result.begin(), result.end(), neighbourIndex) == result.end())
		{
			result.push_back(neighbourIndex);
			openTiles.push_back(neighbourIndex);
		}

		neighbourIndex = currentIndex + mapWidth;
		if (neighbourIndex < mapSize && map[neighbourIndex] >= 0 && find(result.begin(), result.end(), neighbourIndex) == result.end())
		{
			result.push_back(neighbourIndex);
			openTiles.push_back(neighbourIndex);
		}

		neighbourIndex = currentIndex - mapWidth;
		if (neighbourIndex >= 0 && map[neighbourIndex] >= 0 && find(result.begin(), result.end(), neighbourIndex) == result.end())
		{
			result.push_back(neighbourIndex);
			openTiles.push_back(neighbourIndex);
		}
	}

	return result;
}

void EvaluateBombPositions(const int mapWidth, const int mapSize, const int* map, const Player &player, int &bestTileIndex,
		int &secondTileIndex, float &bestTilePriority, float &secondTilePriority)
{
	vector<int> reachableTiles = ComputeReachableTiles(player.x, player.y, map, mapWidth, mapSize);

	for (unsigned int i = 0; i < reachableTiles.size(); i++)
	{
		int index = reachableTiles[i];

		if (map[index] != 0)
			continue;

		float priority = 0;
		int newIndex = -1;
		bool up = false, down = false, left = false, right = false;

		for (int r = 1; r < player.explosionRange; r++)
		{
			newIndex = index + r;
			if (!right && newIndex < mapSize && newIndex % mapWidth != 0)
			{
				if (map[newIndex] == -1)
				{
					priority++;
					right = true;
				}
				else if (map[newIndex] == -4)
				{
					right = true;
				}
			}

			newIndex = index - r;
			if (!left && newIndex >= 0 && newIndex % mapWidth != mapWidth - 1)
			{
				if (map[newIndex] == -1)
				{
					priority++;
					left = true;
				}
				else if (map[newIndex] == -4)
				{
					left = true;
				}
			}

			newIndex = index + r * mapWidth;
			if (!down && newIndex < mapSize)
			{
				if (map[newIndex] == -1)
				{
					priority++;
					down = true;
				}
				else if (map[newIndex] == -4)
				{
					down = true;
				}
			}

			newIndex = index - r * mapWidth;
			if (!up && newIndex >= 0)
			{
				if (map[newIndex] == -1)
				{
					priority++;
					up = true;
				}
				else if (map[newIndex] == -4)
				{
					up = true;
				}
			}
		}

		if (priority > 0)
		{
			int cX, cY;
			ComputePos(index, mapWidth, cX, cY);
			float distance = ComputeDistance(player.x, player.y, cX, cY);
			float score;

			if (distance == 0)
				score = 2;
			else
				score = 2.0f / distance;

			priority += score;

			if (priority > bestTilePriority)
			{
				secondTileIndex = bestTileIndex;
				secondTilePriority = bestTilePriority;

				bestTileIndex = index;
				bestTilePriority = priority;
			}
			else if (priority > secondTilePriority)
			{
				secondTileIndex = index;
				secondTilePriority = priority;
			}
		}
	}

	if (bestTileIndex == -1)
	{
		cerr << "Problem with bomb position evaluation!" << endl;
	}
}

bool ComputeNearestItemPos(const int itemType, vector<Item> &items, const Player &player, const int* map, const int mapWidth,
		const int mapSize, int &resultX, int &resultY)
{
	Item nearestItem;
	float shortestDistance = 500;
	vector<int> reachableTiles = ComputeReachableTiles(player.x, player.y, map, mapWidth, mapSize);
	bool itemFound = false;

	for (unsigned int i = 0; i < items.size(); i++)
	{
		Item &item = items[i];
		int itemPosIndex = ComputeIndex(item.x, item.y, mapWidth);

		if (item.type == itemType && find(reachableTiles.begin(), reachableTiles.end(), itemPosIndex) != reachableTiles.end())
		{
			float distance = ComputeDistance(player.x, player.y, item.x, item.y);

			if (distance < shortestDistance)
			{
				nearestItem = item;
				shortestDistance = distance;
				itemFound = true;
			}
		}
	}

	resultX = nearestItem.x;
	resultY = nearestItem.y;

	return itemFound;
}

//==========================================================================================================
//==========================================================================================================
//
//==========================================================================================================
//==========================================================================================================

/**
 *
 * map legend:
 * 	"0" = empty tile
 * 	"-1" = box that is not in range of a bomb
 * 	"-2" = box that is in range of a bomb
 * 	"-3" = bomb
 * 	"-4" = wall
 * 	">1" = empty tile that is in explosion range of a bomb, number is the number of turns until the bomb explodes
 **/
int main()
{
	int _mapWidth;
	int _mapHeight;
	int _myId;
	cin >> _mapWidth >> _mapHeight >> _myId;
	cin.ignore();

	int _mapSize = _mapWidth * _mapHeight;

	int _map[_mapSize];

	Player _player;
	vector<Item> _items;

	bool _extraBombItemAvailable = false;
	bool _extraRangeItemAvailable = false;
	int _bestTileIndex = -1;
	int _secondTileIndex = -1;
	float _bestTilePriority = -1;
	float _secondTilePriority = -1;

// game loop
	while (1)
	{
		for (int i = 0; i < _mapHeight; i++)
		{
			int index;
			string row;
			cin >> row;
			cin.ignore();
			//cerr << "row input: " << row << endl;

			for (int j = 0; j < _mapWidth; j++)
			{
				index = ComputeIndex(j, i, _mapWidth);

				switch (row[j])
				{
				case '.':
					_map[index] = 0;
					break;
				case 'X':
					_map[index] = -4;
					break;
				default:
					_map[index] = -1;
				}
			}
		}

		int entities;
		cin >> entities;
		cin.ignore();
		for (int i = 0; i < entities; i++)
		{
			int entityType;
			int owner;
			int x;
			int y;
			int param1;
			int param2;
			cin >> entityType >> owner >> x >> y >> param1 >> param2;
			cin.ignore();

			if (entityType == 0) //player
			{
				if (owner == _myId)
				{
					_player = Player(owner, x, y, param1, param2);
				}
			}
			else if (entityType == 1) //bomb
			{
				Bomb bomb = Bomb(owner, x, y, param1, param2);
				int index = ComputeIndex(x, y, _mapWidth);
				int newIndex;
				bool up = false, down = false, left = false, right = false;

				_map[index] = -3;

				for (int r = 1; r < param2; r++)
				{
					newIndex = index + r;
					if (!right && newIndex < _mapSize && newIndex % _mapWidth != 0)
						right = FillBombRangeInfo(newIndex, bomb, _map);

					newIndex = index - r;
					if (!left && newIndex >= 0 && newIndex % _mapWidth != _mapWidth - 1)
						left = FillBombRangeInfo(newIndex, bomb, _map);

					newIndex = index + r * _mapWidth;
					if (!down && newIndex < _mapSize)
						down = FillBombRangeInfo(newIndex, bomb, _map);

					newIndex = index - r * _mapWidth;
					if (!up && newIndex >= 0)
						up = FillBombRangeInfo(newIndex, bomb, _map);
				}
			}
			else if (entityType == 2) //item
			{
				_items.push_back(Item(x, y, param1));

				switch (param1)
				{
				case 1:
					_extraRangeItemAvailable = true;
					break;
				case 2:
					_extraBombItemAvailable = true;
					break;
				}
			}
		}

		//***************************************************************
		//***************************************************************
		//decision-making

		string command;
		int dX, dY;

		if (_player.numAvailBombs < 1 && (_extraBombItemAvailable || _extraRangeItemAvailable))
		{
			command = "MOVE";

			if (_extraRangeItemAvailable && _player.explosionRange < 6)
			{
				ComputeNearestItemPos(1, _items, _player, _map, _mapWidth, _mapSize, dX, dY);
			}
			else if (_extraBombItemAvailable)
			{
				ComputeNearestItemPos(2, _items, _player, _map, _mapWidth, _mapSize, dX, dY);
			}
			else if (_extraRangeItemAvailable)
			{
				ComputeNearestItemPos(1, _items, _player, _map, _mapWidth, _mapSize, dX, dY);
			}
		}
		else
		{
			EvaluateBombPositions(_mapWidth, _mapSize, _map, _player, _bestTileIndex, _secondTileIndex, _bestTilePriority,
					_secondTilePriority);

			int bX, bY;
			ComputePos(_bestTileIndex, _mapWidth, bX, bY);

			if (bX == _player.x && bY == _player.y)
			{
				if (_player.numAvailBombs == 0)
				{
					command = "BOMB";
					dX = bX;
					dY = bY;
				}
				else
				{
					//TODO: reevaluate map
					int sX, sY;
					ComputePos(_secondTileIndex, _mapWidth, sX, sY);

					command = "BOMB";
					dX = sX;
					dY = sY;
				}
			}
			else
			{
				command = "MOVE";

				if (_bestTileIndex > 0)
				{
					dX = bX;
					dY = bY;
				}
				else
				{
					dX = _player.x;
					dY = _player.y;
				}
			}
		}

		//****

		//TODO: replace this by proper pathfinding

		int currentPosIndex = ComputeIndex(_player.x, _player.y, _mapWidth);
		int destinationPosIndex = ComputeIndex(dX, dY, _mapWidth);

		if (_map[destinationPosIndex] == 1)
		{
			if (_map[currentPosIndex] != 1)
			{
				dX = _player.x;
				dY = _player.y;
			}
			else
			{
				int newIndexRight = currentPosIndex + 1;
				int newIndexLeft = currentPosIndex - 1;
				int newIndexDown = currentPosIndex + _mapWidth;
				int newIndexUp = currentPosIndex - _mapWidth;

				if (newIndexRight < _mapSize && newIndexRight % _mapWidth != 0 && newIndexRight != destinationPosIndex
						&& _map[newIndexRight] >= 0 && _map[newIndexRight] != 1)
				{
					dX = _player.x + 1;
					dY = _player.y;
				}
				else if (newIndexLeft >= 0 && newIndexLeft % _mapWidth != _mapWidth - 1 && newIndexLeft != destinationPosIndex
						&& _map[newIndexLeft] >= 0 && _map[newIndexLeft] != 1)
				{
					dX = _player.x - 1;
					dY = _player.y;
				}
				else if (newIndexDown < _mapSize && newIndexDown != destinationPosIndex && _map[newIndexDown] >= 0
						&& _map[newIndexDown] != 1)
				{
					dX = _player.x;
					dY = _player.y + 1;
				}
				else if (newIndexUp >= 0 && newIndexUp != destinationPosIndex && _map[newIndexUp] >= 0 && _map[newIndexUp] != 1)
				{
					dX = _player.x;
					dY = _player.y - 1;
				}
			}
		}

		//****
		cout << command << " " << dX << " " << dY << endl;

		//***************************************************************
		//***************************************************************
		// cleaning up

		for (int i = 0; i < _mapSize; i++)
		{
			_map[i] = 0;
		}

		_items.clear();

		_extraBombItemAvailable = _extraRangeItemAvailable = false;
		_bestTileIndex = _bestTilePriority = _secondTileIndex = _secondTilePriority = -1;
	}
}
