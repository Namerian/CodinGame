#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

//################################################################################################

// -- UTILITY

static double ComputeDistance(int ax, int ay, int bx, int by)
{
    return std::sqrt(std::pow(ax - bx, 2) + std::pow(ay - by, 2));
}

//################################################################################################
//################################################################################################

// -- NAMESPACE MODEL

namespace MODEL
{

struct SITE
{
    int ID;
    int X;
    int Y;
    int Radius;

    int StructureType;
    int Owner;
    int Param1;
    int Param2;
};

struct UNIT
{
    int X;
    int Y;
    int Owner;
    int UnitType;
    int Health;

    UNIT(int x, int y, int owner, int unit_type, int health)
    {
        X = x;
        Y = y;
        Owner = owner;
        UnitType = unit_type;
        Health = health;
    }
};

class GAMESTATE
{
    // -- ATTRIBUTES
  public:
    int SiteCount;
    int Gold;
    int TouchedSite;

  private:
    std::vector<SITE> SiteVector;
    std::vector<UNIT> UnitVector;

    // -- INQUIRIES
  public:
    const SITE *GetSite(int site_id) const
    {
        for (int site_index = 0; site_index < SiteVector.size(); site_index++)
        {
            const SITE &currentSite = SiteVector.at(site_index);
            if (currentSite.ID == site_id)
            {
                return &currentSite;
            }
        }
    }

    const std::vector<SITE> &GetSites() const
    {
        return SiteVector;
    }

    const std::vector<UNIT> &GetUnits() const
    {
        return UnitVector;
    }

    // -- OPERATIONS
  public:
    void Initialize(int site_count)
    {
        SiteCount = site_count;
        SiteVector.clear();
        SiteVector.resize(SiteCount);

        for (int site_index = 0; site_index < SiteCount; site_index++)
        {
            SiteVector.at(site_index).ID = site_index;
        }
    }

    void SetSite(int site_id, int x, int y, int radius)
    {
        if (site_id > -1 && site_id < SiteCount)
        {
            SITE &site = SiteVector.at(site_id);
            site.X = x;
            site.Y = y;
            site.Radius = radius;
        }
    }

    void UpdateSite(int site_id, int structure_type, int owner, int param1, int param2)
    {
        if (site_id > -1 && site_id < SiteCount)
        {
            SITE &site = SiteVector.at(site_id);
            site.StructureType = structure_type;
            site.Owner = owner;
            site.Param1 = param1;
            site.Param2 = param2;
        }
    }

    void ResetUnitVector(int unit_count)
    {
        UnitVector.clear();
        UnitVector.reserve(unit_count);
    }

    void SetUnit(int x, int y, int owner, int unit_type, int health)
    {
        UnitVector.emplace_back(UNIT(x, y, owner, unit_type, health));
    }
};
}

//################################################################################################
//################################################################################################

// -- NAMESPACE AI

namespace AI
{

const std::string DEFAULT_QUEEN_COMMAND = "WAIT";
const std::string DEFAULT_TRAINING_COMMAND = "TRAIN";

const int BARRACKS_STRUCTURE_TYPE = 2;

const int NO_OWNER = -1;
const int FRIENDLY_OWNER = 0;
const int ENEMY_OWNER = 1;

const int QUEEN_UNIT_TYPE = -1;
const int KNIGHT_UNIT_TYPE = 0;
const int ARCHER_UNIT_TYPE = 1;

class BLACKBOARD
{
  public:
    const MODEL::GAMESTATE *CurrentGameState;

  public:
    void UpdateBlackboard(const MODEL::GAMESTATE *current_gamestate)
    {
        CurrentGameState = current_gamestate;
    }
};

class QUEEN
{
  private:
    BLACKBOARD *Blackboard;

  public:
    QUEEN(BLACKBOARD *blackboard)
    {
        Blackboard = blackboard;
    }

    std::string ComputeCommands()
    {
        return DEFAULT_QUEEN_COMMAND;
    }
};

class QUARTERMASTER
{
  private:
    const int DesiredKnightCount = 8;

  private:
    BLACKBOARD *Blackboard;

  public:
    QUARTERMASTER(BLACKBOARD *blackboard)
    {
        Blackboard = blackboard;
    }

    std::string ComputeCommands()
    {
        return DEFAULT_TRAINING_COMMAND;
    }
};

class HIGH_COMMAND
{
  private:
    BLACKBOARD Blackboard;
    QUEEN Queen = QUEEN(&Blackboard);
    QUARTERMASTER Quartermaster = QUARTERMASTER(&Blackboard);

  public:
    HIGH_COMMAND()
    {
    }

    std::pair<std::string, std::string> PlayTurn(MODEL::GAMESTATE *gamestate)
    {
        Blackboard.UpdateBlackboard(gamestate);

        return std::make_pair(Queen.ComputeCommands(), Quartermaster.ComputeCommands());
    }

    //private:
    // std::string ComputeQueenActions(MODEL::GAMESTATE *gamestate)
    // {
    //     std::string result = "";
    //     MODEL::UNIT *queen = gamestate->GetUnit(FRIENDLY_OWNER, QUEEN_UNIT_TYPE);
    //     std::vector<MODEL::SITE *> neutral_sites = gamestate->GetSitesOwnedBy(NO_OWNER);

