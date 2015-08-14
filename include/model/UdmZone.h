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

#ifndef _UDMZONE_H_
#define _UDMZONE_H_

/**
 * @file UdmZone.h
 * ゾーンクラスのヘッダーファイル
 */

#include "config/UdmDfiConfig.h"
#include "model/UdmGeneral.h"
#include "model/UdmGridCoordinates.h"
#include "model/UdmSections.h"
#include "model/UdmElements.h"
#include "model/UdmFlowSolutions.h"
#include "model/UdmRankConnectivity.h"
#include "model/UdmUserDefinedDatas.h"
#include "model/UdmCell.h"
#include "model/UdmNode.h"

namespace udm
{
class UdmModel;
class UdmGridCoordinates;
class UdmSections;
class UdmFlowSolutions;
class UdmRankConnectivity;
class UdmUserDefinedDatas;
class UdmCell;

/**
 * ゾーンクラス.
 * CGNS:Zoneに相当するクラスです.
 */
class UdmZone: public UdmGeneral, public UdmISerializable
{
private:
    UdmZoneType_t zone_type;        ///< ゾーンタイプ
    UdmSize_t vertex_size;            ///< ノード（頂点）サイズ
    UdmSize_t cell_size;            ///< 要素（セル）サイズ

    UdmGridCoordinates *grid_coordinates;        ///< グリッド座標
    UdmSections *sections;        ///< 要素管理クラス
    UdmFlowSolutions *solutions;        ///< 物理量管理クラス
    UdmUserDefinedDatas *user_datas;        ///< ユーザ定義データ管理クラス
    UdmModel *parent_model;                ///< 親モデル（ベース）

    /**
     * 内部境界情報
     */
    UdmRankConnectivity  *inner_boundary;

    /**
     * 分割重み設定フラグ
     * true = 分割重み設定
     */
    bool set_partition_weight;

public:
    UdmZone();
    UdmZone(UdmModel *parent_model);
    UdmZone(const std::string &zone_name, int zone_id);
    virtual ~UdmZone();
    UdmZoneType_t getZoneType() const;
    void setZoneType(UdmZoneType_t zoneType);
    UdmZoneType_t setZoneType(ZoneType_t cgns_zonetype);

    // UdmGridCoordinates
    UdmGridCoordinates* getGridCoordinates() const;
    // UdmSections
    UdmSections* getSections() const;
    // UdmFlowSolutions
    UdmFlowSolutions* getFlowSolutions() const;
    // UdmUserDefinedDatas
    UdmUserDefinedDatas* getUserDefinedDatas() const;
    // UdmModel
    UdmModel* getParentModel() const;
    UdmError_t setParentModel(UdmModel *model);

    // CGNS
    UdmError_t readCgns(int index_file, int index_base, int index_zone, int timeslice_step = -1);
    UdmSize_t getCellSize() const;
    void setCellSize(UdmSize_t cellSize);
    UdmSize_t getVertexSize() const;
    void setVertexSize(UdmSize_t vertexSize);
    UdmError_t writeCgnsTimeSliceFile(int index_file, int index_base, int timeslice_step, bool grid_timeslice, bool write_constsolutions);
    UdmError_t writeCgnsGridCoordinatesFile(int index_file, int index_base, int timeslice_step, bool grid_timeslice);
    UdmError_t writeCgnsFlowSolutionFile(int index_file, int index_base, int timeslice_step, bool write_constsolutions);
    UdmError_t writeCgnsLinkFile(
                            int index_file,
                            int index_base,
                            const std::string &link_output_path,
                            const std::vector<std::string> &linked_files,
                            int timeslice_step);
    UdmError_t writeRankConnectivity(
                            int index_file,
                            int index_base);
    UdmError_t writeVirtualCells(
                            int index_file,
                            int index_base);
    UdmError_t initializeWriteCgns();
    int getNumCgnsIterativeDatas() const;
    UdmError_t writeCgnsIterativeDatas(int index_file, int index_base, int timeslice_step);
    int getCellDimension() const;
    UdmError_t joinCgnsZone(const UdmZone *zone);
    bool validateCgns() const;
    void clearPreviousInfos();
    UdmError_t createComponentCells();

    // for debug
    void toString(std::string &buf) const;

    // DFI
    UdmDfiConfig* getDfiConfig() const;
    UdmError_t updateProcessConfigs();

    // シリアライズ
    UdmSerializeArchive& serialize(UdmSerializeArchive &archive) const;
    UdmSerializeArchive& deserialize(UdmSerializeArchive &archive);

    // MPI
    MPI_Comm getMpiComm() const;
    int getMpiRankno() const;
    int getMpiProcessSize() const;
    UdmError_t broadcastZone();
    UdmError_t transferVirtualCells(std::vector<UdmNode*>& virtual_nodes);

