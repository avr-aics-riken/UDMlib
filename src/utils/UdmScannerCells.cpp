/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */


#include "model/UdmSolutionData.h"
#include "model/UdmZone.h"
#include "model/UdmSections.h"
#include "model/UdmNode.h"
#include "model/UdmComponent.h"
#include "model/UdmICellComponent.h"
#include "utils/UdmScannerCells.h"

/**
 * @file UdmVisitorCells.h
 * 要素（セル）接続情報収集クラスのソースファイル
 */

namespace udm
{

/**
 * コンストラクタ
 */
UdmScannerCells::UdmScannerCells() : scan_zone(NULL), num_gid_entries(1), num_lid_entries(1), scan_iterator(0)
{
}

/**
 * コンストラクタ
 * @param zone       スキャン対象ゾーン
 */
UdmScannerCells::UdmScannerCells(UdmZone* zone): scan_zone(zone), num_gid_entries(1), num_lid_entries(1), scan_iterator(0)
{
}

/**
 * デストラクタ
 */
UdmScannerCells::~UdmScannerCells()
{
    this->clearImportCells();
    this->clearExportCells();
}

/**
 * スキャン対象ゾーンを取得する:const.
 * @return        スキャン対象ゾーン
 */
const UdmZone* UdmScannerCells::getScanZone() const
{
    return this->scan_zone;
}

/**
 * スキャン対象ゾーンを取得する.
 * @return        スキャン対象ゾーン
 */
UdmZone* UdmScannerCells::getScanZone()
{
    return this->scan_zone;
}

/**
 * スキャン対象ゾーンを設定する.
 * @param zone        スキャン対象ゾーン
 */
void UdmScannerCells::setScanZone(UdmZone* zone)
{
    this->scan_zone = zone;
}


/**
 * グローバルID、ローカルIDのサイズを設定する.
 * @param num_gid_entries        グローバルIDのデータサイズ:NUM_GID_ENTRIES
 * @param num_lid_entries        ローカルIDのデータサイズ:NUM_LID_ENTRIES
 */
void UdmScannerCells::setEntriesIdSize(int num_gid_entries, int num_lid_entries)
{
    if (this->num_gid_entries > 0) {
        this->num_gid_entries = num_gid_entries;
    }
    if (num_lid_entries > 0) {
        this->num_lid_entries = num_lid_entries;
    }
}

/**
 * 接続ノード（頂点）情報を収集する:要素（セル）リスト.
 * @param cell_list                要素（セル）リスト
 * @return                        接続ノード（頂点）数
 */
UdmSize_t UdmScannerCells::scannerElements(const std::vector<UdmCell*> &cell_list)
{
    UdmSize_t num_nodes = 0;
    std::vector<UdmCell*>::const_iterator itr;
    for (itr=cell_list.begin(); itr!=cell_list.end();itr++) {
        num_nodes += (*itr)->scannerCells(this);
    }
    return num_nodes;
}

/**
 * 接続ノード（頂点）情報を収集する:要素（セル）リスト.
 * @param cell_list                要素（セル）リスト
 * @return                        接続ノード（頂点）数
 */
UdmSize_t UdmScannerCells::scannerElements(const std::vector<const UdmCell*> &cell_list)
{
    UdmSize_t num_nodes = 0;
    std::vector<const UdmCell*>::const_iterator itr;
    for (itr=cell_list.begin(); itr!=cell_list.end();itr++) {
        num_nodes += this->scannerElements(*itr);
    }
    return num_nodes;
}

/**
 * 接続ノード（頂点）情報を収集する:要素（セル）リスト.
 * @param cell_list                要素（セル）リスト
 * @return                        接続ノード（頂点）数
 */
UdmSize_t UdmScannerCells::scannerElements(const std::vector<UdmICellComponent*> &cell_list)
{
    UdmSize_t num_nodes = 0;
    UdmSize_t n;
    std::vector<UdmICellComponent*>::const_iterator itr;
    for (itr=cell_list.begin(); itr!=cell_list.end();itr++) {
        UdmICellComponent *component = (*itr);

        UdmSize_t num_nodes = component->getNumNodes();
        if (num_nodes == 0) continue;

        std::vector<UdmEntity*> nodes;
        for (n=1; n<=num_nodes; n++) {
            nodes.push_back(component->getNode(n));
        }
        this->scan_arrays.push_back(nodes);
        num_nodes += num_nodes;
    }
    return num_nodes;
}

/**
 * 接続ノード（頂点）情報を収集する:要素（セル）.
 * @param cell                    要素（セル）
 * @return                        接続ノード（頂点）数
 */
UdmSize_t UdmScannerCells::scannerElements(const UdmCell* cell)
{
    UdmSize_t n;
    UdmSize_t num_nodes = cell->getNumNodes();
    if (num_nodes == 0) return 0;

    std::vector<UdmEntity*> nodes;
    for (n=1; n<=num_nodes; n++) {
        nodes.push_back(cell->getNode(n));
    }
    this->scan_arrays.push_back(nodes);

    return num_nodes;
}

/**
 * 接続ノード（頂点）情報の収集データ数を取得する.
 * 要素（セル）の形状タイプ=MIXEDの場合は、形状タイプ値分のデータ数(+1)する
 * @param element_type            要素（セル）の形状タイプ
 * @return        収集データ数
 */
size_t UdmScannerCells::getNumElementsDatas(UdmElementType_t element_type) const
{
    size_t count = 0;
    std::vector< std::vector<UdmEntity*> >::const_iterator itr;
    for (itr=this->scan_arrays.begin(); itr!=this->scan_arrays.end(); itr++) {
        count += (*itr).size();
        if (element_type == Udm_MIXED) {
            count++;
        }
    }
    return count;
}

/**
 *
 * 収集データは{node_id1,node_id2,node_id3,...}
 * MIXEDの場合、収集IDは{element_type,node_id1,node_id2,node_id3,..}
 * @param [in]  element_type            要素（セル）の形状タイプ
 * @param [out] array            接続ノード（頂点）IDリスト
 * @return            接続ノード（頂点）IDリスト数
 */
template <class DATA_TYPE>
UdmSize_t UdmScannerCells::getScanCgnsElementsIds(UdmElementType_t element_type, DATA_TYPE *array) const
{
    if (this->scan_arrays.size() <= 0) return 0;
    std::vector< std::vector<UdmEntity*> >::const_iterator itr;
    std::vector<UdmEntity*>::const_iterator node_itr;
    UdmSize_t n = 0;
    for (itr=this->scan_arrays.begin(); itr!=this->scan_arrays.end(); itr++) {
        if (itr->size() <= 0) continue;
        if (element_type == Udm_MIXED) {
            UdmElementType_t type = (*itr)[0]->getElementType();
            array[n++] = UdmGeneral::toCgnsElementType(type);
        }
        for (node_itr=itr->begin(); node_itr!=itr->end(); node_itr++) {
            array[n++] = (DATA_TYPE)(*node_itr)->getId();
        }
    }
    return n;
}

template UdmSize_t UdmScannerCells::getScanCgnsElementsIds(UdmElementType_t element_type, int *array) const;
template UdmSize_t UdmScannerCells::getScanCgnsElementsIds(UdmElementType_t element_type, unsigned int *array) const;
template UdmSize_t UdmScannerCells::getScanCgnsElementsIds(UdmElementType_t element_type, long *array) const;
template UdmSize_t UdmScannerCells::getScanCgnsElementsIds(UdmElementType_t element_type, unsigned long *array) const;
template UdmSize_t UdmScannerCells::getScanCgnsElementsIds(UdmElementType_t element_type, long long *array) const;
template UdmSize_t UdmScannerCells::getScanCgnsElementsIds(UdmElementType_t element_type, unsigned long long *array) const;


/**
 * グラフ頂点情報を収集する:ゾーン.
 * ゾーン内のセクションのグラフ頂点情報を収集する
 * @param cell_list                要素（セル）リスト
 * @return                        HyperGraph接続数
 */
UdmSize_t UdmScannerCells::scannerGraph()
{
    if (this->scan_zone == NULL) return NULL;
    if (this->scan_zone->getSections() == NULL) return NULL;

    return this->scannerGraph(this->scan_zone->getSections());
}


/**
 * グラフ頂点情報を収集する:セクション.
 * @param sections                セクション
 * @return                        HyperGraph接続数
 */
UdmSize_t UdmScannerCells::scannerGraph(const UdmSections *sections)
{
    if (sections == NULL) return 0;

    UdmSize_t num_cells = sections->getNumEntities();
    UdmSize_t n, count = 0;
    for (n=1; n<=num_cells; n++) {
        UdmCell *cell = sections->getEntityCell(n);
        // グラフデータリストの先頭は接続の中心要素（セル）
        this->current_scan.clear();
        this->current_scan.push_back(cell);

        count += cell->scannerGraph(this);
        this->scan_arrays.push_back(this->current_scan);
    }

    return count;
}

/**
 * グラフ頂点情報を収集する:セクション.
 * @param sections                セクション
 * @return                        HyperGraph接続数
 */
UdmSize_t UdmScannerCells::scannerGraph(const std::vector<UdmCell*> &scan_cells)
{
    if (scan_cells.size() == 0) return 0;

    UdmSize_t count = 0;
    std::vector<UdmCell*>::const_iterator itr;
    for (itr=scan_cells.begin(); itr!=scan_cells.end(); itr++) {
        UdmCell *cell = *itr;
        // グラフデータリストの先頭は接続の中心要素（セル）
        this->current_scan.clear();
        this->current_scan.push_back(cell);

        count += cell->scannerGraph(this);
        this->scan_arrays.push_back(this->current_scan);
    }
    return count;
}

/**
 * グラフ頂点情報を収集する:要素（セル）.
 * @param cell                要素（セル）
 * @return                     グラフ接続数
 */
UdmSize_t UdmScannerCells::scannerGraph(const UdmCell* cell)
{
    if (cell == NULL) return 0;

    UdmSize_t n, m, count = 0;
    if (this->current_scan.size() <= 0) return 0;
    const UdmEntity *parent_cell = this->current_scan[0];
    if (parent_cell == cell) {
        UdmSize_t num_cells = cell->getNumComponentCells();
        for (n=1; n<=num_cells; n++) {
            const UdmComponent* component_cell = cell->getComponentCell(n);

            // 部品要素の親要素の取得
            int num_comp_parents = component_cell->getNumParentCells();
            count += this->scannerGraph(component_cell);
        }
    }
    else {
        UdmSize_t num_parents = cell->getNumParentCells();
        for (n=1; n<=num_parents; n++) {
            if (cell->getParentCell(n) != parent_cell) {
                this->current_scan.push_back(cell->getParentCell(n));
            }
        }
    }

    return count;
}


/**
 * グラフ頂点情報を収集する:要素（セル）.
 * @param component            部品要素（セル）
 * @return                     グラフ接続数
 */
UdmSize_t UdmScannerCells::scannerGraph(const UdmComponent* component)
{
    if (component == NULL) return 0;

    UdmSize_t n, count = 0;
    if (this->current_scan.size() <= 0) return 0;
    const UdmEntity *parent_cell = this->current_scan[0];
    UdmSize_t num_parents = component->getNumParentCells();
    for (n=1; n<=num_parents; n++) {
        if (component->getParentCell(n) != parent_cell) {
            this->current_scan.push_back(component->getParentCell(n));
        }
    }

    return count;
}


/**
 * グラフグローバルID数を取得する
 * @return
 */
UdmSize_t UdmScannerCells::getNumObjects() const
{
    return this->scan_arrays.size();
}

/**
 * HyperGraphデータ数を取得する
 * @param [out] num_globalids            グローバルID数
 * @param [out] num_neighbors            接続頂点総数
 * @return        グローバルID数
 */
UdmSize_t UdmScannerCells::getNumHyperGraphDatas(UdmSize_t& num_globalids, UdmSize_t& num_neighbors) const
{
    UdmSize_t count = 0;
    std::vector< std::vector<UdmEntity*> >::const_iterator itr;
    for (itr=this->scan_arrays.begin(); itr!=this->scan_arrays.end(); itr++) {
        count += (*itr).size();
    }

    num_globalids = this->scan_arrays.size();
    num_neighbors = count;
    return num_globalids;
}

/**
 * HyperGraphデータを取得する.
 * @param [out] globalids            グローバルIDリスト
 * @param [out] vtxedge_ptr            接続頂点ポインタリスト
 * @param [out] neighborids            接続頂点リスト
 * @return        接続頂点リスト数
 */
template<class DATA_TYPE>
UdmSize_t UdmScannerCells::getScanHyperGraphDatas(
                        DATA_TYPE* globalids,
                        int* vtxedge_ptr,
                        DATA_TYPE* neighborids) const
{
    int ptr = 0;
    UdmSize_t m=0, n=0, ptr_pos = 0;
    UdmSize_t id = 0, part_id;
    int rankno = 0, part_rankno;
    std::vector<UdmEntity*> part_ids;
    std::vector<UdmEntity*>::iterator part_itr;

    std::vector< std::vector<UdmEntity*> >::const_iterator itr;
    std::vector<UdmEntity*>::const_iterator nbor_itr;
    for (itr=this->scan_arrays.begin(); itr!=this->scan_arrays.end(); itr++) {
        if (itr->size() <= 0) continue;
        const UdmEntity* entity = itr->at(0);
        id = entity->getId();
        rankno = entity->getMyRankno();

        // globalidsにグローバルIDをセットする.
        m += udm_set_entryid(globalids+m, id, rankno, this->num_gid_entries);
        vtxedge_ptr[ptr_pos++] = ptr;

        part_ids.clear();
        for (nbor_itr=itr->begin(); nbor_itr!=itr->end(); nbor_itr++) {
            part_ids.push_back((*nbor_itr));
            ptr++;
        }
        // ランク番号,ID順にソートする.
        std::sort(part_ids.begin(),part_ids.end(), UdmEntity::compareIds);
        for (part_itr=part_ids.begin(); part_itr!=part_ids.end();part_itr++) {
            const UdmEntity* part_entity = (*part_itr);
            part_id = part_entity->getId();
            part_rankno = part_entity->getMyRankno();

            // neighboridsに接続情報をセットする.
            n += udm_set_entryid(neighborids+n, part_id, part_rankno, this->num_gid_entries);
        }
    }

    return n;
}
template UdmSize_t UdmScannerCells::getScanHyperGraphDatas(int* globalids, int* vtxedge_ptr, int* neighborids) const;
template UdmSize_t UdmScannerCells::getScanHyperGraphDatas(unsigned int* globalids, int* vtxedge_ptr, unsigned int* neighborids) const;
template UdmSize_t UdmScannerCells::getScanHyperGraphDatas(long* globalids, int* vtxedge_ptr, long* neighborids) const;
template UdmSize_t UdmScannerCells::getScanHyperGraphDatas(unsigned long* globalids, int* vtxedge_ptr, unsigned long* neighborids) const;
template UdmSize_t UdmScannerCells::getScanHyperGraphDatas(long long* globalids, int* vtxedge_ptr, long long* neighborids) const;
template UdmSize_t UdmScannerCells::getScanHyperGraphDatas(unsigned long long* globalids, int* vtxedge_ptr, unsigned long long* neighborids) const;


/**
 * グラフグローバルIDリストを取得する
 * @param [out] globalids        グローバルIDリスト
 * @param [out] localids        ローカルIDリスト
 * @return        グローバルIDリスト数
 */
template<class DATA_TYPE>
UdmSize_t UdmScannerCells::getScanObjects(DATA_TYPE* globalids, DATA_TYPE* localids) const
{
    UdmSize_t n=0;
    UdmSize_t id = 0;
    int rankno = 0;
    int pos_gids = 0, pos_lids = 0;

    std::vector< std::vector<UdmEntity*> >::const_iterator itr;
    for (itr=this->scan_arrays.begin(); itr!=this->scan_arrays.end(); itr++) {
        if (itr->size() <= 0) continue;
        const UdmEntity* entity = itr->at(0);
        id = entity->getId();
        rankno = entity->getMyRankno();

        if (globalids != NULL) {
            pos_gids += udm_set_entryid(globalids+pos_gids, id, rankno, this->num_gid_entries);
        }
        if (localids != NULL) {
            pos_lids += udm_set_entryid(localids+pos_lids, id, rankno, this->num_lid_entries);
        }
    }
    return n;
}

template UdmSize_t UdmScannerCells::getScanObjects(int* globalids, int* localids) const;
template UdmSize_t UdmScannerCells::getScanObjects(unsigned int* globalids, unsigned int* localids) const;
template UdmSize_t UdmScannerCells::getScanObjects(long* globalids, long* localids) const;
template UdmSize_t UdmScannerCells::getScanObjects(unsigned long* globalids, unsigned long* localids) const;
template UdmSize_t UdmScannerCells::getScanObjects(long long* globalids, long long* localids) const;
template UdmSize_t UdmScannerCells::getScanObjects(unsigned long long* globalids, unsigned long long* localids) const;


/**
 * オブジェクト：要素（セル）の重みリストを取得する
 * @param [out] obj_wgts        オブジェクト：要素（セル）の重みリスト
 * @return        オブジェクト：要素（セル）数
 */
UdmSize_t UdmScannerCells::getObjectWeights(float* obj_wgts) const
{
    UdmSize_t n=0;
    if (obj_wgts == NULL) return 0;

    std::vector< std::vector<UdmEntity*> >::const_iterator itr;
    for (itr=this->scan_arrays.begin(); itr!=this->scan_arrays.end(); itr++) {
        if (itr->size() <= 0) continue;
        const UdmEntity* entity = itr->at(0);
        float weight = entity->getPartitionWeight();
        obj_wgts[n++] = weight;
    }
    return n;
}

/**
 * Graphデータ数を取得する
 * @param [out] num_globalids            グローバルID数
 * @param [out] num_edges                接続頂点数リスト
 * @return        グローバルID数
 */
UdmSize_t UdmScannerCells::getNumGraphDatas(UdmSize_t& num_globalids, int* num_edges) const
{
    int n = 0;
    if (num_edges != NULL) {
        std::vector< std::vector<UdmEntity*> >::const_iterator itr;
        for (itr=this->scan_arrays.begin(); itr!=this->scan_arrays.end(); itr++) {
            if (itr->size() <= 0) continue;
            num_edges[n++] = (*itr).size()-1;
        }
    }

    num_globalids = this->scan_arrays.size();
    return num_globalids;
}

/**
 * Graphデータを取得する.
 * @param neighborids        接続グローバルIDリスト
 * @param nbor_procs        接続プロセス番号リスト
 * @return
 */
template<class DATA_TYPE>
UdmSize_t UdmScannerCells::getScanGraphDatas(DATA_TYPE* neighborids, int* nbor_procs) const
{
    UdmSize_t n=0, nbor_pos = 0;
    std::vector<UdmEntity*> part_entites;
    std::vector<UdmEntity*>::iterator part_itr;
    int rankno;
    UdmSize_t id = 0;

    std::vector< std::vector<UdmEntity*> >::const_iterator itr;
    std::vector<UdmEntity*>::const_iterator nbor_itr;
    for (itr=this->scan_arrays.begin(); itr!=this->scan_arrays.end(); itr++) {
        if (itr->size() <= 0) continue;
        part_entites.clear();
        for (nbor_itr=itr->begin(); nbor_itr!=itr->end(); nbor_itr++) {
            if (nbor_itr == itr->begin()) continue;        // 先頭は接続元（中心）の要素
            part_entites.push_back((*nbor_itr));
        }

        // ランク番号,ID順にソートする.
        std::sort(part_entites.begin(),part_entites.end(), UdmEntity::compareIds);
        for (part_itr=part_entites.begin(); part_itr!=part_entites.end();part_itr++) {
            const UdmEntity* part_entity = (*part_itr);
            id = part_entity->getId();
            rankno = part_entity->getMyRankno();

            // neighboridsに接続情報をセットする.
            nbor_pos += udm_set_entryid(neighborids+nbor_pos, id, rankno, this->num_gid_entries);
            // プロセス番号(=ランク番号)をセットする.
            nbor_procs[n] = rankno;
            n++;
        }
    }

    return n;
}
template UdmSize_t UdmScannerCells::getScanGraphDatas(int* neighborids, int* nbor_procs) const;
template UdmSize_t UdmScannerCells::getScanGraphDatas(unsigned int* neighborids, int* nbor_procs) const;
template UdmSize_t UdmScannerCells::getScanGraphDatas(long* neighborids, int* nbor_procs) const;
template UdmSize_t UdmScannerCells::getScanGraphDatas(unsigned long* neighborids, int* nbor_procs) const;
template UdmSize_t UdmScannerCells::getScanGraphDatas(long long* neighborids, int* nbor_procs) const;
template UdmSize_t UdmScannerCells::getScanGraphDatas(unsigned long long* neighborids, int* nbor_procs) const;


/**
 * Zoltan分割によるインポート要素（セル）を追加する.
 * @param cell        インポート要素（セル）
 * @return        インポート要素（セル）数
 */
UdmSize_t UdmScannerCells::insertImportCell(UdmCell* cell)
{
    this->import_cells.push_back(cell);
    return this->import_cells.size();
}

/**
 * Zoltan分割によるエクスポート要素（セル）を追加する.
 * @param cell            エクスポート要素（セル）
 * @return        エクスポート要素（セル）数
 */
UdmSize_t UdmScannerCells::insertExportCell(UdmCell* cell)
{
    this->export_cells.push_back(cell);
    return this->export_cells.size();
}

/**
 * Zoltan分割によるインポート要素（セル）リストを取得する.
 * @return        インポート要素（セル）リスト
 */
const std::vector<UdmCell*>& UdmScannerCells::getImportCells() const
{
    return this->import_cells;
}

/**
 * Zoltan分割によるエクスポート要素（セル）リストを取得する.
 * @return        エクスポート要素（セル）リスト
 */
const std::vector<UdmCell*>& UdmScannerCells::getExportCells() const
{
    return this->export_cells;
}

/**
 * Zoltan分割によるインポート要素（セル）リストをクリアする.
 * インポート要素（セル）はZoltan分割によって生成されているので、領域開放(delete)を行う。
 */
void UdmScannerCells::clearImportCells()
{
/********************
    std::vector<UdmCell*>::reverse_iterator ritr = this->import_cells.rbegin();
    while (ritr != this->import_cells.rend()) {
        UdmCell* cell = *(--(ritr.base()));
        if (cell != NULL) {
            UdmCell::freeDeserialize(cell);
        }
        this->import_cells.erase((++ritr).base());
        ritr = this->import_cells.rbegin();
    }
********************/

    std::vector<UdmCell*>::iterator itr = this->import_cells.begin();
    for (itr=this->import_cells.begin(); itr != this->import_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        if (cell != NULL) {
            UdmCell::freeDeserialize(cell);
        }
    }

    this->import_cells.clear();
}

/**
 *  Zoltan分割によるエクスポート要素（セル）リストをクリアする.
 */
void UdmScannerCells::clearExportCells()
{
    this->export_cells.clear();
}

/**
 * スキャンデータを取得する.
 * すべてのスキャンデータから重複オブジェクトは除外したUdmEntityを取得する.
 * @param [out] entities        スキャンデータ
 * @return        スキャンデータ数
 */
UdmSize_t UdmScannerCells::getScanEntities(std::vector<UdmEntity*> &entities) const
{
    if (this->scan_arrays.size() <= 0) return 0;
    std::vector< std::vector<UdmEntity*> >::const_iterator itr;
    std::vector<UdmEntity*>::const_iterator node_itr;
    std::vector<UdmEntity*>::iterator find_itr;
    UdmGlobalRankidList entity_list;

    UdmSize_t array_size = 0;
    for (itr=this->scan_arrays.begin(); itr!=this->scan_arrays.end(); itr++) {
        array_size += itr->size();
    }
    entities.reserve(array_size);
    entity_list.reserve(array_size);

    for (itr=this->scan_arrays.begin(); itr!=this->scan_arrays.end(); itr++) {
        for (node_itr=itr->begin(); node_itr!=itr->end(); node_itr++) {
            const UdmEntity* entity = *node_itr;
            if (entity_list.existsGlobalRankid(entity->getMyRankno(), entity->getId())) {
                continue;
            }
            entities.push_back(*node_itr);
            entity_list.addGlobalRankid(entity->getMyRankno(), entity->getId());
            // find_itr = std::find(entities.begin(), entities.end(), *node_itr);
            // if (find_itr == entities.end()) {
            //     entities.push_back(*node_itr);
            // }
        }
    }
    return entities.size();
}

/**
 * 親要素（セル）を探索する.
 * @param node_list        探索節点（ノード）リスト
 * @return        探索結果親数（重複を含む）
 */
UdmSize_t UdmScannerCells::scannerParents(const std::vector<UdmNode*>& node_list)
{
    UdmSize_t count = 0;
    std::vector<UdmNode*>::const_iterator itr;
    for (itr=node_list.begin(); itr!=node_list.end(); itr++) {
        count += scannerParents(*itr);
    }
    return count;
}

/**
 * 親要素（セル）を探索する.
 * @param node        探索節点（ノード）
 * @return        探索結果親数（重複を含む）
 */
UdmSize_t UdmScannerCells::scannerParents(const UdmNode* node)
{
    if (node == NULL) return 0;

    std::vector<UdmEntity*> parents;
    int n;
    int num_parents = node->getNumParentCells();
    for (n=1; n<=num_parents; n++) {
        UdmICellComponent* parent_cell = node->getParentCell(n);
        // 要素(セル)クラスであること。部品要素ではない。
        if (parent_cell->getCellClass() != Udm_CellClass) continue;
        parents.push_back(dynamic_cast<UdmCell*>(parent_cell));
    }
    if (parents.size() > 0) {
        this->scan_arrays.push_back(parents);
    }
    return parents.size();
}

/**
 * スキャン結果を取得する.
 * @return        スキャン結果
 */
const std::vector<std::vector<UdmEntity*> >&
UdmScannerCells::getScanArrays() const
{
    return this->scan_arrays;
}

/**
 * 節点（ノード）の隣接節点（ノード）を取得する.
 * @param node        探索対象節点（ノード）
 * @return        隣接節点（ノード）数
 */
UdmSize_t UdmScannerCells::scannerNeighborNodes(const UdmNode* node)
{
    if (node == NULL) return 0;

    std::vector<UdmEntity*> neighbors;
    int n, m;
    int num_parents = node->getNumParentCells();
    for (n=1; n<=num_parents; n++) {
        UdmICellComponent* parent_cell = node->getParentCell(n);
        // 部品要素の検索
        if (parent_cell->getNumParentCells() == 0) continue;
        // 部品要素の構成節点（ノード）の取得
        int num_nodes = parent_cell->getNumNodes();
        UdmSize_t node_pos = 0;
        for (m=1; m<=num_nodes; m++) {
            UdmNode *node_incell = parent_cell->getNode(m);
            if (node_incell == node) {
                node_pos = m;
                break;
            }
        }
        // 前後の節点（ノード）を取得する
        if (node_pos >= 1 && node_pos <= num_nodes) {
            // 前方節点（ノード）
            UdmNode *forward_node = NULL;
            if (node_pos == 1) {
                forward_node = parent_cell->getNode(num_nodes);
            }
            else {
                forward_node = parent_cell->getNode(node_pos-1);
            }
            if (node != forward_node
                && std::find( neighbors.begin(), neighbors.end() , forward_node) == neighbors.end() ) {
                neighbors.push_back(forward_node);
            }

            // 後方節点（ノード）
            UdmNode *backward_node = NULL;
            if (node_pos == num_nodes) {
                backward_node = parent_cell->getNode(1);
            }
            else {
                backward_node = parent_cell->getNode(node_pos+1);
            }
            if (node != backward_node
                && std::find( neighbors.begin(), neighbors.end() , backward_node) == neighbors.end() ) {
                neighbors.push_back(backward_node);
            }
        }
        else {
            UDM_WARNING_HANDLER(UDM_WARNING_INVALID_NODE, "not exists[%d] node in cell[%d].", node->getId(), parent_cell->getId());
        }
    }

    // ソート
    std::sort(neighbors.begin(), neighbors.end(), UdmEntity::compareIds);

    if (neighbors.size() > 0) {
        this->scan_arrays.push_back(neighbors);
    }
    return neighbors.size();
}


/**
 * 要素（セル）の隣接要素（セル）を取得する.
 * @param cell        探索対象要素（セル）
 * @return        隣接要素（セル）数
 */
UdmSize_t UdmScannerCells::scannerNeighborCells(const UdmCell* cell)
{
    if (cell == NULL) return 0;

    std::vector<UdmEntity*> neighbors;
    int n, m;

    // 部品要素の検索
    int num_components = cell->getNumComponentCells();
    for (n=1; n<=num_components; n++) {
        UdmComponent* component_cell = cell->getComponentCell(n);
        // 部品要素の親要素の取得
        int num_parents = component_cell->getNumParentCells();
        for (m=1; m<=num_parents; m++) {
            UdmCell *parent_cell = component_cell->getParentCell(m);
            if (parent_cell == NULL) continue;
            if (parent_cell == cell) continue;
            if (std::find( neighbors.begin(), neighbors.end() , parent_cell) == neighbors.end() ) {
                neighbors.push_back(parent_cell);
            }
        }
    }

    // ソート
    std::sort(neighbors.begin(), neighbors.end(), UdmEntity::compareIds);

    if (neighbors.size() > 0) {
        this->scan_arrays.push_back(neighbors);
    }
    return neighbors.size();
}

/**
 * ノード、セルリストにグローバルID(ランク番号、ID)の昇順にノード、セルを挿入する.
 * 挿入済みグローバルIDノード、セルは挿入しない.
 * @param [out] entities            ノード、セルリスト
 * @param [in] entity                挿入ノード、セル
 * @return            ノード、セルリスト数
 */
UdmSize_t UdmScannerCells::insertEntityList(
                std::vector<UdmEntity*>& entities,
                UdmEntity* entity) const
{
    if (entity == NULL) return entities.size();
    if (entities.size() <= 0) {
        entities.push_back(entity);
        return entities.size();
    }
    UdmSize_t src_id = entity->getId();
    int src_rankno = entity->getMyRankno();

    UdmSize_t left = 0;
    UdmSize_t right = entities.size();
    UdmSize_t mid;
    UdmSize_t size = entities.size();
    UdmEntity *first = (UdmEntity*)entities[left];
    UdmEntity *last = (UdmEntity*)entities[right-1];
    UdmEntity *mid_entity = NULL;
    UdmSize_t first_id = first->getId();
    UdmSize_t last_id = last->getId();
    UdmSize_t value;
    std::vector<UdmEntity*>::iterator insert_itr;
    int compare;

    compare = first->compareGlobalId(src_rankno, src_id);
    if (compare == 0) return entities.size();
    if (compare > 0) {
        // 先頭に挿入
        entities.insert(entities.begin(), entity);
        return entities.size();
    }
    compare = last->compareGlobalId(src_rankno, src_id);
    if (compare == 0) return entities.size();
    if (compare < 0) {
        // 末尾に挿入
        entities.push_back(entity);
        return entities.size();
    }

    // src_id位置で検索
    if (src_id >= first_id && src_id-first_id < size) {
        UdmEntity *list_entity = entities[src_id-first_id];
        compare = list_entity->compareGlobalId(src_rankno, src_id);
        insert_itr = entities.begin() + (src_id-first_id);
        if (compare == 0) {
            return entities.size();
        }
        else if (compare > 0) {
            right = std::distance(entities.begin(), insert_itr);
        }
        else {
            left = std::distance(entities.begin(), insert_itr);
        }
    }
    int diff = 0;
    while (left <= right) {
        mid = (left + right) / 2;
        if (mid >= size) {
            break;
        }
        mid_entity = (UdmEntity*)entities[mid];
        value = mid_entity->getId();
        diff = mid_entity->compareGlobalId(src_rankno, src_id);
        if (diff == 0) {
            return entities.size();
        }
        else if (diff < 0) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }
    if (diff < 0) {
        insert_itr = entities.begin() + mid + 1;
    }
    else {
        insert_itr = entities.begin() + mid;
    }
    entities.insert(insert_itr, entity);

    return entities.size();
}


/**
 * 境界節点（ノード）を探索する.
 * 親部品要素が１つの要素（セル）の節点（ノード）を検索する.
 * @param node_list        探索節点（ノード）リスト
 * @return        探索結果親数（重複を含む）
 */
UdmSize_t UdmScannerCells::scannerBoundaryNode(const std::vector<UdmNode*>& node_list)
{
    UdmSize_t count = 0;
    std::vector<UdmNode*>::const_iterator itr;
    for (itr=node_list.begin(); itr!=node_list.end(); itr++) {
        count += scannerBoundaryNode(*itr);
    }
    return count;
}

/**
 * 境界節点（ノード）を探索する.
 * 親部品要素が１つの要素（セル）の節点（ノード）を検索する.
 * @param node        探索節点（ノード）
 * @return        探索結果親数（重複を含む）
 */
UdmSize_t UdmScannerCells::scannerBoundaryNode(const UdmNode* node)
{
    if (node == NULL) return 0;

    std::vector<UdmEntity*> boundaries;
    int n, m;
    int num_parents = node->getNumParentCells();
    for (n=1; n<=num_parents; n++) {
        UdmICellComponent* component_cell = node->getParentCell(n);
        // 親要素が設定されていること。部品要素には設定されている。
        int num_parent = component_cell->getNumParentCells();
        if (num_parent == 0) continue;
        // 部品要素の親要素数が１であること。
        int found_parent = 0;
        for (m=1; m<=num_parent; m++) {
            UdmCell* parent_cell = component_cell->getParentCell(m);
            if (parent_cell->getRealityType() == Udm_Actual) {
                found_parent++;
            }
        }
        if (found_parent == 1) {
            boundaries.push_back((UdmEntity*)node);
        }
    }
    if (boundaries.size() > 0) {
        this->scan_arrays.push_back(boundaries);
    }
    return boundaries.size();
}

/**
 * データ転送を行うセルサイズを取得する.
 * @param [in] num_gid_entries    グローバルIDのデータサイズ
 * @param [in] num_lid_entries    ローカルIDのデータサイズ
 * @param [in] num_ids            データ転送頂点数
 * @param [in] global_ids        グローバルIDリスト
 * @param [in] local_ids        ローカルIDリスト
 * @param [in] exportProcs        エクスポート先プロセス番号
 * @return        総セルサイズ
 */
template <class DATA_TYPE>
size_t UdmScannerCells::scannerCellsSize(
                            int num_gid_entries,
                            int num_lid_entries,
                            int num_ids,
                            DATA_TYPE *global_ids,
                            DATA_TYPE *local_ids,
                            int *exportProcs)
{
    if (num_ids <= 0) return 0;

    // 要素（セル）のシリアライズサイズを取得する.
    const UdmZone *zone = this->getScanZone();
    if (zone == NULL) {
        UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "UdmZone is null.");
        return 0;
    }

