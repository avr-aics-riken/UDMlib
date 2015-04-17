/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmCoordinates.cpp
 * グリッド座標クラスのソースファイル
 */

#include "model/UdmGridCoordinates.h"
#include "model/UdmModel.h"
#include "model/UdmZone.h"
#include "model/UdmFlowSolutions.h"
#include "model/UdmNode.h"
#include "model/UdmRankConnectivity.h"

namespace udm
{
/**
 * コンストラクタ
 */
UdmGridCoordinates::UdmGridCoordinates()
{
    this->initialize();
}

/**
 * コンストラクタ
 * @param zone        ゾーン
 */
UdmGridCoordinates::UdmGridCoordinates(UdmZone* zone)
{
    this->initialize();
    this->parent_zone = zone;
}


/**
 * デストラクタ
 */
UdmGridCoordinates::~UdmGridCoordinates()
{
    this->finalize();
}


/**
 * 初期化を行う.
 */
void UdmGridCoordinates::initialize()
{
    this->setDataType(Udm_RealSingle);
    this->clearNodes();
    this->parent_zone = NULL;
    this->max_nodeid = 0;
}

/**
 * 節点（ノード）の破棄処理を行う.
 */
void UdmGridCoordinates::finalize()
{
/*********************************
    std::vector<UdmNode*>::reverse_iterator ritr = this->node_list.rbegin();
    while (ritr != this->node_list.rend()) {
        UdmNode* node = *(--(ritr.base()));
        if (node != NULL) {
            node->finalize();
            delete node;
        }
        this->node_list.erase((++ritr).base());
        ritr = this->node_list.rbegin();
    }
    this->node_list.clear();

    ritr = this->virtual_nodes.rbegin();
    while (ritr != this->virtual_nodes.rend()) {
        UdmNode* node = *(--(ritr.base()));
        if (node != NULL) {
            node->finalize();
            delete node;
        }
        this->virtual_nodes.erase((++ritr).base());
        ritr = this->virtual_nodes.rbegin();
    }
    this->virtual_nodes.clear();
***************************/

    std::vector<UdmNode*>::iterator itr = this->node_list.begin();
    for (itr=this->node_list.begin(); itr!=this->node_list.end(); itr++) {
        UdmNode* node = (*itr);
        if (node != NULL) {
            node->finalize();
            delete node;
        }
    }
    this->node_list.clear();

    itr = this->virtual_nodes.begin();
    for (itr=this->virtual_nodes.begin(); itr!=this->virtual_nodes.end(); itr++) {
        UdmNode* node = (*itr);
        if (node != NULL) {
            node->finalize();
            delete node;
        }
    }
    this->virtual_nodes.clear();

    return;
}

/**
 * グリッド構成ノード数を取得する.
 * @return        グリッド構成ノード
 */
UdmSize_t UdmGridCoordinates::getNumNodes() const
{
    return this->node_list.size();
}

/**
 * ノードIDのグリッド構成ノードを取得する.
 * @param node_id        ノードID（１～getNumNodes()）
 * @return        グリッド構成ノード
 */
UdmNode* UdmGridCoordinates::getNodeById(UdmSize_t node_id) const
{
    if (this->node_list.size() == 0) return NULL;
    if (node_id <= 0) return NULL;
    if (node_id > this->node_list.size()) return NULL;
    return (this->node_list[node_id - 1]);
}


/**
 * ローカルノードIDの節点（ノード）を取得する.
 * 構成節点（ノード）と仮想節点（ノード）から節点（ノード）を取得する.
 * @param node_id        ノードID（１～getNumNodes()+getNumVirtualNodes()）
 * @return        節点（ノード）
 */
UdmNode* UdmGridCoordinates::getNodeByLocalId(UdmSize_t node_id) const
{
    if (node_id <= 0) return NULL;
    if (node_id <= this->node_list.size()) {
        return this->getNodeById(node_id);
    }
    else if (node_id > this->node_list.size()) {
        return this->getVirtualNodeById(node_id - this->node_list.size());
    }

    return NULL;
}


/**
 * グリッド構成ノードを追加する.
 * @param node        グリッド構成ノード
 * @return        ノードID(1～) : 0の場合は挿入エラー
 */
UdmSize_t UdmGridCoordinates::insertNode(UdmNode* node)
{
    if (node == NULL) return 0;

    // グリッド構成ノードを追加する.
    this->node_list.push_back(node);

    /// NODE_ID = グリッド構成ノード(node_list)サイズ
    UdmSize_t node_id = 1;
    if (this->max_nodeid > 0) {
        node_id = this->max_nodeid + 1;
    }
    else if (this->node_list.size() > 0) {
        node_id = this->node_list[this->node_list.size()-1]->getId() + 1;
    }

    node->setId(node_id);
    this->max_nodeid = node_id;
    node->setParentGridcoordinates(this);
    // MPIランク番号
    node->setMyRankno(this->getMpiRankno());

    // 内部境界に追加する
    if (node->getNumMpiRankInfos() > 0) {
        if (this->getRankConnectivity() != NULL) {
            this->getRankConnectivity()->insertRankConnectivityNode(node);
        }
    }

    return node->getId();
}


/**
 * グリッド構成ノードを追加する.
 * ID,ランク番号の設定は行わない.
 * @param node        グリッド構成ノード
 */
void UdmGridCoordinates::pushbackNode(UdmNode* node)
{
    if (node == NULL) return;

    // グリッド構成ノードを追加する.
    this->node_list.push_back(node);

    /// NODE_ID = グリッド構成ノード(node_list)サイズ
    UdmSize_t node_id = node->getId();
    this->max_nodeid = node_id;
    node->setParentGridcoordinates(this);

    // ランク番号,IDを退避する.
    node->addPreviousRankInfo(node->getMyRankno(), node->getId());

    // 内部境界に追加する
    if (node->getNumMpiRankInfos() > 0) {
        if (this->getRankConnectivity() != NULL) {
            this->getRankConnectivity()->insertRankConnectivityNode(node);
        }
    }

    return;
}


/**
 * 仮想ノードリスト数を取得する.
 * @return        仮想ノードリスト数
 */
UdmSize_t UdmGridCoordinates::getNumVirtualNodes() const
{
    return this->virtual_nodes.size();
}

/**
 * 仮想ノードIDの仮想ノードリストを取得する.
 * @param node_id        仮想ノードID（１～getNumVirtualNodes()）
 * @return        仮想ノードリスト
 */
UdmNode* UdmGridCoordinates::getVirtualNodeById(UdmSize_t node_id) const
{
    if (this->virtual_nodes.size() == 0) return NULL;
    if (node_id <= 0) return NULL;
    if (node_id > this->virtual_nodes.size()) return NULL;
    return (this->virtual_nodes[node_id - 1]);
}

/**
 * 仮想ノードを追加する.
 * 仮想節点（ノード）のID, 自ランク番号は、実体ランクのID,ランク番号とする.
 * 仮想節点（ノード）のID, 自ランク番号は上位で設定済みとする.
 * 仮想節点（ノード）の自ランク番号,IDで昇順に挿入する.
 * @param node        仮想ノード
 * @return        仮想ノード数 : 0の場合は挿入エラー
 */
UdmSize_t UdmGridCoordinates::insertVirtualNode(UdmNode* virtual_node)
{
    if (virtual_node == NULL) return 0;

    std::vector<UdmNode*>::const_iterator itr;
    std::vector<UdmNode*>::iterator ins_itr;
    itr = this->searchUpperGlobalId(this->virtual_nodes, virtual_node);
    if (itr == this->virtual_nodes.end()) {
        this->virtual_nodes.push_back(virtual_node);
    }
    else {
        ins_itr = this->virtual_nodes.begin() + (itr - this->virtual_nodes.begin());
        ins_itr = this->virtual_nodes.insert(ins_itr, virtual_node);
    }

    virtual_node->setParentGridcoordinates(this);
    virtual_node->setRealityType(Udm_Virtual);

    return this->virtual_nodes.size();
}

/**
 * ノードをすべて削除する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::clearNodes()
{
/************************
    std::vector<UdmNode*>::reverse_iterator ritr = this->node_list.rbegin();
    while (ritr != this->node_list.rend()) {
        UdmNode* node = *(--(ritr.base()));
        if (node != NULL) {
            delete node;
        }
        this->node_list.erase((++ritr).base());
        ritr = this->node_list.rbegin();
    }
    this->node_list.clear();

    ritr = this->virtual_nodes.rbegin();
    while (ritr != this->virtual_nodes.rend()) {
        UdmNode* node = *(--(ritr.base()));
        if (node != NULL) {
            delete node;
        }
        this->virtual_nodes.erase((++ritr).base());
        ritr = this->virtual_nodes.rbegin();
    }
    this->virtual_nodes.clear();
******************/

    std::vector<UdmNode*>::iterator itr = this->node_list.begin();
    for (itr=this->node_list.begin(); itr!=this->node_list.end(); itr++) {
        UdmNode* node = (*itr);
        if (node != NULL) {
            node->finalize();
            delete node;
        }
    }
    this->node_list.clear();

    itr = this->virtual_nodes.begin();
    for (itr=this->virtual_nodes.begin(); itr!=this->virtual_nodes.end(); itr++) {
        UdmNode* node = (*itr);
        if (node != NULL) {
            node->finalize();
            delete node;
        }
    }
    this->virtual_nodes.clear();

    return UDM_OK;
}

/**
 * 仮想節点（ノード）をすべて削除する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::clearVirtualNodes()
{
/********************
    std::vector<UdmNode*>::reverse_iterator ritr = this->node_list.rbegin();
    ritr = this->virtual_nodes.rbegin();
    while (ritr != this->virtual_nodes.rend()) {
        UdmNode* node = *(--(ritr.base()));
        if (node != NULL) {
            delete node;
        }
        this->virtual_nodes.erase((++ritr).base());
        ritr = this->virtual_nodes.rbegin();
    }
    this->virtual_nodes.clear();
*************/

    std::vector<UdmNode*>::iterator itr = this->virtual_nodes.begin();
    for (itr=this->virtual_nodes.begin(); itr!=this->virtual_nodes.end(); itr++) {
        UdmNode* node = (*itr);
        if (node != NULL) {
            delete node;
        }
    }
    this->virtual_nodes.clear();

    return UDM_OK;
}


