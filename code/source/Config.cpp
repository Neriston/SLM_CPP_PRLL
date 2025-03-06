#include <fstream>
#include <iostream>

#include "../../lib/include/json.hpp"

#include "../include/Config.h"

void Config::readConfig() {
  try {
    using json = nlohmann::json;
    auto currentPath = std::filesystem::current_path();
    std::string exePath = currentPath.generic_string();
    std::string localPath = exePath + "/config.json";
    std::string externalPath = "../config.json";
    std::ifstream localFile(localPath);
    std::ifstream externalFile(externalPath);
    configPath = "";
    if (localFile) {
      configPath = localPath;
      printf("Local config file found\n");
    } else if (externalFile) {
      configPath = externalPath;
      printf("No local config, but external config file found\n");
    } else {
      printf("No config file\n");
      exit(3);
    }
    printf("~~~~~~\n");
    std::ifstream configFile(configPath);
    json processedFile{json::parse(configFile)};
    //

    processedFile = processedFile[0];
    Processes::count = uint32_t{processedFile["processes"]["in parallel"]};
    if (Processes::count == 0) {
      printf("0 processes, computation is not possible\n");
      exit(1);
    }

    Directory::project = std::string{processedFile["path to project"]};

    // Geometry::size = Vec3(
    //	double{ processedFile["geometry"]["size"]["x"] },
    //	double{ processedFile["geometry"]["size"]["y"] },
    //	double{ processedFile["geometry"]["size"]["z"] }
    //);
    Geometry::resolution = Vec3I(int{processedFile["geometry"]["elems"]["x"]},
                                 int{processedFile["geometry"]["elems"]["y"]},
                                 int{processedFile["geometry"]["elems"]["z"]});
    Geometry::step = Vec3(double{processedFile["geometry"]["step"]["x"]},
                          double{processedFile["geometry"]["step"]["y"]},
                          double{processedFile["geometry"]["step"]["z"]});
    Geometry::stepRev =
        Vec3(1 / Geometry::step.x, 1 / Geometry::step.y, 1 / Geometry::step.z);
    Geometry::coarsen.push_back(processedFile["geometry"]["coarsen after"]["x"]
                                    .get<std::vector<int32_t>>());
    Geometry::coarsen.push_back(processedFile["geometry"]["coarsen after"]["y"]
                                    .get<std::vector<int32_t>>());
    Geometry::coarsen.push_back(processedFile["geometry"]["coarsen after"]["z"]
                                    .get<std::vector<int32_t>>());
    Geometry::stepCoeff = 0.5 * Geometry::stepRev.x * Geometry::stepRev.x;
    Geometry::powderThickness =
        double{processedFile["geometry"]["powder thickness"]};
    Geometry::surfaceArea = Geometry::step.x * Geometry::step.y;
    Geometry::buffer =
        Vec3I(int32_t{processedFile["geometry"]["buffer zone, elems"]},
              int32_t{processedFile["geometry"]["buffer zone, elems"]}, 0);
    Geometry::adaptiveLandscape =
        bool{processedFile["geometry"]["adaptive landscape"]};

    Time::start = double{processedFile["time"]["start"]};
    Time::step = double{processedFile["time"]["step"]};
    Time::end = Time::step * double{processedFile["time"]["iterations"]};

    Log::desiredEntries = uint32_t{processedFile["logging"]["log entries"]};

    Temperature::air = double{processedFile["temperatures"]["air"]};
    Temperature::air4 = Temperature::air * Temperature::air * Temperature::air *
                        Temperature::air;
    Temperature::initial = double{processedFile["temperatures"]["initial"]};
    Temperature::melting = double{processedFile["temperatures"]["melting"]};
    Temperature::vaporization =
        double{processedFile["temperatures"]["vaporization"]};
    Temperature::cutOff = double{processedFile["temperatures"]["cut off"]};

    Mass::Rho::solid = double{processedFile["mass"]["rho"]["solid"]};
    Mass::Rho::packing = double{processedFile["mass"]["rho"]["packing"]};
    Mass::Rho::packingRev = 1 / Mass::Rho::packing;
    Mass::Rho::liquid = double{processedFile["mass"]["rho"]["liquid"]};
    Mass::Rho::vapor = double{processedFile["mass"]["rho"]["vapor"]};
    Mass::solid = Geometry::step.x * Geometry::step.y * Geometry::step.z *
                  Mass::Rho::solid;
    Mass::liquid = Geometry::step.x * Geometry::step.y * Geometry::step.z *
                   Mass::Rho::liquid;
    Mass::vapor = Geometry::step.x * Geometry::step.y * Geometry::step.z *
                  Mass::Rho::vapor;
    Mass::powder = Mass::solid * Mass::Rho::packing;

    Misc::sigmoidConst = 1 / (1 + exp(-12.0 * Mass::Rho::packing + 6.0));
    Misc::sigmoidConstRev = 1 / Misc::sigmoidConst;

    Energy::Solid::C = double{processedFile["energy"]["solid"]["C"]};
    Energy::Solid::KA = double{processedFile["energy"]["solid"]["KA"]};
    Energy::Solid::KB = double{processedFile["energy"]["solid"]["KB"]};
    Energy::Solid::mc = Mass::solid * Energy::Solid::C;
    Energy::Solid::mcRev = 1 / Energy::Solid::mc;
    Energy::Liquid::C = double{processedFile["energy"]["liquid"]["C"]};
    Energy::Liquid::KA = double{processedFile["energy"]["liquid"]["KA"]};
    Energy::Liquid::KB = double{processedFile["energy"]["liquid"]["KB"]};
    Energy::Liquid::mc = Mass::liquid * Energy::Liquid::C;
    Energy::Liquid::mcRev = 1 / Energy::Liquid::mc;
    Energy::Vapor::C = double{processedFile["energy"]["vapor"]["C"]};
    Energy::Vapor::KA = double{processedFile["energy"]["vapor"]["KA"]};
    Energy::Vapor::KB = double{processedFile["energy"]["vapor"]["KB"]};
    Energy::Vapor::anisotropyOfK =
        double{processedFile["energy"]["vapor"]["anisotropy of K"]};
    Energy::Vapor::mc = Mass::liquid * Energy::Vapor::C;
    Energy::Vapor::mcRev = 1 / Energy::Vapor::mc;
    Energy::Powder::C = Energy::Solid::C;
    Energy::Powder::mc = Mass::powder * Energy::Powder::C;
    Energy::Powder::mcRev = 1 / Energy::Powder::mc;
    Energy::Enthalpy::fusion =
        double{processedFile["energy"]["enthalpy"]["fusion"]};
    Energy::Enthalpy::vaporization =
        double{processedFile["energy"]["enthalpy"]["vaporization"]};
    Energy::Enthalpy::minusRegular =
        Mass::solid * Energy::Solid::C * Temperature::melting;
    Energy::Enthalpy::plusRegular =
        Energy::Enthalpy::minusRegular + Mass::solid * Energy::Enthalpy::fusion;
    Energy::Enthalpy::minusPowder =
        Mass::powder * Energy::Powder::C * Temperature::melting;
    Energy::Enthalpy::plusPowder =
        Energy::Enthalpy::minusPowder + Mass::powder * Energy::Enthalpy::fusion;
    Energy::Enthalpy::minusVapor =
        Energy::Enthalpy::plusRegular +
        Mass::liquid * Config::Energy::Liquid::C *
            (Config::Temperature::vaporization - Config::Temperature::melting);
    Energy::Enthalpy::plusVapor =
        Energy::Enthalpy::minusVapor +
        Mass::liquid * Config::Energy::Enthalpy::vaporization;

    Radiation::stefanBoltzmannConst = 5.67e-8;
    Radiation::emmisivity = double{processedFile["radiation"]["emmisivity"]};
    Radiation::fluxConst =
        Radiation::stefanBoltzmannConst * Radiation::emmisivity;

    Laser::vec = Vec3(double{processedFile["laser"]["loc"]["x"]},
                      double{processedFile["laser"]["loc"]["y"]},
                      double{processedFile["laser"]["loc"]["z"]});
    Laser::vel = Vec3(double{processedFile["laser"]["vel"]["x"]},
                      double{processedFile["laser"]["vel"]["y"]},
                      double{processedFile["laser"]["vel"]["z"]});
    Laser::radius = double{processedFile["laser"]["radius"]};
    Laser::absorbtion = double{processedFile["laser"]["absorbtion"]};
    Laser::power = Laser::absorbtion * double{processedFile["laser"]["power"]};
    Laser::goUntill = double{processedFile["laser"]["go untill"]["x"]};
    Laser::sideStep =
        Laser::radius * double{processedFile["laser"]["side step multiplier"]};
    Laser::state = bool{processedFile["laser"]["state"]};
    Laser::tracks = uint32_t{processedFile["laser"]["tracks"]};
    Laser::layers = uint32_t{processedFile["laser"]["layers"]};
  } catch (...) {
    std::cout << "Error while parsing config file" << std::endl;
    exit(5);
  }
}

