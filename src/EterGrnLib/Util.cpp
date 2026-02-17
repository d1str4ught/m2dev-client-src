#include "StdAfx.h"
#include "Util.h"

bool GrannyMeshIsDeform(granny_mesh *pgrnMesh) {
  if (GrannyMeshIsRigid(pgrnMesh))
    return false;

  return true;
}