    // Partition
    UdmError_t importCells( const std::vector<UdmCell*>& import_cells,
                            std::vector<UdmNode*>& import_nodes);
    UdmError_t exportCells( const std::vector<UdmCell*>& export_cells,
                            std::vector<UdmEntity*>& export_nodes);
    UdmError_t rebuildZone(const std::vector<UdmNode*> import_nodes);
    UdmError_t importVirturalCells(
                            const std::vector<UdmCell*>& import_virtuals,
                            std::vector<UdmNode*>& import_nodes);
    UdmError_t clearVertialCells();
    UdmError_t createComponentCells(const std::vector<UdmNode*> import_nodes);

    // RankConnectivity
    UdmRankConnectivity* getRankConnectivity() const;

    // 物理量
    UdmError_t getSolutionInfo(
                        const std::string &solution_name,
                        UdmDataType_t         &data_type,
                        UdmGridLocation_t     &grid_location,
                        UdmVectorType_t       &vector_type,
                        int                   &nvector_size,
                        bool                  &constant_flag) const;
    // 節点（ノード）
    UdmSize_t getNumNodes() const;
    UdmNode* getNode(UdmSize_t node_id) const;
    // 物理量：節点（ノード）
    template <class VALUE_TYPE>
    UdmError_t getSolutionScalarInNode(UdmSize_t node_id, const std::string &solution_name, VALUE_TYPE &value) const;
    template <class VALUE_TYPE>
    unsigned int getSolutionVectorinNode(UdmSize_t node_id, const std::string &solution_name, VALUE_TYPE *value)  const;
    template <class VALUE_TYPE>
    UdmError_t setSolutionScalarInNode(UdmSize_t node_id, const std::string &solution_name, VALUE_TYPE value);
    template <class VALUE_TYPE>
    UdmError_t setSolutionVectorInNode(UdmSize_t node_id, const std::string &solution_name, const VALUE_TYPE* values, unsigned int size = 3);

    // 要素（セル）
    UdmSize_t getNumCells() const;
    UdmCell *getCell(UdmSize_t cell_id) const;
    // 物理量：要素（セル）
    template <class VALUE_TYPE>
    UdmError_t getSolutionScalarInCell(UdmSize_t cell_id, const std::string &solution_name, VALUE_TYPE &value) const;
    template <class VALUE_TYPE>
    unsigned int getSolutionVectorInCell(UdmSize_t cell_id, const std::string &solution_name, VALUE_TYPE *value)  const;
    template <class VALUE_TYPE>
    UdmError_t setSolutionScalarInCell(UdmSize_t cell_id, const std::string &solution_name, VALUE_TYPE value);
    template <class VALUE_TYPE>
    UdmError_t setSolutionVectorInCell(UdmSize_t cell_id, const std::string &solution_name, const VALUE_TYPE* values, unsigned int size = 3);

    // 接続要素（セル）
    int getNumConnectivityCells(UdmSize_t node_id) const;
    UdmCell* getConnectivityCell(UdmSize_t node_id, int connectivity_id) const;
    // 接続節点（ノード）
    int getNumConnectivityNodes(UdmSize_t cell_id) const;
    UdmNode* getConnectivityNode(UdmSize_t cell_id, int connectivity_id) const;
    // 隣接要素（セル）
    int getNumNeighborCells(UdmSize_t cell_id) const;
    UdmCell* getNeighborCell(UdmSize_t cell_id, int neighbor_id) const;
    // 隣接節点（ノード）
    int getNumNeighborNodes(UdmSize_t node_id) const;
    UdmNode* getNeighborNode(UdmSize_t node_id, int neighbor_id) const;

    // 分割重み
    bool isSetPartitionWeight() const;
    void setPartitionWeight(bool set_weight);
    void clearPartitionWeight();

private:
    void initialize();
    UdmError_t writeCgnsZone(int index_file, int index_base, int &index_zone);
    bool existsCgnsNode(int index_file, int index_base, int index_zone, const std::string &node_name) const;
    UdmError_t cloneZone(const UdmZone& src);
    bool equalsZone(const UdmZone &zone) const;
    void updateVertexSize();
    void updateCellSize();
    UdmError_t readCgnsZoneIterativeDatas(
                    int index_file,
                    int index_base,
                    int index_zone,
                    std::vector<std::string> &iterative_grids,
                    std::vector<std::string> &iterative_solutions);
    UdmError_t writeVirtualZone(int index_file, int index_base, int &index_zone);
    void insertImportGlobalRankids(
                    UdmGlobalRankidList &import_globalids,
                    const UdmNode* import_node) const;
};

} /* namespace udm */

#endif /* _UDMZONE_H_ */
