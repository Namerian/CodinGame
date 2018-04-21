#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

//################################################

const int BARRACKS_STRUCTURE_TYPE = 2;

const int NO_OWNER = -1;
const int FRIENDLY_OWNER = 0;
const int ENEMY_OWNER = 1;

const int QUEEN_UNIT_TYPE = -1;
const int KNIGHT_UNIT_TYPE = 0;
const int ARCHER_UNIT_TYPE = 1;

//################################################

static double ComputeDistance(int ax, int ay, int bx, int by)
{
    return std::sqrt(std::pow(ax - bx, 2) + std::pow(ay - by, 2));
}

//################################################
//################################################

// MODEL

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

class MODEL
{
  public:
    int SiteCount;
    int Gold;
    int TouchedSite;

  private:
    std::vector<SITE> SiteVector;
    std::vector<UNIT> UnitVector;

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

    SITE *GetSite(int site_id)
    {
        for (int site_index = 0; site_index < SiteVector.size(); site_index++)
        {
            SITE &currentSite = SiteVector.at(site_index);
            if (currentSite.ID == site_id)
            {
                return &currentSite;
            }
        }
    }

    std::vector<SITE *> GetSitesOwnedBy(int owner)
    {
        std::vector<SITE *> result;

        for (int site_index = 0; site_index < SiteCount; site_index++)
        {
            SITE &currentSite = SiteVector.at(site_index);
            if (currentSite.Owner == owner)
            {
                result.push_back(&currentSite);
            }
        }

        return result;
    }

    void ResetUnitVector()
    {
        UnitVector.clear();
    }

    void SetUnit(int x, int y, int owner, int unit_type, int health)
    {
        UnitVector.emplace_back(UNIT(x, y, owner, unit_type, health));
    }

    UNIT *GetUnit(int owner, int unit_type)
    {
        for (UNIT &unit : UnitVector)
        {
            if (unit.Owner == owner && unit.UnitType == unit_type)
            {
                return &unit;
            }
        }
    }

    std::vector<UNIT *> GetUnits(int owner, int unit_type)
    {
        std::vector<UNIT *> result;

        for (UNIT &unit : UnitVector)
        {
            if (unit.Owner == owner && unit.UnitType == unit_type)
            {
                result.push_back(&unit);
            }
        }

        return result;
    }
};

//################################################
//################################################

// AI

class AI
{
  private:
    MODEL *Model;

  public:
    AI(MODEL *model)
    {
        Model = model;
    }

    std::string ComputeQueenActions(MODEL &model)
    {
        std::string result = "";
        UNIT *queen = model.GetUnit(FRIENDLY_OWNER, QUEEN_UNIT_TYPE);
        std::vector<SITE *> neutral_sites = model.GetSitesOwnedBy(NO_OWNER);

        if (queen == nullptr)
        {
            std::cerr << "My Queen is null!" << std::endl;
            return "WAIT";
        }

        if (model.TouchedSite != -1)
        {
            SITE *touched_site = model.GetSite(model.TouchedSite);
            if (touched_site->Owner == NO_OWNER)
            {
                result.append("BUILD ");
                result.append(std::to_string(touched_site->ID));
                result.append(" BARRACKS-");
                result.append("KNIGHT");
                return result;
            }
        }

        if (neutral_sites.size() > 0)
        {
            SITE *nearestSite = neutral_sites.at(0);
            double distance = ComputeDistance(queen->X, queen->Y, nearestSite->X, nearestSite->Y);

            for (int site_index = 1; site_index < neutral_sites.size(); site_index++)
            {
                SITE *currentSite = neutral_sites.at(site_index);
                double newDistance = ComputeDistance(queen->X, queen->Y, currentSite->X, currentSite->Y);

                if (newDistance < distance)
                {
                    nearestSite = currentSite;
                    distance = newDistance;
                }
            }

            result.append("MOVE ");
            result.append(std::to_string(nearestSite->X));
            result.append(" ");
            result.append(std::to_string(nearestSite->Y));
            return result;
        }
        else
        {
            std::cerr << "No neutral sites left!" << std::endl;
        }

        return "WAIT";
    }

    std::string ComputeTrainingActions(MODEL &model)
    {
        std::vector<SITE *> owned_sites = model.GetSitesOwnedBy(FRIENDLY_OWNER);
        std::vector<SITE *> knight_barracks;
        std::vector<SITE *> archer_barracks;
        std::string result = "TRAIN";

        for (SITE *site : owned_sites)
        {
            if (site->StructureType == BARRACKS_STRUCTURE_TYPE && site->Param1 == 0)
            {
                if (site->Param2 == KNIGHT_UNIT_TYPE)
                {
                    knight_barracks.push_back(site);
                }
                else if (site->Param2 == ARCHER_UNIT_TYPE)
                {
                    archer_barracks.push_back(site);
                }
            }
        }

        if (knight_barracks.size() > 0)
        {
            for (SITE *site : knight_barracks)
            {
                result.append(" ");
                result.append(std::to_string(site->ID));
            }
        }

        return result;
    }
};

//################################################
//################################################

// MAIN

void InitMain(MODEL &model)
{
    int site_count;
    std::cin >> site_count;
    std::cin.ignore();
    model.Initialize(site_count);

    for (int site_index = 0; site_index < site_count; site_index++)
    {
        int id;
        int x;
        int y;
        int radius;

        std::cin >> id >> x >> y >> radius;
        std::cin.ignore();

        SITE *site = model.GetSite(id);
        site->X = x;
        site->Y = y;
        site->Radius = radius;
    }
}

void UpdateModel(MODEL &model)
{
    int gold;
    int touchedSite; // -1 if none
    std::cin >> gold >> touchedSite;
    std::cin.ignore();
    model.Gold = gold;
    model.TouchedSite = touchedSite;

    for (int i = 0; i < model.SiteCount; i++)
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

        SITE *site = model.GetSite(site_id);
        site->StructureType = structure_type;
        site->Owner = owner;
        site->Param1 = param1;
        site->Param2 = param2;
    }

    int numUnits;
    std::cin >> numUnits;
    std::cin.ignore();
    model.ResetUnitVector();

    for (int i = 0; i < numUnits; i++)
    {
        int x;
        int y;
        int owner;
        int unit_type; // -1 = QUEEN, 0 = KNIGHT, 1 = ARCHER
        int health;
        std::cin >> x >> y >> owner >> unit_type >> health;
        std::cin.ignore();

        model.SetUnit(x, y, owner, unit_type, health);
    }
}

int main()
{
    MODEL model = MODEL();
    AI AI_ = AI(&model);

    InitMain(model);

    // game loop
    while (1)
    {
        UpdateModel(model);

        std::string queen_actions = AI_.ComputeQueenActions(model);
        std::cout << queen_actions << std::endl;

        std::string training_actions = AI_.ComputeTrainingActions(model);
        std::cout << training_actions << std::endl;
    }
}

//################################################