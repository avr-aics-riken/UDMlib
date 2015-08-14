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

#ifndef _UDMCELL_H_
#define _UDMCELL_H_

/**
 * @file UdmCell.h
 * 要素（セル）基底クラスのヘッダーファイル
 */

#include "model/UdmEntity.h"
#include "model/UdmICellComponent.h"

namespace udm
{
class UdmSections;
class UdmElements;
class UdmNode;
class UdmScannerCells;
class UdmComponent;

/**
 * 要素（セル）基底クラス.
 * Bar,Shell,Solid要素の基底クラス
 * 複数のノード（UdmNode）から構成される要素（セル）の基底クラス
 */
class UdmCell: public UdmEntity, public UdmICellComponent
{
private:

    /**
     * 管理CGNS要素ID.
     * 読込CGNSの要素IDは連続or連番が保証されていない。
     * CGNS:Elements毎に連続ではない可能性がある
     */
    UdmSize_t elem_id;

    /**
     * 構成ノード
     */
    std::vector<UdmNode*>  nodes;

    /**
     * 親セクション（要素構成）情報クラス
     */
    UdmElements *parent_elements;

    /**
     * 親セクション管理クラス
     */
    UdmSections *parent_sections;

    /**
     * 部品要素（セル）.
     * Solid(3D)の構成Shell(2D), Shell(2D)の構成Bar(1D)を設定する.
     * componentsの実体はUdmSections::component_cellsとする.
     * よって、UdmCellではオブジェクトの削除は行わない。
     */
    std::vector<UdmComponent*> components;

public:
    UdmCell();
    UdmCell(UdmElementType_t element_type);
    virtual ~UdmCell();

    // 管理要素ID
    UdmSize_t getElemId() const;
    void setElemId(UdmSize_t elemId);

    // UdmNode
    UdmSize_t getNumNodes() const;
    UdmNode* getNode(UdmSize_t node_id) const;
    const std::vector<UdmNode*>& getNodes() const;
    UdmError_t setNodes(UdmNode *node[], UdmSize_t size);
    UdmError_t setNodes(const std::vector<UdmNode*> &nodes);
    UdmError_t insertNode(UdmNode *node);
    UdmError_t clearNodes();
    void removeNode(const UdmNode *node);
    bool equalsNodeIds(const UdmCell* dest_cell) const;
    UdmSize_t scannerCells(UdmScannerCells* scanner);
    UdmSize_t scannerGraph(UdmScannerCells* scanner) const;
    void setExportRankno(int rankno);

    // 親セクション（要素構成）情報
    UdmElements *getParentElements() const;
    UdmError_t setParentElements(UdmElements *parent_elements);
    UdmSections* getParentSections() const;
    void setParentSections(UdmSections* parentSections);

    // 構成要素（セル）
    virtual UdmError_t createComponentCells() = 0;
    UdmSize_t getNumComponentCells() const;
    UdmComponent* getComponentCell(UdmSize_t components_id) const;
    UdmError_t clearComponentCells();
    virtual int getDimensions() const = 0;
    UdmSize_t removeComponentCell(const UdmComponent* cell);

    // シリアライズ
    UdmSerializeArchive& serialize(UdmSerializeArchive &archive) const;
    UdmSerializeArchive& deserialize(UdmSerializeArchive &archive);
    static UdmError_t freeDeserialize(UdmCell* cell);

    // for debug
    void toString(std::string &buf) const;
    // memory size
    size_t getMemSize() const;

    // Factory
    static UdmCell* factoryCell(UdmElementType_t elem_type);
    UdmError_t cloneCell(const UdmCell& src);

    // 隣接要素（セル）
    UdmSize_t getNumNeighborCells() const;
    UdmCell* getNeighborCell(int neighbor_id) const;

    UdmCellClass_t getCellClass() const;
    UdmSize_t getNumParentCells() const;
    UdmCell* getParentCell(UdmSize_t cell_id) const;
    UdmSize_t getId() const;
    UdmElementType_t getElementType() const;
    // 破棄処理
    void finalize();

private:
    void initialize();
    UdmError_t removeReferencedCells();
    UdmComponent* factoryComponentCell(UdmElementType_t elem_type);

protected:
    UdmZone* getZone() const;
    UdmSize_t createComponentCell(UdmElementType_t element_type, unsigned int num_nodes, UdmSize_t face_nodes[]);
};

} /* namespace udm */

#endif /* _UDMCELL_H_ */