/**
 * 削除節点（ノード）を削除する.
 * @param node        削除節点（ノード）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::removeNode(const UdmEntity* node)
{
    if (node == NULL) return UDM_ERROR;

    // 内部境界リストから削除する
    this->removeRankConnectivity(node);

    bool found_node = false;
    UdmSize_t node_id = node->getId();

    std::vector<UdmNode*>::const_iterator itr;
    std::vector<UdmNode*>::iterator rm_itr;
    // 構成ノードから検索
    itr = this->searchGlobalId(this->node_list, node);
    if (itr != this->node_list.end()) {
        if ((*itr) == node) {
            delete (*itr);
            rm_itr = this->node_list.begin() + (itr - this->node_list.begin());
            this->node_list.erase(rm_itr);
            found_node = true;;
        }
    }

    // 仮想ノードから検索
    if (found_node == false) {
        itr = this->searchGlobalId(this->virtual_nodes, node);
        if (itr != this->virtual_nodes.end()) {
            if ((*itr) == node) {
                delete (*itr);
                rm_itr = this->virtual_nodes.begin() + (itr - this->virtual_nodes.begin());
                this->virtual_nodes.erase(rm_itr);
                found_node = true;;
            }
        }
    }

    if (found_node == false) {
        // node_idが存在しない
        UDM_WARNING_HANDLER(UDM_WARNING_INVALID_NODE, "not found node[node_id=%ld]", node_id);
        return UDM_ERROR;
    }

    return UDM_OK;
}

/**
 * 削除節点（ノード）を削除する.
 * ゾーン内の一意節点（ノード）IDにてグリッド構成ノード、仮想ノードから節点（ノード）を削除する.
 * @param node_id            ゾーン内節点（ノード）ID（１～）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::removeNode(UdmSize_t node_id)
{
    if (node_id <= 0) return UDM_ERROR;

    std::vector<UdmNode*>::const_iterator itr;
    std::vector<UdmNode*>::iterator rm_itr;

    // 構成ノードから検索
    itr = this->searchEntityId(this->node_list, node_id);
    if (itr != this->node_list.end()) {
        // 内部境界リストから削除する
        this->removeRankConnectivity(*itr);

        delete (*itr);
        rm_itr = this->node_list.begin() + (itr - this->node_list.begin());
        this->node_list.erase(rm_itr);
        return UDM_OK;
    }

    // 仮想ノードから検索
    itr = this->searchEntityId(this->virtual_nodes, node_id);
    if (itr != this->virtual_nodes.end()) {
        delete (*itr);
        rm_itr = this->virtual_nodes.begin() + (itr - this->virtual_nodes.begin());
        this->virtual_nodes.erase(rm_itr);
        return UDM_OK;
    }

    // node_idが存在しない
    UDM_WARNING_HANDLER(UDM_WARNING_INVALID_NODE, "not found node_id=%ld", node_id);

    return UDM_ERROR;
}


/**
 * 親ゾーンを取得する.
 * @return        親ゾーン
 */
UdmZone* UdmGridCoordinates::getParentZone() const
{
    return this->parent_zone;
}


/**
 * 親ゾーンを設定する.
 * @param zone        親ゾーン
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::setParentZone(UdmZone* zone)
{
    this->parent_zone = zone;
    return UDM_OK;
}

/**
 * CGNSファイルからGridCoordinates（ノード座標データ）の読込みを行う.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param timeslice_step       CGNS読込ステップ回数 (default=-1)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::readCgns(int index_file, int index_base, int index_zone, int timeslice_step)
{
    char zonename[33] = {0x00}, gridname[33] = {0x00}, arrayname[33] = {0x00};
    cgsize_t sizes[9] = {0x00};
    int num_coords = 0;
    DataType_t cgns_datatype;
    UdmDataType_t datatype = Udm_RealSingle;
    void *x_coords = NULL, *y_coords = NULL, *z_coords = NULL;
    int datasize;
    int dimension;
    cgsize_t vectors[3];
    UdmError_t error = UDM_OK;
    int n;

    // ゾーンからノードサイズの取得
    if (cg_zone_read(index_file, index_base, index_zone, zonename, sizes) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure:cg_zone_read");
    }

    // GridCoordinates名称の取得
    error = this->getCgnsIterativeGridCoordinatesName(gridname, index_file, index_base, index_zone, timeslice_step);
    if (error != UDM_OK) {
        if (error == UDM_WARNING_CGNS_NOTEXISTS_ITERATIVEDATA) {
            // 警告メッセージ:CGNS:BaseIterativeData, ZoneIterativeDataが存在しない。
            UDM_WARNINGNO_HANDLER(UDM_WARNING_CGNS_NOTEXISTS_ITERATIVEDATA);
        }

        int num_grids = 0;
        cg_ngrids(index_file, index_base, index_zone, &num_grids);
        if (num_grids > 1 && timeslice_step > 0) {
            // 複数のGridCoordinatesが存在し、ステップ番号が1以上であるので、GridCoordinates_%010dとする
            sprintf(gridname, UDM_CGNS_FORMAT_GRIDCOORDINATES, timeslice_step);
        }
        else {
            // timeslice_step=0の場合は必ずGridCoordinates
            strcpy(gridname, UDM_CGNS_NAME_GRIDCOORDINATES);
        }
    }

    if (cg_goto(index_file, index_base, zonename, 0, gridname, 0, "end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO, "zonename=%s,gridname=%s", zonename, gridname);
    }

    error = UDM_OK;
    // GridCoordinatesデータ数
    cg_narrays(&num_coords);
    if (num_coords <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "num_coords=%d", num_coords);
    }
    for (n=1; n<=num_coords; n++) {
        // CGNS:CoordinateX,CoordinateY,CoordinateZの読込
        dimension = 0; vectors[0] = 0;
        cg_array_info(n, arrayname, &cgns_datatype,&dimension, vectors);
        if (dimension != 1 && vectors[0] <= 0) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "coords[%d]:dimension=%d,vectors[0]=%d", n, dimension, vectors[0]);
        }
        if (vectors[0] != sizes[0]) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "invalid coordinates size:vectors[0]=%d, sizes[0]=%d", vectors[0], sizes[0]);
        }
        if (n==1) {
            datatype = this->setCoordsDatatype(cgns_datatype);
            if (datatype == Udm_RealSingle) {
                x_coords = new float[sizes[0]];
                y_coords = new float[sizes[0]];
                z_coords = new float[sizes[0]];
                datasize = sizeof(float);
            }
            else if (datatype == Udm_RealDouble) {
                x_coords = new double[sizes[0]];
                y_coords = new double[sizes[0]];
                z_coords = new double[sizes[0]];
                datasize = sizeof(double);
            }
            else {
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "invalid coordinates datatype");
            }
            memset(x_coords, 0x00, datasize*sizes[0]);
            memset(y_coords, 0x00, datasize*sizes[0]);
            memset(z_coords, 0x00, datasize*sizes[0]);
        }
        datatype = this->getDataType();
        cgns_datatype = this->toCgnsDataType(datatype);
        // 座標データの読込
        if (strcmp(arrayname, "CoordinateX") == 0) {
            if (cg_array_read_as(n,cgns_datatype,x_coords) != CG_OK) {
                error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : CoordinateX");
            }
        }
        else if (strcmp(arrayname, "CoordinateY") == 0) {
            if (cg_array_read_as(n,cgns_datatype,y_coords) != CG_OK) {
                error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : CoordinateY");
            }
        }
        else if (strcmp(arrayname, "CoordinateZ") == 0) {
            if (cg_array_read_as(n,cgns_datatype,z_coords) != CG_OK) {
                error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : CoordinateZ");
            }
        }
    }
    if (error != UDM_OK) {
        this->deleteDataArray(x_coords, datatype);
        this->deleteDataArray(y_coords, datatype);
        this->deleteDataArray(z_coords, datatype);
        return UDM_ERRORNO_HANDLER(error);
    }

    // ノードデータの作成
    UdmSize_t insert_size = 0;
    if (datatype == Udm_RealSingle) {
        insert_size = this->setGridCoordinatesArray(sizes[0], (float*)x_coords, (float*)y_coords, (float*)z_coords);
    }
    else if (datatype == Udm_RealDouble) {
        insert_size = this->setGridCoordinatesArray(sizes[0], (double*)x_coords, (double*)y_coords, (double*)z_coords);
    }
    this->deleteDataArray(x_coords, datatype);
    this->deleteDataArray(y_coords, datatype);
    this->deleteDataArray(z_coords, datatype);

    if (insert_size <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "invalid coordinates datatype");
    }

    // CGNS:UdmRankConnectivityを読み込む
    if (this->getRankConnectivity() != NULL) {
        if (this->getRankConnectivity()->readCgns(index_file, index_base, index_zone) != UDM_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : UdmRankConnectivity::readCgns(index_file=%d, index_base=%d, index_zone=%d)", index_file, index_base, index_zone);
        }
    }

    return error;
}

/**
 * CGNSデータ型から座標値のデータ型を設定する.
 * @param cgns_datatype        CGNSデータ型
 * @return        UDMlib:座標値データ型
 */
UdmDataType_t UdmGridCoordinates::setCoordsDatatype(DataType_t cgns_datatype)
{
    UdmDataType_t datatype = this->toUdmDataType(cgns_datatype);
    this->setDataType(datatype);
    return datatype;
}

/**
 * FlowSolutionクラスを取得する.
 * @return        FlowSolutionクラス
 */
UdmFlowSolutions* UdmGridCoordinates::getFlowSolutions() const
{
    if (this->parent_zone == NULL) return NULL;
    return this->parent_zone->getFlowSolutions();
}

/**
 * ステップ数に設定されている時系列CGNS:GridCoordinates名を取得する
 * @param grid_name            ゾーン名
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param timeslice_step    ステップ数 (default = -1)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::getCgnsIterativeGridCoordinatesName(char *grid_name, int index_file, int index_base, int index_zone, int timeslice_step)
{
    char arrayname[33], zitername[33];
    int  narrays;
    int dimension, num_iterative;
    cgsize_t n, vectors[3], iterative_index;
    DataType_t idatatype;

    if (timeslice_step < 0) {
        strcpy(grid_name, UDM_CGNS_NAME_GRIDCOORDINATES);
        return UDM_OK;
    }

    // CGNS:BaseIterativeData,ZoneIterativeDataが存在するかチェックする.
    if (!this->existsCgnsIterativeData(index_file, index_base, index_zone)) {
        return UDM_WARNING_CGNS_NOTEXISTS_ITERATIVEDATA;
    }

    // CGNS:BaseIterativeDataからステップ数と一致するIterativeIDを取得する.
    std::vector<UdmSize_t> iterative_ids;
    num_iterative = this->getCgnsBaseIterativeDataIds(iterative_ids, index_file, index_base, timeslice_step);
    if (num_iterative <= 0) {
        // GridCoordinatesPointersが存在しない。グリッド座標の時系列座標はない。
        return UDM_ERROR;
    }

    // 時系列CGNS:GridCoordinatesは１つのみ
    iterative_index = iterative_ids[0];
    // CGNS:ZoneIterativeDataの読込
    cg_ziter_read(index_file,index_base,index_zone,zitername);
    cg_goto(index_file,index_base,"Zone_t",index_zone,"ZoneIterativeData_t",1,"end");

    // CGNS:ZoneIterativeData/DataArray_tの数の取得 : cg_narrays
    narrays = 0;
    cg_narrays(&narrays);
    for (n=1; n<=narrays; n++) {
        // CGNS:ZoneIterativeData/DataArray_tの名前,データ数の取得 : cg_array_info
        cg_array_info(n, arrayname, &idatatype,&dimension,vectors);
        if (dimension != 2) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "invalid ZoneIterativeData[dataDimension=%d].", dimension);
        }
        if (vectors[1] < iterative_index) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "invalid ZoneIterativeData[dimensionVector[1]=%d,iterative_index=%d].", vectors[1], iterative_index);
        }
        if (strcmp(arrayname, "GridCoordinatesPointers")==0) {
            // char grid_names[vectors[1]][vectors[0]];
            char *grid_names = new char[vectors[1]*vectors[0]];
            // (時系列GridCoordinates名リストの取得 : GridCoordinatesPointers
            cg_array_read_as(n,Character,grid_names);
            // 時系列インデックスの名前の取得（１つのみ）
            strncpy(grid_name, grid_names+(iterative_index-1)*vectors[0], vectors[0]);
            this->trim(grid_name);

            delete []grid_names;
            return UDM_OK;
        }
    }

    // GridCoordinatesPointersが存在しない。グリッド座標の時系列座標はない。
    return UDM_ERROR;
}


/**
 * クラス情報を文字列出力する:デバッグ用.
 * @param buf        出力文字列
 */
