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

//============================
//============================
struct SITE
{
    int ID = -1;
    int X = -1;
    int Y = -1;
    int Radius = -1;

    int StructureType = -1;
    int Owner = -1;
    int Param1 = -1;
    int Param2 = -1;

    SITE(int id, int x, int y, int radius)
    {
        ID = id;
        X = x;
        Y = y;
        Radius = radius;
    }
};

//============================
//============================
struct UNIT
{
    int X = -1;
    int Y = -1;
    int Owner = -1;
    int UnitType = -1;
    int Health = -1;

    UNIT(int x, int y, int owner, int unit_type, int health)
    {
        X = x;
        Y = y;
        Owner = owner;
        UnitType = unit_type;
        Health = health;
    }
};

//============================
//============================
class GAMESTATE
{
    // -- ATTRIBUTES
  public:
    int Gold;
    int TouchedSite;

    int SiteCount;

    std::vector<SITE> SiteVector;
    std::vector<UNIT> UnitVector;

    // -- CONSTRUCTORS
  public:
    GAMESTATE()
    {
    }

    GAMESTATE(const GAMESTATE &original)
        : Gold(original.Gold),
          TouchedSite(original.TouchedSite),
          SiteVector(original.SiteVector),
          UnitVector(original.UnitVector)
    {
    }

    // -- INQUIRIES
  public:
    SITE *GetSite(int site_id)
    {
        for (SITE &site : SiteVector)
        {
            if (site.ID == site_id)
            {
                return &site;
            }
        }

        return nullptr;
    }

    std::vector<SITE *> GetSites()
    {
        std::vector<SITE *> sites;

        for (SITE &site : SiteVector)
        {
            sites.push_back(&site);
        }

        return sites;
    }

    std::vector<UNIT *> GetUnits()
    {
        std::vector<UNIT *> units;

        for (UNIT &unit : UnitVector)
        {
            units.push_back(&unit);
        }

        return units;
    }

    // -- OPERATIONS
  public:
    void ResetSiteVector(int site_count = 0)
    {
        SiteVector.clear();
        SiteVector.reserve(site_count);
    }

    void AddSite(int site_id, int x, int y, int radius)
    {
        SiteVector.emplace_back(SITE(site_id, x, y, radius));
    }

    void UpdateSite(int site_id, int structure_type, int owner, int param1, int param2)
    {
        for (SITE &site : SiteVector)
        {
            if (site.ID == site_id)
            {
                site.StructureType = structure_type;
                site.Owner = owner;
                site.Param1 = param1;
                site.Param2 = param2;
                break;
            }
        }
    }

    void ResetUnitVector(int unit_count = 0)
    {
        UnitVector.clear();
        UnitVector.reserve(unit_count);
    }

    void AddUnit(int x, int y, int owner, int unit_type, int health)
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

const int UNIT_PRICES[2] = {80, 100};
const int UNIT_SQUAD_SIZES[2] = {4, 2};

//========================================================
//========================================================
// BLACKBOARD
//========================================================
//========================================================

class BLACKBOARD
{
    // -- ATTRIBUTES
  private:
    const MODEL::GAMESTATE *CurrentGameState;
    MODEL::GAMESTATE PlanningGameState;

    int TotalFriendlyUnitCounts[2];

    std::vector<int> TotalFriendlyBarracks[2];
    std::vector<int> AvailableFriendlyBarracks[2];

    std::vector<int> NeutralSites;

    // -- INQUIRIES
  public:
    int GetGold()
    {
        return PlanningGameState.Gold;
    }

    int GetTouchedSite()
    {
        return PlanningGameState.TouchedSite;
    }

    int GetTotalFriendlyUnitCount(int unit_type)
    {
        return TotalFriendlyUnitCounts[unit_type];
    }

    int GetTotalFriendlyBarracksCount(int unit_type)
    {
        return TotalFriendlyBarracks[unit_type].size();
    }

