/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */
#ifndef _UDMGRIDCOORDINATES_H_
#define _UDMGRIDCOORDINATES_H_

/**
 * @file UdmGridCoordinates.h
 * グリッド座標クラスのヘッダーファイル
 */

#include "model/UdmEntityVoxels.h"

namespace udm
{

class UdmModel;
class UdmZone;
class UdmNode;
class UdmFlowSolutions;
class UdmRankConnectivity;

/**
 * グリッド座標クラス.
 * CGNS:GridCoordinatesに相当するクラスです.
 */
class UdmGridCoordinates: public UdmEntityVoxels
{
private:
    std::vector<UdmNode*> node_list;        ///< グリッド構成ノードリスト
    std::vector<UdmNode*> virtual_nodes;        ///< 仮想ノードリスト
    UdmZone *parent_zone;            ///< 親ゾーン
    std::string  cgns_writegridcoordnates;        ///< 出力CGNS:GrdiCoordinates名
    UdmSize_t max_nodeid;

public:
    UdmGridCoordinates();
    UdmGridCoordinates(UdmZone* zone);
    virtual ~UdmGridCoordinates();

    UdmSize_t getNumNodes() const;
    UdmNode* getNodeById(UdmSize_t node_id) const;
    UdmNode* getNodeByLocalId(UdmSize_t node_id) const;
    UdmSize_t insertNode(UdmNode* node);
    UdmSize_t getNumEntities() const;
    template <class VALUE_TYPE> UdmError_t initializeValueEntities(const std::string &solution_name, VALUE_TYPE value);
    UdmNode* findNodeByGlobalId(int src_rankno, UdmSize_t src_nodeid) const;
    UdmSize_t getNumNodesWithoutLessRankno() const;

    UdmSize_t getNumVirtualNodes() const;
    UdmNode* getVirtualNodeById(UdmSize_t node_id) const;
    UdmSize_t insertVirtualNode(UdmNode* virtual_node);
    const std::vector<UdmNode*>& getVirtualNodes() const;
    UdmNode* findVirtualNodeByGlobalId(int src_rankno, UdmSize_t src_nodeid) const;

    UdmError_t clearNodes();
    UdmError_t clearVirtualNodes();
    UdmError_t removeNode(const UdmEntity* node);
    UdmError_t removeNode(UdmSize_t node_id);
    UdmError_t removeNodes(const std::vector<UdmNode*>& remove_nodes);

    // UdmZone
    UdmZone* getParentZone() const;
    UdmError_t setParentZone(UdmZone *zone);

    // CGNS
    UdmError_t readCgns(int index_file, int index_base, int index_zone, int timeslice_step = -1);
    UdmError_t getCgnsIterativeGridCoordinatesName(char *zone_name, int index_file, int index_base, int index_zone, int timeslice_step = -1);
    UdmError_t writeCgns(int index_file, int index_base, int index_zone, int timeslice_step, bool grid_timeslice);
    UdmError_t writeCgnsLinkFile(int index_file, int index_base, int index_zone, const std::string &link_output_path, const std::vector<std::string> &linked_files, int timeslice_step);
    UdmError_t writeVirtualNodes(int index_file, int index_base, int index_zone);
    UdmError_t initializeWriteCgns();
    const std::string& getCgnsWriteGridCoordnates() const;
    void setCgnsWriteGridCoordnates(const std::string& gridcoordnates_name);
    void clearCgnsWriteGridCoordnates();
    int getNumCgnsWriteGridCoordnates() const;
    static bool existsCgnsGridCoordnates(const std::string &filename, int index_base = 1, int index_zone = 1);
    UdmError_t joinCgnsGridCoordinates(UdmGridCoordinates *dest_grid);

    // Partition
    UdmError_t importNodes(const std::vector<UdmNode*>& import_nodes);
    UdmError_t rebuildNodes();
    UdmError_t importVirtualNodes(const std::vector<UdmNode*>& virtual_nodes);
    void clearPreviousInfos();
    UdmError_t rebuildVirtualNodes();

    // シリアライズ
    UdmSerializeArchive& serialize(UdmSerializeArchive &archive) const;
    UdmSerializeArchive& deserialize(UdmSerializeArchive &archive);
    UdmError_t brodcastGridCoordinates();