void UdmGridCoordinates::toString(std::string& buf) const
{
#ifdef _DEBUG_TRACE
    std::stringstream stream;
    UdmSize_t count = 0;
    // XYZ座標
    std::vector<UdmNode*>::const_iterator itr;
    for (itr=this->node_list.begin(); itr!= this->node_list.end(); itr++) {
        std::string node_buf;
        (*itr)->toString(node_buf);
        stream << "GridCoordinates[" << (*itr)->getId() << "] : " << node_buf;
        stream  << std::endl;
    }

    buf += stream.str();
#endif

    return;
}

/**
 * グリッド構成ノード数を取得する.
 * @return        グリッド構成ノード
 */
UdmSize_t UdmGridCoordinates::getNumEntities() const
{
    return this->getNumNodes();
}

/**
 * ノードIDのグリッド構成ノードを取得する.
 * @param node_id        ノードID（１～）
 * @return        グリッド構成ノード
 */
UdmEntity* UdmGridCoordinates::getEntityById(UdmSize_t entity_id)
{
    return this->getNodeById(entity_id);
}

/**
 * ノードIDのグリッド構成ノードを取得する:const.
 * @param node_id        ノードID（１～）
 * @return        グリッド構成ノード
 */
const UdmEntity* UdmGridCoordinates::getEntityById(UdmSize_t entity_id) const
{
    return this->getNodeById(entity_id);
}

/**
 * CGNS:GridCoordinatesを出力する.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param timeslice_step       CGNS出力ステップ回数
 * @param grid_timeslice       CGNS:GridCoordinatesの時系列出力の有無:true=GridCoordinates時系列出力する
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::writeCgns(int index_file, int index_base, int index_zone, int timeslice_step, bool grid_timeslice)
{
    char gridname[33] = {0x00};
    int  index_grid, index_coord;
    UdmDataType_t datatype;
    DataType_t cgns_datatype;
    cgsize_t coords_size;
    int cg_error;
    UdmError_t udm_error = UDM_OK;

    // 出力CGNS:GridCoordinates名のクリア
    this->clearCgnsWriteGridCoordnates();

    // GridCoordinatesが存在するか
    sprintf(gridname, "GridCoordinates");
    if (cg_goto(index_file, index_base, "Zone_t", index_zone, gridname, 0, "end") == CG_OK) {
        if (!grid_timeslice) {
            // 時系列出力の無であるので、CGNS:GridCoordinatesの時系列出力しない。
            // GridCoordinatesが存在するので、CGNS:GridCoordinatesは出力しない。
            // 出力CGNS:GridCoordinates名を設定する.
            this->setCgnsWriteGridCoordnates(std::string(gridname));
            return UDM_OK;
        }
        char itr_name[33] = {0x00};
        if (this->getCgnsIterativeGridCoordinatesName(itr_name, index_file, index_base, index_zone, timeslice_step) == UDM_OK) {
            // 既存出力のGridCoordinates名を使用する.
            strcpy(gridname, itr_name);
        }
        else {
            // 時系列ステップ番号付きのGridCoordinates名を使用する.
            // GridCoordinates_%010d
            sprintf(gridname, UDM_CGNS_FORMAT_GRIDCOORDINATES, timeslice_step);
        }
    }

    // CGNS:GridCoordinatesの出力
    index_grid = this->findCgnsGridCoordinates(index_file, index_base, index_zone, gridname);
    if (index_grid <= 0) {
        if (cg_grid_write(index_file,index_base,index_zone, gridname, &index_grid) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : cg_grid_write(gridname=%s)", gridname);
        }
    }

    if (cg_goto(index_file,index_base, "Zone_t", index_zone, "GridCoordinates_t",index_grid, "end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : cg_goto(index_grid=%d)", index_grid);
    }

    udm_error = UDM_OK;
    cg_error = 0;
    datatype = this->getDataType();
    cgns_datatype = this->toCgnsDataType(datatype);
    coords_size = this->getNumNodes();

    if (datatype == Udm_RealSingle) {
        float *coords = new float[coords_size];

        // CoordinateX出力
        this->getGridCoordinatesX(1, coords_size, (float*)coords);
        if ((cg_error = cg_array_write("CoordinateX", cgns_datatype, 1, &coords_size, coords)) != CG_OK) {
            udm_error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : cg_array_write(CoordinateX)");
        }
        if (udm_error != UDM_OK) {
            delete  []coords;
            return udm_error;
        }

        // CoordinateY出力
        this->getGridCoordinatesY(1, coords_size, (float*)coords);
        if ((cg_error = cg_array_write("CoordinateY", cgns_datatype, 1, &coords_size, coords)) != CG_OK) {
            udm_error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : cg_array_write(CoordinateY)");
        }
        if (udm_error != UDM_OK) {
            delete  []coords;
            return udm_error;
        }

        // CoordinateZ出力
        this->getGridCoordinatesZ(1, coords_size, (float*)coords);
        if ((cg_error = cg_array_write("CoordinateZ", cgns_datatype, 1, &coords_size, coords)) != CG_OK) {
            udm_error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : cg_array_write(CoordinateZ)");
        }
        if (udm_error != UDM_OK) {
            delete  []coords;
            return udm_error;
        }

        delete  []coords;
    }
    else if (datatype == Udm_RealDouble) {
        double *coords = new double[coords_size];

        // CoordinateX出力
        this->getGridCoordinatesX(1, coords_size, (double*)coords);
        if ((cg_error = cg_array_write("CoordinateX", cgns_datatype, 1, &coords_size, coords)) != CG_OK) {
            udm_error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : cg_array_write(CoordinateX)");
        }
        if (udm_error != UDM_OK) {
            delete  []coords;
            return udm_error;
        }

        // CoordinateY出力
        this->getGridCoordinatesY(1, coords_size, (double*)coords);
        if ((cg_error = cg_array_write("CoordinateY", cgns_datatype, 1, &coords_size, coords)) != CG_OK) {
            udm_error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : cg_array_write(CoordinateY)");
        }
        if (udm_error != UDM_OK) {
            delete  []coords;
            return udm_error;
        }

        // CoordinateZ出力
        this->getGridCoordinatesZ(1, coords_size, (double*)coords);
        if ((cg_error = cg_array_write("CoordinateZ", cgns_datatype, 1, &coords_size, coords)) != CG_OK) {
            udm_error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : cg_array_write(CoordinateZ)");
        }
        if (udm_error != UDM_OK) {
            delete  []coords;
            return udm_error;
        }

        delete  []coords;
    }
    else {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "invalid coordinates datatype");
    }

    // 出力CGNS:GridCoordinates名を設定する.
    this->setCgnsWriteGridCoordnates(std::string(gridname));

    // CGNS:UdmRankConenctivityを書き込む.
    if (this->getRankConnectivity() != NULL) {
        if (this->getRankConnectivity()->writeCgns(index_file, index_base, index_zone) != UDM_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : UdmRankConnectivity::writeCgns(index_file=%d, index_base=%d, index_zone=%d)", index_file, index_base, index_zone);
        }
    }

    return UDM_OK;
}

/**
 * CGNS:GridCoordinatesのリンクを出力する.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param link_output_path        リンク出力ファイル
 * @param linked_files            リンクファイルリスト
 * @param timeslice_step       CGNS出力ステップ回数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::writeCgnsLinkFile(
                        int index_file,
                        int index_base,
                        int index_zone,
                        const std::string &link_output_path,
                        const std::vector<std::string> &linked_files,
                        int timeslice_step)
{
    UdmError_t error = UDM_OK;
    char basename[33] = {0x00};
    char zonename[33] = {0x00};
    char gridname[33] = {0x00};
    char linked_gridname[33] = {0x00};
    int index_linkfile;
    std::vector<std::string>::const_iterator itr;
    char path[256] = {0x00};
    std::string filename;
    int cell_dim, phys_dim;
    cgsize_t sizes[9] = {0x00};
    int cg_ret = 0;

    // 出力CGNS:GridCoordinates名のクリア
    this->clearCgnsWriteGridCoordnates();

    // リンクファイルにGridCoordinatesが存在するかチェックする.
    filename.clear();
    for (itr=linked_files.begin(); itr!= linked_files.end(); itr++) {
        filename = (*itr);
        // リンクファイルオープン
        if (cg_open(filename.c_str(), CG_MODE_READ, &index_linkfile) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_OPENERROR, "filename=%s, cgns_error=%s", filename.c_str(), cg_get_error());
        }
        // 出力リンクファイルと同じCGNSベースインデックス, CGNSゾーンインデックスとする.
        cg_ret = cg_base_read(index_linkfile, index_base, basename, &cell_dim, &phys_dim);
        cg_ret = cg_zone_read(index_linkfile, index_base, index_zone, zonename, sizes);

        // GridCoordinates名称の取得
        if ((error = this->getCgnsIterativeGridCoordinatesName(gridname, index_linkfile, index_base, index_zone, timeslice_step)) != UDM_OK) {
            strcpy(gridname, UDM_CGNS_NAME_GRIDCOORDINATES);
        }
        if (cg_goto(index_linkfile, index_base, "Zone_t", index_zone, gridname, 0, "end") != CG_OK) {
            continue;        // CGNS:GridCoordinatesが存在しない.
        }
        // リンク元GridCoordinates名
        strcpy(linked_gridname, gridname);

        cg_close(index_linkfile);
        break;
    }
    // linked_gridname:リンク元名, gridname:リンク先名
    if (strlen(linked_gridname) <= 0) {
        // GridCoordinatesはリンクしない。
        return UDM_OK;
    }
    if (filename.empty()) {
        return UDM_OK;
    }

    // リンク元GridCoordinates名が"GridCoordinates"であるので別名とする。
    if (strcmp(linked_gridname, UDM_CGNS_NAME_GRIDCOORDINATES) == 0) {
        // 出力リンクファイルにGridCoordinatesが存在するか
        if (cg_goto(index_file, index_base, "Zone_t", index_zone, UDM_CGNS_NAME_GRIDCOORDINATES, 0, "end") == CG_OK) {
            // 出力リンクファイルにGridCoordinatesが存在し、リンク元GridCoordinatesと同名であるので、別名とする.
            // GridCoordinates_%010d
            sprintf(gridname, UDM_CGNS_FORMAT_GRIDCOORDINATES, timeslice_step);
        }
    }
    // 出力リンクファイルにリンク別名と同じGridCoordinatesが存在することはない。
    if (cg_goto(index_file, index_base, "Zone_t", index_zone, gridname, 0, "end") == CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "exists linked GridCoordinates name[gridname=%s].", gridname);
    }

    // GridCoordinatesリンクの作成
    if (cg_goto(index_file, index_base, "Zone_t", index_zone, "end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO, "index_file=%d,index_base=%d,index_zone=%d", index_file, index_base, index_zone);
    }
    sprintf(path, "/%s/%s/%s", basename, zonename, linked_gridname);
    // リンクファイルをリンク出力ファイルからの相対パスに変換する.
    std::string linked_relativepath;
    this->getLinkedRelativePath(link_output_path, filename, linked_relativepath);

    // CGNSリンク出力
    if (cg_link_write(gridname, linked_relativepath.c_str(), path) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "failure:cg_link_write(%s,%s,%s)",gridname, filename.c_str(), path);
    }

    // 出力CGNS:GridCoordinates名を設定する.
    this->setCgnsWriteGridCoordnates(std::string(gridname));

    // CGNS:UdmRankConenctivityを書き込む.:実際に書き込む（リンク先が削除されることを考慮して、リンクとしない）
    if (this->getRankConnectivity() != NULL) {
        if (this->getRankConnectivity()->writeCgns(index_file, index_base, index_zone) != UDM_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : UdmRankConnectivity::writeCgns(index_file=%d, index_base=%d, index_zone=%d)", index_file, index_base, index_zone);
        }
    }

    return UDM_OK;
}

/**
 * 節点（ノード）グリッド座標を設定する.
 * 構成節点（ノード）はクリアして設定節点（ノード）数,グリッド座標に置き換える.
 * グリッド座標から節点（ノード）クラスを生成する.
 * @param num_nodes            節点（ノード）数
 * @param coords_x        グリッド座標X
 * @param coords_y        グリッド座標Y
 * @param coords_z        グリッド座標Z
 * @return        生成節点（ノード）数
 */
