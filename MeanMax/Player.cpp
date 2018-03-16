#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

//=============================================================

struct Vehicle
{
    int unitId;
    int unitType;
    int player;
    float mass;
    int radius;
    int x;
    int y;
    int vx;
    int vy;
    int extra;
    int extra2;

    bool updated;
};

struct MyVehicle : Vehicle
{
    std::string output = "WAIT";
    int targetUnitId = -1;
};

struct Model
{
    MyVehicle myReaper;
    MyVehicle myDestroyer;
    MyVehicle myDoof;

    std::vector<Vehicle> wrecks;
    std::vector<Vehicle> tankers;
};

//=============================================================
//=============================================================

double GetFriction(int unitType)
{
    if (unitType == 0)
    {
        return 0.2;
    }

    return 0;
}

double ComputeDistance(int ax, int ay, int bx, int by)
{
    double distX = bx - ax;
    double distY = by - ay;

    distX *= distX;
    distY *= distY;

    return std::sqrt(distX + distY);
}

Vehicle *GetVehicle(std::vector<Vehicle> &vehicles, int vehicleId)
{
    for (unsigned int i = 0; i < vehicles.size(); i++)
    {
        if (vehicles.at(i).unitId == vehicleId)
        {
            return &vehicles.at(i);
        }
    }

    return nullptr;
}

Vehicle *FindNearest(std::vector<Vehicle> &targets, Vehicle &source)
{
    if (targets.size() == 0)
    {
        //std::cerr << "FindNearest(): target.size == 0" << std::endl;
        return nullptr;
    }
    else if (targets.size() == 1)
    {
        //std::cerr << "FindNearest(): target.size == 1" << std::endl;
        return &targets.at(0);
    }

    int resultIndex = 0;
    double smallestDistance = ComputeDistance(source.x, source.y, targets.at(0).x, targets.at(0).y);

    for (unsigned int i = 1; i < targets.size(); i++)
    {
        double newDistance = ComputeDistance(source.x, source.y, targets.at(i).x, targets.at(i).y);
        if (newDistance < smallestDistance)
        {
            resultIndex = i;
            smallestDistance = newDistance;
        }
    }

    return &targets.at(resultIndex);
}

// void ComputeOutput(MyVehicle &vehicle, int targetX, int targetY)
// {
//     //double distance = ComputeDistance(vehicle.x, vehicle.y, targetX, targetY);
//     //double friction = GetFriction(vehicle.unitType);

//     //int directionX = targetX - vehicle.x;
//     //int directionY = targetY - vehicle.y;

//     //double length = std::sqrt(directionX * directionX + directionY * directionY);
//     //directionX = std::round(directionX * (1.0 / length));
//     //directionY = std::round(directionY * (1.0 / length));

//     vehicle.output = "" + std::to_string(targetX) + " " + std::to_string(targetY) + " 300";
// }

//=============================================================
//=============================================================

void HandleReaper(Model &model)
{
    model.myReaper.output = "0 0 150";

    //checking if the target still exists
    if (model.myReaper.targetUnitId != -1)
    {
        Vehicle *target = GetVehicle(model.wrecks, model.myReaper.targetUnitId);

        if (target == nullptr)
        {
            std::cerr << "HandleReaper(): target does not exist anymore: id=" << model.myReaper.targetUnitId << std::endl;
            model.myReaper.targetUnitId = -1;
        }
    }

    if (model.myReaper.targetUnitId == -1)
    {
        Vehicle *target = FindNearest(model.wrecks, model.myReaper);

        if (target != nullptr)
        {
            model.myReaper.targetUnitId = target->unitId;
            std::cerr << "HandleReaper(): target set: id=" << target->unitId << std::endl;
        }
        else
        {
            std::cerr << "HandleReaper(): FindNearest() returned null" << std::endl;
        }
    }

    if (model.myReaper.targetUnitId != -1)
    {
        Vehicle *target = GetVehicle(model.wrecks, model.myReaper.targetUnitId);

        if (target == nullptr)
        {
            std::cerr << "HandleReaper(): target is null: id=" << model.myReaper.targetUnitId << std::endl;
        }

        model.myReaper.output = "" + std::to_string(target->x) + " " + std::to_string(target->y) + " 300";
    }
}

