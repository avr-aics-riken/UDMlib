/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmRankConnectivity.cpp
 * 内部境界管理クラスのヘッダーファイル
 */

#include "model/UdmRankConnectivity.h"
#include "model/UdmModel.h"
#include "model/UdmZone.h"
#include "model/UdmGridCoordinates.h"
#include "model/UdmCell.h"
#include "model/UdmSolid.h"
#include "model/UdmNode.h"
#include "utils/UdmScannerCells.h"
#include "utils/UdmSearchTable.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmRankConnectivity::UdmRankConnectivity() : parent_zone(NULL)
{
    // 初期化を行う.
    this->initialize();
}

/**
 * コンストラクタ
 * @param  parent_zone    親ゾーン
 */
UdmRankConnectivity::UdmRankConnectivity(UdmZone *parent_zone)
{
    this->initialize();
    this->parent_zone = parent_zone;
}

/**
 * デストラクタ
 */
UdmRankConnectivity::~UdmRankConnectivity()
{
    this->boundary_nodes.clear();
}

/**
 * 初期化を行う.
 */
void UdmRankConnectivity::initialize(void)
{
    this->boundary_nodes.clear();
    this->search_table.clear();
}

/**
 * 内部境界ノード数を取得する.
 * @return        内部境界ノード
 */
UdmSize_t UdmRankConnectivity::getNumBoundaryNodes() const
{
    return this->boundary_nodes.size();
}

/**
 * 内部境界ノードを追加する.
 * @param node        内部境界ノードリスト.
 * @return        内部境界ノード数
 */
UdmSize_t UdmRankConnectivity::insertRankConnectivityNode(UdmNode* node)
{
    if (node == NULL) return this->boundary_nodes.size();

    UdmSize_t node_id = node->getId();
    UdmSize_t last_node_id = 0;
    UdmNode *last = NULL;
    bool last_insert = false;
    if (this->boundary_nodes.size() > 0) {
        last = *(this->boundary_nodes.end()-1);
        if (last->compareGlobalId(node) < 0) {
            last_insert = true;
        }
    }
    else {
        last_insert = true;
    }

    if (last_insert == true) {
        // 最終に追加する.
        this->boundary_nodes.push_back(node);
        // this->addSearchTable(node);
        return this->boundary_nodes.size();
    }

    // 同一節点（ノード）が存在するかチェックする.
    std::vector<UdmNode*>::const_iterator itr;
    std::vector<UdmNode*>::iterator ins_itr;
    itr = this->searchGlobalId(this->boundary_nodes, node);
    if (itr != this->boundary_nodes.end()) {
        // 同一節点（ノード）が存在するので追加しない
        return this->boundary_nodes.size();
    }

    itr = this->searchUpperGlobalId(this->boundary_nodes, node);
    if (itr == this->boundary_nodes.end()) {
        this->boundary_nodes.push_back(node);
    }
    else {
        ins_itr = this->boundary_nodes.begin() + (itr - this->boundary_nodes.begin());
        this->boundary_nodes.insert(ins_itr, node);
    }

    // this->addSearchTable(node);

    return this->boundary_nodes.size();
}

/**
 * 内部境界ノードを取得する.
 * @param bnd_id        内部境界ノードID（１～）
 * @return        内部境界ノード
 */
UdmNode* UdmRankConnectivity::getBoundaryNode(UdmSize_t bnd_id)
{
    if (bnd_id <= 0) return NULL;
    if (bnd_id > this->boundary_nodes.size()) return NULL;
    return this->boundary_nodes[bnd_id-1];
}

/**
 * 内部境界ノードを取得する:const.
 * @param bnd_id        内部境界ノードID（１～）
 * @return        内部境界ノード
 */
const UdmNode* UdmRankConnectivity::getBoundaryNode(UdmSize_t bnd_id) const
{
    if (bnd_id <= 0) return NULL;
    if (bnd_id > this->boundary_nodes.size()) return NULL;
    return this->boundary_nodes[bnd_id-1];
}

/**
 * 内部境界ノードリストをクリアする.
 */
void UdmRankConnectivity::clear()
{
    std::vector<UdmNode*>::iterator itr;
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        node->clearMpiRankInfos();
    }
    this->boundary_nodes.clear();

    // ランク番号,ID検索テーブルをクリアする
    this->clearSearchTable();

    return;
}


/**
 * ノードの内部境界情報をクリアする.
 */
void UdmRankConnectivity::clearMpiRankInfos()
{
    std::vector<UdmNode*>::iterator itr;
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        node->clearMpiRankInfos();
    }

    // ランク番号,ID検索テーブルを再作成する
    this->createSearchTable();

    return;
}


/**
 * 内部境界ノードを削除する.
 * @param node        削除内部境界ノード
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConnectivity::removeBoundaryNode(const UdmEntity* node)
{
    if (node == NULL) return UDM_ERROR;

    UdmSize_t node_id = node->getId();
    std::vector<UdmNode*>::const_iterator itr;
    std::vector<UdmNode*>::iterator rm_itr;
    // 内部境界ノードから検索
    itr = this->searchGlobalId(this->boundary_nodes, node);
    if (itr != this->boundary_nodes.end()) {
        if ((*itr) == node) {
            rm_itr = this->boundary_nodes.begin() + (itr - this->boundary_nodes.begin());
            this->boundary_nodes.erase(rm_itr);
            return UDM_OK;
        }
    }

    return UDM_ERROR;
}

/**
 * MPIランク番号,ローカルIDが一致する節点（ノード）を取得する.
 * @param  rankno        MPIランク番号
 * @param  localid       ローカルID
 * @return        節点（ノード）
 */
UdmNode* UdmRankConnectivity::findMpiRankInfo(int rankno, UdmSize_t localid) const
{
    if (rankno < 0) return NULL;
    if (localid <= 0) return NULL;
#if 0
    if (this->search_table.size() > 0) {
        UdmEntity *search_node = this->search_table.getReferenceEntity(rankno, localid);
        UdmEntity *find_node = UdmNode::findMpiRankInfo(this->boundary_nodes, rankno, localid);
        if (search_node != find_node) {
            UdmSize_t search_id = 0;
            int search_rankno = -1;
            UdmSize_t find_id = 0;
            int find_rankno = -1;
            if (search_node != NULL) {
                search_id = search_node->getId();
                search_rankno = search_node->getMyRankno();
            }
            if (find_node != NULL) {
                find_id = find_node->getId();
                find_rankno = find_node->getMyRankno();
            }
            printf("src=%d[%d] -> search=%d[%d] != find=%d[%d]\n",
                    localid, rankno, search_id, search_rankno, find_id, find_rankno);
        }
    }
#endif

    if (this->search_table.size() > 0) {
        UdmEntity *node = this->search_table.getReferenceEntity(rankno, localid);
        return (UdmNode*)node;
    }
    else {
        return UdmNode::findMpiRankInfo(this->boundary_nodes, rankno, localid);
    }
}

/**
 * MPIランク番号,ローカルIDが一致する節点（ノード）を取得する.
 * @param  node          検索対象節点（ノード）
 * @return        一致節点（ノード）
 */
UdmNode* UdmRankConnectivity::findMpiRankInfo(const UdmNode* node) const
{
    if (node == NULL) return NULL;
    if (this->boundary_nodes.size() <= 0) return NULL;

    int n;
    int rankno;
    UdmSize_t localid;
    UdmNode *find_node = NULL;

    // 共有節点（ノード）,内部境界情報から検索する.
    int num_rankinfo = node->getNumMpiRankInfos();
    for (n=1; n<=num_rankinfo; n++) {
        node->getMpiRankInfo(n, rankno, localid);
        if (rankno < 0) continue;
        if (localid <= 0) continue;
        find_node = this->findMpiRankInfo(rankno, localid);
        if (find_node != NULL) {
            break;
        }
    }

    return find_node;
}


/**
 * 内部境界情報をすべてのプロセス間に送受信を行い、内部境界情報をマージする.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConnectivity::migrationBoundary()
{
#ifdef _DEBUG_TRACE
    UDM_DEBUG_PRINTF("%s:%d [%s]",  __FILE__, __LINE__, __FUNCTION__);
#endif
#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START(__FUNCTION__);
#endif

    const int TRANS_IDS_SIZE = 4;
    UdmError_t error = UDM_OK;
    // 内部境界数を送受信する
    MPI_Comm comm = this->getMpiComm();
    int num_process = this->getMpiProcessSize();
    int my_rankno = this->getMpiRankno();
    // int num_sends[num_process];
    // int num_recvs[num_process];
    int *num_sends = new int[num_process];
    int *num_recvs = new int[num_process];
    memset(num_sends, 0x00, num_process*sizeof(int));
    memset(num_recvs, 0x00, num_process*sizeof(int));


#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        std::stringstream stream;
        int conn_size;
        MPI_Comm_size(comm, &conn_size);
        stream << "migrationBoundary::RankConnectivity(begin) rank=" << this->getMpiRankno() << std::endl;
        UDM_DEBUG_PRINTF(stream.str().c_str());
        std::string buf;
        this->toString(buf);
        UDM_DEBUG_PRINTF(buf.c_str());
    }
#endif

    // ランク番号、ID検索テーブルを作成する。
    this->createSearchTable();

    // ランク番号別の内部境界情報を取得する
    UdmSize_t n, m, i;
    UdmSize_t num_mpiinfo;
    int num_previousinfo;
    int mpi_rankno;
    UdmSize_t node_id;
    UdmSize_t mpi_localid;
    std::vector<UdmNode*>::iterator itr;
    std::map< int, UdmGlobalRankidPairList* > send_infos;
    std::map< int, UdmGlobalRankidPairList* > recv_infos;
    std::map<int, UdmGlobalRankidPairList* >::const_iterator infos_itr;

    // 送信情報の作成
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        num_mpiinfo = node->getNumMpiRankInfos();
        for (n=1; n<=num_mpiinfo; n++) {
            node->getMpiRankInfo(n, mpi_rankno, mpi_localid);
            if (mpi_rankno < 0) continue;
            // 内部境界データの追加：内部境界データ={自node_id, 相手ランクのlocalid}
            // 挿入送受信情報 = {以前のランク番号,ID, 接続ランク番号,接続ID}
            num_previousinfo = node->getNumPreviousRankInfos();
            for (m=1; m<=num_previousinfo; m++) {
                int send_rankno;
                UdmSize_t send_nodeid;
                node->getPreviousRankInfo(m, send_rankno, send_nodeid);
                if (send_nodeid <= 0) {
                    send_nodeid = node->getId();
                    send_rankno = node->getMyRankno();
                }
                UdmGlobalRankidPair info(send_rankno, send_nodeid, mpi_rankno, mpi_localid);
                this->insertBoundaryInfo(send_infos, mpi_rankno, info);
            }
        }
        // std::vector<UdmGlobalRankidPair>::iterator info_itr;
        // for (info_itr=mpi_infos.begin();info_itr!=mpi_infos.end(); info_itr++) {
        //     mpi_rankno = (*info_itr).getDestRankno();
        //     this->insertBoundaryInfo(send_infos, mpi_rankno, *info_itr);
        //}
    }

    // ID変更情報を送信する.
    error = this->mpi_sendrecvBoundaryInfos(send_infos, recv_infos);
    if (error != UDM_OK) {
        delete []num_sends; delete []num_recvs;
        return UDM_ERRORNO_HANDLER(error);
    }

    // 内部境界の節点（ノード）に内部境界情報を追加する
    this->addReieveMpiRankInfo(send_infos, recv_infos);

    // 内部境界ではない内部境界情報を削除する
    //this->removeRankConnectivity(send_infos, recv_infos);

    // 内部境界でない節点（ノード）を削除する
    // this->eraseRankConnectivity();

#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        std::stringstream stream;
        stream << "migrationBoundary::RankConnectivity(before transferUpdatedIds) rank=" << this->getMpiRankno() << std::endl;
        UDM_DEBUG_PRINTF(stream.str().c_str());
        std::string buf;
        this->toString(buf);
        UDM_DEBUG_PRINTF(buf.c_str());
    }
#endif

    // 内部境界を送受信する
    error = this->transferRankConnectivity();
    if (error != UDM_OK) {
        error =  UDM_ERRORNO_HANDLER(error);
    }

#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        std::stringstream stream;
        stream << "migrationBoundary::RankConnectivity(finish) rank=" << this->getMpiRankno() << std::endl;
        UDM_DEBUG_PRINTF(stream.str().c_str());
        std::string buf;
        this->toString(buf);
        UDM_DEBUG_PRINTF(buf.c_str());
    }
#endif

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_STOP(__FUNCTION__);
    UdmSize_t send_size = 0, recv_size = 0;
    for (infos_itr=send_infos.begin(); infos_itr!=send_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList* rank_infos = infos_itr->second;
        send_size += rank_infos->getCommSize();
    }
    for (infos_itr=recv_infos.begin(); infos_itr!=recv_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList* rank_infos = infos_itr->second;
        recv_size += rank_infos->getCommSize();
    }
    char info[128] = {0x00};
    sprintf(info, "send_size=%ld,recv_size=%ld", send_size, recv_size);
    UDM_STOPWATCH_INFORMATION(__FUNCTION__, info);
#endif

    for (infos_itr=send_infos.begin(); infos_itr!=send_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList* rank_infos = infos_itr->second;
        if (rank_infos != NULL) delete rank_infos;
    }
    for (infos_itr=recv_infos.begin(); infos_itr!=recv_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList* rank_infos = infos_itr->second;
        if (rank_infos != NULL) delete rank_infos;
    }

    delete []num_sends; delete []num_recvs;
    return UDM_OK;
}

/**
 * MPIコミュニケータを取得する.
 * @return        MPIコミュニケータ
 */