template<class DATA_TYPE>
UdmSize_t UdmGridCoordinates::setGridCoordinatesArray(
                    UdmSize_t num_nodes,
                    DATA_TYPE* coords_x,
                    DATA_TYPE* coords_y,
                    DATA_TYPE* coords_z)
{
    UdmSize_t n;

    // 節点（ノード）をクリアする.
    this->clearNodes();

    // ノードデータの作成
    for (n=0; n<num_nodes; n++) {
        UdmNode *node = NULL;
        node = new UdmNode(coords_x[n], coords_y[n], coords_z[n]);
        // ノードID、ランク番号を設定する
        node->addPreviousRankInfo(this->getCgnsRankno(), n+1);
        node->setId(n+1);
        node->setMyRankno(this->getCgnsRankno());

        // 節点（ノード）の挿入
        this->pushbackNode(node);
    }
    return num_nodes;
}
template UdmSize_t UdmGridCoordinates::setGridCoordinatesArray(UdmSize_t num_nodes, float* coords_x, float* coords_y, float* coords_z);
template UdmSize_t UdmGridCoordinates::setGridCoordinatesArray(UdmSize_t num_nodes, double* coords_x, double* coords_y, double* coords_z);

/**
 * 節点（ノード）グリッド座標を取得する.
 * @param [in]  node_id        節点（ノード）ID：１～
 * @param [out] x                X座標
 * @param [out] y                Y座標
 * @param [out] z                Z座標
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class DATA_TYPE>
UdmError_t UdmGridCoordinates::getGridCoordinates(
                        UdmSize_t node_id,
                        DATA_TYPE &x,
                        DATA_TYPE &y,
                        DATA_TYPE &z) const
{
    const UdmNode *node = this->getNodeById(node_id);
    if (node == NULL) {
        return UDM_ERROR;
    }
    // グリッド座標を取得する.
    node->getCoords(x, y, z);

    return UDM_OK;
}
template UdmError_t UdmGridCoordinates::getGridCoordinates(UdmSize_t node_id, float &x, float &y, float &z) const;
template UdmError_t UdmGridCoordinates::getGridCoordinates(UdmSize_t node_id, double &x, double &y, double &z) const;


/**
 * 節点（ノード）グリッド座標を設定する.
 * @param  node_id        節点（ノード）ID：１～
 * @param  x                X座標
 * @param  y                Y座標
 * @param  z                Z座標
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class DATA_TYPE>
UdmError_t UdmGridCoordinates::setGridCoordinates(
                        UdmSize_t node_id,
                        DATA_TYPE x,
                        DATA_TYPE y,
                        DATA_TYPE z)
{
    UdmNode *node = this->getNodeById(node_id);
    if (node == NULL) {
        return UDM_ERROR;
    }
    // グリッド座標を設定する.
    node->setCoords(x, y, z);

    return UDM_OK;
}
template UdmError_t UdmGridCoordinates::setGridCoordinates(UdmSize_t node_id, float x, float y, float z);
template UdmError_t UdmGridCoordinates::setGridCoordinates(UdmSize_t node_id, double x, double y, double z);


/**
 * 節点（ノード）グリッド座標を追加する.
 * @param x        グリッド座標X
 * @param y        グリッド座標Y
 * @param z        グリッド座標Z
 * @return        節点（ノード）ID
 */
template<class DATA_TYPE>
UdmSize_t UdmGridCoordinates::insertGridCoordinates(
                            DATA_TYPE x,
                            DATA_TYPE y,
                            DATA_TYPE z)
{
    UdmSize_t n;
    // ノードデータの作成
    UdmNode *node = new UdmNode(x, y, z);
    return this->insertNode(node);;
}
template UdmSize_t UdmGridCoordinates::insertGridCoordinates(float x, float y, float z);
template UdmSize_t UdmGridCoordinates::insertGridCoordinates(double x, double y, double z);



/**
 * 構成節点（ノード）のグリッド座標を取得する.
 * @param start_id            取得開始ID
 * @param end_id            取得終了ID
 * @param coords_x        グリッド座標X
 * @param coords_y        グリッド座標Y
 * @param coords_z        グリッド座標Z
 * @return        取得節点（ノード）数
 */
template<class DATA_TYPE>
UdmSize_t UdmGridCoordinates::getGridCoordinatesArray(
                        UdmSize_t start_id,
                        UdmSize_t end_id,
                        DATA_TYPE* coords_x,
                        DATA_TYPE* coords_y,
                        DATA_TYPE* coords_z)
{
    if (start_id <= 0) return 0;
    if (start_id > end_id) return 0;
    if (this->node_list.size() <= 0) return 0;
    if (end_id > this->node_list.size()) return 0;
    UdmSize_t n;
    UdmSize_t id = 0;
    for (n=start_id-1; n<=end_id-1; n++) {
        this->node_list[n]->getCoords(coords_x[id], coords_y[id], coords_z[id]);
        ++id;
    }
    return id;
}
template UdmSize_t UdmGridCoordinates::getGridCoordinatesArray(UdmSize_t start_id, UdmSize_t end_id, float* coords_x, float* coords_y, float* coords_z);
template UdmSize_t UdmGridCoordinates::getGridCoordinatesArray(UdmSize_t start_id, UdmSize_t end_id, double* coords_x, double* coords_y, double* coords_z);


/**
 * 構成節点（ノード）のグリッド座標Xを取得する.
 * @param start_id            取得開始ID
 * @param end_id            取得終了ID
 * @param coords        グリッド座標X
 * @return        取得節点（ノード）数
 */
template<class DATA_TYPE>
UdmSize_t UdmGridCoordinates::getGridCoordinatesX(
                        UdmSize_t start_id,
                        UdmSize_t end_id,
                        DATA_TYPE* coords)
{
    if (start_id <= 0) return 0;
    if (start_id > end_id) return 0;
    if (this->node_list.size() <= 0) return 0;
    if (end_id > this->node_list.size()) return 0;
    UdmSize_t n;
    UdmSize_t id = 0;
    DATA_TYPE coords_x, coords_y, coords_z;
    for (n=start_id-1; n<=end_id-1; n++) {
        this->node_list[n]->getCoords(coords_x, coords_y, coords_z);
        coords[id++] = coords_x;
    }
    return id;
}
template UdmSize_t UdmGridCoordinates::getGridCoordinatesX(UdmSize_t start_id, UdmSize_t end_id, float* coords);
template UdmSize_t UdmGridCoordinates::getGridCoordinatesX(UdmSize_t start_id, UdmSize_t end_id, double* coords);


/**
 * 構成節点（ノード）のグリッド座標Yを取得する.
 * @param start_id            取得開始ID
 * @param end_id            取得終了ID
 * @param coords        グリッド座標Y
 * @return        取得節点（ノード）数
 */
template<class DATA_TYPE>
UdmSize_t UdmGridCoordinates::getGridCoordinatesY(
                        UdmSize_t start_id,
                        UdmSize_t end_id,
                        DATA_TYPE* coords)
{
    if (start_id <= 0) return 0;
    if (start_id > end_id) return 0;
    if (this->node_list.size() <= 0) return 0;
    if (end_id > this->node_list.size()) return 0;
    UdmSize_t n;
    UdmSize_t id = 0;
    DATA_TYPE coords_x, coords_y, coords_z;
    for (n=start_id-1; n<=end_id-1; n++) {
        this->node_list[n]->getCoords(coords_x, coords_y, coords_z);
        coords[id++] = coords_y;
    }
    return id;
}
template UdmSize_t UdmGridCoordinates::getGridCoordinatesY(UdmSize_t start_id, UdmSize_t end_id, float* coords);
template UdmSize_t UdmGridCoordinates::getGridCoordinatesY(UdmSize_t start_id, UdmSize_t end_id, double* coords);

