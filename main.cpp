#include "main.h"

int main() {
  printIntro();
  Config::readConfig();
  auto laser = Laser();
  auto *meshSectors = new MeshSector[Config::Processes::count]();
  auto mesh = Mesh(meshSectors, &laser);
  auto timeFlow = TimeFlow(laser);
  auto bodyData = BodyData(&mesh, meshSectors);
  auto dataWriter = DataWriter(timeFlow, bodyData);
  auto *dataContainers = new DataContainer[Config::Processes::count]();
  auto *processors = new Processor[Config::Processes::count]();

  for (size_t i = 0; i < Config::Processes::count; i++) {
    auto *timeFlowPtr = &timeFlow;
    if (i != (size_t)Config::Processes::count - 1)
      timeFlowPtr = nullptr;
    dataContainers[i].init(i, &meshSectors[i], meshSectors, &laser,
                           timeFlowPtr);
    processors[i].putData(&dataContainers[i]);
  }

  while (!timeFlow.stopSimulation) {
    for (Processor::step = 0; Processor::step < 2; Processor::step++) {
      while (1) {
        bool finished = true;
        for (size_t i = 0; i < Config::Processes::count; i++) {
          if (!processors[i].isReady()) {
            finished = false;
            break;
          }
        }
        if (finished)
          break;
      }
    }
    if (timeFlow.logThisStep) {
      timeFlow.removeFlag();
      bodyData.advance(meshSectors);
      dataWriter.advance(timeFlow, bodyData);
    }
    if (laser.needForNewLayer) {
      for (size_t i = 0; i < Config::Processes::count; i++) {
        dataContainers[i].addNewLayerOfPowder();
      }
      laser.needForNewLayer = false;
    }
  }

  if (processors) {
    delete[] processors;
  }
  printOutro();
  return 0;
}
