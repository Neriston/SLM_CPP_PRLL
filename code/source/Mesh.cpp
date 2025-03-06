#include "../include/Mesh.h"
#include "../../lib/include/Timer.h"
#include "../include/Config.h"
#include "../include/Laser.h"
#include "../include/MeshSector.h"
#include "../include/functions.h"
#include <cstring>
#include <iostream>

Mesh::Mesh() {
  resolution = returnResolution();
  powderLayers = (uint32_t)round(Config::Geometry::powderThickness /
                                 Config::Geometry::step.z);
  startPowderAtLayer = (uint32_t)resolution.z - powderLayers;
  nodesCount = (1 + resolution.x) * (1 + resolution.y) * (1 + resolution.z);
  elemsCount = resolution.x * resolution.y * resolution.z;
  nodes = new Node[nodesCount](); // allocates memory for array of nodes and
                                  // calls the default constructor
  elems = new Elem[elemsCount];   // allocates memory and calls nothing
  memset(elems, 0,
         sizeof(Elem) *
             elemsCount); // filling the allocated memory with zeroes by hands
  currentNodeID = 0;
  currentElemID = 0;
  createMesh();
}

Mesh::Mesh(MeshSector *meshSectors, Laser *laser) {
  resolution = returnResolution();
  powderLayers = (uint32_t)round(Config::Geometry::powderThickness /
                                 Config::Geometry::step.z);
  startPowderAtLayer = (uint32_t)resolution.z - powderLayers;
  nodesCount = (1 + resolution.x) * (1 + resolution.y) * (1 + resolution.z);
  elemsCount = resolution.x * resolution.y * resolution.z;
  nodes = new Node[nodesCount](); // allocates memory for array of nodes and
                                  // calls the default constructor
  elems = new Elem[elemsCount];   // allocates memory and calls nothing
  memset(elems, 0,
         sizeof(Elem) *
             elemsCount); // filling the allocated memory with zeroes by hands
  currentNodeID = 0;
  currentElemID = 0;
  createMesh();
  sectorPreprocessor(meshSectors, laser);
  laser->scaleVectorToGeometrySize(nodes[nodesCount - 1].vec);
}

Mesh::~Mesh() {
  if (nodes)
    delete[] nodes;
  if (elems)
    delete[] elems;
}

void Mesh::createElement(uint32_t elemID, const Vec3I &INDEX_VEC,
                         const Neighbours &NEIGHBOURS,
                         const Neighbours &NEIGHBOURS_TRUNCATED,
                         const uint32_t STATE) {
  uint32_t nodeID;
  auto *newElem = &elems[elemID];
  if (newElem->init(elems, elemID, INDEX_VEC, NEIGHBOURS, NEIGHBOURS_TRUNCATED,
                    STATE) != true) {
    std::exit(2);
  } else {
    for (uint32_t nodePos = 0; nodePos < 8; nodePos++) {
      nodeID = findNodeForElement(nodePos, newElem->vec, newElem->nodeScaleVec,
                                  NEIGHBOURS);
      newElem->vertices[nodePos] = nodeID;
    }
    Vec3 size =
        Vec3(nodes[newElem->vertices[6]].vec - nodes[newElem->vertices[0]].vec);
    newElem->volume = size.x * size.y * size.z;
    if (newElem->neighbours.zPlus == -1)
      newElem->mayBeUnderLaser = true;
  }
}

void Mesh::createNode(uint32_t nodeID, uint32_t nodePos, const Vec3 &ANCHOR_VEC,
                      const Vec3 &NODE_SCALE_VEC) {
  Vec3 vec = Node::nodalVec(nodePos, ANCHOR_VEC, NODE_SCALE_VEC);
  nodes[nodeID] = Node(nodeID, vec);
}

uint32_t Mesh::findNodeForElement(uint32_t nodePos, const Vec3 &ELEM_VEC,
                                  const Vec3 &NODE_SCALE_VEC,
                                  const Neighbours &NEIGHBOURS) {
  const int32_t RELATIONS[8][3] = {{3, 1, 4},    {2, -1, 5}, {-1, -1, 6},
                                   {-1, 2, 7},   {7, 5, -1}, {6, -1, -1},
                                   {-1, -1, -1}, {-1, 6, -1}};
  uint32_t nodeID;
  if (NEIGHBOURS.xMinus != -1 and RELATIONS[nodePos][0] != -1) {
    nodeID = elems[NEIGHBOURS.xMinus].vertices[RELATIONS[nodePos][0]];
  } else if (NEIGHBOURS.yMinus != -1 and RELATIONS[nodePos][1] != -1) {
    nodeID = elems[NEIGHBOURS.yMinus].vertices[RELATIONS[nodePos][1]];
  } else if (NEIGHBOURS.zMinus != -1 and RELATIONS[nodePos][2] != -1) {
    nodeID = elems[NEIGHBOURS.zMinus].vertices[RELATIONS[nodePos][2]];
  } else {
    nodeID = currentNodeID++;
    createNode(nodeID, nodePos, ELEM_VEC, NODE_SCALE_VEC);
  }
  return nodeID;
}