/**
 * 構成節点（ノード）のグリッド座標Zを取得する.
 * @param start_id            取得開始ID
 * @param end_id            取得終了ID
 * @param coords        グリッド座標Z
 * @return        取得節点（ノード）数
 */
template<class DATA_TYPE>
UdmSize_t UdmGridCoordinates::getGridCoordinatesZ(
                        UdmSize_t start_id,
                        UdmSize_t end_id,
                        DATA_TYPE* coords)
{
    if (start_id <= 0) return 0;
    if (start_id > end_id) return 0;
    if (this->node_list.size() <= 0) return 0;
    if (end_id > this->node_list.size()) return 0;
    UdmSize_t n;
    UdmSize_t id = 0;
    DATA_TYPE coords_x, coords_y, coords_z;
    for (n=start_id-1; n<=end_id-1; n++) {
        this->node_list[n]->getCoords(coords_x, coords_y, coords_z);
        coords[id++] = coords_z;
    }
    return id;
}
template UdmSize_t UdmGridCoordinates::getGridCoordinatesZ(UdmSize_t start_id, UdmSize_t end_id, float* coords);
template UdmSize_t UdmGridCoordinates::getGridCoordinatesZ(UdmSize_t start_id, UdmSize_t end_id, double* coords);

/**
 * 仮想節点（ノード）のグリッド座標を取得する.
 * @param start_id            取得開始ID
 * @param end_id            取得終了ID
 * @param coords_x        グリッド座標X
 * @param coords_y        グリッド座標Y
 * @param coords_z        グリッド座標Z
 * @return        取得節点（ノード）数
 */
template<class DATA_TYPE>
UdmSize_t UdmGridCoordinates::getGridCoordinatesArrayOfVirtual(
                        UdmSize_t start_id,
                        UdmSize_t end_id,
                        DATA_TYPE* coords_x,
                        DATA_TYPE* coords_y,
                        DATA_TYPE* coords_z)
{
    if (start_id <= 0) return 0;
    if (start_id > end_id) return 0;
    if (this->virtual_nodes.size() <= 0) return 0;
    if (end_id > this->virtual_nodes.size()) return 0;
    UdmSize_t n;
    UdmSize_t id = 0;
    for (n=start_id-1; n<=end_id-1; n++) {
        this->virtual_nodes[n]->getCoords(coords_x[id], coords_y[id], coords_z[id]);
        ++id;
    }
    return id;
}
template UdmSize_t UdmGridCoordinates::getGridCoordinatesArrayOfVirtual(UdmSize_t start_id, UdmSize_t end_id, float* coords_x, float* coords_y, float* coords_z);
template UdmSize_t UdmGridCoordinates::getGridCoordinatesArrayOfVirtual(UdmSize_t start_id, UdmSize_t end_id, double* coords_x, double* coords_y, double* coords_z);


/**
 * 仮想節点（ノード）のグリッド座標Xを取得する.
 * @param start_id            取得開始ID
 * @param end_id            取得終了ID
 * @param coords        グリッド座標X
 * @return        取得節点（ノード）数
 */
template<class DATA_TYPE>
UdmSize_t UdmGridCoordinates::getGridCoordinatesXOfVirtual(
                        UdmSize_t start_id,
                        UdmSize_t end_id,
                        DATA_TYPE* coords)
{
    if (start_id <= 0) return 0;
    if (start_id > end_id) return 0;
    if (this->virtual_nodes.size() <= 0) return 0;
    if (end_id > this->virtual_nodes.size()) return 0;
    UdmSize_t n;
    UdmSize_t id = 0;
    DATA_TYPE coords_x, coords_y, coords_z;
    for (n=start_id-1; n<=end_id-1; n++) {
        this->virtual_nodes[n]->getCoords(coords_x, coords_y, coords_z);
        coords[id++] = coords_x;
    }
    return id;
}
template UdmSize_t UdmGridCoordinates::getGridCoordinatesXOfVirtual(UdmSize_t start_id, UdmSize_t end_id, float* coords);
template UdmSize_t UdmGridCoordinates::getGridCoordinatesXOfVirtual(UdmSize_t start_id, UdmSize_t end_id, double* coords);


/**
 * 仮想節点（ノード）のグリッド座標Yを取得する.
 * @param start_id            取得開始ID
 * @param end_id            取得終了ID
 * @param coords        グリッド座標Y
 * @return        取得節点（ノード）数
 */
template<class DATA_TYPE>
UdmSize_t UdmGridCoordinates::getGridCoordinatesYOfVirtual(
                        UdmSize_t start_id,
                        UdmSize_t end_id,
                        DATA_TYPE* coords)
{
    if (start_id <= 0) return 0;
    if (start_id > end_id) return 0;
    if (this->virtual_nodes.size() <= 0) return 0;
    if (end_id > this->virtual_nodes.size()) return 0;
    UdmSize_t n;
    UdmSize_t id = 0;
    DATA_TYPE coords_x, coords_y, coords_z;
    for (n=start_id-1; n<=end_id-1; n++) {
        this->virtual_nodes[n]->getCoords(coords_x, coords_y, coords_z);
        coords[id++] = coords_y;
    }
    return id;
}
template UdmSize_t UdmGridCoordinates::getGridCoordinatesYOfVirtual(UdmSize_t start_id, UdmSize_t end_id, float* coords);
template UdmSize_t UdmGridCoordinates::getGridCoordinatesYOfVirtual(UdmSize_t start_id, UdmSize_t end_id, double* coords);


/**
 * 仮想節点（ノード）のグリッド座標Zを取得する.
 * @param start_id            取得開始ID
 * @param end_id            取得終了ID
 * @param coords        グリッド座標Z
 * @return        取得節点（ノード）数
 */
template<class DATA_TYPE>
UdmSize_t UdmGridCoordinates::getGridCoordinatesZOfVirtual(
                        UdmSize_t start_id,
                        UdmSize_t end_id,
                        DATA_TYPE* coords)
{
    if (start_id <= 0) return 0;
    if (start_id > end_id) return 0;
    if (this->virtual_nodes.size() <= 0) return 0;
    if (end_id > this->virtual_nodes.size()) return 0;
    UdmSize_t n;
    UdmSize_t id = 0;
    DATA_TYPE coords_x, coords_y, coords_z;
    for (n=start_id-1; n<=end_id-1; n++) {
        this->virtual_nodes[n]->getCoords(coords_x, coords_y, coords_z);
        coords[id++] = coords_z;
    }
    return id;
}
template UdmSize_t UdmGridCoordinates::getGridCoordinatesZOfVirtual(UdmSize_t start_id, UdmSize_t end_id, float* coords);
template UdmSize_t UdmGridCoordinates::getGridCoordinatesZOfVirtual(UdmSize_t start_id, UdmSize_t end_id, double* coords);

/**
 * 物理量フィールド情報を取得する.
 * @param solution_name        物理量データ名称
 * @return        物理量フィールド情報
 */
const UdmSolutionFieldConfig* UdmGridCoordinates::getSolutionFieldConfig(const std::string& solution_name) const
{
    const UdmZone* zone = this->getParentZone();
    if (zone == NULL) return NULL;
    const UdmFlowSolutions* solutions = zone->getFlowSolutions();
    if (solutions == NULL) return NULL;

    return solutions->getSolutionField(solution_name);
}

/**
 * 出力CGNS:GrdiCoordinates名を取得する.
 * @return        出力CGNS:GrdiCoordinates名
 */
const std::string& UdmGridCoordinates::getCgnsWriteGridCoordnates() const
{
    return this->cgns_writegridcoordnates;
}

/**
 * 出力CGNS:GrdiCoordinates名を設定する.
 * @param gridcoordnates_name        出力CGNS:GrdiCoordinates名
 */
void UdmGridCoordinates::setCgnsWriteGridCoordnates(const std::string& gridcoordnates_name)
{
    this->cgns_writegridcoordnates = gridcoordnates_name;
}


/**
 * 出力CGNS:GrdiCoordinates名をクリアする.
 */
void UdmGridCoordinates::clearCgnsWriteGridCoordnates()
{
    this->cgns_writegridcoordnates.clear();
}

/**
 * 出力結果CGNS:GridCoordnates名があるかチェックする.
 * 出力結果CGNS:GridCoordnates名がある場合は１と返す。
 * @return        1=出力結果CGNS:GridCoordnates名がある, 0=出力結果CGNS:GridCoordnates名がない。
 */
int UdmGridCoordinates::getNumCgnsWriteGridCoordnates() const
{
    return (this->cgns_writegridcoordnates.empty()?0:1);
}



/**
 * CGNS出力前の初期化を行う.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::initializeWriteCgns()
{
    this->clearCgnsWriteGridCoordnates();
    return UDM_OK;
}


/**
 * 物理量データの初期化を行う.
 * @param solution_name        物理量名称
 * @param value                初期設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmGridCoordinates::initializeValueEntities(const std::string& solution_name, VALUE_TYPE value)
{
    // 物理量名称が存在するか？ノード（頂点）定義物理量であるかチェックする.
    const UdmZone* zone = this->getParentZone();
    if (zone == NULL) return UDM_ERROR;
    const UdmFlowSolutions* solutions = zone->getFlowSolutions();
    if (solutions == NULL) return UDM_ERROR;
    const UdmSolutionFieldConfig* config = solutions->getSolutionField(solution_name);
    if (config == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "not found solution_name=%s", solution_name.c_str());
    }
    if (config->getGridLocation() != Udm_Vertex) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "solution_name[%s] is not Vertex.", solution_name.c_str());
    }

    std::vector<UdmNode*>::iterator itr;
    for (itr=this->node_list.begin(); itr!=this->node_list.end(); itr++) {
        UdmNode* node = (*itr);
        node->initializeValue(solution_name, value);
    }

    return UDM_OK;
}
template UdmError_t UdmGridCoordinates::initializeValueEntities<int>(const std::string& solution_name, int value);
template UdmError_t UdmGridCoordinates::initializeValueEntities<long long>(const std::string& solution_name, long long value);
template UdmError_t UdmGridCoordinates::initializeValueEntities<float>(const std::string& solution_name, float value);
template UdmError_t UdmGridCoordinates::initializeValueEntities<double>(const std::string& solution_name, double value);

/**
 * MPIランク番号を取得する.
 * @return        MPIランク番号
 */
int UdmGridCoordinates::getMpiRankno() const
{
    if (this->getParentZone() == NULL) return -1;
    if (this->getParentZone()->getParentModel() == NULL) return -1;
    return this->getParentZone()->getParentModel()->getMpiRankno();
}

/**
 * MPIプロセス数を取得する.
 * @return        MPIプロセス数
 */