    int GetNeutralSiteCount()
    {
        return NeutralSites.size();
    }

    MODEL::SITE *GetNearestSite(int x, int y)
    {
        MODEL::SITE *nearestSite = nullptr;
        double distance = 999999;

        for (MODEL::SITE *site : PlanningGameState.GetSites())
        {
            double newDistance = ComputeDistance(x, y, site->X, site->Y);

            if (newDistance < distance)
            {
                nearestSite = site;
                distance = newDistance;
            }
        }

        return nearestSite;
    }

    MODEL::UNIT *GetQueen(int owner)
    {
        for (MODEL::UNIT *unit : PlanningGameState.GetUnits())
        {
            if (unit->UnitType == QUEEN_UNIT_TYPE && unit->Owner == owner)
            {
                return unit;
            }
        }

        return nullptr;
    }

    // -- OPERATIONS
  public:
    void UpdateBlackboard(const MODEL::GAMESTATE *current_gamestate)
    {
        CurrentGameState = current_gamestate;
        PlanningGameState = MODEL::GAMESTATE(*current_gamestate);

        for (MODEL::SITE &site : PlanningGameState.SiteVector)
        {
            if (site.Owner == FRIENDLY_OWNER)
            {
                if (site.StructureType == BARRACKS_STRUCTURE_TYPE)
                {
                    TotalFriendlyBarracks[site.Param2].push_back(site.ID);

                    if (site.Param1 == 0)
                    {
                        AvailableFriendlyBarracks[site.Param2].push_back(site.ID);
                    }
                    else
                    {
                        TotalFriendlyUnitCounts[site.Param2] += UNIT_SQUAD_SIZES[site.Param2];
                    }
                }
            }
            else if (site.Owner == NO_OWNER)
            {
                NeutralSites.push_back(site.ID);
            }
        }

        for (MODEL::UNIT &unit : PlanningGameState.UnitVector)
        {
            if (unit.Owner == FRIENDLY_OWNER)
            {
                if (unit.UnitType > -1)
                {
                    TotalFriendlyUnitCounts[unit.UnitType]++;
                }
            }
        }
    }

    int TrainUnit(int unit_type)
    {
        if (PlanningGameState.Gold < UNIT_PRICES[unit_type] || AvailableFriendlyBarracks[unit_type].size() == 0)
        {
            return -1;
        }

        // Selecting Barracks
        int barracks_id = AvailableFriendlyBarracks[unit_type].at(0);
        AvailableFriendlyBarracks[unit_type].erase(AvailableFriendlyBarracks[unit_type].begin());

        // Paying
        PlanningGameState.Gold -= UNIT_PRICES[unit_type];

        // Updating Barracks
        PlanningGameState.GetSite(barracks_id)->Param1 = 99;

        // Updating Blackboard
        TotalFriendlyUnitCounts[unit_type] += UNIT_SQUAD_SIZES[unit_type];

        return barracks_id;
    }

  private:
};

//========================================================
//========================================================
// QUEEN
//========================================================
//========================================================

class QUEEN_STATE
{
  protected:
    BLACKBOARD *Blackboard;

  public:
    QUEEN_STATE(BLACKBOARD *blackboard)
    {
        Blackboard = blackboard;
    }

  public:
    virtual double Evaluate();

    virtual void OnEnter();
    virtual void OnExit();

    virtual std::string ComputeCommands();
};

class IDLE_QUEEN_STATE : public QUEEN_STATE
{
  public:
    IDLE_QUEEN_STATE(BLACKBOARD *blackboard)
        : QUEEN_STATE(blackboard)
    {
    }

  public:
    double Evaluate() override
    {
        return 1;
    }

    void OnEnter() override
    {
    }

    void OnExit()
    {
    }

    std::string ComputeCommands()
    {
        return DEFAULT_QUEEN_COMMAND;
    }
};

class BUILD_BARRACKS_QUEEN_STATE : public QUEEN_STATE
{
  private:
    int TargetSiteId = -1;

