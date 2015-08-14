// ##################################################################################
//
// UDMlib - Unstructured Data Management Library
//
// Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
// All rights reserved.
//
// Copyright (c) 2015 Advanced Institute for Computational Science, RIKEN.
// All rights reserved.
//
// ###################################################################################

/**
 * @file UdmICellComponent.h
 * 要素、部品要素のインターフェイスクラス定義ファイル
 */

#ifndef _UDMICELLCOMPONENT_H_
#define _UDMICELLCOMPONENT_H_

#include "UdmBase.h"
#include "model/UdmNode.h"

namespace udm
{
class UdmCell;

/**
 * 要素、部品要素のインターフェイスクラス
 */
class UdmICellComponent
{
public:

    /**
     * デストラクタ
     */
    virtual ~UdmICellComponent() {};

    /**
     * 要素クラスタイプを取得する.
     * @return        UdmCellClass_t={Udm_CellClass:要素（セル）クラス, Udm_ComponentClass:部品要素（セル）クラス}
     */
    virtual UdmCellClass_t getCellClass() const = 0;

    /**
     * IDを取得する.
     * @return        ID
     */
    virtual UdmSize_t getId() const = 0;

    virtual UdmSize_t getNumParentCells() const = 0;
    virtual UdmCell* getParentCell(UdmSize_t cell_id) const = 0;

    virtual UdmSize_t getNumNodes() const = 0;
    virtual UdmNode* getNode(UdmSize_t node_id)  const = 0;
    virtual void removeNode(const UdmNode* node) = 0;
    virtual UdmElementType_t getElementType() const = 0;

    /**
     * 頂点（ノード）構成、IDが同一の要素（セル）であるかチェックする.
     * @param dest_cell        比較対象要素（セル）
     * @return        true=同一要素（セル）
     */
    bool equalsNodeIds(const UdmICellComponent* dest_cell) const
    {
        if (dest_cell == NULL) return false;
        if (this->getNumNodes() != dest_cell->getNumNodes()) return false;

        std::set<UdmSize_t> src_ids;
        UdmGlobalRankidList src_rankids(this->getNumNodes());
        UdmGlobalRankidList dest_rankids(this->getNumNodes());
        UdmSize_t n, size;
        size = this->getNumNodes();
        for (n=1; n<=size; n++) {
            UdmNode *src_node = this->getNode(n);
            src_rankids.addGlobalRankid(src_node->getMyRankno(), src_node->getId());
        }

        size = dest_cell->getNumNodes();
        for (n=1; n<=size; n++) {
            UdmNode *dest_node = dest_cell->getNode(n);
            dest_rankids.addGlobalRankid(dest_node->getMyRankno(), dest_node->getId());
        }

        if (!src_rankids.equals(dest_rankids)) {
            return false;
        }

        return true;
    }

};

} /* namespace udm */

#endif /* _UDMICELLCOMPONENT_H_ */