uint32_t Config::Processes::count = 0;
std::string Config::configPath = "null";
std::string Config::Directory::project = "null";
Vec3I Config::Geometry::resolution = Vec3I();
Vec3 Config::Geometry::step = Vec3();
Vec3 Config::Geometry::stepRev = Vec3();
std::vector<std::vector<int32_t>> Config::Geometry::coarsen{};
double Config::Geometry::stepCoeff = 0.0;
double Config::Geometry::powderThickness = 0.0;
double Config::Geometry::surfaceArea = 0.0;
Vec3I Config::Geometry::buffer = Vec3I();
bool Config::Geometry::adaptiveLandscape = false;
double Config::Time::start = 0.0;
double Config::Time::step = 0.0;
double Config::Time::end = 0.0;
uint32_t Config::Log::desiredEntries = 0;
double Config::Temperature::air = 0.0;
double Config::Temperature::air4 = 0.0;
double Config::Temperature::initial = 0.0;
double Config::Temperature::melting = 0.0;
double Config::Temperature::cutOff = 0.0;
double Config::Temperature::vaporization = 0.0;
double Config::Mass::Rho::solid = 0.0;
double Config::Mass::Rho::packing = 0.0;
double Config::Mass::Rho::packingRev = 0.0;
double Config::Mass::Rho::liquid = 0.0;
double Config::Mass::Rho::vapor = 0.0;
double Config::Mass::solid = 0.0;
double Config::Mass::liquid = 0.0;
double Config::Mass::powder = 0.0;
double Config::Mass::vapor = 0.0;
double Config::Energy::Solid::C = 0.0;
double Config::Energy::Solid::KA = 0.0;
double Config::Energy::Solid::KB = 0.0;
double Config::Energy::Solid::mc = 0.0;
double Config::Energy::Solid::mcRev = 0.0;
double Config::Energy::Liquid::C = 0.0;
double Config::Energy::Liquid::KA = 0.0;
double Config::Energy::Liquid::KB = 0.0;
double Config::Energy::Liquid::mc = 0.0;
double Config::Energy::Liquid::mcRev = 0.0;
double Config::Energy::Vapor::C = 0.0;
double Config::Energy::Vapor::KA = 0.0;
double Config::Energy::Vapor::KB = 0.0;
double Config::Energy::Vapor::mc = 0.0;
double Config::Energy::Vapor::mcRev = 0.0;
double Config::Energy::Vapor::anisotropyOfK = 0.0;
double Config::Energy::Powder::C = 0.0;
double Config::Energy::Powder::mc = 0.0;
double Config::Energy::Powder::mcRev = 0.0;
double Config::Energy::Enthalpy::fusion = 0.0;
double Config::Energy::Enthalpy::vaporization = 0.0;
double Config::Energy::Enthalpy::minusRegular = 0.0;
double Config::Energy::Enthalpy::plusRegular = 0.0;
double Config::Energy::Enthalpy::minusPowder = 0.0;
double Config::Energy::Enthalpy::plusPowder = 0.0;
double Config::Energy::Enthalpy::minusVapor = 0.0;
double Config::Energy::Enthalpy::plusVapor = 0.0;
double Config::Radiation::stefanBoltzmannConst = 0.0;
double Config::Radiation::emmisivity = 0.0;
double Config::Radiation::fluxConst = 0.0;
Vec3 Config::Laser::vec = Vec3();
Vec3 Config::Laser::vel = Vec3();
double Config::Laser::radius = 0.0;
double Config::Laser::power = 0.0;
double Config::Laser::absorbtion = 0.0;
double Config::Laser::goUntill = 0.0;
double Config::Laser::sideStep = 0.0;
bool Config::Laser::state = false;
uint32_t Config::Laser::tracks = 0;
uint32_t Config::Laser::layers = 0;
double Config::Misc::sigmoidConst = 0.0;
double Config::Misc::sigmoidConstRev = 0.0;