MPI_Comm UdmRankConnectivity::getMpiComm() const
{
    if (this->getParentZone() == NULL) return MPI_COMM_NULL;
    return this->getParentZone()->getMpiComm();
}


/**
 * MPIランク番号を取得する.
 * @return        MPIランク番号
 */
int UdmRankConnectivity::getMpiRankno() const
{
    if (this->getParentZone() == NULL) return -1;
    return this->getParentZone()->getMpiRankno();
}

/**
 * MPIプロセス数を取得する.
 * @return        MPIプロセス数
 */
int UdmRankConnectivity::getMpiProcessSize() const
{
    if (this->getParentZone() == NULL) return -1;
    return this->getParentZone()->getMpiProcessSize();
}

/**
 * 親ゾーンを取得する.
 * @return        親ゾーン
 */
UdmZone* UdmRankConnectivity::getParentZone() const
{
    return this->parent_zone;
}

/**
 * 親ゾーンを設定する.
 * @param zone        親ゾーン
 */
void UdmRankConnectivity::setParentZone(UdmZone* zone)
{
    parent_zone = zone;
}

/**
 * 内部境界送受信情報を挿入する.
 * @param [out] infos            挿入内部境界送受信情報
 * @param [in]  rankno           挿入のランク番号
 * @param [in]  info        内部境界送受信情報
 */
void UdmRankConnectivity::insertBoundaryInfo(
                        std::map< int, UdmGlobalRankidPairList* > &infos,
        int rankno,
        const UdmGlobalRankidPair  &info) const
{
    // ランク番号の作成
    if (infos.find(rankno) == infos.end()) {
        UdmGlobalRankidPairList *ids = new UdmGlobalRankidPairList();
        infos.insert(std::make_pair(rankno, ids));
    }
    // ランク番号別に挿入する.
    UdmGlobalRankidPairList* bnd_infos = infos[rankno];
    bnd_infos->addGlobalRankidPair(info);

    return;
}

/**
 * 内部境界送受信情報リストを挿入する.
 * 挿入ランク番号が存在する場合は、既存内部境界送受信情報リストの末尾に追加する.
 * @param [out] info_map            挿入内部境界送受信情報
 * @param [in]  rankno           挿入のランク番号
 * @param [in]  infos        内部境界送受信情報リスト
 */
void UdmRankConnectivity::insertBoundaryInfos(
        std::map< int, UdmGlobalRankidPairList* > &info_map,
        int rankno,
        UdmGlobalRankidPairList  *infos) const
{
    // ランク番号の作成
    if (info_map.find(rankno) == info_map.end()) {
        info_map.insert(std::make_pair(rankno, infos));
        return;
    }

    // ランク番号別に挿入する.
    UdmGlobalRankidPairList* bnd_infos = info_map[rankno];
    bnd_infos->addGlobalRankidPairs(*infos);
    return;
}

/**
 * 内部境界ではない節点（ノード）から内部境界情報を削除する.
 * 自内部境界情報の内、収集内部境界情報に含まれないものは、内部境界ではない。
 * @param send_infos        自内部境界情報
 * @param recv_infos        収集内部境界情報
 */
void UdmRankConnectivity::removeRankConnectivity(
        const std::map<int, UdmGlobalRankidPairList* > &send_infos,
        const std::map<int, UdmGlobalRankidPairList* > &recv_infos)
{
    UdmGlobalRankidPairList delete_infos;        // 送信内部境界
    std::map<int, UdmGlobalRankidPairList* >::const_iterator mpi_itr;
    std::map<int, UdmGlobalRankidPairList* >::const_iterator find_itr;
    std::vector<UdmGlobalRankidPair>::iterator info_itr;

    UdmSize_t array_size = 0;
    for (mpi_itr=send_infos.begin(); mpi_itr!=send_infos.end(); mpi_itr++) {
        array_size += mpi_itr->second->size();
    }
    delete_infos.reserve(array_size);

    // 自分の内部境界節点（ノード）の内、受信しなかった節点（ノード）を検索する
    for (mpi_itr=send_infos.begin(); mpi_itr!=send_infos.end(); mpi_itr++) {
        int mpi_rankno = mpi_itr->first;
        UdmGlobalRankidPairList *inner_infos = mpi_itr->second;
        for (info_itr=inner_infos->begin(); info_itr!=inner_infos->end(); info_itr++) {
            UdmGlobalRankidPair send_info = *info_itr;
            UdmGlobalRankidPair *found_info = this->findBoundaryInfo(recv_infos, send_info);

            if (found_info == NULL) {
                delete_infos.addGlobalRankidPair(send_info);
            }
            else if (found_info->getSrcRankno() == found_info->getDestRankno()) {
                delete_infos.addGlobalRankidPair(send_info);
            }
        }
    }

    int i, n;
    int del_size = delete_infos.size();
    int node_size = this->boundary_nodes.size();
    if (del_size <= 0) return;
    if (node_size <= 0) return;

    UdmError_t error = UDM_OK;

    std::vector<UdmNode*>::const_iterator found_itr;
    for (info_itr=delete_infos.begin(); info_itr!=delete_infos.end(); info_itr++) {
        UdmGlobalRankidPair info = (*info_itr);

        // 内部境界を削除する.
        UdmNode *found_node = this->findMpiRankInfo(info.getSrcRankno(), info.getSrcLocalid());
        if (found_node != NULL) {
            error = found_node->removeMpiRankInfo(info.getDestRankno(), info.getDestLocalid());
        }
    }

    return;
}

/**
 * 内部境界の節点（ノード）に内部境界情報を追加する.
 * 自内部境界情報に収集内部境界情報と一致する節点（ノード）に対し内部境界情報を設定する.
 * @param send_infos        自内部境界情報
 * @param recv_infos        収集内部境界情報
 */
void UdmRankConnectivity::addReieveMpiRankInfo(
        const std::map<int, UdmGlobalRankidPairList* > &send_infos,
        const std::map<int, UdmGlobalRankidPairList* > &recv_infos)
{
    std::map<int, UdmGlobalRankidPairList* >::const_iterator infos_itr;
    int recv_rankno;
    UdmSize_t recv_localid;

    // 自分の内部境界節点（ノード）の内、受信した節点（ノード）を検索する
    for (infos_itr=recv_infos.begin(); infos_itr!=recv_infos.end(); infos_itr++) {
        int rankno = infos_itr->first;
        UdmGlobalRankidPairList *recv_rank_infos = infos_itr->second;
        std::vector<UdmGlobalRankidPair>::iterator recv_itr;
        for (recv_itr=recv_rank_infos->begin(); recv_itr!=recv_rank_infos->end(); recv_itr++) {
            recv_rankno = recv_itr->getSrcRankno();
            recv_localid = recv_itr->getSrcLocalid();
            UdmNode *node = this->findMpiRankInfo(recv_rankno, recv_localid);
            if (node != NULL) {
                UdmSize_t node_id = node->getId();
                node->addMpiRankInfo(recv_itr->getSrcRankno(), recv_itr->getSrcLocalid());
            }
        }
    }

    return;
}

/**
 * 内部境界がない節点（ノード）を削除する.
 */
void UdmRankConnectivity::eraseRankConnectivity()
{
    int size = this->boundary_nodes.size();
    if (size <= 0) return;

//    std::vector<UdmNode*>::reverse_iterator rev_itr;
//    for (rev_itr=this->boundary_nodes.rbegin(); rev_itr!=this->boundary_nodes.rend(); rev_itr++) {
    std::vector<UdmNode*>::iterator itr;
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); ) {
        UdmNode *node = (*itr);
        UdmSize_t node_id = node->getId();
        int rankno = node->getMyRankno();

        /*
        // MPIランク番号、ローカル番号が不正な情報を削除する.
        // UdmLoadBalance::zoltan_pack_obj_multiで転送先プロセス番号を仮挿入したものを削除する.
        node->eraseInvalidMpiRankInfos();
         */
        node->removeMpiRankInfo(node->getMyRankno(), node->getId());

        if (node->getNumMpiRankInfos() <= 0) {
            itr = this->boundary_nodes.erase( itr );
            this->removeSearchTable(node);
            continue;
        }

        // 内部境界節点（ノード）は親要素が１つのみの部品要素の構成節点（ノード）である。
        if (!this->isBoundaryNode(node)) {
            // 内部境界情報のクリア
            node->clearMpiRankInfos();
            // 内部境界節点（ノード）ではないので削除する.
            itr = this->boundary_nodes.erase( itr );
            this->removeSearchTable(node);
            continue;
        }
        itr++;
    }

    return;
}

/**
 * 内部境界情報リストから一致する検索内部境界情報が存在するかチェックする.
 * 内部境界情報リストは{node_id, mpi_rankno, mpi_localid}にてソートされているものとする。
 * @param boundary_infos        内部境界情報リスト
 * @param find_info            検索内部境界情報
 * @return        true=存在する
 */
