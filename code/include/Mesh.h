#ifndef MESH_H
#define MESH_H

#include "../../lib/include/Vec.h"
#include "Element.h"
#include "Node.h"
#include <cstdint>
#include <stdint.h>

class Laser;
class MeshSector;

class Mesh {
public:
  Vec3I resolution;
  uint32_t powderLayers;
  uint32_t startPowderAtLayer;
  uint32_t nodesCount;
  uint32_t elemsCount;
  Node *nodes = nullptr;
  Elem *elems = nullptr;
  uint32_t currentNodeID;
  uint32_t currentElemID;

  Mesh();
  Mesh(MeshSector *meshSectors, Laser *laser);
  ~Mesh();

  void createElement(uint32_t elemID, const Vec3I &INDEX_VEC,
                     const Neighbours &NEIGHBOURS,
                     const Neighbours &NEIGHBOURS_TRUNCATED,
                     const uint32_t STATE);
  void createNode(uint32_t nodeID, uint32_t nodePos, const Vec3 &ANCHOR_VEC,
                  const Vec3 &NODE_SCALE_VEC);
  uint32_t findNodeForElement(uint32_t nodePos, const Vec3 &ELEM_VEC,
                              const Vec3 &NODE_SCALE_VEC,
                              const Neighbours &NEIGHBOURS);
  void createMesh();
  void sectorPreprocessor(MeshSector *meshSectors, Laser *laser);
  void sectorGeometryCalculator(MeshSector *meshSectors);
  void sectorLaserAssign(MeshSector *meshSectors, Laser *laser);
  void sectorLookUpTable(MeshSector *meshSectors);
  void sectorPointerToItself(MeshSector *meshSectors);
  void splitMesh(MeshSector *meshSectors);
  uint32_t isElemInsideSector(const Elem *elem, const MeshSector *meshSectors);
  Vec3I returnResolution() const;
  double overhed(MeshSector *meshSectors);
};

#endif // !MESH_H