void Mesh::createMesh() {
  Timer timer = Timer();
  timer.start();
  uint32_t elemID = 0;
  Vec3I indexVector = Vec3I();
  Neighbours neighbours = Neighbours();
  Neighbours neighboursTruncated = Neighbours();
  uint32_t state = 2;
  for (indexVector.z = 0; indexVector.z < resolution.z; indexVector.z++) {
    if (indexVector.z >= (int32_t)startPowderAtLayer) {
      state = 0;
    }
    for (indexVector.y = 0; indexVector.y < resolution.y; indexVector.y++) {
      for (indexVector.x = 0; indexVector.x < resolution.x; indexVector.x++) {
        neighbours = Neighbours(indexVector, elemID, resolution);
        neighboursTruncated = neighbours;
        neighboursTruncated.truncate();
        createElement(elemID, indexVector, neighbours, neighboursTruncated,
                      state);
        elemID += 1;
      }
    }
  }
  std::cout << "mesh created, " << timer.formatElapsed()
            << " elems = " << elemsCount << " nodes = " << nodesCount
            << std::endl;
  std::cout << "mesh resolution " << resolution.x << "x" << resolution.y << "x"
            << resolution.z << " elems" << std::endl;
  std::cout << "geometry dimentions " << nodes[nodesCount - 1].vec.x * 1000.0
            << "mm x-axis, " << nodes[nodesCount - 1].vec.y * 1000.0
            << "mm y-axis, " << nodes[nodesCount - 1].vec.z * 1000.0
            << "mm z-axis" << std::endl;
  std::cout << "~~~~~~" << std::endl;
}

void Mesh::sectorPreprocessor(MeshSector *meshSectors, Laser *laser) {
  Timer timer = Timer();
  timer.start();
  sectorGeometryCalculator(meshSectors);
  sectorLaserAssign(meshSectors, laser);
  sectorLookUpTable(meshSectors);
  splitMesh(meshSectors);
  std::cout << "sectors created, " << timer.formatElapsed() << std::endl;
  std::cout << "elements relative overhead = " << overhed(meshSectors)
            << ", overhead/processors = "
            << overhed(meshSectors) / MeshSector::count << std::endl;
  std::cout << "~~~~~~" << std::endl;
}

void Mesh::sectorGeometryCalculator(MeshSector *meshSectors) {
  uint32_t processCount = Config::Processes::count;
  uint32_t xDiv;
  uint32_t yDiv;
  if (!findOptimalRatio(&processCount, &resolution.x, &resolution.y, &xDiv,
                        &yDiv)) {
    printf("findOptimalRatio failed\n");
    exit(-1);
  }
  printf("process count = %u\n", processCount);
  printf("divide axis in sectors: x -> %u, y -> %u\n", xDiv, yDiv);
  auto minStepInSubmesh =
      Vec3I(resolution.x / xDiv, resolution.y / yDiv, resolution.z);
  if (minStepInSubmesh.x * minStepInSubmesh.y == 0) {
    printf("min step in submesh = 0\n");
    exit(-2);
  }
  auto residualElems = Vec3I(resolution.x % xDiv, resolution.y % yDiv, 0);
  auto localPosCorrection = Vec3I();
  auto localSizeCorrection = Vec3I();
  size_t ID;
  for (size_t y = 0; y < yDiv; y++) {
    localSizeCorrection = Vec3I(1, 1, 0);
    localPosCorrection.x = 0;
    if (y >= residualElems.y)
      localSizeCorrection.y = 0;
    for (size_t x = 0; x < xDiv; x++) {
      if (x >= residualElems.x)
        localSizeCorrection.x = 0;
      ID = y * xDiv + x;
      Vec3I anchor = Vec3I((uint32_t)x * minStepInSubmesh.x,
                           (uint32_t)y * minStepInSubmesh.y, 0) +
                     localPosCorrection;
      Vec3I resolution = minStepInSubmesh + localSizeCorrection;
      meshSectors[ID].init(anchor, resolution);
      if (localSizeCorrection.x > 0)
        localPosCorrection.x++;
    }
    if (localSizeCorrection.y > 0)
      localPosCorrection.y++;
  }
}

