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

#ifndef _UDMNODE_H_
#define _UDMNODE_H_
/**
 * @file UdmNode.h
 * ノード（節点）クラスのヘッダーファイル
 */

#include "model/UdmEntity.h"
#include "model/UdmCoordsValue.h"

namespace udm
{
class UdmZone;
class UdmGridCoordinates;
class UdmICellComponent;

/**
 * ノード（節点）クラス.
 */
class UdmNode: public UdmEntity
{
private:
    UdmCoordsValue *coords;            ///< XYZ座標値
    UdmGridCoordinates* parent_gridcoordinates;        ///< 親GridCoordinates
    UdmNode *common_node;                ///< 同一節点（ノード）

    /**
     * 親構成要素
     */
    std::vector<UdmICellComponent*> parent_cells;

public:
    UdmNode();
    UdmNode(double x, double y, double z);
    UdmNode(float x, float y, float z);
    virtual ~UdmNode();
    UdmError_t getCoords(double &x, double &y, double &z) const;
    UdmError_t getCoords(float &x, float &y, float &z)  const;
    UdmError_t setCoords(double x, double y, double z);
    UdmError_t setCoords(float x, float y, float z);

    // 関係演算子
    bool operator==(const UdmNode& right) { return (this->compareCoords(right) == 0); };
    bool operator!=(const UdmNode& right) { return (this->compareCoords(right) != 0); };
    bool operator>=(const UdmNode& right) { return (this->compareCoords(right) >= 0); };
    bool operator<=(const UdmNode& right) { return (this->compareCoords(right) <= 0); };
    bool operator>(const UdmNode& right) { return (this->compareCoords(right) > 0); };
    bool operator<(const UdmNode& right) { return (this->compareCoords(right) < 0); };
    UdmGridCoordinates* getParentGridcoordinates() const;
    void setParentGridcoordinates(UdmGridCoordinates* parentGridcoordinates);

    // シリアライズ
    UdmSerializeArchive& serialize(UdmSerializeArchive &archive) const;
    UdmSerializeArchive& deserialize(UdmSerializeArchive &archive);
    UdmError_t cloneNode(const UdmNode& src);
    static UdmNode* findMpiRankInfo(const std::vector<UdmNode*> &inner_nodes, int rankno, UdmSize_t localid);
    static UdmNode* findMpiRankInfo(
                        const std::vector<UdmNode*>& inner_nodes,
                        const UdmNode *src_node);
    static UdmError_t freeDeserialize(UdmNode* node);

    // 同一節点（ノード）
    UdmNode* getCommonNode() const;
    void setCommonNode(UdmNode* movedNode);

    // 接続要素（セル）
    UdmSize_t getNumConnectivityCells() const;
    UdmCell* getConnectivityCell(int connectivity_id) const;
    // 隣接節点（ノード）
    UdmSize_t getNumNeighborNodes() const;
    UdmNode* getNeighborNode(int neighbor_id) const;

    // 親構成要素
    UdmSize_t getNumParentCells() const;
    UdmICellComponent* getParentCell(UdmSize_t cell_id) const;
    UdmError_t setParentCells(UdmICellComponent *parent_cells[], UdmSize_t size);
    UdmError_t insertParentCell(UdmICellComponent *parent_cells);
    UdmError_t clearParentCells();
    UdmSize_t removeReferencedCell(const UdmICellComponent *parent_cell);
    void removeReferencedNode();

    // for debug
    void toString(std::string &buf) const;
    // memory size
    size_t getMemSize() const;
    // 破棄処理
    void finalize();

private:
    void initialize();
    int compareCoords(const UdmNode &right ) const;

protected:
    UdmZone* getZone() const;

};

} /* namespace udm */

#endif /* _UDMNODE_H_ */
