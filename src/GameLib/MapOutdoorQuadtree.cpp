#include "StdAfx.h"
#include "MapOutdoor.h"
#include "TerrainQuadtree.h"

//////////////////////////////////////////////////////////////////////////
// QuadTree
//////////////////////////////////////////////////////////////////////////
void CMapOutdoor::BuildQuadTree() {
  FreeQuadTree();

  // m_wPatchCount는 ConvertPatchSplat에서도 정한다, 안전을 위해 여기서
  // 체크한다.
  if (0 == m_wPatchCount) {
    TraceError("MapOutdoor::BuildQuadTree : m_wPatchCount is zero, you must "
               "call ConvertPatchSplat before call this method.");
    return;
  }

  m_pRootNode = AllocQuadTreeNode(0, 0, m_wPatchCount - 1, m_wPatchCount - 1);
  if (!m_pRootNode)
    TraceError("CMapOutdoor::BuildQuadTree() RootNode is NULL");

  if (m_pRootNode->Size > 1)
    SubDivideNode(m_pRootNode);
}

CTerrainQuadtreeNode *CMapOutdoor::AllocQuadTreeNode(long x0, long y0, long x1,
                                                     long y1) {
  CTerrainQuadtreeNode *Node;
  long xsize, ysize;

  xsize = x1 - x0 + 1;
  ysize = y1 - y0 + 1;
  if ((xsize == 0) || (ysize == 0))
    return NULL;

  Node = new CTerrainQuadtreeNode;
  Node->x0 = x0;
  Node->y0 = y0;
  Node->x1 = x1;
  Node->y1 = y1;

  if (ysize > xsize)
    Node->Size = ysize;
  else
    Node->Size = xsize;

  Node->PatchNum = y0 * m_wPatchCount + x0;

  Node->center.x = 0.0f;
  Node->center.y = 0.0f;
  Node->center.z = 0.0f;

  Node->radius = 0.0f;

  return Node;
}

void CMapOutdoor::SubDivideNode(CTerrainQuadtreeNode *Node) {
  long nw_size;
  CTerrainQuadtreeNode *tempnode;

  nw_size = Node->Size / 2;

  Node->NW_Node = AllocQuadTreeNode(Node->x0, Node->y0, Node->x0 + nw_size - 1,
                                    Node->y0 + nw_size - 1);
  Node->NE_Node = AllocQuadTreeNode(Node->x0 + nw_size, Node->y0, Node->x1,
                                    Node->y0 + nw_size - 1);
  Node->SW_Node = AllocQuadTreeNode(Node->x0, Node->y0 + nw_size,
                                    Node->x0 + nw_size - 1, Node->y1);
  Node->SE_Node = AllocQuadTreeNode(Node->x0 + nw_size, Node->y0 + nw_size,
                                    Node->x1, Node->y1);

  tempnode = (CTerrainQuadtreeNode *)Node->NW_Node;
  if ((tempnode != NULL) && (tempnode->Size > 1))
    SubDivideNode(tempnode);
  tempnode = (CTerrainQuadtreeNode *)Node->NE_Node;
  if ((tempnode != NULL) && (tempnode->Size > 1))
    SubDivideNode(tempnode);
  tempnode = (CTerrainQuadtreeNode *)Node->SW_Node;
  if ((tempnode != NULL) && (tempnode->Size > 1))
    SubDivideNode(tempnode);
  tempnode = (CTerrainQuadtreeNode *)Node->SE_Node;
  if ((tempnode != NULL) && (tempnode->Size > 1))
    SubDivideNode(tempnode);
}

void CMapOutdoor::FreeQuadTree() {
  if (NULL == m_pRootNode)
    return;

  if (m_pRootNode->NW_Node) {
    delete m_pRootNode->NW_Node;
    m_pRootNode->NW_Node = NULL;
  }
  if (m_pRootNode->NE_Node) {
    delete m_pRootNode->NE_Node;
    m_pRootNode->NE_Node = NULL;
  }
  if (m_pRootNode->SW_Node) {
    delete m_pRootNode->SW_Node;
    m_pRootNode->SW_Node = NULL;
  }
  if (m_pRootNode->SE_Node) {
    delete m_pRootNode->SE_Node;
    m_pRootNode->SE_Node = NULL;
  }

  delete m_pRootNode;
  m_pRootNode = NULL;
}