  public:
    BUILD_BARRACKS_QUEEN_STATE(BLACKBOARD *blackboard)
        : QUEEN_STATE(blackboard)
    {
    }

  public:
    double Evaluate() override
    {
        bool barracks_needed =
            Blackboard->GetTotalFriendlyBarracksCount(KNIGHT_UNIT_TYPE) == 0     //
            || Blackboard->GetTotalFriendlyBarracksCount(ARCHER_UNIT_TYPE) == 0; //
        bool site_available = Blackboard->GetNeutralSiteCount() > 0;

        if (barracks_needed && site_available)
        {
            return 80;
        }

        return 0;
    }

    void OnEnter() override
    {
        MODEL::UNIT *queen = Blackboard->GetQueen(FRIENDLY_OWNER);
        TargetSiteId = Blackboard->GetNearestSite(queen->X, queen->Y)->ID;
    }

    void OnExit()
    {
    }

    std::string ComputeCommands()
    {
        if (Blackboard->GetTouchedSite() == TargetSiteId)
        {
            if (Blackboard->GetTotalFriendlyBarracksCount(KNIGHT_UNIT_TYPE) == 0)
            {
                
            }
            else if (Blackboard->GetTotalFriendlyBarracksCount(ARCHER_UNIT_TYPE) == 0)
            {
            }
        }

        //MODEL::SITE *destination = Blackboard->GetNearestSite()
        return DEFAULT_QUEEN_COMMAND;
    }
};

class AVOID_ENEMIES_QUEEN_STATE : public QUEEN_STATE
{
  public:
    AVOID_ENEMIES_QUEEN_STATE(BLACKBOARD *blackboard)
        : QUEEN_STATE(blackboard)
    {
    }

  public:
    double Evaluate() override
    {
        return 0;
    }

    void OnEnter() override
    {
    }

    void OnExit()
    {
    }

    std::string ComputeCommands()
    {
        return DEFAULT_QUEEN_COMMAND;
    }
};

class QUEEN
{
  private:
    BLACKBOARD *Blackboard;

    std::vector<QUEEN_STATE> States;
    QUEEN_STATE *CurrentState;

  public:
    QUEEN(BLACKBOARD *blackboard)
    {
        Blackboard = blackboard;

        States.push_back(IDLE_QUEEN_STATE(Blackboard));
        States.push_back(BUILD_BARRACKS_QUEEN_STATE(Blackboard));
        States.push_back(AVOID_ENEMIES_QUEEN_STATE(Blackboard));
    }

    std::string ComputeCommands()
    {
        QUEEN_STATE *wanted_state = &(States.at(0));
        double score = wanted_state->Evaluate();

        for (int state_index = 1; state_index < States.size(); state_index++)
        {
            double new_score = States.at(state_index).Evaluate();

            if (new_score > score)
            {
                wanted_state = &(States.at(state_index));
                score = new_score;
            }
        }

        if (wanted_state != CurrentState)
        {
            if (CurrentState != nullptr)
            {
                CurrentState->OnExit();
            }

            CurrentState = wanted_state;

            CurrentState->OnEnter();
        }

        return CurrentState->ComputeCommands();
    }
};

//========================================================
//========================================================
// QUARTERMASTER
//========================================================
//========================================================

class QUARTERMASTER
{
  private:
    const int MinimalKnightCount = 1;
    const int DesiredKnightCount = 8;
    const int MinimalArcherCount = 1;
    const int DesiredArcherCount = 4;

  private:
    BLACKBOARD *Blackboard;

  public:
    QUARTERMASTER(BLACKBOARD *blackboard)
    {
        Blackboard = blackboard;
    }