UdmGlobalRankidPair* UdmRankConnectivity::findBoundaryInfo(
        const std::map<int, UdmGlobalRankidPairList* > &boundary_infos,
        const UdmGlobalRankidPair& find_info)
{
    std::map<int, UdmGlobalRankidPairList* >::const_iterator mpi_itr;
    std::vector<UdmGlobalRankidPair>::iterator info_itr;

    for (mpi_itr=boundary_infos.begin(); mpi_itr!=boundary_infos.end(); mpi_itr++) {
        int mpi_rankno = mpi_itr->first;
        UdmGlobalRankidPairList* inner_infos = mpi_itr->second;
        for (info_itr=inner_infos->begin(); info_itr!=inner_infos->end(); info_itr++) {
            // {my_rankno,node_id}と{mpi_rankno,mpi_localid}の相互比較
            if (info_itr->match(find_info.getSrcRankno(), find_info.getSrcLocalid())) return &(*info_itr);
            if (info_itr->match(find_info.getDestRankno(), find_info.getDestLocalid())) return &(*info_itr);
        }
    }

    return NULL;
}

/**
 * 内部境界情報を文字列として出力する.
 * @param buf        出力文字列
 */
void UdmRankConnectivity::toString(std::string &buf) const
{
    int n;
    char tmp[128];
    std::string mpi_buf;
    std::string previous_buf;
    std::vector<UdmNode*>::const_iterator itr;
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        const UdmGlobalRankidList mpi_infos = node->getMpiRankInfos();
        const UdmGlobalRankidList previous_infos = node->getPreviousRankInfos();
        mpi_buf.clear();
        previous_buf.clear();
        mpi_infos.toString(mpi_buf);
        previous_infos.toString(previous_buf);
        sprintf(tmp, "rankinfo : %ld[%d] <= %s : mpi_rank=%s\n",
                node->getId(), node->getMyRankno(),
                previous_buf.c_str(),
                mpi_buf.c_str());
        buf += tmp;
    }
    return;
}