void HandleDestroyer(Model &model)
{
    model.myDestroyer.output = "0 0 150";


}

void HandleDoof(Model &model)
{
    model.myDoof.output = "WAIT";
}

//=============================================================
//=============================================================

int main()
{
    Model model;

    // game loop
    while (1)
    {
        //****************
        //pre-input initialization
        //****************

        //marking  all wrecks as not updated
        for (Vehicle &wreck : model.wrecks)
        {
            wreck.updated = false;
        }

        //****************
        //reading input
        //****************

        int myScore;
        std::cin >> myScore;
        std::cin.ignore();
        int enemyScore1;
        std::cin >> enemyScore1;
        std::cin.ignore();
        int enemyScore2;
        std::cin >> enemyScore2;
        std::cin.ignore();
        int myRage;
        std::cin >> myRage;
        std::cin.ignore();
        int enemyRage1;
        std::cin >> enemyRage1;
        std::cin.ignore();
        int enemyRage2;
        std::cin >> enemyRage2;
        std::cin.ignore();
        int unitCount;
        std::cin >> unitCount;
        std::cin.ignore();
        for (int i = 0; i < unitCount; i++)
        {
            int unitId;
            int unitType;
            int player;
            float mass;
            int radius;
            int x;
            int y;
            int vx;
            int vy;
            int extra;
            int extra2;
            std::cin >> unitId >> unitType >> player >> mass >> radius >> x >> y >> vx >> vy >> extra >> extra2;
            std::cin.ignore();

            Vehicle *currentVehicle = nullptr;

            if (player == 0) //reading info about my vehicles
            {
                if (unitType == 0)
                {
                    currentVehicle = &model.myReaper;
                }
                else if (unitType == 1)
                {
                    currentVehicle = &model.myDestroyer;
                }
                else if (unitType == 2)
                {
                    currentVehicle = &model.myDoof;
                }
            }
            else if (unitType == 3) //this is a tanker
            {
                currentVehicle = GetVehicle(model.tankers, unitId);

                if (currentVehicle == nullptr)
                {
                    model.tankers.push_back(Vehicle());
                    currentVehicle = &model.tankers.at(model.tankers.size() - 1);
                }
            }
            else if (unitType == 4) //this is a wreck
            {
                currentVehicle = GetVehicle(model.wrecks, unitId);

                if (currentVehicle == nullptr)
                {
                    model.wrecks.push_back(Vehicle());
                    currentVehicle = &model.wrecks.at(model.wrecks.size() - 1);
                }
            }

            if (currentVehicle != nullptr)
            {
                currentVehicle->unitId = unitId;
                currentVehicle->unitType = unitType;
                currentVehicle->player = player;
                currentVehicle->mass = mass;
                currentVehicle->radius = radius;
                currentVehicle->x = x;
                currentVehicle->y = y;
                currentVehicle->vx = vx;
                currentVehicle->vy = vy;
                currentVehicle->extra = extra;
                currentVehicle->extra2 = extra2;

                currentVehicle->updated = true;
            }
        }

        //****************
        //post-input initialization
        //****************

        //removing all not updated wrecks
        int wreckCountA = model.wrecks.size();
        model.wrecks.erase(std::remove_if(model.wrecks.begin(), model.wrecks.end(), [&model](const Vehicle &x) { return !x.updated; }), model.wrecks.end());
        int wreckCountB = model.wrecks.size();

        if (wreckCountA != wreckCountB)
        {
            //std::cerr << "wreck count changed from " << wreckCountA << " to " << wreckCountB << std::endl;
        }

        //****************
        //handling my vehicles
        //****************

        HandleDoof(model);
        HandleDestroyer(model);
        HandleReaper(model);

        //****************
        //output
        //****************

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        //Reaper output
        std::cout << model.myReaper.output << std::endl;

        //xx output
        std::cout << model.myDestroyer.output << std::endl;

        //xx output
        std::cout << model.myDoof.output << std::endl;
    }
}

//=============================================================