int UdmGridCoordinates::getMpiProcessSize() const
{
    if (this->getParentZone() == NULL) return -1;
    if (this->getParentZone()->getParentModel() == NULL) return -1;
    return this->getParentZone()->getParentModel()->getMpiProcessSize();
}


/**
 * CGNS:UdmInfoのランク番号を取得する.
 * @return        CGNS:UdmInfoランク番号
 */
int UdmGridCoordinates::getCgnsRankno() const
{
    if (this->getParentZone() == NULL) return -1;
    if (this->getParentZone()->getParentModel() == NULL) return -1;
    return this->getParentZone()->getParentModel()->getCgnsRankno();
}

/**
 * ランク番号とIDが一致している節点（ノード）を検索する.
 * 節点（ノード）の現在のランク番号とIDから検索する.
 * 存在しなければ、以前のランク番号とIDから検索する.
 * @param src_rankno        ランク番号
 * @param src_nodeid        節点（ノード）ID
 * @return        節点（ノード）
 */
UdmNode* UdmGridCoordinates::findNodeByGlobalId(int src_rankno, UdmSize_t src_nodeid) const
{
    if (src_rankno < 0) return NULL;
    if (src_nodeid <= 0) return NULL;

    UdmNode *find_node = NULL;

    // グリッド構成ノードリストの配列添字として検索
    find_node = this->getNodeById(src_nodeid);
    if (find_node != NULL) {
        if (find_node->getMyRankno() == src_rankno) {
            return find_node;
        }
        find_node = NULL;
    }

    // ２分探索でグローバルIDを検索する
    std::vector<UdmNode*>::const_iterator find_itr;
    find_itr = this->searchCurrentGlobalId(this->node_list, src_rankno, src_nodeid);
    if (find_itr != this->node_list.end()) {
        find_node = (*find_itr);
        return find_node;
    }

    // 以前のグローバルIDを検索する
    find_itr = this->searchPreviousGlobalId(this->node_list, src_rankno, src_nodeid);
    if (find_itr != this->node_list.end()) {
        find_node = (*find_itr);
        return find_node;
    }

    return NULL;
}


/**
 * Zoltan分割によるインポート節点（ノード）を追加する.
 * @param import_nodes        インポート節点（ノード）リスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::importNodes(const std::vector<UdmNode*>& import_nodes)
{
    UdmRankConnectivity* inner = this->getRankConnectivity();
    if (inner == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "UdmRankConnectivity is null");
    }
    std::vector<UdmNode*>::const_iterator itr;
    std::vector<UdmNode*>::const_iterator find_itr;
    std::vector<UdmNode*> insert_nodes;

    // 内部境界節点（ノード）のランク番号、ID検索テーブルを作成する。
    inner->createSearchTable();

    for (itr=import_nodes.begin(); itr!=import_nodes.end(); itr++) {
        UdmNode *find_node = inner->findMpiRankInfo(*itr);
        // 既存節点（ノード）であるかチェックする.
        if (find_node != NULL) continue;
        // 追加ノードの格納
        insert_nodes.push_back(*itr);
    }

    // 格納した追加ノードを追加する
    for (itr=insert_nodes.begin(); itr!=insert_nodes.end(); itr++) {
        // インポート節点（ノード）を追加する.
        this->insertNode((*itr));
        // 無効MPIランク情報を削除する
        (*itr)->removeMpiRankInfo((*itr)->getMyRankno(), 0);
    }

    return UDM_OK;
}


/**
 * 仮想節点（ノード）を追加する.
 * @param virtual_nodes        仮想節点（ノード）リスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::importVirtualNodes(const std::vector<UdmNode*>& virtual_nodes)
{
    std::vector<UdmNode*>::const_iterator itr;
    std::vector<UdmNode*>::const_iterator found_itr;
    for (itr=virtual_nodes.begin(); itr!=virtual_nodes.end(); itr++) {
        UdmNode* node = (*itr);
        if (node->getRealityType() != Udm_Virtual) continue;

        // 既存仮想節点（ノード）が存在するかチェックする.
        found_itr  = this->searchGlobalId(this->virtual_nodes, node);
        if (found_itr != this->virtual_nodes.end()) {
            continue;
        }

        // インポート節点（ノード）を追加する.
        this->insertVirtualNode(node);
    }

    // 仮想要素（セル）ローカルIDの再構築
    this->rebuildVirtualNodes();

    return UDM_OK;
}


/**
 * 内部境界管理クラスを取得する.
 * @return        内部境界管理クラス
 */
UdmRankConnectivity* UdmGridCoordinates::getRankConnectivity() const
{
    if (this->parent_zone == NULL) return NULL;
    return this->parent_zone->getRankConnectivity();
}

/**
 * 内部境界リストから節点（ノード）を削除する.
 * @param node            削除節点（ノード）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::removeRankConnectivity(const UdmEntity* node) const
{
    if (node == NULL) return UDM_ERROR;
    UdmRankConnectivity* inner = this->getRankConnectivity();
    if (inner == NULL) {
        return UDM_ERROR;
    }
    // 内部境界リストから削除する.
    inner->removeBoundaryNode(node);

    return UDM_OK;
}

/**
 * 節点（ノード）のIDの再構築を行う。
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::rebuildNodes()
{
    UdmSize_t node_id = 0;
    std::vector<UdmNode*>::iterator itr;
    for (itr=this->node_list.begin(); itr!= this->node_list.end(); itr++) {
        UdmNode *node = (*itr);
        // 現在のIDを以前のIDに設定する.
        node_id = ++node_id;
        //if (node->getMyRankno() != this->getMpiRankno() || node->getId() != node_id) {
            node->addPreviousRankInfo(node->getMyRankno(), node->getId());
        //}
        node->setId(node_id);            // IDをインクリメントして設定する
        node->setLocalId(node_id);            // ローカルIDを設定する
        node->setMyRankno(this->getMpiRankno());
    }
    // 設定済み最大節点（ノード）ID
    this->max_nodeid = node_id;

    return UDM_OK;
}



/**
 * グリッド座標の基本情報を全プロセスに送信する.
 * すべてのプロセスにて同一グリッド座標情報が存在するかチェックする.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::brodcastGridCoordinates()
{
    int mpi_flag;
    int mpi_rankno;
    UdmError_t error = UDM_OK;

    // MPI初期化済みであるかチェックする.
    udm_mpi_initialized(&mpi_flag);
    if (!mpi_flag) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_MPI, "Please execute MPI_Init beforehand.");
    }
    if (!(this->getMpiRankno() >= 0 && this->getMpiProcessSize() >= 1)) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_MPI, "invalid mpi process [mpi_rankno=%d,mpi_num_process=%d].", this->getMpiRankno(), this->getMpiProcessSize());
    }

    mpi_rankno = this->getMpiRankno();
    UdmSerializeBuffer streamBuffer;
    UdmSerializeArchive archive(&streamBuffer);

    int  buf_size = 0;
    char* buf = NULL;
    if (mpi_rankno == 0) {
        // シリアライズを行う:バッファーサイズ取得
        archive << *this;
        buf_size = archive.getOverflowSize();
        if (buf_size > 0) {
            // バッファー作成
            buf = new char[buf_size];
            streamBuffer.initialize(buf, buf_size);
            // シリアライズを行う
            archive << *this;
        }
    }

    // バッファーサイズ送信
    udm_mpi_bcast(&buf_size, 1, MPI_INT, 0, this->getMpiComm());
    if (buf_size <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_SERIALIZE, "buffer size is zero.");
    }

    if (buf == NULL) {
        // バッファー作成
        buf = new char[buf_size];
    }

    // UdmModelシリアライズバッファー送信
    udm_mpi_bcast(buf, buf_size, MPI_CHAR, 0, this->getMpiComm());

    // デシリアライズ
    UdmGridCoordinates mpi_grid;
    streamBuffer.initialize(buf, buf_size);
    archive >> mpi_grid;
    if (buf != NULL) {
        delete []buf;
        buf = NULL;
    }

    // グリッド座標情報が作成済みであるかチェックする.
    if (this->getNumNodes() == 0) {
        // グリッド座標情報未作成であるので、グリッド座標情報をコピーする.
        error = this->cloneGridCoordinates(mpi_grid);
        if (error != UDM_OK) {
            UDM_ERRORNO_HANDLER(error);
        }
    }
    // グリッド座標情報が存在するので、グリッド座標情報が同一であるかチェックする.
    else if (!this->equalsGridCoordinates(mpi_grid)) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "not equals GridCoordinates.");
    }

    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }

    return UDM_OK;
}


/**
 * グリッド座標の基本情報のシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmGridCoordinates::serialize(UdmSerializeArchive& archive) const
{
    // Genaral基本情報
    // ID : CGNSノードID、要素ローカルID, ノードローカルID
    // CGNSノードのデータ型
    UdmGeneral::serializeGeneralBase(archive, this->getId(), this->getDataType(), this->getName());

    return archive;
}


/**
 * グリッド座標の基本情報のデシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmGridCoordinates::deserialize(UdmSerializeArchive& archive)
{
    UdmSize_t grid_id;
    UdmDataType_t grid_datatype;
    std::string grid_name;

    // Genaral基本情報
    UdmGeneral::deserializeGeneralBase(archive, grid_id, grid_datatype, grid_name);
    this->setId(grid_id);
    this->setDataType(grid_datatype);
    this->setName(grid_name);

    return archive;
}


/**
 * グリッド座標の基本情報をコピーする.
 * @param src        コピー元グリッド座標
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::cloneGridCoordinates(const UdmGridCoordinates& src)
{
    // ID
    this->setId(src.getId());
    // データ型
    this->setDataType(src.getDataType());
    // 名前
    this->setName(src.getName());

    return UDM_OK;
}

/**
 * 同一グリッド座標情報であるかチェックする.
 * グリッド座標情報の基本情報が同じであるかチェックする.
 * @param model        チェック対象グリッド座標情報
 * @return            true=同一
 */
bool UdmGridCoordinates::equalsGridCoordinates(const UdmGridCoordinates& grid) const
{
    if (this->getId() != grid.getId()) return false;
    if (this->getDataType() != grid.getDataType()) return false;
    if (this->getName() != grid.getName()) return false;

    return true;
}

/**
 * 仮想節点（ノード）リストを取得する.
 * @return        仮想節点（ノード）リスト
 */
const std::vector<UdmNode*>& UdmGridCoordinates::getVirtualNodes() const
{
    return this->virtual_nodes;
}

/**
 * CGNS:GridCoordinatesが出力済みであるかチェックする.
 * @param filename        CGNSファイル名
 * @param index_base        CGNSベースインデックス：デフォルト=1
 * @param index_zone        CGNSゾーンインデックス：デフォルト=1
 * @return        true=CGNS:GridCoordinatesが出力済み
 */