/**
 * 以前のランク番号、IDを転送して自身の内部境界情報を更新する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConnectivity::transferUpdatedIds()
{
#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        UDM_DEBUG_PRINTF("%s:%d [%s]",  __FILE__, __LINE__, __FUNCTION__);
        std::string buf;
        this->toString(buf);
        UDM_DEBUG_PRINTF(buf.c_str());
    }
#endif

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START(__FUNCTION__);
#endif

    const int TRANS_IDS_SIZE = 4;
    UdmError_t error = UDM_OK;
    MPI_Comm comm = this->getMpiComm();
    int num_process = this->getMpiProcessSize();
    int my_rankno = this->getMpiRankno();
    // int num_sends[num_process];
    // int num_recvs[num_process];
    int *num_sends = new int[num_process];
    int *num_recvs = new int[num_process];
    memset(num_sends, 0x00, num_process*sizeof(int));
    memset(num_recvs, 0x00, num_process*sizeof(int));

    // ランク番号別の変更ランク番号、IDを取得する
    int n, m, i;
    int num_mpiinfo;
    int num_previousinfo;
    int rankno;
    UdmSize_t localid;
    UdmSize_t node_id;
    UdmSize_t previous_id;
    int previous_rankno;
    std::vector<UdmNode*>::iterator itr;
    // 送受信ランク情報 = {送受信ランク番号, 接続ランク情報}
    std::map< int, UdmGlobalRankidPairList* > send_infos;
    std::map< int, UdmGlobalRankidPairList* > recv_infos;
    std::map< int, UdmGlobalRankidPairList* >::iterator infos_itr;

    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        if (!node->getNumPreviousRankInfos()) continue;    // 変更なし
        my_rankno = node->getMyRankno();
        node_id = node->getId();
        num_previousinfo = node->getNumPreviousRankInfos();
        for (n=1; n<=num_previousinfo; n++) {
            node->getPreviousRankInfo(n, previous_rankno, previous_id);
            if (my_rankno == previous_rankno && node_id == previous_id) continue;

            // 送受信ID変更情報 = {現在のランク番号,ID, 以前のランク番号,ID}
            UdmGlobalRankidPair info(my_rankno, node_id, previous_rankno, previous_id);

            // 送信先
            num_mpiinfo = node->getNumMpiRankInfos();
            for (m=1; m<=num_mpiinfo; m++) {
                node->getMpiRankInfo(m, rankno, localid);
                this->insertBoundaryInfo(send_infos, rankno, info);
            }
        }
    }

    // ID変更情報を送信する.
    error = this->mpi_sendrecvBoundaryInfos(send_infos, recv_infos);
    if (error != UDM_OK) {
        delete []num_sends; delete []num_recvs;
        return UDM_ERRORNO_HANDLER(error);
    }

    // ID変更情報を反映する
    error = this->updateMpiRankInfo(recv_infos);

#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        UDM_DEBUG_PRINTF("%s:%d [%s] : after updateMpiRankInfo ",  __FILE__, __LINE__, __FUNCTION__);
        std::string buf;
        this->toString(buf);
        UDM_DEBUG_PRINTF(buf.c_str());
    }
#endif

// 以前のIDをクリアする
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        node->clearPreviousInfos();
    }

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_STOP(__FUNCTION__);
    UdmSize_t send_size = 0, recv_size = 0;
    for (infos_itr=send_infos.begin(); infos_itr!=send_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList *rank_infos = infos_itr->second;
        send_size += rank_infos->getCommSize();
    }
    for (infos_itr=recv_infos.begin(); infos_itr!=recv_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList *rank_infos = infos_itr->second;
        recv_size += rank_infos->getCommSize();
    }
    char info[128] = {0x00};
    sprintf(info, "send_size=%ld,recv_size=%ld", send_size, recv_size);
    UDM_STOPWATCH_INFORMATION(__FUNCTION__, info);
#endif

    for (infos_itr=send_infos.begin(); infos_itr!=send_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList* rank_infos = infos_itr->second;
        if (rank_infos != NULL) delete rank_infos;
    }
    for (infos_itr=recv_infos.begin(); infos_itr!=recv_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList* rank_infos = infos_itr->second;
        if (rank_infos != NULL) delete rank_infos;
    }

    delete []num_sends; delete []num_recvs;
    return error;
}


/**
 * 仮想要素（セル）情報を転送する.
 * グラフのプロセス間の接続情報である仮想要素（セル）を作成する.
 * @param [out] import_nodes        インポート節点（ノード）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConnectivity::transferVirtualCells(std::vector<UdmNode*>& import_nodes)
{
#ifdef _UDM_PROFILER
    UDM_STOPWATCH_START(__FUNCTION__);
#endif

    UdmError_t error = UDM_OK;
    MPI_Comm comm = this->getMpiComm();
    int num_process = this->getMpiProcessSize();
    int my_rankno = this->getMpiRankno();
    // int num_sends[num_process][2];
    // int num_recvs[num_process][2];
    int *num_sends = new int[num_process*2];
    int *num_recvs = new int[num_process*2];
    memset(num_sends, 0x00, num_process*2*sizeof(int));
    memset(num_recvs, 0x00, num_process*2*sizeof(int));

    // ランク番号別の内部境界要素（セル）を取得する
    int i, n;
    std::vector<UdmNode*>::iterator itr;
    std::map<int, std::vector<UdmCell*> > parent_cells;
    for (n=0; n<num_process; n++) {
        std::vector<UdmCell*> ids;
        parent_cells.insert(std::make_pair(n, ids));
    }

    // 内部境界節点（ノード）の親要素（セル）を探索する.
    this->createSendVirtualCells(parent_cells);

    // 送受信バッファサイズを取得する.
    // int buf_sizes[num_process];
    int *buf_sizes = new int[num_process];
    memset(buf_sizes, 0x00, num_process*sizeof(int));
    this->getCellsBufferSize(parent_cells, buf_sizes);

    // 要素（セル）送信数の取得
    for (n=0; n<num_process; n++) {
        // num_sends[n][0] = parent_cells[n].size();    // 送信要素（セル）数
        // num_sends[n][1] = buf_sizes[n];                // バッファーサイズ
        num_sends[n*2] = parent_cells[n].size();    // 送信要素（セル）数
        num_sends[n*2+1] = buf_sizes[n];                // バッファーサイズ
    }

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START("MPI_Alltoall");
#endif

    // ランク番号別の送受信バッファサイズを送信する.
    udm_mpi_alltoall(num_sends, 2, MPI_INT, num_recvs, 2, MPI_INT, comm);

#ifdef _UDM_PROFILER
    UDM_STOPWATCH_STOP("MPI_Alltoall");
#endif

    // 送受信バッファー作成
    int send_size = 0, recv_size = 0;
    int send_pos = 0, recv_pos = 0;
    for (n=0; n<num_process; n++) {
        // send_size += num_sends[n][1];
        // recv_size += num_recvs[n][1];
        send_size += num_sends[n*2+1];
        recv_size += num_recvs[n*2+1];
    }
    char *send_buf = NULL;
    if (send_size > 0) {
        send_buf = new char[send_size];
        memset(send_buf, 0x00, send_size*sizeof(char));
    }
    char *recv_buf = NULL;
    if (recv_size > 0) {
        recv_buf = new char[recv_size];
        memset(recv_buf, 0x00, recv_size*sizeof(char));
    }

    // 送信データの作成
    send_pos = 0, recv_size = 0;
    for (n=0; n<num_process; n++) {
        if (n == my_rankno) continue;
        // if (num_sends[n][0] == 0) continue;
        // if (num_sends[n][1] == 0) continue;
        if (num_sends[n*2] == 0) continue;
        if (num_sends[n*2+1] == 0) continue;
        if (send_buf == NULL) continue;

        // send_pos += this->createCellsBuffer(parent_cells[n], send_buf + send_pos, num_sends[n][1]);
        send_pos += this->createCellsBuffer(parent_cells[n], send_buf + send_pos, num_sends[n*2+1]);
    }

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START("MPI_Isend-MPI_Irecv");
#endif

    // ランク番号別に要素（セル）を送信する.
    MPI_Status   *send_status = new MPI_Status[num_process];
    MPI_Status   *recv_status = new MPI_Status[num_process];
    MPI_Request  *send_requests = new MPI_Request[num_process];
    MPI_Request  *recv_requests = new MPI_Request[num_process];

    send_pos = 0, recv_size = 0;
    for (n=0; n<num_process; n++) {
        send_requests[n] = MPI_REQUEST_NULL;
        recv_requests[n] = MPI_REQUEST_NULL;
        if (n == my_rankno) continue;
        // if (num_sends[n][0] > 0) {
        if (num_sends[n*2] > 0) {
            if (send_buf != NULL) {
                // udm_mpi_isend(send_buf + send_pos, num_sends[n][1], MPI_CHAR, n, 0, comm, &send_requests[n]);
                udm_mpi_isend(send_buf + send_pos, num_sends[n*2+1], MPI_CHAR, n, 0, comm, &send_requests[n]);
            }
        }

        // if (num_recvs[n][0] > 0) {
        if (num_recvs[n*2] > 0) {
            if (recv_buf != NULL) {
                // udm_mpi_irecv(recv_buf + recv_pos, num_recvs[n][1], MPI_CHAR, n, 0, comm, &recv_requests[n]);
                udm_mpi_irecv(recv_buf + recv_pos, num_recvs[n*2+1], MPI_CHAR, n, 0, comm, &recv_requests[n]);
            }
        }

        // 送受信バッファー位置
        // send_pos += num_sends[n][1];
        // recv_pos += num_recvs[n][1];
        send_pos += num_sends[n*2+1];
        recv_pos += num_recvs[n*2+1];
    }

    udm_mpi_waitall(num_process, send_requests, send_status);
    udm_mpi_waitall(num_process, recv_requests, recv_status);

#ifdef _UDM_PROFILER
    UDM_STOPWATCH_STOP("MPI_Isend-MPI_Irecv");
#endif

    // 受信データ
    recv_pos = 0;
    std::vector<UdmCell*> import_virtuals;
    for (n=0; n<num_process; n++) {
        if (n == my_rankno) continue;
        // if (num_recvs[n][0] == 0) continue;
        if (num_recvs[n*2] == 0) continue;
        if (recv_buf == NULL) continue;

        // 仮想要素（セル）のデシリアライズを行い、仮想要素（セル）を生成する.
        // error = this->createImportVirturalCells(num_recvs[n][0], recv_buf + recv_pos, num_recvs[n][1], import_virtuals);
        error = this->createImportVirturalCells(num_recvs[n*2], recv_buf + recv_pos, num_recvs[n*2+1], import_virtuals);
        if (error != UDM_OK) {
            delete []num_sends; delete []num_recvs;
            delete []buf_sizes;
            return UDM_ERRORNO_HANDLER(error);
        }
        // recv_pos += num_recvs[n][1];
        recv_pos += num_recvs[n*2+1];
    }

    // 仮想要素（セル）の挿入
    if (error == UDM_OK) {
        error = this->getParentZone()->importVirturalCells(import_virtuals, import_nodes);
        if (error != UDM_OK) {
            UDM_ERRORNO_HANDLER(error);
        }
    }

    // デシリアライズ要素（セル）の削除
    int cell_size = import_virtuals.size();
    for (n=0; n<cell_size; n++) {
        UdmCell::freeDeserialize(import_virtuals[n]);
    }
    import_virtuals.clear();

    if (send_buf != NULL) delete []send_buf;
    if (recv_buf != NULL) delete []recv_buf;
    if (send_status != NULL) delete []send_status;
    if (recv_status != NULL) delete []recv_status;
    if (send_requests != NULL) delete []send_requests;
    if (recv_requests != NULL) delete []recv_requests;
    delete []num_sends; delete []num_recvs;
    delete []buf_sizes;

#ifdef _UDM_PROFILER
    UDM_STOPWATCH_STOP(__FUNCTION__);
    char info[128] = {0x00};
    sprintf(info, "send_size=%d,recv_size=%d", send_size, recv_size);
    UDM_STOPWATCH_INFORMATION(__FUNCTION__, info);
#endif

    return error;
}


/**
 * 受信ID変更情報から内部境界のMPI情報を更新する.
 * @param recv_infos        受信ID変更情報
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConnectivity::updateMpiRankInfo(
        const std::map<int, UdmGlobalRankidPairList* >& recv_infos)
{
    if (recv_infos.size() <= 0) return UDM_OK;
    if (this->boundary_nodes.size() <= 0) return UDM_OK;

    UdmError_t error = UDM_OK;
    std::vector<UdmNode*>::iterator itr;
    std::vector<UdmGlobalRankidPair>::iterator info_itr;
    std::map< int, UdmGlobalRankidPairList* >::const_iterator recv_itr;
    UdmGlobalRankidPairList update_ids;
    std::vector<UdmNode*> update_nodes;

    int recv_rankno;
    int n;
#if 0
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        for (recv_itr=recv_infos.begin();recv_itr!=recv_infos.end(); recv_itr++) {
            recv_rankno = recv_itr->first;
            UdmGlobalRankidPairList *infos = recv_itr->second;
            for (info_itr=infos->begin(); info_itr!=infos->end(); info_itr++) {
                UdmGlobalRankidPair info = *info_itr;
                if (node->existsMpiRankInfo(info.getDestRankno(), info.getDestLocalid())) {
                    update_nodes.push_back(node);
                    update_ids.pushbackGlobalRankidPair(info);
                }
            }
        }
    }
#endif

    // ランク番号,IDの検索テーブルの作成
    UdmSearchTable search_table;
    search_table.createMpiRankidTable(this->boundary_nodes);

    for (recv_itr=recv_infos.begin();recv_itr!=recv_infos.end(); recv_itr++) {
        recv_rankno = recv_itr->first;
        UdmGlobalRankidPairList *infos = recv_itr->second;
        for (info_itr=infos->begin(); info_itr!=infos->end(); info_itr++) {
            UdmGlobalRankidPair info = *info_itr;
            UdmNode *node = search_table.findMpiRankInfo(info.getDestRankno(), info.getDestLocalid());
            if (node != NULL) {
                update_nodes.push_back(node);
                update_ids.pushbackGlobalRankidPair(info);
            }
        }
    }

    // 節点（ノード）のMPI情報を更新する.
    int update_size = update_nodes.size();
    for (n=0; n<update_size; n++) {
        UdmNode* node = update_nodes[n];
        UdmGlobalRankidPair info;
        update_ids.getGlobalRankidPair(n+1, info);
        node->updateMpiRankInfo(info.getDestRankno(), info.getDestLocalid(), info.getSrcRankno(), info.getSrcLocalid());
    }

    return UDM_OK;
}


/**
 * CGNSファイルから内部境界情報の読込みを行う.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConnectivity::readCgns(int index_file, int index_base, int index_zone)
{
    int *info_array;
    int i, n, num_mpiinfo;
    UdmSize_t my_rankno;
    UdmSize_t node_id;
    int mpi_rankno;
    UdmSize_t mpi_localid;
    UdmGlobalRankidPairList mpi_infos;
    std::vector<UdmGlobalRankidPair>::iterator info_itr;

    UdmGridCoordinates *grid = this->getGridCoordinates();
    if (grid == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "GridCoordinates is null");
    }

    // CGNS:UserDefinedData[@name="UdmRankConnectivity"]
    if (cg_goto(index_file, index_base, "Zone_t", index_zone,
            UDM_CGNS_NAME_RANKCONNECTIVITY, 0, "end") != CG_OK) {
        return UDM_OK;
        //        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO,
        //                        "index_file=%d,index_base=%d,index_zone=%d,%s",
        //                        index_file, index_base, index_zone,
        //                        UDM_CGNS_NAME_RANKCONNECTIVITY);
    }

    // 内部境界情報の読込
    DataType_t cgns_datatype;
    int dimension;
    cgsize_t dimension_vector[3];
    char array_name[33];
    int num_arrays, len = 0, index_array = 0;
    cg_narrays(&num_arrays);
    for (n=1; n<=num_arrays; n++) {
        if (cg_array_info(n, array_name, &cgns_datatype, &dimension, dimension_vector) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_info(%d)", n);
        }
        if (dimension != 2) continue;
        if (dimension_vector[0] != 4) continue;
        if (strncmp(array_name, UDM_CGNS_NAME_RANKCONNECTIVITY_ARRAY, strlen(UDM_CGNS_NAME_RANKCONNECTIVITY_ARRAY)) == 0) {
            index_array = n;
            break;
        }
    }
    if (index_array <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "not found %s", UDM_CGNS_NAME_RANKCONNECTIVITY_ARRAY);
    }

    // 読込バッファの作成
    len = 1;
    for (i=0; i<dimension; i++) len*=dimension_vector[i];
    info_array = new int[len];
    memset(info_array, 0x00, len*sizeof(int));

    // データの読込
    if (cg_array_read_as(index_array, Integer, info_array) != CG_OK) {
        delete[] info_array;
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_read_as(%d=%s)", n, UDM_CGNS_NAME_RANKCONNECTIVITY_ARRAY);
    }
    n=0;
    mpi_infos.reserve(dimension_vector[1]);
    for (i=0; i<dimension_vector[1]; i++) {
        my_rankno = info_array[n++];
        node_id = info_array[n++];
        mpi_rankno = info_array[n++];
        mpi_localid = info_array[n++];
        // 内部境界情報
        UdmGlobalRankidPair info(my_rankno, node_id, mpi_rankno, mpi_localid);
        mpi_infos.addGlobalRankidPair(info);
    }

    // 内部境界情報の設定
    for (info_itr=mpi_infos.begin(); info_itr!=mpi_infos.end(); info_itr++) {
        UdmGlobalRankidPair info = (*info_itr);
        my_rankno = info.getSrcRankno();
        node_id = info.getSrcLocalid();
        UdmNode* node = grid->findNodeByGlobalId(my_rankno, node_id);
        if (node == NULL) {
            delete[] info_array;
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "not found Node(my_rankno=%d,node_id=%d)", my_rankno, node_id);
        }

        // 内部境界情報の設定
        node->addMpiRankInfo(info.getDestRankno(), info.getDestLocalid());

        // 内部境界に追加する
        if (node->getNumMpiRankInfos() > 0) {
            this->insertRankConnectivityNode(node);
        }
    }

    delete[] info_array;

    return UDM_OK;
}

/**
 * CGNSファイルに内部境界情報を出力する.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConnectivity::writeCgns(int index_file, int index_base, int index_zone)
{
    cgsize_t user_dims[2];
    int *info_array;
    std::vector<UdmNode*>::iterator itr;
    int n, num_mpiinfo;
    int rankno;
    UdmSize_t localid;
    UdmGlobalRankidPairList mpi_infos;
    std::vector<UdmGlobalRankidPair>::iterator info_itr;

    // 出力内部境界データのチェック
    if (this->boundary_nodes.size() <= 0) {
        return UDM_OK;
    }

    // "UdmRankConnectivity" : 内部境界情報
    if (cg_goto(index_file, index_base, "Zone_t", index_zone, "end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO, "index_file=%d,index_base=%d,index_zone=%d", index_file, index_base, index_zone);
    }

    // CGNS:UserDefinedData[@name="UdmRankConnectivity"]の作成 : cg_user_data_write
    if (cg_user_data_write(UDM_CGNS_NAME_RANKCONNECTIVITY) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_user_data_write(%s)", UDM_CGNS_NAME_RANKCONNECTIVITY);
    }
    if (cg_goto(index_file, index_base, "Zone_t", index_zone, UDM_CGNS_NAME_RANKCONNECTIVITY, 0, "end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO, "index_file=%d,index_base=%d,index_zone=%d", index_file, index_base, index_zone);
    }

    // 内部境界情報の取得
    UdmSize_t array_size = 0;
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        array_size += node->getNumMpiRankInfos();
    }
    mpi_infos.reserve(array_size);

    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        num_mpiinfo = node->getNumMpiRankInfos();
        for (n=1; n<=num_mpiinfo; n++) {
            node->getMpiRankInfo(n, rankno, localid);
            // 内部境界情報
            UdmGlobalRankidPair info(node->getMyRankno(), node->getId(), rankno, localid);
            mpi_infos.addGlobalRankidPair(info);
        }
    }
    if (mpi_infos.size() <= 0) {
        return UDM_OK;
    }

    // 出力データ数
    user_dims[0] = 4;
    user_dims[1] = mpi_infos.size();
    info_array = new int[user_dims[0]*user_dims[1]];
    memset(info_array, 0x00, user_dims[0]*user_dims[1]*sizeof(int));

    // 出力データ={ノードID,ランク番号,ローカル番号}
    n=0;
    for (info_itr=mpi_infos.begin(); info_itr!=mpi_infos.end(); info_itr++) {
        UdmGlobalRankidPair  info = (*info_itr);
        info_array[n++] = info.getSrcRankno();
        info_array[n++] = info.getSrcLocalid();
        info_array[n++] = info.getDestRankno();
        info_array[n++] = info.getDestLocalid();
    }
    // CGNS:DataArray[@name="RankConnectivity"]の作成 : cg_array_write
    if (cg_array_write( UDM_CGNS_NAME_RANKCONNECTIVITY_ARRAY,
            Integer,
            2,
            user_dims,
            info_array) != CG_OK) {
        delete[] info_array;
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_write(%s)", UDM_CGNS_NAME_RANKCONNECTIVITY_ARRAY);
    }

    delete[] info_array;
    return UDM_OK;
}

/**
 * 内部境界節点（ノード）の親要素（セル）を取得する.
 * 3Dモデルの場合、取得要素（セル）は構成節点（ノード）３つ以上が内部境界節点（ノード）であること。
 * 内部境界の接続先ランク番号別に取得を行う.
 * @param [out] parent_cells        接続先ランク毎の親要素（セル）
 * @return        取得親要素（セル）数
 */
