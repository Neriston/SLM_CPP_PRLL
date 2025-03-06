#ifndef MESHBLOCK_H
#define MESHBLOCK_H

#include "../../lib/include/Vec.h"
#include <cstdint>

class Elem;
class Laser;

class MeshSector {
public:
  static uint32_t count;
  Vec3I anchor;
  Vec3I anchorBuff;
  Vec3I resolution;
  Vec3I resolutionBuff;
  uint32_t elemsCount;
  uint32_t elemsCountBuff;
  uint32_t vacantElemID;
  Elem *elems;
  uint32_t *volatileElemsIDs;
  uint32_t volatileElemsCount;
  int32_t *lookUpTable;
  Laser *laserPtr;
  MeshSector *meshSectorsPtr;
  uint32_t powderElemsCount;

  MeshSector();
  ~MeshSector();

  void init(const Vec3I &anchor, const Vec3I &resolution);
  void advance();
  void getThisElem(Elem *elem, uint32_t sectorOrBuffer);
  void copyThisElem(const Elem *elem);
  void neighboursFix(Elem *elem);
  void checkIfElemIsOnBorder(Elem *elem);
  void syncBorders(const MeshSector *meshSectors);
  void addNewLayerOfPowder();
  void moveDownVaporizedElems();
  void modifyLaserImpactLandscape(const size_t ID);
};

#endif // !MESHBLOCK_H