    // GridCoordinates作成
    template<class DATA_TYPE>
    UdmSize_t setGridCoordinatesArray(UdmSize_t num_nodes, DATA_TYPE* coords_x, DATA_TYPE* coords_y, DATA_TYPE* coords_z);
    template<class DATA_TYPE>
    UdmSize_t getGridCoordinatesArray(UdmSize_t start_id, UdmSize_t end_id, DATA_TYPE* coords_x, DATA_TYPE* coords_y, DATA_TYPE* coords_z);
    template<class DATA_TYPE>
    UdmSize_t getGridCoordinatesX(UdmSize_t start_id, UdmSize_t end_id, DATA_TYPE* coords);
    template<class DATA_TYPE>
    UdmSize_t getGridCoordinatesY(UdmSize_t start_id, UdmSize_t end_id, DATA_TYPE* coords);
    template<class DATA_TYPE>
    UdmSize_t getGridCoordinatesZ(UdmSize_t start_id, UdmSize_t end_id, DATA_TYPE* coords);
    template<class DATA_TYPE>
    UdmSize_t insertGridCoordinates(DATA_TYPE x, DATA_TYPE y, DATA_TYPE z);
    template<class DATA_TYPE>
    UdmError_t getGridCoordinates(UdmSize_t node_id, DATA_TYPE &x, DATA_TYPE &y, DATA_TYPE &z) const;
    template<class DATA_TYPE>
    UdmError_t setGridCoordinates(UdmSize_t node_id, DATA_TYPE x, DATA_TYPE y, DATA_TYPE z);
    template<class DATA_TYPE>
    UdmSize_t getGridCoordinatesArrayOfVirtual(UdmSize_t start_id, UdmSize_t end_id, DATA_TYPE* coords_x, DATA_TYPE* coords_y, DATA_TYPE* coords_z);
    template<class DATA_TYPE>
    UdmSize_t getGridCoordinatesXOfVirtual(UdmSize_t start_id, UdmSize_t end_id, DATA_TYPE* coords);
    template<class DATA_TYPE>
    UdmSize_t getGridCoordinatesYOfVirtual(UdmSize_t start_id, UdmSize_t end_id, DATA_TYPE* coords);
    template<class DATA_TYPE>
    UdmSize_t getGridCoordinatesZOfVirtual(UdmSize_t start_id, UdmSize_t end_id, DATA_TYPE* coords);
    UdmError_t insertRankConnectivity(UdmSize_t node_id, int rankno, UdmSize_t localid);

    // FlowSoution
    template <class VALUE_TYPE>
    UdmError_t getSolutionScalar(UdmSize_t node_id, const std::string &solution_name, VALUE_TYPE &value) const;
    template <class VALUE_TYPE>
    unsigned int getSolutionVector(UdmSize_t node_id, const std::string &solution_name, VALUE_TYPE *value)  const;
    template <class VALUE_TYPE>
    UdmError_t setSolutionScalar(UdmSize_t node_id, const std::string &solution_name, VALUE_TYPE value);
    template <class VALUE_TYPE>
    UdmError_t setSolutionVector(UdmSize_t node_id, const std::string &solution_name, const VALUE_TYPE* values, unsigned int size = 3);

    // for debug
    void toString(std::string &buf) const;
    // memory size
    size_t getMemSize() const;

private:
    void initialize();
    void finalize();
    UdmDataType_t setCoordsDatatype(DataType_t cgns_datatype);
    UdmFlowSolutions* getFlowSolutions() const;
    UdmRankConnectivity* getRankConnectivity() const;
    UdmEntity* getEntityById(UdmSize_t entity_id);
    const UdmEntity* getEntityById(UdmSize_t entity_id) const;
    const UdmSolutionFieldConfig* getSolutionFieldConfig(const std::string &solution_name) const;
    UdmError_t removeRankConnectivity(const UdmEntity *node) const;
    void pushbackNode(UdmNode* node);

    // MPI
    int getMpiRankno() const;
    int getMpiProcessSize() const;
    UdmError_t cloneGridCoordinates(const UdmGridCoordinates& src);
    bool equalsGridCoordinates(const UdmGridCoordinates &grid) const;
    int getCgnsRankno() const;

    // CGNS
    int findCgnsGridCoordinates(int index_file, int index_base, int index_zone, const char *gridname) const;

};

} /* namespace udm */
#endif /* _UDMGRIDCOORDINATES_H_ */