    // サイズリストクリア
    this->cell_sizes.clear();

#ifdef _DEBUG
    size_t proc1_count = 0;
    size_t proc1_size = 0;
#endif
    UdmSerializeBuffer streamBuffer;        // バッファ(char*)をNULLとする
    UdmSerializeArchive archive(&streamBuffer);
    UdmSize_t cell_id;
    int rankno;
    int i;
    int pos = 0;
    size_t total_size = 0;
    for (i = 0; i < num_ids; i++) {
        // global_idsから要素（セル）IDの取得
        pos += udm_get_entryid(global_ids+pos, cell_id, rankno, num_gid_entries);
        const UdmCell *cell = zone->getCell(cell_id);
        if (cell == NULL) {
            (int)UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "not found cell(cell_id=%d,rankno=%d)", cell_id, rankno);
            return 0;
        }
        archive << *cell;

        int buf_size = archive.getOverflowSize();
        this->cell_sizes.push_back(buf_size);
        total_size += buf_size;
        // バッファークリア
        archive.clear();
#ifdef _DEBUG
        if (exportProcs[i] == 1) {
            proc1_count++;
            proc1_size += buf_size;
        }
#endif
    }

#ifdef _DEBUG
    printf("UdmScannerCells::scannerCellsSize: proc1_count=%d, proc1_size=%ld \n",
            proc1_count, proc1_size);
