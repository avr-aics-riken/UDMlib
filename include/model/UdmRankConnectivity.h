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

#ifndef _UDMRANKCONNECTIVITY_H_
#define _UDMRANKCONNECTIVITY_H_

/**
 * @file UdmRankConnectivity.h
 * 内部境界管理クラスのヘッダーファイル
 */

#include "model/UdmGeneral.h"
#include "model/UdmGlobalRankid.h"

namespace udm
{
class UdmZone;
class UdmGridCoordinates;
class UdmCell;
class UdmNode;



/**
 * 内部境界管理クラス
 * 内部境界のノード管理を行う.
 */
class UdmRankConnectivity: public UdmGeneral
{
private:
    /**
     * 内部境界ノードリスト.
     * ２つ以上のプロセスの共通ノード
     */
    std::vector<UdmNode*> boundary_nodes;

    /**
     * 親ゾーン
     */
    UdmZone *parent_zone;

    /**
     * ランク番号、ID検索テーブル
     */
    UdmGlobalRankidList  search_table;

public:
    UdmRankConnectivity();
    UdmRankConnectivity(UdmZone *parent_zone);
    virtual ~UdmRankConnectivity();

    UdmSize_t getNumBoundaryNodes() const;
    UdmSize_t insertRankConnectivityNode(UdmNode *node);
    UdmNode* getBoundaryNode(UdmSize_t bnd_id);
    const UdmNode* getBoundaryNode(UdmSize_t bnd_id) const;
    void clear();
    void clearMpiRankInfos();
    UdmError_t removeBoundaryNode(const UdmEntity *node);
    UdmNode* findMpiRankInfo(int rankno, UdmSize_t localid) const;
    UdmNode* findMpiRankInfo(const UdmNode *src) const;
    UdmSize_t getNumNodesWithoutLessRankno() const;
    UdmError_t removeBoundaryNodes(const std::vector<UdmNode*>& remove_nodes);

    // MPI:Send/Recv
    UdmError_t migrationBoundary();
    UdmError_t transferUpdatedIds();
    UdmError_t transferVirtualCells(std::vector<UdmNode*>& import_cells);
    UdmError_t transferRankConnectivity();
    UdmError_t exportProcess();

    // Zone
    UdmZone* getParentZone() const;
    void setParentZone(UdmZone* parentZone);
    UdmGridCoordinates* getGridCoordinates() const;

    // CGNS
    UdmError_t readCgns(int index_file, int index_base, int index_zone);
    UdmError_t writeCgns(int index_file, int index_base, int index_zone);
    UdmError_t writeRankConnectivity(int index_file, int index_base, int index_zone);
    void eraseRankConnectivity();
    bool mpi_validateRankConnectivity() const;
    UdmError_t rebuildRankConnectivity();

    // ランク番号,ID検索テーブル
    void createSearchTable();
    void clearSearchTable();
    void createSearchMpiRankidTable();

    // for debug
    void toString(std::string &buf) const;

private:
    void initialize(void);
    void insertBoundaryInfo(
                    std::map< int, UdmGlobalRankidPairList* > &infos,
                    int rankno,
                    const UdmGlobalRankidPair &info) const;
    void insertBoundaryInfos(
                    std::map< int, UdmGlobalRankidPairList* > &info_map,
                    int rankno,
                    UdmGlobalRankidPairList *infos) const;
    void removeRankConnectivity(
                    const std::map< int, UdmGlobalRankidPairList* > &send_infos,
                    const std::map< int, UdmGlobalRankidPairList* > &recv_infos);
    void addReieveMpiRankInfo(
                    const std::map< int, UdmGlobalRankidPairList* > &send_infos,
                    const std::map< int, UdmGlobalRankidPairList* > &recv_infos);
    UdmGlobalRankidPair* findBoundaryInfo(
                    const std::map<int, UdmGlobalRankidPairList* > &boundary_infos,
                    const UdmGlobalRankidPair& recv_infos);
    UdmError_t updateMpiRankInfo(
                    const std::map< int, UdmGlobalRankidPairList* > &recv_infos);
    unsigned int createSendVirtualCells(std::map< int, std::vector<UdmCell*> > &parent_cells) const;
    unsigned int getCellsBufferSize(const std::map< int, std::vector<UdmCell*> > &parent_cells, int *buf_sizes) const;
    unsigned int createCellsBuffer(const std::vector<UdmCell*> &parent_cells, char *buf, size_t buf_length) const;
    UdmError_t createImportVirturalCells(
                    int num_cells, char *buf, size_t buf_length,
                    std::vector<UdmCell*>& import_virtuals);
    bool existsNode(int rankno, UdmSize_t node_id) const;

    // MPI
    MPI_Comm getMpiComm() const;
    int getMpiRankno() const;
    int getMpiProcessSize() const;
    UdmError_t mpi_sendrecvBoundaryInfos(
                    const std::map< int, UdmGlobalRankidPairList* > &send_infos,
                    std::map< int, UdmGlobalRankidPairList* > &recv_infos) const;
    bool isBoundaryNode(const UdmNode* node) const;

    // ランク番号,ID検索テーブル
    void addSearchTable(UdmNode* node);
    void removeSearchTable(UdmNode* node);
};


} /* namespace udm */

#endif /* _UDMRANKCONNECTIVITY_H_ */