unsigned int UdmRankConnectivity::createSendVirtualCells(std::map<int, std::vector<UdmCell*> >& parent_cells) const
{
    int i, n;
    // 内部境界節点（ノード）の親要素（セル）を探索する.
    std::vector<UdmEntity*> scan_cells;
    std::vector<UdmEntity*>::iterator itr;
    std::vector<UdmNode*>::const_iterator find_itr;
    UdmScannerCells scanner;
    UdmSize_t num_parents = scanner.scannerParents(this->boundary_nodes);
    scanner.getScanEntities(scan_cells);
    if (scan_cells.size() <= 0) return 0;

    int num_process = this->getMpiProcessSize();
    // int rank_counter[num_process];
    int *rank_counter = new int[num_process];
    memset(rank_counter, 0x00, num_process*sizeof(int));

    // セル次元数
    int dimmension = this->getParentZone()->getCellDimension();
    int num_nodes = 0, num_mpi;
    int rankno;
    int found_count = 0;
    UdmSize_t localid;
    for (itr=scan_cells.begin(); itr!=scan_cells.end(); itr++) {
        UdmCell* cell = static_cast<UdmCell*>(*itr);
        if (cell == NULL) continue;
        // 送信要素（セル）は実態要素（セル）のみ
        if (cell->getRealityType() != Udm_Actual) continue;

        UdmSize_t cell_id = cell->getId();
        int cell_rankno = cell->getMyRankno();
        memset(rank_counter, 0x00, num_process*sizeof(int));
        num_nodes = cell->getNumNodes();
        for (n=1; n<=num_nodes; n++) {
            UdmNode *node = cell->getNode(n);
            num_mpi = node->getNumMpiRankInfos();
            if (num_mpi == 0) continue;
            for (i=1; i<=num_mpi; i++) {
                node->getMpiRankInfo(i, rankno, localid);
                if (rankno >= num_process) continue;
                rank_counter[rankno]++;
            }
        }
        // 3点で接している（面で接している）要素（セル）を取得する.
        for (i=0; i<num_process; i++) {
            if (rank_counter[i] >= dimmension) {
                parent_cells[i].push_back(cell);
                found_count++;
            }
        }
    }
    delete []rank_counter;

    return found_count;
}

/**
 * 要素（セル）のバッファサイズ（シリアライズサイズ）を取得する.
 * @param [in]  parent_cells    シリアライズ要素（セル）
 * @param [out] buf_sizes        ランク別バッファサイズ
 * @return            バッファ総数
 */
unsigned int UdmRankConnectivity::getCellsBufferSize(
        const std::map<int, std::vector<UdmCell*> >& parent_cells,
        int* buf_sizes) const
{
    int total_size = 0;
    int num_process = this->getMpiProcessSize();
    std::map<int, std::vector<UdmCell*> >::const_iterator itr;
    std::vector<UdmCell*>::const_iterator cell_itr;
    for (itr=parent_cells.begin(); itr!=parent_cells.end(); itr++) {
        int rankno = itr->first;
        if (rankno >= num_process) continue;
        for (cell_itr=itr->second.begin(); cell_itr!=itr->second.end(); cell_itr++) {
            UdmCell *cell = (*cell_itr);

            UdmSerializeBuffer streamBuffer;        // バッファ(char*)をNULLとする
            UdmSerializeArchive archive(&streamBuffer);
            archive << *cell;
            size_t ser_size = archive.getOverflowSize();
            buf_sizes[rankno] += ser_size;
            total_size += ser_size;
        }
    }
    return total_size;
}

/**
 * 送信データを作成する
 * @param [in]  parent_cells            送信要素（セル）リスト
 * @param [out] buf                    送信バッファー
 * @return        作成バッファーサイズ
 */
unsigned int UdmRankConnectivity::createCellsBuffer(
        const std::vector<UdmCell*>& parent_cells,
        char* buf,
        size_t buf_length) const
{
    UdmSerializeBuffer streamBuffer(buf, buf_length);
    UdmSerializeArchive archive(&streamBuffer);

    std::vector<UdmCell*>::const_iterator cell_itr;
    for (cell_itr=parent_cells.begin(); cell_itr!=parent_cells.end(); cell_itr++) {
        UdmCell *cell = (*cell_itr);
        // シリアライズ
        archive << *cell;

        // 検証
        if (!archive.validateFinish()) {
            return 0;
        }
    }

    return buf_length;

}

/**
 * 受信データからデシリアライズを行い、仮想セル、ノードを作成する.
 * @param [in]  num_cells        仮想セル数
 * @param [in]  buf            受信バッファー
 * @param [in]  buf_length    受信バッファーサイズ
 * @param [out] import_virtuals    受信バッファーサイズ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConnectivity::createImportVirturalCells(
        int num_cells,
        char* buf,
        size_t buf_length,
        std::vector<UdmCell*>& import_virtuals)
{
    if (buf == NULL) return UDM_ERROR;
    if (buf_length <= 0) return UDM_ERROR;
    if (this->getParentZone() == NULL) return UDM_ERROR;

    int n;
    UdmError_t error = UDM_OK;;
    UdmSerializeBuffer streamBuffer(buf, buf_length);
    UdmSerializeArchive archive(&streamBuffer);

    for (n=0; n<num_cells; n++) {

#if 0        // tellg, seekg削除
        // 要素（セル）:現在位置
        // std::iostream::pos_type cur = archive.tellg();
        size_t cur = archive.tellg();

        UdmSize_t id;
        UdmElementType_t element_type;

        // UdmEntity基本情報
        UdmCell::deserializeEntityBase(archive, id, element_type);

        // 要素（セル）の生成
        UdmCell *deser_cell = UdmCell::factoryCell(element_type);
        if (deser_cell == NULL) {
            error = UDM_ERROR_HANDLER(UDM_ERROR_DESERIALIZE, "deserialize can not create cell[n/num_cells=%d/%d].", n, num_cells);
            break;
        }

        // 要素（セル）:現在位置まで戻す。
        archive.seekg(cur);
#endif

        // デシリアライズを行う。UdmSolid固定とする
        UdmCell *deser_cell = new UdmSolid();
        archive >> *deser_cell;

        if (!archive.validateFinish()) {
            error = UDM_ERROR_HANDLER(UDM_ERROR_DESERIALIZE, "abend deserialize [n/num_cells=%d/%d]", n, num_cells);
            break;
        }

        // インポート要素（セル）の追加
        import_virtuals.push_back(deser_cell);
    }

    return error;
}

/**
 * 自ランク番号以下と接続している節点（ノード）を除いた節点（ノード）数を取得する。
 * 節点（ノード）数ー（接続節点数ー自ランク番号以上の接続節点数）
 * @return        自ランク番号以上の接続る節点数
 */
UdmSize_t UdmRankConnectivity::getNumNodesWithoutLessRankno() const
{
    UdmSize_t over_nodes = 0;
    int n = 0, num_mpiinfo;
    int rankno;
    UdmSize_t localid;
    int my_rankno = this->getMpiRankno();

    std::vector<UdmNode*>::const_iterator itr;
    // 内部境界情報の取得
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        num_mpiinfo = node->getNumMpiRankInfos();
        rankno = -1;
        for (n=1; n<=num_mpiinfo; n++) {
            node->getMpiRankInfo(n, rankno, localid);
            // 自ランク番号以下と接続している節点（ノード）は除く
            if (my_rankno > rankno) break;
        }
        if (my_rankno > rankno) continue;
        // 自ランク番号以上と接続している節点（ノード）をカウントする.
        over_nodes++;
    }

    return over_nodes;
}