    std::string ComputeCommands()
    {
        std::string commands = DEFAULT_TRAINING_COMMAND;

        if (Blackboard->GetTotalFriendlyUnitCount(KNIGHT_UNIT_TYPE) < MinimalKnightCount)
        {
            commands.append(TrainUnit(KNIGHT_UNIT_TYPE));
        }

        if (Blackboard->GetTotalFriendlyUnitCount(ARCHER_UNIT_TYPE) < MinimalArcherCount)
        {
            commands.append(TrainUnit(ARCHER_UNIT_TYPE));
        }

        if (Blackboard->GetTotalFriendlyUnitCount(KNIGHT_UNIT_TYPE) < DesiredKnightCount)
        {
            commands.append(TrainUnit(KNIGHT_UNIT_TYPE));
        }

        if (Blackboard->GetTotalFriendlyUnitCount(ARCHER_UNIT_TYPE) < DesiredArcherCount)
        {
            commands.append(TrainUnit(ARCHER_UNIT_TYPE));
        }

        return commands;
    }

  private:
    std::string TrainUnit(int unit_type)
    {
        int barracks_id = Blackboard->TrainUnit(unit_type);
        if (barracks_id == -1)
        {
            return "";
        }
        else
        {
            return " " + std::to_string(barracks_id);
        }
    }
};

//========================================================
//========================================================
// HIGH COMMAND
//========================================================
//========================================================

struct COMMANDS
{
    std::string QueenCommands;
    std::string QuartermasterCommands;
};

class HIGH_COMMAND
{
    // -- ATTRIBUTES
  private:
    BLACKBOARD Blackboard;
    QUEEN Queen = QUEEN(&Blackboard);
    QUARTERMASTER Quartermaster = QUARTERMASTER(&Blackboard);

    MODEL::GAMESTATE Gamestate;

    // -- CONSTRUCTORS
  public:
    HIGH_COMMAND()
    {
        int site_count;
        std::cin >> site_count;
        std::cin.ignore();

        Gamestate.SiteCount = site_count;
        Gamestate.ResetSiteVector(site_count);

        for (int site_index = 0; site_index < site_count; site_index++)
        {
            int id;
            int x;
            int y;
            int radius;

            std::cin >> id >> x >> y >> radius;
            std::cin.ignore();

            Gamestate.AddSite(id, x, y, radius);
        }
    }

    // -- OPERATIONS
  public:
    COMMANDS PlayTurn()
    {
        Blackboard.UpdateBlackboard(&Gamestate);

        COMMANDS commands;
        commands.QueenCommands = Queen.ComputeCommands();
        commands.QuartermasterCommands = Quartermaster.ComputeCommands();

        return commands;
    }

  private:
    void UpdateGamestate()
    {
        int gold;
        int touchedSite; // -1 if none
        std::cin >> gold >> touchedSite;
        std::cin.ignore();

        Gamestate.Gold = gold;
        Gamestate.TouchedSite = touchedSite;

        for (int i = 0; i < Gamestate.SiteCount; i++)
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

            Gamestate.UpdateSite(site_id, structure_type, owner, param1, param2);
        }

        int unit_count;
        std::cin >> unit_count;
        std::cin.ignore();

        Gamestate.ResetUnitVector(unit_count);

        for (int i = 0; i < unit_count; i++)
        {
            int x;
            int y;
            int owner;
            int unit_type; // -1 = QUEEN, 0 = KNIGHT, 1 = ARCHER
            int health;
            std::cin >> x >> y >> owner >> unit_type >> health;
            std::cin.ignore();

            Gamestate.AddUnit(x, y, owner, unit_type, health);
        }
    }
};
}

//################################################################################################
//################################################################################################

// -- MAIN

int main()
{
    AI::HIGH_COMMAND high_command;

    // game loop
    while (1)
    {
        AI::COMMANDS commands = high_command.PlayTurn();

        std::cout << commands.QueenCommands << std::endl;
        std::cout << commands.QuartermasterCommands << std::endl;
    }
}

//################################################################################################