bool UdmGridCoordinates::existsCgnsGridCoordnates(
                        const std::string& filename,
                        int index_base,
                        int index_zone)
{
    int index_file;
    // CGNSファイルオープン
    if (cg_open(filename.c_str(), CG_MODE_READ, &index_file) != CG_OK) {
        // UDM_ERROR_HANDLER(UDM_ERROR_CGNS_OPENERROR, "filename=%s, cgns_error=%s", filename.c_str(), cg_get_error());
        return false;
    }

    if (cg_goto(index_file, index_base, "Zone_t", index_zone, UDM_CGNS_NAME_GRIDCOORDINATES, 0, "end") != CG_OK) {
        return false;        // CGNS:GridCoordinatesが存在しない.
    }

    return true;
}

/**
 * 自ランク番号以下と接続している節点（ノード）を除いた節点（ノード）数を取得する。
 * 節点（ノード）数ー（接続節点数ー自ランク番号以上の接続節点数）
 * @return        自ランク番号以上の接続る節点数
 */
UdmSize_t UdmGridCoordinates::getNumNodesWithoutLessRankno() const
{
    UdmSize_t num_nodes = this->getNumNodes();
    UdmRankConnectivity* rank_conn = this->getRankConnectivity();
    if (rank_conn == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "UdmRankConnectivity is null");
    }

    // 接続節点数
    UdmSize_t num_conns = rank_conn->getNumBoundaryNodes();
    if (num_conns <= 0) return num_nodes;
    if (num_conns > num_nodes) return num_nodes;

    // 自ランク番号以上の接続節点数
    UdmSize_t num_conn_ranks = rank_conn->getNumNodesWithoutLessRankno();
    if (num_conns < num_conn_ranks) return num_nodes;

    return num_nodes - (num_conns - num_conn_ranks);
}

/**
 * GridCoordinates名称のGridCoordinatesインデックスを取得する.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param gridname            GridCoordinates名称
 * @return        GridCoordinatesインデックス（GridCoordinates名称が存在しない場合は、0を返す)
 */
int UdmGridCoordinates::findCgnsGridCoordinates(
                        int index_file,
                        int index_base,
                        int index_zone,
                        const char* gridname) const
{
    int num_grids = 0;
    int n;
    char cgns_name[33] = {0x00};

    cg_ngrids(index_file, index_base, index_zone, &num_grids);
    if (num_grids <= 0) {
        return 0;
    }
    for (n=1; n<num_grids; n++) {
        cg_grid_read(index_file, index_base, index_zone, n, cgns_name);
        if (strcmp(cgns_name, gridname) == 0) {
            return n;
        }
    }
    return 0;
}


/**
 * 節点（ノード）の物理量データ値を取得する:スカラデータ.
 * @param [in]  node_id        節点（ノード）ID（=１～）
 * @param [in]  solution_name        物理量名
 * @param [out]  value                物理量データ値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmGridCoordinates::getSolutionScalar(
                    UdmSize_t node_id,
                    const std::string& solution_name,
                    VALUE_TYPE& value) const
{
    UdmEntity *entity = this->getNodeById(node_id);
    if (entity == NULL) return UDM_ERROR;
    return entity->getSolutionScalar<VALUE_TYPE>(solution_name, value);
}

/**
 * 節点（ノード）の物理量データ値を取得する:ベクトルデータ.
 * @param [in]  node_id        節点（ノード）ID（=１～）
 * @param [in]  solution_name        物理量名
 * @param [out]  values                物理量データ値リスト
 * @return      ベクトルデータ数
 */
template<class VALUE_TYPE>
unsigned int UdmGridCoordinates::getSolutionVector(
                    UdmSize_t node_id,
                    const std::string& solution_name,
                    VALUE_TYPE* values) const
{
    UdmEntity *entity = this->getNodeById(node_id);
    if (entity == NULL) return 0;
    return entity->getSolutionVector<VALUE_TYPE>(solution_name, values);
}

/**
 * 節点（ノード）の物理量データ値を設定する:スカラデータ
 * @param node_id                節点（ノード）ID（=１～）
 * @param solution_name            物理量名
 * @param value                    物理量データ値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmGridCoordinates::setSolutionScalar(
                        UdmSize_t node_id,
                        const std::string& solution_name,
                        VALUE_TYPE value)
{
    UdmEntity *entity = this->getNodeById(node_id);
    if (entity == NULL) return UDM_ERROR;

    return entity->setSolutionScalar<VALUE_TYPE>(solution_name, value);
}

/**
 * 節点（ノード）の物理量データ値を設定する:ベクトルデータ
 * @param node_id                節点（ノード）ID（=１～）
 * @param solution_name            物理量名
 * @param values                    物理量データリスト
 * @param size                    物理量データ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmGridCoordinates::setSolutionVector(
                        UdmSize_t node_id,
                        const std::string& solution_name,
                        const VALUE_TYPE* values,
                        unsigned int size)
{
    UdmEntity *entity = this->getNodeById(node_id);
    if (entity == NULL) return UDM_ERROR;

    return entity->setSolutionVector<VALUE_TYPE>(solution_name, values, size);
}

template UdmError_t UdmGridCoordinates::getSolutionScalar<int>(UdmSize_t node_id, const std::string& solution_name, int& value) const;
template UdmError_t UdmGridCoordinates::getSolutionScalar<long long>(UdmSize_t node_id, const std::string& solution_name, long long& value) const;
template UdmError_t UdmGridCoordinates::getSolutionScalar<float>(UdmSize_t node_id, const std::string& solution_name, float& value) const;
template UdmError_t UdmGridCoordinates::getSolutionScalar<double>(UdmSize_t node_id, const std::string& solution_name, double& value) const;

template unsigned int UdmGridCoordinates::getSolutionVector<int>(UdmSize_t node_id, const std::string& solution_name, int* values) const;
template unsigned int UdmGridCoordinates::getSolutionVector<long long>(UdmSize_t node_id, const std::string& solution_name, long long* values) const;
template unsigned int UdmGridCoordinates::getSolutionVector<float>(UdmSize_t node_id, const std::string& solution_name, float* values) const;
template unsigned int UdmGridCoordinates::getSolutionVector<double>(UdmSize_t node_id, const std::string& solution_name, double* values) const;

template UdmError_t UdmGridCoordinates::setSolutionScalar<int>(UdmSize_t node_id, const std::string& solution_name, int value);
template UdmError_t UdmGridCoordinates::setSolutionScalar<long long>(UdmSize_t node_id, const std::string& solution_name, long long value);
template UdmError_t UdmGridCoordinates::setSolutionScalar<float>(UdmSize_t node_id, const std::string& solution_name, float value);
template UdmError_t UdmGridCoordinates::setSolutionScalar<double>(UdmSize_t node_id, const std::string& solution_name, double value);

template UdmError_t UdmGridCoordinates::setSolutionVector<int>(UdmSize_t node_id, const std::string& solution_name, const int* values, unsigned int size);
template UdmError_t UdmGridCoordinates::setSolutionVector<long long>(UdmSize_t node_id, const std::string& solution_name, const long long* values, unsigned int size);
template UdmError_t UdmGridCoordinates::setSolutionVector<float>(UdmSize_t node_id, const std::string& solution_name, const float* values, unsigned int size);
template UdmError_t UdmGridCoordinates::setSolutionVector<double>(UdmSize_t node_id, const std::string& solution_name, const double* values, unsigned int size);

/**
 * CGNS:GridCoordinatesを出力する.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::writeVirtualNodes(int index_file, int index_base, int index_zone)
{
    char gridname[33] = {0x00};
    int  index_grid;
    UdmDataType_t datatype = Udm_RealSingle;
    DataType_t cgns_datatype;
    cgsize_t coords_size;
    cgsize_t virtual_size;
    cgsize_t actual_size;
    int cg_error = 0;
    UdmError_t udm_error = UDM_OK;

    // 出力CGNS:GridCoordinates名のクリア
    this->clearCgnsWriteGridCoordnates();

    // ローカルIDを設定する.
    this->rebuildVirtualNodes();

    // GridCoordinatesが存在するか
    sprintf(gridname, "GridCoordinates");
    cg_error = 0;
    udm_error = UDM_OK;
    // CGNS:GridCoordinatesの出力先
    index_grid = this->findCgnsGridCoordinates(index_file, index_base, index_zone, gridname);
    if (index_grid <= 0) {
        if (cg_grid_write(index_file,index_base,index_zone, gridname, &index_grid) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : cg_grid_write(gridname=%s)", gridname);
        }
    }

    if (cg_goto(index_file,index_base, "Zone_t", index_zone, "GridCoordinates_t",index_grid, "end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : cg_goto(index_grid=%d)", index_grid);
    }

    datatype = this->getDataType();
    cgns_datatype = this->toCgnsDataType(datatype);
    actual_size = this->getNumNodes();
    virtual_size = this->getNumVirtualNodes();
    coords_size = actual_size + virtual_size;
    if (datatype == Udm_RealSingle) {
        float *coords = new float[coords_size];

        // CoordinateX出力
        this->getGridCoordinatesX(1, actual_size, (float*)coords);
        this->getGridCoordinatesXOfVirtual(1, virtual_size, (float*)coords + actual_size);
        if ((cg_error = cg_array_write("CoordinateX", cgns_datatype, 1, &coords_size, coords)) != CG_OK) {
            udm_error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : cg_array_write(CoordinateX)");
        }
        if (udm_error != UDM_OK) {
            delete  []coords;
            return udm_error;
        }

        // CoordinateY出力
        this->getGridCoordinatesY(1, actual_size, (float*)coords);
        this->getGridCoordinatesYOfVirtual(1, virtual_size, (float*)coords + actual_size);
        if ((cg_error = cg_array_write("CoordinateY", cgns_datatype, 1, &coords_size, coords)) != CG_OK) {
            udm_error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : cg_array_write(CoordinateY)");
        }
        if (udm_error != UDM_OK) {
            delete  []coords;
            return udm_error;
        }

        // CoordinateZ出力
        this->getGridCoordinatesY(1, actual_size, (float*)coords);
        this->getGridCoordinatesYOfVirtual(1, virtual_size, (float*)coords + actual_size);
        if ((cg_error = cg_array_write("CoordinateZ", cgns_datatype, 1, &coords_size, coords)) != CG_OK) {
            udm_error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : cg_array_write(CoordinateZ)");
        }
        if (udm_error != UDM_OK) {
            delete  []coords;
            return udm_error;
        }
        delete  []coords;
    }
    else if (datatype == Udm_RealDouble) {
        double *coords = new double[coords_size];

        // CoordinateX出力
        this->getGridCoordinatesX(1, actual_size, (double*)coords);
        this->getGridCoordinatesXOfVirtual(1, virtual_size, (double*)coords + actual_size);
        if ((cg_error = cg_array_write("CoordinateX", cgns_datatype, 1, &coords_size, coords)) != CG_OK) {
            udm_error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : cg_array_write(CoordinateX)");
        }
        if (udm_error != UDM_OK) {
            delete  []coords;
            return udm_error;
        }

        // CoordinateY出力
        this->getGridCoordinatesY(1, actual_size, (double*)coords);
        this->getGridCoordinatesYOfVirtual(1, virtual_size, (double*)coords + actual_size);
        if ((cg_error = cg_array_write("CoordinateY", cgns_datatype, 1, &coords_size, coords)) != CG_OK) {
            udm_error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : cg_array_write(CoordinateY)");
        }
        if (udm_error != UDM_OK) {
            delete  []coords;
            return udm_error;
        }

        // CoordinateZ出力
        this->getGridCoordinatesY(1, actual_size, (double*)coords);
        this->getGridCoordinatesYOfVirtual(1, virtual_size, (double*)coords + actual_size);
        if ((cg_error = cg_array_write("CoordinateZ", cgns_datatype, 1, &coords_size, coords)) != CG_OK) {
            udm_error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : cg_array_write(CoordinateZ)");
        }
        if (udm_error != UDM_OK) {
            delete  []coords;
            return udm_error;
        }

        delete  []coords;
    }
    else {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "invalid coordinates datatype");
    }
    if (udm_error != UDM_OK) return udm_error;

    // 出力CGNS:GridCoordinates名を設定する.
    this->setCgnsWriteGridCoordnates(std::string(gridname));

    // CGNS:UdmRankConenctivityを書き込む.
    if (this->getRankConnectivity() != NULL) {
        if (this->getRankConnectivity()->writeCgns(index_file, index_base, index_zone) != UDM_OK) {
            udm_error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : UdmRankConnectivity::writeCgns(index_file=%d, index_base=%d, index_zone=%d)", index_file, index_base, index_zone);
        }
    }

    return udm_error;
}

/**
 * 仮想節点（ノード）のIDの再構築を行う。
 * ローカルIDに構成節点（ノード）＋連番を設定する
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::rebuildVirtualNodes()
{
    UdmSize_t node_id = this->node_list.size();
    std::vector<UdmNode*>::iterator itr;
    for (itr=this->virtual_nodes.begin(); itr!= this->virtual_nodes.end(); itr++) {
        UdmNode *node = (*itr);
        node->setLocalId(++node_id);            // IDをインクリメントして設定する
    }

    return UDM_OK;
}


/**
 * CGNS:GrdiCoordinatesを結合する.
 * @param dest_grid        結合GridCoordinates
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::joinCgnsGridCoordinates(UdmGridCoordinates* dest_grid)
{
    UdmSize_t n;
    if (dest_grid == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "dest_grid is null.");
    }
    UdmRankConnectivity *inner_boundary = this->getRankConnectivity();
    if (inner_boundary == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "UdmRankConnectivity is null.");
    }

    // MPI接続ランク番号, IDの検索テーブルを作成する.
    inner_boundary->createSearchMpiRankidTable();

    UdmSize_t num_nodes = dest_grid->getNumNodes();
    for (n=1; n<=num_nodes; n++) {
        UdmNode *dest_node = dest_grid->getNodeById(n);
        int dest_rankno = dest_node->getMyRankno();
        UdmSize_t node_id = dest_node->getId();
        UdmNode *src_node = NULL;
        if (dest_node->getNumMpiRankInfos() > 0) {
            // 内部境界から同一節点（ノード）を検索する
            src_node = inner_boundary->findMpiRankInfo(dest_rankno, node_id);
        }
        if (src_node != NULL) {
            UdmSize_t src_nodeid = src_node->getId();
            int src_rankno = src_node->getMyRankno();

            // 追加済み節点（ノード）である。
            // 接続ランク番号,IDを削除する.
            src_node->removeMpiRankInfo(dest_rankno, node_id);

            // ランク番号,IDを退避する.
            src_node->addPreviousRankInfo(dest_rankno, node_id);

            // 共通節点を設定する.
            dest_node->setCommonNode(src_node);
            continue;
        }

        // ランク番号,IDを退避する.
        dest_node->addPreviousRankInfo(dest_rankno, node_id);

        // 節点（ノード）を挿入する.
        this->insertNode(dest_node);

        // 共通節点はなし。
        dest_node->setCommonNode(NULL);
    }

    // UdmZoneのdeleteにてUdmNodeがdeleteされない様に挿入節点（ノード）をeraseする
    // 残りの節点（ノード）は同一節点（ノード）が存在するので、UdmZoneの削除にて削除する。
    /********************
    std::vector<UdmNode*>::reverse_iterator rev_itr;
    for (rev_itr=dest_grid->node_list.rbegin(); rev_itr!=dest_grid->node_list.rend(); rev_itr++) {
        UdmNode *node = (*rev_itr);
        if (node->getCommonNode() == NULL) {
            // 構成節点（ノード）リストから削除
            dest_grid->node_list.erase( --(rev_itr.base()) );
        }
    }
    ******************/

    std::vector<UdmNode*>::iterator itr;
    for (itr=dest_grid->node_list.begin(); itr!=dest_grid->node_list.end(); ) {
        UdmNode *node = (*itr);
        if (node->getCommonNode() == NULL) {
            // 構成節点（ノード）リストから削除
            itr = dest_grid->node_list.erase( itr );
            continue;
        }
        itr++;
    }
    return UDM_OK;
}

