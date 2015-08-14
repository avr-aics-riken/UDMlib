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

#ifndef _UDMVISITORCELLS_H_
#define _UDMVISITORCELLS_H_

#include <algorithm>
#include "UdmBase.h"

/**
 * @file UdmVisitorCells.h
 * 要素（セル）接続情報収集クラスのヘッダーファイル
 */

namespace udm
{
class UdmZone;
class UdmSections;
class UdmCell;
class UdmNode;
class UdmEntity;


/**
 * 要素（セル）接続情報収集クラス
 */
class UdmScannerCells : public UdmBase
{
private:
    /// 収集対象UdmZone
    UdmZone *scan_zone;
    /// グローバルIDのデータサイズ:NUM_GID_ENTRIES
    int num_gid_entries;
    /// ローカルIDのデータサイズ:NUM_LID_ENTRIES
    int num_lid_entries;

    // グラフデータ
    std::vector<UdmEntity*> current_scan;            ///< 収集中要素（セル）
    std::vector< std::vector<UdmEntity*> > scan_arrays;        ///< 収集接続情報

    // 転送要素（セル）
    std::vector<UdmCell*> import_cells;
    std::vector<UdmCell*> export_cells;

    /// 要素（セル）シリアライズサイズ
    std::vector<int> cell_sizes;
    int scan_iterator;            ///< アクセスポジション

public:
    UdmScannerCells();
    UdmScannerCells(UdmZone *zone);
    virtual ~UdmScannerCells();
    const UdmZone* getScanZone() const;
    UdmZone* getScanZone();
    void setScanZone(UdmZone *zone);
    void setEntriesIdSize(int num_gid_entries, int num_lid_entries = -1);
    UdmSize_t scannerElements(const std::vector<UdmCell*> &cell_list);
    UdmSize_t scannerElements(const std::vector<const UdmCell*> &cell_list);
    UdmSize_t scannerElements(const UdmCell* cell);
    UdmSize_t scannerElements(const std::vector<UdmICellComponent*> &cell_list);
    UdmSize_t scannerParents(const std::vector<UdmNode*> &node_list);
    UdmSize_t scannerParents(const UdmNode* node);
    UdmSize_t scannerGraph();
    UdmSize_t scannerGraph(const UdmSections *sections);
    UdmSize_t scannerGraph(const UdmCell* cell);
    UdmSize_t scannerGraph(const std::vector<UdmCell*> &scan_cells);
    UdmSize_t scannerGraph(const UdmComponent* component);
    UdmSize_t scannerBoundaryNode(const std::vector<UdmNode*>& node_list);
    UdmSize_t scannerBoundaryNode(const UdmNode* node);
    size_t getNumElementsDatas(UdmElementType_t element_type) const;
    UdmSize_t getNumObjects() const;
    UdmSize_t getNumHyperGraphDatas(UdmSize_t &num_globalids, UdmSize_t &num_neighbors) const;
    UdmSize_t getNumGraphDatas(UdmSize_t &num_globalids, int *num_edges) const;
    template <class DATA_TYPE> UdmSize_t getScanCgnsElementsIds(UdmElementType_t element_type, DATA_TYPE *array) const;
    template <class DATA_TYPE> UdmSize_t getScanObjects(DATA_TYPE *globalids, DATA_TYPE *localids) const;
    template <class DATA_TYPE> UdmSize_t getScanHyperGraphDatas(DATA_TYPE *globalids, int *vtxedge_ptr, DATA_TYPE *neighborids) const;
    template <class DATA_TYPE> UdmSize_t getScanGraphDatas(DATA_TYPE *neighborids, int* nbor_procs) const;
    UdmSize_t getScanEntities(std::vector<UdmEntity*> &entities) const;
    const std::vector< std::vector<UdmEntity*> >&  getScanArrays() const;
    UdmSize_t getObjectWeights(float* obj_wgts) const;
    UdmSize_t scannerNeighborNodes(const UdmNode* node);
    UdmSize_t scannerNeighborCells(const UdmCell* cell);
    template <class DATA_TYPE>
    size_t scannerCellsSize( int num_gid_entries,
                                int num_lid_entries,
                                int num_ids,
                                DATA_TYPE *global_ids,
                                DATA_TYPE *local_ids,
                                int *exportProcs);
    const std::vector<int>& getScanCellSizes() const;

    // Import,Export Cell
    UdmSize_t insertImportCell(UdmCell* cell);
    UdmSize_t insertExportCell(UdmCell* cell);
    const std::vector<UdmCell*>& getImportCells() const;
    const std::vector<UdmCell*>& getExportCells() const;
    void clearImportCells();
    void clearExportCells();
    int getScanIterator() const;
    void setScanIterator(int scanIterator);

private:
    UdmSize_t insertEntityList(std::vector<UdmEntity*> &entities, UdmEntity* entity) const;

};

} /* namespace udm */


#endif /* _UDMVISITORCELLS_H_ */