    //     if (queen == nullptr)
    //     {
    //         std::cerr << "My Queen is null!" << std::endl;
    //         return "WAIT";
    //     }

    //     if (gamestate->TouchedSite != -1)
    //     {
    //         MODEL::SITE *touched_site = gamestate->GetSite(gamestate->TouchedSite);
    //         if (touched_site->Owner == NO_OWNER)
    //         {
    //             result.append("BUILD ");
    //             result.append(std::to_string(touched_site->ID));
    //             result.append(" BARRACKS-");
    //             result.append("KNIGHT");
    //             return result;
    //         }
    //     }

    //     if (neutral_sites.size() > 0)
    //     {
    //         MODEL::SITE *nearestSite = neutral_sites.at(0);
    //         double distance = ComputeDistance(queen->X, queen->Y, nearestSite->X, nearestSite->Y);

    //         for (int site_index = 1; site_index < neutral_sites.size(); site_index++)
    //         {
    //             MODEL::SITE *currentSite = neutral_sites.at(site_index);
    //             double newDistance = ComputeDistance(queen->X, queen->Y, currentSite->X, currentSite->Y);

    //             if (newDistance < distance)
    //             {
    //                 nearestSite = currentSite;
    //                 distance = newDistance;
    //             }
    //         }

    //         result.append("MOVE ");
    //         result.append(std::to_string(nearestSite->X));
    //         result.append(" ");
    //         result.append(std::to_string(nearestSite->Y));
    //         return result;
    //     }
    //     else
    //     {
    //         std::cerr << "No neutral sites left!" << std::endl;
    //     }

    //     return "WAIT";
    // }

    // std::string ComputeTrainingActions(MODEL::GAMESTATE *gamestate)
    // {
    //     std::vector<MODEL::SITE *> owned_sites = gamestate->GetSitesOwnedBy(FRIENDLY_OWNER);
    //     std::vector<MODEL::SITE *> knight_barracks;
    //     std::vector<MODEL::SITE *> archer_barracks;
    //     std::string result = "TRAIN";

    //     for (MODEL::SITE *site : owned_sites)
    //     {
    //         if (site->StructureType == BARRACKS_STRUCTURE_TYPE && site->Param1 == 0)
    //         {
    //             if (site->Param2 == KNIGHT_UNIT_TYPE)
    //             {
    //                 knight_barracks.push_back(site);
    //             }
    //             else if (site->Param2 == ARCHER_UNIT_TYPE)
    //             {
    //                 archer_barracks.push_back(site);
    //             }
    //         }
    //     }

    //     if (knight_barracks.size() > 0)
    //     {
    //         for (MODEL::SITE *site : knight_barracks)
    //         {
    //             result.append(" ");
    //             result.append(std::to_string(site->ID));
    //         }
    //     }

    //     return result;
    // }
};
}

//################################################################################################
//################################################################################################

// -- MAIN

static void InitGamestate(MODEL::GAMESTATE &gamestate)
{
    int site_count;
    std::cin >> site_count;
    std::cin.ignore();

    gamestate.Initialize(site_count);

    for (int site_index = 0; site_index < site_count; site_index++)
    {
        int id;
        int x;
        int y;
        int radius;

        std::cin >> id >> x >> y >> radius;
        std::cin.ignore();

        gamestate.SetSite(id, x, y, radius);
    }
}

static void UpdateGamestate(MODEL::GAMESTATE &gamestate)
{
    int gold;
    int touchedSite; // -1 if none
    std::cin >> gold >> touchedSite;
    std::cin.ignore();

    gamestate.Gold = gold;
    gamestate.TouchedSite = touchedSite;

    for (int i = 0; i < gamestate.SiteCount; i++)
    {
        int site_id;
        int ignore1;        // used in future leagues
        int ignore2;        // used in future leagues
        int structure_type; // -1 = No structure, 2 = Barracks
        int owner;          // -1 = No structure, 0 = Friendly, 1 = Enemy
        int param1;
        int param2;
        std::cin >> site_id >> ignore1 >> ignore2 >> structure_type >> owner >> param1 >> param2;
        std::cin.ignore();

        gamestate.UpdateSite(site_id, structure_type, owner, param1, param2);
    }

    int unit_count;
    std::cin >> unit_count;
    std::cin.ignore();

    gamestate.ResetUnitVector(unit_count);

    for (int i = 0; i < unit_count; i++)
    {
        int x;
        int y;
        int owner;
        int unit_type; // -1 = QUEEN, 0 = KNIGHT, 1 = ARCHER
        int health;
        std::cin >> x >> y >> owner >> unit_type >> health;
        std::cin.ignore();

        gamestate.SetUnit(x, y, owner, unit_type, health);
    }
}

int main()
{
    MODEL::GAMESTATE gamestate;
    AI::HIGH_COMMAND high_command;

    InitGamestate(gamestate);

    // game loop
    while (1)
    {
        UpdateGamestate(gamestate);

        std::pair<std::string, std::string> output = high_command.PlayTurn(&gamestate);

        std::cout << output.first << std::endl;
        std::cout << output.second << std::endl;
    }
}

//################################################################################################