/**
 * 以前のID、ランク番号をクリアする.
 */
void UdmGridCoordinates::clearPreviousInfos()
{
    std::vector<UdmNode*>::iterator itr;
    for (itr=this->node_list.begin(); itr!= this->node_list.end(); itr++) {
        UdmNode *node = (*itr);
        node->clearPreviousInfos();
    }
    return;
}


/**
 * 削除節点（ノード）リストを削除する.
 * @param remove_nodes        削除節点（ノード）リスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::removeNodes(const std::vector<UdmNode*>& remove_nodes)
{
    std::vector<UdmNode*>::const_iterator itr;
    for (itr=remove_nodes.begin(); itr!=remove_nodes.end(); itr++) {
        UdmNode* node = (*itr);
        if (node != NULL) {
            node->setRemoveEntity(true);
        }
    }

    // 構成ノードから削除
    UdmSize_t erase_size = this->node_list.size();
    this->node_list.erase(
            std::remove_if(
                    this->node_list.begin(),
                    this->node_list.end(),
                    std::mem_fun(&UdmEntity::isRemoveEntity)),
            this->node_list.end());
    erase_size -= this->node_list.size();

    if (erase_size < remove_nodes.size()) {
        // 仮想ノードから削除
        erase_size += this->virtual_nodes.size();
        this->virtual_nodes.erase(
                std::remove_if(
                        this->virtual_nodes.begin(),
                        this->virtual_nodes.end(),
                        std::mem_fun(&UdmEntity::isRemoveEntity)),
                this->virtual_nodes.end());
        erase_size -= this->virtual_nodes.size();

    }

    // 内部境界リストから削除する
    UdmRankConnectivity* inner = this->getRankConnectivity();
    if (inner != NULL) {
        // 内部境界リストから削除する.
        inner->removeBoundaryNodes(remove_nodes);
    }

    // 節点（ノード）の削除
    for (itr=remove_nodes.begin(); itr!=remove_nodes.end(); itr++) {
        UdmNode* node = (*itr);
        if (node != NULL) {
            delete node;
        }
    }

    return UDM_OK;
}

/**
 * 内部境界情報を節点（ノード）に追加する.
 * @param node_id        節点（ノード）ID（１～）
 * @param rankno        接続先MPIランク番号（０～）
 * @param localid        接続先節点（ノード）ID（１～）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGridCoordinates::insertRankConnectivity(
                        UdmSize_t node_id,
                        int rankno,
                        UdmSize_t localid)
{
    if (rankno < 0) return UDM_ERROR;
    if (localid <= 0) return UDM_ERROR;
    UdmNode *node = this->getNodeById(node_id);

    // 内部境界情報を追加する.
    node->addMpiRankInfo(rankno, localid);

    // 内部境界に追加する
    if (node->getNumMpiRankInfos() > 0) {
        if (this->getRankConnectivity() != NULL) {
            this->getRankConnectivity()->insertRankConnectivityNode(node);
        }
    }

    return UDM_OK;
}

/**
 * ランク番号とIDが一致している仮想節点（ノード）を検索する.
 * @param src_rankno        ランク番号
 * @param src_nodeid        節点（ノード）ID
 * @return        節点（ノード）
 */
UdmNode* UdmGridCoordinates::findVirtualNodeByGlobalId(int src_rankno, UdmSize_t src_nodeid) const
{
    if (src_rankno < 0) return NULL;
    if (src_nodeid <= 0) return NULL;

    UdmNode *find_node = NULL;

    // グリッド構成ノードリストの配列添字として検索
    find_node = this->getVirtualNodeById(src_nodeid);
    if (find_node != NULL) {
        if (find_node->getMyRankno() == src_rankno) {
            return find_node;
        }
        find_node = NULL;
    }

    // ２分探索でグローバルIDを検索する
    std::vector<UdmNode*>::const_iterator find_itr;
    find_itr = this->searchCurrentGlobalId(this->virtual_nodes, src_rankno, src_nodeid);
    if (find_itr != this->virtual_nodes.end()) {
        find_node = (*find_itr);
        return find_node;
    }

    return NULL;
}

/**
 * メモリサイズを取得する.
 * @return        メモリサイズ
 */
size_t UdmGridCoordinates::getMemSize() const
{
    UdmSize_t size = sizeof(*this);
#ifdef _DEBUG
    printf("this size=%ld\n", sizeof(*this));
    printf("node_list size=%ld [count=%ld] [offset=%ld]\n",
                            sizeof(this->node_list),
                            this->node_list.size(),
                            offsetof(UdmGridCoordinates, node_list));
    printf("virtual_nodes size=%ld [count=%ld] [offset=%ld]\n",
                            sizeof(this->virtual_nodes),
                            this->virtual_nodes.size(),
                            offsetof(UdmGridCoordinates, virtual_nodes));

    printf("parent_zone pointer size=%ld [offset=%ld]\n", sizeof(this->parent_zone), offsetof(UdmGridCoordinates, parent_zone));
    printf("cgns_writegridcoordnates size=%ld [offset=%ld]\n", sizeof(this->cgns_writegridcoordnates), offsetof(UdmGridCoordinates, cgns_writegridcoordnates));
    printf("max_nodeid size=%ld [offset=%ld]\n", sizeof(this->max_nodeid), offsetof(UdmGridCoordinates, max_nodeid));

    size += this->node_list.size()*sizeof(UdmNode*);
    size += this->virtual_nodes.size()*sizeof(UdmNode*);
#endif

    return size;
}

} /* namespace udm */
