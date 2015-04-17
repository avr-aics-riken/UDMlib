/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmComponent.h
 * 要素（セル）の部品要素クラスのヘッダーファイル
 */

#ifndef _UDMCOMPONENT_H_
#define _UDMCOMPONENT_H_

#include "UdmBase.h"
#include "model/UdmICellComponent.h"

namespace udm
{

class UdmCell;
class UdmNode;

/**
 * 部品要素クラス
 */
class UdmComponent: public UdmBase, public UdmICellComponent
{
private:

    /**
     * ID : CGNSノードID、要素ローカルID, ノードローカルID
     */
    UdmSize_t id;

    /**
     * 要素形状タイプ
     */
    UdmElementType_t element_type;

    /**
     * 構成ノード
     */
    std::vector<UdmNode*>  nodes;

    /**
     * 親構成要素.
     * 部品要素の親要素は必ず２つ
     */
    UdmCell *parent_cells[2];

    /**
     * 節点（ノード）、要素（セル）の削除フラグ
     */
    bool remove_entity;

public:
    UdmComponent();
    UdmComponent(UdmElementType_t element_type);
    virtual ~UdmComponent();

    UdmElementType_t getElementType() const;
    UdmError_t setElementType(UdmElementType_t element_type);
    UdmSize_t getNumNodes() const;
    UdmNode* getNode(UdmSize_t node_id)  const;
    UdmError_t setNodes(UdmNode* nodes[], UdmSize_t size);
    UdmError_t setNodes(const std::vector<UdmNode*> &nodes);
    const std::vector<UdmNode*>& getNodes() const;
    UdmError_t insertNode(UdmNode* node);
    void removeNode(const UdmNode* node);
    UdmError_t clearNodes();
    UdmSize_t getNumParentCells() const;
    UdmCell* getParentCell(UdmSize_t cell_id) const;
    UdmError_t setParentCells(UdmCell* parent_cells[], UdmSize_t size);
    UdmError_t insertParentCell(UdmCell* parent_cell);
    UdmError_t clearParentCells();
    UdmSize_t removeReferencedCell(const UdmCell* parent_cell);
    UdmSize_t removeReferencedCells();
    void toString(std::string& buf) const;
    UdmSize_t getId() const;
    void setId(UdmSize_t id);
    UdmCellClass_t getCellClass() const;

    // 削除フラグ
    bool isRemoveEntity() const;
    void setRemoveEntity(bool remove);

    // 破棄処理
    void finalize();

private:
    void initialize();

};

} /* namespace udm */

#endif /* _UDMCOMPONENT_H_ */