#endif
    return total_size;
}
template size_t UdmScannerCells::scannerCellsSize(int num_gid_entries, int num_lid_entries, int num_ids, int *global_ids, int *local_ids, int *exportProcs);
template size_t UdmScannerCells::scannerCellsSize(int num_gid_entries, int num_lid_entries, int num_ids, unsigned int *global_ids, unsigned int *local_ids, int *exportProcs);
template size_t UdmScannerCells::scannerCellsSize(int num_gid_entries, int num_lid_entries, int num_ids, long *global_ids, long *local_ids, int *exportProcs);
template size_t UdmScannerCells::scannerCellsSize(int num_gid_entries, int num_lid_entries, int num_ids, unsigned long *global_ids, unsigned long *local_ids, int *exportProcs);
template size_t UdmScannerCells::scannerCellsSize(int num_gid_entries, int num_lid_entries, int num_ids, long long *global_ids, long long *local_ids, int *exportProcs);
template size_t UdmScannerCells::scannerCellsSize(int num_gid_entries, int num_lid_entries, int num_ids, unsigned long long *global_ids, unsigned long long *local_ids, int *exportProcs);


/**
 * データ転送を行うセルサイズリストを取得する.
 * @return        セルサイズリスト
 */
const std::vector<int>& UdmScannerCells::getScanCellSizes() const
{
    return this->cell_sizes;
}

/**
 * アクセスポジションを取得する.
 * @return        アクセスポジション
 */
int UdmScannerCells::getScanIterator() const
{
    return this->scan_iterator;
}

/**
 * アクセスポジションを設定する.
 * @param iterator        アクセスポジション
 */
void UdmScannerCells::setScanIterator(int iterator)
{
    this->scan_iterator = iterator;
}

} /* namespace udm */