/**
 * 内部境界情報を送受信して接続先を追加する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConnectivity::exportProcess()
{
#ifdef _DEBUG_TRACE
    UDM_DEBUG_PRINTF("%s:%d [%s]",  __FILE__, __LINE__, __FUNCTION__);
#endif
#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START(__FUNCTION__);
#endif

    UdmError_t error = UDM_OK;
    UdmSize_t n, m;
    int num_mpiinfo;
    int rankno;
    UdmSize_t localid;
    std::vector<UdmNode*>::iterator itr;
    std::map< int, UdmGlobalRankidPairList* > send_infos;
    std::map< int, UdmGlobalRankidPairList* > recv_infos;

#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        std::string buf;
        this->toString(buf);
        UDM_DEBUG_PRINTF(buf.c_str());
    }
#endif

    // ランク番号別の内部境界ランク番号、IDを取得する
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        num_mpiinfo = node->getNumMpiRankInfos();
        UdmGlobalRankidPairList infos(num_mpiinfo);
        std::vector<int> send_ranks;
        for (n=1; n<=num_mpiinfo; n++) {
            node->getMpiRankInfo(n, rankno, localid);
            // 内部境界情報
            infos.addGlobalRankidPair(node->getMyRankno(), node->getId(), rankno, localid);
            send_ranks.push_back(rankno);
        }
        std::vector<int>::iterator rank_itr;
        for (rank_itr=send_ranks.begin(); rank_itr!=send_ranks.end(); rank_itr++) {
            std::vector<UdmGlobalRankidPair>::iterator info_itr;
            for (info_itr=infos.begin(); info_itr!=infos.end(); info_itr++) {
                this->insertBoundaryInfo(send_infos, *rank_itr, *info_itr);
            }
        }
    }

    // 内部境界情報を送信する.
    error = this->mpi_sendrecvBoundaryInfos(send_infos, recv_infos);
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    // 受信内部境界情報を追加する.
    UdmGlobalRankidPairList found_infos;        // 受信内部境界情報
    std::map<int, UdmGlobalRankidPairList* >::const_iterator infos_itr;

    // 自分の内部境界節点（ノード）の内、受信した節点（ノード）を検索する
    for (infos_itr=recv_infos.begin(); infos_itr!=recv_infos.end(); infos_itr++) {
        int rankno = infos_itr->first;
        UdmGlobalRankidPairList *recv_rank_infos = infos_itr->second;
        std::vector<UdmGlobalRankidPair>::iterator recv_itr;
        for (recv_itr=recv_rank_infos->begin(); recv_itr!=recv_rank_infos->end(); recv_itr++) {
            UdmNode *node = this->findMpiRankInfo(recv_itr->getSrcRankno(), recv_itr->getSrcLocalid());
            if (node != NULL) {
                node->addMpiRankInfo(recv_itr->getDestRankno(), recv_itr->getDestLocalid());
            }
        }
    }

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_STOP(__FUNCTION__);
    UdmSize_t send_size = 0, recv_size = 0;
    for (infos_itr=send_infos.begin(); infos_itr!=send_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList *rank_infos = infos_itr->second;
        send_size += rank_infos->getCommSize();
    }
    for (infos_itr=recv_infos.begin(); infos_itr!=recv_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList *rank_infos = infos_itr->second;
        recv_size += rank_infos->getCommSize();
    }
    char info[128] = {0x00};
    sprintf(info, "send_size=%ld,recv_size=%ld", send_size, recv_size);
    UDM_STOPWATCH_INFORMATION(__FUNCTION__, info);

#endif
#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        std::string buf;
        this->toString(buf);
        UDM_DEBUG_PRINTF(buf.c_str());
    }
#endif

    for (infos_itr=send_infos.begin(); infos_itr!=send_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList* rank_infos = infos_itr->second;
        if (rank_infos != NULL) delete rank_infos;
    }
    for (infos_itr=recv_infos.begin(); infos_itr!=recv_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList* rank_infos = infos_itr->second;
        if (rank_infos != NULL) delete rank_infos;
    }
    return UDM_OK;
}


/**
 * 内部境界情報を接続先に送信する.
 * 受信した内部境界情報を元に節点（ノード）の接続ランク情報を更新する.
 * 受信しなかった節点（ノード）は接続ランク情報をクリアし、内部境界から削除する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConnectivity::transferRankConnectivity()
{
#ifdef _DEBUG_TRACE
    UDM_DEBUG_PRINTF("%s:%d [%s]",  __FILE__, __LINE__, __FUNCTION__);
#endif

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START(__FUNCTION__);
#endif

    UdmError_t error = UDM_OK;
    int n;
    int my_rankno = this->getMpiRankno();
    int num_mpiinfo;
    int rankno;
    UdmSize_t node_id;
    UdmSize_t localid;
    std::vector<UdmNode*>::iterator itr;
    std::map< int, UdmGlobalRankidPairList* > send_infos;
    std::map< int, UdmGlobalRankidPairList* > recv_infos;

    // ランク番号別の内部境界ランク番号、IDを取得する
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        num_mpiinfo = node->getNumMpiRankInfos();
        UdmGlobalRankidPairList infos(num_mpiinfo);
        std::vector<int> send_ranks(num_mpiinfo);
        for (n=1; n<=num_mpiinfo; n++) {
            node->getMpiRankInfo(n, rankno, localid);
            if (localid > 0) {
                // 内部境界情報
                UdmGlobalRankidPair info(node->getMyRankno(), node->getId(), rankno, localid);
                infos.addGlobalRankidPair(info);
            }
            if (rankno == my_rankno) continue;            // 自身は送信対象外
            send_ranks.push_back(rankno);
        }
        std::vector<int>::iterator rank_itr;
        std::vector<UdmGlobalRankidPair>::iterator info_itr;
        for (rank_itr=send_ranks.begin(); rank_itr!=send_ranks.end(); rank_itr++) {
            for (info_itr=infos.begin(); info_itr!=infos.end(); info_itr++) {
                this->insertBoundaryInfo(send_infos, *rank_itr, *info_itr);
            }
        }
    }

    // 内部境界情報を送信する.
    error = this->mpi_sendrecvBoundaryInfos(send_infos, recv_infos);
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    // 受信内部境界情報をマージする.
    UdmGlobalRankidPairList rank_comms;
    std::vector<UdmNode*> del_nodes;
    std::map< int, UdmGlobalRankidPairList* >::iterator infos_itr;
#if 0
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        int node_rankno = node->getMyRankno();
        node_id = node->getId();

        // 自分の内部境界節点（ノード）の内、受信した節点（ノード）を検索する
        rank_comms.clear();
        for (infos_itr=recv_infos.begin(); infos_itr!=recv_infos.end(); infos_itr++) {
            int recv_rankno = infos_itr->first;
            UdmGlobalRankidPairList recv_rank_infos = infos_itr->second;
            std::vector<UdmGlobalRankidPair>::iterator recv_itr;
            for (recv_itr=recv_rank_infos.begin(); recv_itr!=recv_rank_infos.end(); recv_itr++) {
                int src_rankno = recv_itr->getSrcRankno();
                UdmSize_t src_localid = recv_itr->getSrcLocalid();
                if (recv_itr->equalsDest(node_rankno, node_id)) {
                    rank_comms.addGlobalRankidPair(*recv_itr);
                }
                else if (node->existsMpiRankInfo(recv_itr->getDestRankno(), recv_itr->getDestLocalid())) {
                    rank_comms.addGlobalRankidPair(*recv_itr);
                }
                else if (node->existsPreviousRankInfo(recv_itr->getDestRankno(), recv_itr->getDestLocalid())) {
                    rank_comms.addGlobalRankidPair(*recv_itr);
                }
            }
        }
        // 受信情報から検索結果の内部境界節点（ノード）を設定する.
        node->clearMpiRankInfos();
        std::vector<UdmGlobalRankidPair>::iterator info_itr;
        for (info_itr=rank_comms.begin(); info_itr!=rank_comms.end(); info_itr++) {
            node->addMpiRankInfo(info_itr->getSrcRankno(), info_itr->getSrcLocalid());
        }
        if (node->getNumMpiRankInfos() <= 0) {
            del_nodes.push_back(node);
        }
    }

    // 内部境界の無くなったノードを削除する
    std::vector<UdmNode*>::iterator node_itr;
    for (node_itr=del_nodes.begin(); node_itr!=del_nodes.end(); node_itr++) {
        UdmNode *node = (*node_itr);
        this->removeBoundaryNode(node);
    }

#endif

    // 内部境界情報をすべて削除する. ランク番号、ID検索テーブルを作成する。
    this->clearMpiRankInfos();

    // 自分の内部境界節点（ノード）の内、受信した節点（ノード）を検索する
    for (infos_itr=recv_infos.begin(); infos_itr!=recv_infos.end(); infos_itr++) {
        int recv_rankno = infos_itr->first;
        UdmGlobalRankidPairList *recv_rank_infos = infos_itr->second;
        std::vector<UdmGlobalRankidPair>::iterator recv_itr;
        for (recv_itr=recv_rank_infos->begin(); recv_itr!=recv_rank_infos->end(); recv_itr++) {
            int src_rankno = recv_itr->getSrcRankno();
            UdmSize_t src_localid = recv_itr->getSrcLocalid();
            int mpi_rankno = recv_itr->getDestRankno();
            UdmSize_t mpi_localid = recv_itr->getDestLocalid();

            UdmNode *node = this->findMpiRankInfo(mpi_rankno, mpi_localid);
            if (node != NULL) {
                node->addMpiRankInfo(src_rankno, src_localid);
                this->insertRankConnectivityNode(node);
            }
        }
    }

    // 内部境界を挿入しなかった節点（ノード）を削除する。
    this->eraseRankConnectivity();

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_STOP(__FUNCTION__);
    UdmSize_t send_size = 0, recv_size = 0;
    for (infos_itr=send_infos.begin(); infos_itr!=send_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList *rank_infos = infos_itr->second;
        send_size += rank_infos->getCommSize();
    }
    for (infos_itr=recv_infos.begin(); infos_itr!=recv_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList *rank_infos = infos_itr->second;
        recv_size += rank_infos->getCommSize();
    }
    char info[128] = {0x00};
    sprintf(info, "send_size=%ld,recv_size=%ld", send_size, recv_size);
    UDM_STOPWATCH_INFORMATION(__FUNCTION__, info);
#endif

    for (infos_itr=send_infos.begin(); infos_itr!=send_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList* rank_infos = infos_itr->second;
        if (rank_infos != NULL) delete rank_infos;
    }
    for (infos_itr=recv_infos.begin(); infos_itr!=recv_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList* rank_infos = infos_itr->second;
        if (rank_infos != NULL) delete rank_infos;
    }
    return UDM_OK;
}


/**
 * 内部境界情報をすべてのプロセス間にMPI送受信を行う.
 * @param  [in]  send_infos     送信内部境界情報
 * @param  [out] recv_infos     受信内部境界情報
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConnectivity::mpi_sendrecvBoundaryInfos(
        const std::map< int, UdmGlobalRankidPairList* > &send_infos,
        std::map< int, UdmGlobalRankidPairList* > &recv_infos) const
{
    const int TRANS_IDS_SIZE = 4;
    // 内部境界数を送受信する
    MPI_Comm comm = this->getMpiComm();
    int num_process = this->getMpiProcessSize();
    int my_rankno = this->getMpiRankno();
    // int num_sends[num_process];
    // int num_recvs[num_process];
    int *num_sends = new int[num_process];
    int *num_recvs = new int[num_process];
    memset(num_sends, 0x00, num_process*sizeof(int));
    memset(num_recvs, 0x00, num_process*sizeof(int));
    int n, i;
    const UdmDfiConfig *config = this->getParentZone()->getDfiConfig();

    std::map< int, UdmGlobalRankidPairList* > sendranks_info;
    if (config->existsIoRankTable()) {
        // 入出力ランク番号テーブルに従って送信先を入れ替える。
        std::map< int, UdmGlobalRankidPairList* >::const_iterator info_itr;
        for (info_itr=send_infos.begin(); info_itr!=send_infos.end(); info_itr++) {
            int out_rankno = info_itr->first;
            UdmGlobalRankidPairList* infos = info_itr->second;
            // 送信先ランク番号
            int in_rankno = config->getInputRankno(out_rankno);
            if (in_rankno >= 0) {
                this->insertBoundaryInfos(sendranks_info, in_rankno, infos);
            }
            else {
                this->insertBoundaryInfos(sendranks_info, out_rankno, infos);
            }
        }
    }
    else {
        sendranks_info = send_infos;
    }

    // 内部境界数の取得
    std::map< int, UdmGlobalRankidPairList* >::const_iterator find_itr;
    for (n=0; n<num_process; n++) {
        find_itr = sendranks_info.find(n);
        if (find_itr != sendranks_info.end()) {
            num_sends[n] = find_itr->second->size();
        }
    }

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START("MPI_Alltoall");
#endif

    // ランク番号別の内部境界数を送信する.
    udm_mpi_alltoall(num_sends, 1, MPI_INT, num_recvs, 1, MPI_INT, comm);

#ifdef _UDM_PROFILER
    UDM_STOPWATCH_STOP("MPI_Alltoall");
#endif

    // 送受信バッファー作成
    UdmSize_t send_size = 0, recv_size = 0;
    UdmSize_t send_pos = 0, recv_pos = 0;
    for (n=0; n<num_process; n++) {
        if (n == my_rankno) continue;
        send_size += num_sends[n]*TRANS_IDS_SIZE;
        recv_size += num_recvs[n]*TRANS_IDS_SIZE;
    }
    long long *send_buf = NULL;
    long long *recv_buf = NULL;
    if (send_size > 0) {
        send_buf = new long long[send_size];
        memset(send_buf, 0x00, send_size*sizeof(long long ));
    }

    if (recv_size > 0) {
        recv_buf = new long long[recv_size];
        memset(recv_buf, 0x00, recv_size*sizeof(long long ));
    }

#ifdef _DEBUG_TRACE
    {
        UDM_DEBUG_PRINTF("mpi_sendrecvBoundaryInfos::UdmGlobalRankidPair(Send) rank=%d", this->getMpiRankno());
    }
#endif

    // 送信データの作成={自ランク番号,ID, 接続先ランク番号,ID}
    send_pos = 0, recv_size = 0;
    for (n=0; n<num_process; n++) {
        if (n == my_rankno) continue;
        if (num_sends[n] == 0) continue;
        if (send_buf == NULL) continue;

        find_itr = sendranks_info.find(n);
        if (find_itr == sendranks_info.end()) continue;
        UdmGlobalRankidPairList* bnd_infos = find_itr->second;
        std::vector<UdmGlobalRankidPair>::iterator itr;
        for (itr=bnd_infos->begin(); itr!=bnd_infos->end(); itr++) {
            send_buf[send_pos++] = (*itr).getSrcRankno();
            send_buf[send_pos++] = (*itr).getSrcLocalid();
            send_buf[send_pos++] = (*itr).getDestRankno();
            send_buf[send_pos++] = (*itr).getDestLocalid();

#ifdef _DEBUG_TRACE
        if (UDM_IS_DEBUG_LEVEL()) {
            UDM_DEBUG_PRINTF("[rank %d] Send rank=%d : send node_id=%d[%d], mpi_rankno=%d, mpi_localid=%d",
                this->getMpiRankno(), n,
                (*itr).getSrcLocalid(), (*itr).getSrcRankno(), (*itr).getDestRankno(), (*itr).getDestLocalid());
    }
#endif
        }
    }

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START("MPI_Isend-MPI_Irecv");
#endif

    // ランク番号別に内部境界情報を送信する.
    MPI_Status   *send_status = new MPI_Status[num_process];
    MPI_Status   *recv_status = new MPI_Status[num_process];
    MPI_Request  *send_requests = new MPI_Request[num_process];
    MPI_Request  *recv_requests = new MPI_Request[num_process];

    send_pos = 0, recv_size = 0;
    for (n=0; n<num_process; n++) {
        send_requests[n] = MPI_REQUEST_NULL;
        recv_requests[n] = MPI_REQUEST_NULL;
        if (n == my_rankno) continue;

        if (num_sends[n] > 0) {
            if (send_buf != NULL) {
                udm_mpi_isend(&send_buf[send_pos], num_sends[n]*TRANS_IDS_SIZE, MPI_LONG_LONG, n, 0, comm, &send_requests[n]);
            }
        }

        if (num_recvs[n] > 0) {
            if (recv_buf != NULL) {
                udm_mpi_irecv(recv_buf + recv_pos, num_recvs[n]*TRANS_IDS_SIZE, MPI_LONG_LONG, n, 0, comm, &recv_requests[n]);
            }
        }

        // 送受信バッファー位置
        send_pos += num_sends[n]*TRANS_IDS_SIZE;
        recv_pos += num_recvs[n]*TRANS_IDS_SIZE;
    }

    udm_mpi_waitall(num_process, send_requests, send_status);
    udm_mpi_waitall(num_process, recv_requests, recv_status);

#ifdef _UDM_PROFILER
    UDM_STOPWATCH_STOP("MPI_Isend-MPI_Irecv");
#endif

#ifdef _DEBUG_TRACE
    {
        UDM_DEBUG_PRINTF("mpi_sendrecvBoundaryInfos::UdmGlobalRankidPair(Recieve) rank=%d", this->getMpiRankno());
    }
#endif
    // 受信データ
    recv_pos = 0;
    for (n=0; n<num_process; n++) {
        if (n == my_rankno) continue;
        if (num_recvs[n] == 0) continue;
        if (recv_buf == NULL) continue;

        if (recv_infos.find(n) == recv_infos.end()) {
            UdmGlobalRankidPairList *ids = new UdmGlobalRankidPairList(num_recvs[n]);
            recv_infos.insert(std::make_pair(n, ids));
        }

        for (i=0; i<num_recvs[n]; i++) {
            int recv_rankno = recv_buf[recv_pos++];
            UdmSize_t node_id = recv_buf[recv_pos++];
            int mpi_rankno = recv_buf[recv_pos++];
            UdmSize_t mpi_localid = recv_buf[recv_pos++];

            // 受信ID変更情報
            UdmGlobalRankidPair info(recv_rankno, node_id, mpi_rankno, mpi_localid);

            // 受信内部境界データの追加：内部境界データ={自node_id, 相手ランクのlocalid}
            this->insertBoundaryInfo(recv_infos, n, info);

    #ifdef _DEBUG_TRACE
            if (UDM_IS_DEBUG_LEVEL()) {
                UDM_DEBUG_PRINTF("Recieve rank=%d : recviece node_id=%d[%d], mpi_rankno=%d, mpi_localid=%d",  n, node_id, recv_rankno, mpi_rankno, mpi_localid);
            }
    #endif
        }
    }

    if (send_buf != NULL) delete []send_buf;
    if (recv_buf != NULL) delete []recv_buf;
    if (send_status != NULL) delete []send_status;
    if (recv_status != NULL) delete []recv_status;
    if (send_requests != NULL) delete []send_requests;
    if (recv_requests != NULL) delete []recv_requests;
    delete []num_sends; delete []num_recvs;

    return UDM_OK;
}


/**
 * CGNS:Elementsに内部境界情報を面情報として出力する.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConnectivity::writeRankConnectivity(int index_file, int index_base, int index_zone)
{
    UdmError_t error = UDM_OK;
    int n, m;
    std::vector<UdmGlobalRankidPair>::iterator info_itr;

    // 出力内部境界データのチェック
    if (this->boundary_nodes.size() <= 0) {
        return UDM_OK;
    }

    // "UdmRankConnectivity" : 内部境界情報をユーザ定義ノードとして出力する
    if ((error = this->writeCgns(index_file, index_base, index_zone)) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    // "UdmRankConnectivity" : 内部境界情報をCGNS:Elementsとして出力する
    int cell_dimensions = this->getParentZone()->getParentModel()->getCellDimension();
    std::vector<UdmNode*>::iterator itr;
    std::vector<UdmICellComponent*> components;

    UdmSize_t array_size = 0;
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        array_size += node->getNumParentCells();
    }
    components.reserve(array_size);

    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        int num_cells = node->getNumParentCells();
        for (n=1; n<=num_cells; n++) {
            UdmICellComponent *cell = node->getParentCell(n);
            // 部品要素(=2D)のみ取得する
            if (cell->getCellClass() != Udm_ComponentClass) continue;
            // 境界面のみ取得する
            int num_cells = cell->getNumParentCells();
            if (num_cells <= 1) continue;
            int virtual_count = 0;
            for (m=1; m<=num_cells; m++) {
                UdmCell *parent_cell = cell->getParentCell(m);
                if (parent_cell->getRealityType() == Udm_Virtual) {
                    virtual_count++;
                }
            }
            // 仮想セルに面していること。
            if (virtual_count <= 0) continue;

            components.push_back(cell);
        }
    }

    // 重複削除
    std::sort(components.begin(), components.end());
    components.erase( std::unique(components.begin(), components.end()), components.end() );

    // 内部境界節点（ノード）からのみの部品要素を取得
    std::vector<UdmICellComponent*>::iterator cell_itr;
    std::vector<UdmICellComponent*> write_cells;
    for (cell_itr=components.begin(); cell_itr!=components.end(); cell_itr++) {
        UdmICellComponent *cell = *cell_itr;
        int num_nodes = cell->getNumNodes();
        bool found = true;
        for (n=1; n<=num_nodes; n++) {
            UdmNode *node = cell->getNode(n);
            if (!this->existsNode(node->getMyRankno(), node->getId())) {
                found = false;
                break;
            }
        }
        if (!found) continue;
        write_cells.push_back(cell);
    }

    // 部品要素を出力する
    cgsize_t start=1;
    cgsize_t end=write_cells.size();
    UdmElementType_t elem_type = Udm_ElementTypeUnknown;
    for (cell_itr=write_cells.begin(); cell_itr!=write_cells.end(); cell_itr++) {
        UdmICellComponent *cell = *cell_itr;
        if (elem_type == Udm_ElementTypeUnknown) {
            elem_type = cell->getElementType();
        }
        else if (elem_type != cell->getElementType()) {
            elem_type = Udm_MIXED;
            break;
        }
    }
    ElementType_t cgns_elemtype = this->toCgnsElementType(elem_type);
    if (cgns_elemtype == ElementTypeNull) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_ELEMENTTYPE, "cgns_elemtype=%d", cgns_elemtype);
    }

    UdmScannerCells scanner_cells;
    scanner_cells.scannerElements(write_cells);
    size_t data_size = scanner_cells.getNumElementsDatas(elem_type);
    cgsize_t *element_ids = new cgsize_t[data_size];
    scanner_cells.getScanCgnsElementsIds<cgsize_t>(elem_type, element_ids);

    // 要素の接続情報の設定 : cg_section_write
    char elem_name[33] = "Elements_RankConnectivity";
    int nbndry = 0;
    int index_section;
    if (cg_section_write(index_file,index_base,index_zone, elem_name, cgns_elemtype,start,end,nbndry,element_ids, &index_section) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "failure : cg_section_write");
    }

    delete []element_ids;

    return UDM_OK;
}

/**
 * ランク番号、IDの節点（ノード）が存在するかチェックする.
 * 節点（ノード）のランク番号、IDのみチェックする。MPI接続情報はチェックしない.
 * @param rankno        ランク番号
 * @param node_id        ID
 * @return        true=存在する
 */