void Mesh::sectorLaserAssign(MeshSector *meshSectors, Laser *laser) {
  for (size_t i = 0; i < meshSectors->count; i++) {
    meshSectors[i].laserPtr = laser;
  }
}

void Mesh::sectorLookUpTable(MeshSector *meshSectors) {
  for (size_t i = 0; i < meshSectors->count; i++) {
    meshSectors[i].lookUpTable = new int32_t[elemsCount]();
    memset(meshSectors[i].lookUpTable, 0xff,
           sizeof(meshSectors[i].lookUpTable[0]) * elemsCount);
  }
}

void Mesh::sectorPointerToItself(MeshSector *meshSectors) {
  for (size_t i = 0; i < meshSectors->count; i++) {
    meshSectors[i].meshSectorsPtr = meshSectors;
  }
}

void Mesh::splitMesh(MeshSector *meshSectors) {
  int32_t persistentElemID = -1;
  int32_t persistentSectorID = -1;
  int32_t *elemInBufferID = new int32_t[MeshSector::count];
  int32_t *internalElemID = new int32_t[MeshSector::count];
  uint32_t i = 0;
  for (size_t elemID = 0; elemID < elemsCount; elemID++) {
    i = 0;
    for (size_t sectorID = 0; sectorID < MeshSector::count; sectorID++) {
      uint32_t sectorOrBuffer =
          isElemInsideSector(&elems[elemID], &meshSectors[sectorID]);
      if (sectorOrBuffer == 1) {
        meshSectors[sectorID].getThisElem(&elems[elemID], sectorOrBuffer);
        persistentSectorID = (int32_t)sectorID;
        persistentElemID = meshSectors[sectorID].vacantElemID - 1;
      }
      if (sectorOrBuffer == 2) {
        meshSectors[sectorID].getThisElem(&elems[elemID], sectorOrBuffer);
        elemInBufferID[i] = (int32_t)sectorID;
        internalElemID[i] = meshSectors[sectorID].vacantElemID - 1;
        i++;
      }
    }
    for (uint32_t j = 0; j < i; j++) {
      meshSectors[elemInBufferID[j]]
          .elems[internalElemID[j]]
          .persistentSectorID = persistentSectorID;
      meshSectors[elemInBufferID[j]].elems[internalElemID[j]].persistentElemID =
          persistentElemID;
    }
  }
  for (size_t sectorID = 0; sectorID < MeshSector::count; sectorID++) {
    for (size_t elemID = 0; elemID < meshSectors[sectorID].elemsCountBuff;
         elemID++) {
      meshSectors[sectorID].neighboursFix(&meshSectors[sectorID].elems[elemID]);
    }
  }
  delete[] elemInBufferID;
  delete[] internalElemID;
}

uint32_t Mesh::isElemInsideSector(const Elem *elem,
                                  const MeshSector *meshSectors) {
  if ((elem->index.x >= meshSectors->anchor.x) and
      (elem->index.x < (meshSectors->anchor.x + meshSectors->resolution.x))) {
    if ((elem->index.y >= meshSectors->anchor.y) and
        (elem->index.y < (meshSectors->anchor.y + meshSectors->resolution.y))) {
      return 1;
    }
  }
  if ((elem->index.x >= meshSectors->anchorBuff.x) and
      (elem->index.x <
       (meshSectors->anchorBuff.x + meshSectors->resolutionBuff.x))) {
    if ((elem->index.y >= meshSectors->anchorBuff.y) and
        (elem->index.y <
         (meshSectors->anchorBuff.y + meshSectors->resolutionBuff.y))) {
      return 2;
    }
  }
  return 0;
}

Vec3I Mesh::returnResolution() const {
  // int32_t xRes = (uint32_t)round(Config::Geometry::size.x /
  // Config::Geometry::step.x); int32_t yRes =
  // (uint32_t)round(Config::Geometry::size.y / Config::Geometry::step.y);
  // int32_t zRes = (uint32_t)round(Config::Geometry::size.z /
  // Config::Geometry::step.z); return IntVec3(xRes, yRes, zRes);
  return Config::Geometry::resolution;
}

double Mesh::overhed(MeshSector *meshSectors) {
  double result = 0.0;
  for (size_t i = 0; i < MeshSector::count; i++) {
    result +=
        (double)meshSectors[i].vacantElemID / (double)meshSectors[i].elemsCount;
  }
  result = result / (double)MeshSector::count;
  return result;
}