bool UdmRankConnectivity::existsNode(int rankno, UdmSize_t node_id) const
{
    std::vector<UdmNode*>::const_iterator itr;
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        if (node->getMyRankno() != rankno) continue;
        if (node->getId() != node_id) continue;
        return true;
    }
    return false;
}


/**
 * GridCoordinatesクラスを取得する.
 * @return        GridCoordinatesクラス
 */
UdmGridCoordinates* UdmRankConnectivity::getGridCoordinates() const
{
    if (this->getParentZone() == NULL) return NULL;
    if (this->getParentZone()->getGridCoordinates() == NULL) return NULL;
    return this->getParentZone()->getGridCoordinates();
}

/**
 * 内部境界情報をすべてのプロセスから収集行い、整合性、過不足な節点（ノード）が存在しているかチェックする.
 * @return        true=内部境界情報のチェックOK
 */
bool UdmRankConnectivity::mpi_validateRankConnectivity() const
{
    UdmError_t error = UDM_OK;
    bool validate = true;
    int n;
    int num_process = this->getMpiProcessSize();
    int my_rankno = this->getMpiRankno();
    int num_mpiinfo;
    int rankno;
    UdmSize_t node_id;
    UdmSize_t localid;
    std::vector<UdmNode*>::const_iterator itr;
    std::map< int, UdmGlobalRankidPairList* > send_infos;
    std::map< int, UdmGlobalRankidPairList* > recv_infos;
    std::map<int, UdmGlobalRankidPairList* >::const_iterator infos_itr;

    // ランク番号別の内部境界ランク番号、IDを取得する
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        num_mpiinfo = node->getNumMpiRankInfos();
        UdmGlobalRankidPairList infos(num_mpiinfo);
        for (n=1; n<=num_mpiinfo; n++) {
            node->getMpiRankInfo(n, rankno, localid);
            if (localid == 0) {
                // mpi_localid無効
                error = UDM_WARNING_HANDLER(UDM_WARNING_CGNS_VALIDATE_RANKCONNECTIVITY, "mpi_localid is zero.[my_rankno=%d,node_id=%d,mpi_rankno=%d,mpi_localid=%d]",
                        node->getMyRankno(), node->getId(), rankno, localid);
                validate = false;
                continue;
            }
            if (rankno == my_rankno) {
                // mpi_rankno無効（自身は送信対象外）
                error = UDM_WARNING_HANDLER(UDM_WARNING_CGNS_VALIDATE_RANKCONNECTIVITY, "mpi_rankno equals my_rankno.[my_rankno=%d,node_id=%d,mpi_rankno=%d,mpi_localid=%d]",
                        node->getMyRankno(), node->getId(), rankno, localid);
                validate = false;
                continue;
            }
            if (rankno < 0 || rankno >= num_process) {
                // mpi_rankno無効
                error = UDM_WARNING_HANDLER(UDM_WARNING_CGNS_VALIDATE_RANKCONNECTIVITY, "mpi_rankno invalid.[my_rankno=%d,node_id=%d,mpi_rankno=%d,mpi_localid=%d, process_size=%d]",
                        node->getMyRankno(), node->getId(), rankno, localid,
                        num_process);
                validate = false;
                continue;
            }

            // 内部境界情報
            UdmGlobalRankidPair info(node->getMyRankno(), node->getId(), rankno, localid);
            this->insertBoundaryInfo(send_infos, rankno, info);
        }
    }

    // 内部境界情報を送信する.
    error = this->mpi_sendrecvBoundaryInfos(send_infos, recv_infos);
    if (error != UDM_OK) {
        validate = false;
        return validate;
    }

    std::map< int, UdmGlobalRankidPairList* >::const_iterator send_itr;
    std::map< int, UdmGlobalRankidPairList* >::const_iterator recv_itr;
    std::map< int, UdmGlobalRankidPairList* >::const_iterator mpi_find_itr;
    std::vector<UdmGlobalRankidPair>::iterator info_itr;
    std::vector<UdmGlobalRankidPair>::iterator find_itr;

    // 送信データをすべて受信しているかチェックする.
    for (send_itr=send_infos.begin(); send_itr!=send_infos.end(); send_itr++) {
        int mpi_rankno = send_itr->first;
        mpi_find_itr = recv_infos.find(mpi_rankno);
        if (mpi_find_itr == recv_infos.end()) {
            // mpi_rankno受信なし
            error = UDM_WARNING_HANDLER(UDM_WARNING_CGNS_VALIDATE_RANKCONNECTIVITY, "not receive RankConnectivity from mpi_rankno[%d].", mpi_rankno);
            validate = false;
            continue;
        }

        // 送信データと受信データが一致しているかチェックする.
        UdmGlobalRankidPairList *infos = send_itr->second;
        UdmGlobalRankidPairList *find_infos = mpi_find_itr->second;
        for (info_itr=infos->begin(); info_itr!=infos->end(); info_itr++) {
            UdmGlobalRankidPair send_info = (*info_itr);
            bool find_node = false;
            for (find_itr=find_infos->begin(); find_itr!=find_infos->end(); find_itr++) {
                UdmGlobalRankidPair recv_info = (*find_itr);
                if (send_info.equalsConnectivity(recv_info)) {
                    find_node = true;
                }
            }
            if (!find_node) {
                // 送信したが受信なし
                error = UDM_WARNING_HANDLER(UDM_WARNING_CGNS_VALIDATE_RANKCONNECTIVITY,
                        "not receive my node. send_info=[my_rankno=%d,node_id=%d,mpi_rankno=%d,mpi_localid=%d]",
                        send_info.getSrcRankno(), send_info.getSrcLocalid(), send_info.getDestRankno(), send_info.getDestLocalid());
                validate = false;
            }
        }
    }

    // 受信データをすべて送信しているかチェックする.
    for (recv_itr=recv_infos.begin(); recv_itr!=recv_infos.end(); recv_itr++) {
        int mpi_rankno = recv_itr->first;
        mpi_find_itr = send_infos.find(mpi_rankno);
        if (mpi_find_itr == send_infos.end()) {
            // mpi_rankno送信なし
            error = UDM_WARNING_HANDLER(UDM_WARNING_CGNS_VALIDATE_RANKCONNECTIVITY, "not send RankConnectivity to mpi_rankno[%d].", mpi_rankno);
            validate = false;
            continue;
        }

        // 送信データと受信データが一致しているかチェックする.
        UdmGlobalRankidPairList *infos = recv_itr->second;
        UdmGlobalRankidPairList *find_infos = mpi_find_itr->second;
        for (info_itr=infos->begin(); info_itr!=infos->end(); info_itr++) {
            UdmGlobalRankidPair recv_info = (*info_itr);
            bool find_node = false;
            for (find_itr=find_infos->begin(); find_itr!=find_infos->end(); find_itr++) {
                UdmGlobalRankidPair send_info = (*find_itr);
                if (send_info.equalsConnectivity(recv_info)) {
                    find_node = true;
                }
            }
            if (!find_node) {
                // 送信したが受信なし
                error = UDM_WARNING_HANDLER(UDM_WARNING_CGNS_VALIDATE_RANKCONNECTIVITY,
                        "not send node. recv_info=[my_rankno=%d,node_id=%d,mpi_rankno=%d,mpi_localid=%d]",
                        recv_info.getSrcRankno(), recv_info.getSrcLocalid(), recv_info.getDestRankno(), recv_info.getDestLocalid());
                validate = false;
            }
        }
    }

    // 同一MPIランク番号の２つの節点（ノード）と接続することはない。
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        num_mpiinfo = node->getNumMpiRankInfos();
        rankno = -1, localid = 0;
        int check_rankno = -1;
        UdmSize_t check_localid = -1;
        for (n=1; n<=num_mpiinfo; n++) {
            node->getMpiRankInfo(n, rankno, localid);
            if (check_rankno == rankno) {
                error = UDM_WARNING_HANDLER(UDM_WARNING_CGNS_VALIDATE_RANKCONNECTIVITY,
                        "duplicate connectivity from same MPI rankno. node=[my_rankno=%d,node_id=%d],mpi_nodes[mpi_rankno=%d,mpi_localid=%d]-[mpi_rankno=%d,mpi_localid=%d]",
                        node->getMyRankno(), node->getId(), check_rankno, check_localid, rankno, localid);
                validate = false;
            }
            else {
                check_rankno = rankno;
                check_localid = localid;
            }
        }
    }

#ifdef _DEBUG_TRACE
    if (validate) {
        UDM_DEBUG_PRINTF("UdmRankConnectivity::mpi_validateRankConnectivity() = OK\n");
    }
    else {
        UDM_DEBUG_PRINTF("UdmRankConnectivity::mpi_validateRankConnectivity() = NG\n");
    }
#endif

    for (infos_itr=send_infos.begin(); infos_itr!=send_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList* rank_infos = infos_itr->second;
        if (rank_infos != NULL) delete rank_infos;
    }
    for (infos_itr=recv_infos.begin(); infos_itr!=recv_infos.end(); infos_itr++) {
        UdmGlobalRankidPairList* rank_infos = infos_itr->second;
        if (rank_infos != NULL) delete rank_infos;
    }
    return validate;
}

/**
 * 内部境界節点（ノード）をグローバルID順にソートする.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConnectivity::rebuildRankConnectivity()
{
    // ランク番号,ID順にソートする.
    std::sort(this->boundary_nodes.begin(), this->boundary_nodes.end(), UdmEntity::compareIds);

    // ランク番号、ID検索テーブルを作成する。
    this->createSearchTable();

    return UDM_OK;
}

/**
 * 最外郭の節点（ノード）又は内部境界節点（ノード）であるかのチェックを行う.
 * 親が１つのみの部品要素の構成節点（ノード）であるかチェックする.
 * @param node        チェック節点（ノード）
 * @return            true = 最外郭の節点（ノード）又は内部境界節点（ノード）
 */
bool UdmRankConnectivity::isBoundaryNode(const UdmNode* node) const
{
    std::vector<UdmICellComponent*> parents;
    int n;
    int num_parents = node->getNumParentCells();
    for (n=1; n<=num_parents; n++) {
        UdmICellComponent* parent_cell = node->getParentCell(n);
        // 親要素が設定されていること。部品要素には設定されている。
        // 親要素が１つのみであること.
        if (parent_cell->getNumParentCells() != 1) continue;
        parents.push_back(parent_cell);
    }
    if (parents.size() <= 0) {
        return false;
    }
    return true;
}


/**
 * ランク番号、ID検索テーブルを作成する.
 * 節点（ノード）のランク番号、ID、以前のランク番号、ID、MPI接続のランク番号、IDの検索テーブルを作成する.
 * @param node        節点（ノード）
 */
void UdmRankConnectivity::addSearchTable(UdmNode* node)
{
    int n;
    int len;
    int rankno;
    UdmSize_t localid;

    // ランク番号、ID
    this->search_table.addGlobalRankid(node->getMyRankno(), node->getId(), node);

    // 以前のランク番号, ID
    len = node->getNumPreviousRankInfos();
    for (n=1; n<=len; n++) {
        node->getPreviousRankInfo(n, rankno, localid);
        this->search_table.addGlobalRankid(rankno, localid, node);
    }

    // MPI接続ランク番号, ID
    len = node->getNumMpiRankInfos();
    for (n=1; n<=len; n++) {
        node->getMpiRankInfo(n, rankno, localid);
        this->search_table.addGlobalRankid(rankno, localid, node);
    }

    return;
}

/**
 * ランク番号、ID検索テーブルから節点（ノード）を削除する
 * @param node        削除節点（ノード）
 */
void UdmRankConnectivity::removeSearchTable(UdmNode* node)
{
    int rankno;
    UdmSize_t localid;
    int len;
    int n;

    // ランク番号、ID
    rankno = node->getMyRankno();
    localid = node->getId();
    this->search_table.removeReferenceEntity(rankno, localid, node);

    // 以前のランク番号, ID
    len = node->getNumPreviousRankInfos();
    for (n=1; n<=len; n++) {
        node->getPreviousRankInfo(n, rankno, localid);
        this->search_table.removeReferenceEntity(rankno, localid, node);
    }

    // MPI接続ランク番号, ID
    len = node->getNumMpiRankInfos();
    for (n=1; n<=len; n++) {
        node->getMpiRankInfo(n, rankno, localid);
        this->search_table.removeReferenceEntity(rankno, localid, node);
    }

    return;
}

/**
 * ランク番号、ID検索テーブルをクリアする。
 */
void UdmRankConnectivity::clearSearchTable()
{
    // ランク番号,ID検索テーブルをクリアする
    this->search_table.clear();

    return;
}

/**
 * ランク番号、ID検索テーブルを作成する。
 */
void UdmRankConnectivity::createSearchTable()
{
    // ランク番号,ID検索テーブルをクリアする
    this->clearSearchTable();

    // ランク番号、ID検索テーブルを作成する。
    std::vector<UdmNode*>::iterator itr;
    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        this->addSearchTable(*itr);
    }

    return;
}

/**
 * MPI接続ランク番号, IDの検索テーブルを作成する.
 */
void UdmRankConnectivity::createSearchMpiRankidTable()
{
    this->clearSearchTable();

    int n, len;
    int rankno;
    UdmSize_t localid;
    std::vector<UdmNode*>::const_iterator itr;

    for (itr=this->boundary_nodes.begin(); itr!=this->boundary_nodes.end(); itr++) {
        UdmNode *node = *itr;
        // MPI接続ランク番号, ID
        len = node->getNumMpiRankInfos();
        for (n=1; n<=len; n++) {
            node->getMpiRankInfo(n, rankno, localid);
            if (localid <= 0) continue;
            if (rankno < 0) continue;
            this->search_table.addGlobalRankid(rankno, localid, node);
        }
    }

    return;
}


/**
 * 内部境界ノードリストを削除する.
 * @param node        削除内部境界ノードリスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConnectivity::removeBoundaryNodes(const std::vector<UdmNode*>& remove_nodes)
{
    std::vector<UdmNode*>::const_iterator itr;
    for (itr=remove_nodes.begin(); itr!=remove_nodes.end(); itr++) {
        UdmNode* node = (*itr);
        if (node != NULL) {
            node->setRemoveEntity(true);
        }
    }

    // 内部境界ノードから削除
    UdmSize_t erase_size = this->boundary_nodes.size();
    this->boundary_nodes.erase(
            std::remove_if(
                    this->boundary_nodes.begin(),
                    this->boundary_nodes.end(),
                    std::mem_fun(&UdmEntity::isRemoveEntity)),
            this->boundary_nodes.end());
    erase_size -= this->boundary_nodes.size();

    return UDM_ERROR;
}

} /* namespace udm */
