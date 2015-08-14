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

/**
 * @file UdmNode.cpp
 * ノード（節点）クラスのソースファイル
 */

#include "model/UdmNode.h"
#include "model/UdmGridCoordinates.h"
#include "model/UdmCell.h"
#include "utils/UdmScannerCells.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmNode::UdmNode() : coords(NULL), parent_gridcoordinates(NULL), common_node(NULL)
{
    this->initialize();
}

/**
 * コンストラクタ : double XYZ座標.
 * @param x        X座標
 * @param y        Y座標
 * @param z        Z座標
 */
UdmNode::UdmNode(double x, double y, double z) : coords(NULL), parent_gridcoordinates(NULL), common_node(NULL)
{
    this->initialize();
    this->setCoords(x, y, z);
}


/**
 * コンストラクタ : float XYZ座標.
 * @param x        X座標
 * @param y        Y座標
 * @param z        Z座標
 */
UdmNode::UdmNode(float x, float y, float z) : coords(NULL), parent_gridcoordinates(NULL), common_node(NULL)
{
    this->initialize();
    this->setCoords(x, y, z);
}

/**
 * デストラクタ
 */
UdmNode::~UdmNode()
{
    // 親要素（セル）からこのノードの参照を削除する。
    this->removeReferencedNode();
    this->parent_cells.clear();

    if (this->coords != NULL) delete this->coords;
    this->coords = NULL;
    this->common_node = NULL;


}


/**
 * ノード座標を取得する:double.
 * @param [out] x        X座標
 * @param [out] y        Y座標
 * @param [out] z        Z座標
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmNode::getCoords(double& x, double& y, double& z) const
{
    if (this->coords == NULL) return UDM_ERROR;
    this->coords->getCoords<double>(x, y, z);
    return UDM_OK;
}

/**
 * ノード座標を取得する:float.
 * @param [out] x        X座標
 * @param [out] y        Y座標
 * @param [out] z        Z座標
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmNode::getCoords(float& x, float& y, float& z) const
{
    if (this->coords == NULL) return UDM_ERROR;
    this->coords->getCoords<float>(x, y, z);
    return UDM_OK;
}


/**
 * ノード座標を設定する : double.
 * @param x        X座標
 * @param y        Y座標
 * @param z        Z座標
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmNode::setCoords(double x, double y, double z)
{
    if (this->coords == NULL) {
        this->coords = UdmCoordsValue::factoryCoordsValue(Udm_RealDouble);
    }
    this->coords->setCoords<double>(x, y, z);
    return UDM_OK;
}

/**
 * ノード座標を設定する : float.
 * @param x        X座標
 * @param y        Y座標
 * @param z        Z座標
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmNode::setCoords(float x, float y, float z)
{
    if (this->coords == NULL) {
        this->coords = UdmCoordsValue::factoryCoordsValue(Udm_RealSingle);
    }
    this->coords->setCoords<float>(x, y, z);
    return UDM_OK;
}


/**
 * 初期化を行う。
 */
void UdmNode::initialize()
{
    this->coords = NULL;
    // 要素形状タイプ=NODE
    this->setElementType(Udm_NODE);
    this->parent_gridcoordinates = NULL;
    this->common_node = NULL;
    this->parent_cells.clear();
}


/**
 * 親GridCoordinatesを取得する.
 * @return        親GridCoordinates
 */
UdmGridCoordinates* UdmNode::getParentGridcoordinates() const
{
    return this->parent_gridcoordinates;
}

/**
 * 親GridCoordinatesを設定する.
 * @param parent_gridcoordinates        親GridCoordinates
 */
void UdmNode::setParentGridcoordinates(UdmGridCoordinates* parent_gridcoordinates)
{
    this->parent_gridcoordinates = parent_gridcoordinates;
}

/**
 * 座標データを比較する.
 * 比較座標:this - 比較座標:rightの結果を返す.
 * Z座標から比較する. Z座標 -> Y座標 -> X座標
 * @param    right            比較座標
 * @return    -1 = 比較座標:this < 比較座標:right
 *              0 = 比較座標:this = 比較座標:right
 *             +1 = 比較座標:this > 比較座標:right
 */
int UdmNode::compareCoords(const UdmNode &right ) const
{
    if (this->coords == NULL) return -1;
    if (right.coords == NULL) return -1;
    return this->coords->compareCoords(*right.coords);
}

/**
 * 所属ゾーンを取得する.
 * @return        所属ゾーン
 */
UdmZone* UdmNode::getZone() const
{
    if (this->parent_gridcoordinates == NULL) return NULL;
    return this->parent_gridcoordinates->getParentZone();
}


/**
 * 座標値を文字列として出力する.
 * @param buf        出力文字列
 */
void UdmNode::toString(std::string &buf) const
{
    char tmp[128];
    sprintf(tmp, "x,y,z = %e,%e,%e",
            this->coords->getCoordinateX<float>(),
            this->coords->getCoordinateY<float>(),
            this->coords->getCoordinateZ<float>());
    buf += tmp;

    if (this->getNumPreviousRankInfos() > 0) {
        std::string info_buf;
        this->getPreviousRankInfos().toString(info_buf);
        buf += " <- " + info_buf;
    }

    buf += "\n";
    // 物理量の出力
    std::string sol_buf;
    UdmEntity::toString(sol_buf);
    buf += sol_buf;

    return;
}


/**
 * シリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmNode::serialize(UdmSerializeArchive& archive) const
{
    if (this->getId() > 0 && this->coords != NULL) {
        // ID
        archive << this->getId();

        // UdmEntity
        UdmEntity::serialize(archive);

        // XYZ座標値 : UdmCoordsValue *coords;
        archive << (*this->coords);
    }
    else {
        archive << (unsigned int)0;
    }

    return archive;
}

/**
 * デシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmNode::deserialize(UdmSerializeArchive& archive)
{
    if (archive.rdstate() != std::ios_base::goodbit) {
        UDM_ERROR_HANDLER(UDM_ERROR_DESERIALIZE, "rdstate is not goodbit.");
        return archive;
    }
    UdmSize_t node_id;
    // ID
    archive >> node_id;

    if (node_id > 0) {
        // UdmEntity
        UdmEntity::deserialize(archive);

#if 0        // tellg, seekg削除
        // XYZ座標値:現在位置
        // std::iostream::pos_type cur = archive.tellg();
        size_t cur = archive.tellg();
#endif

        // XYZ座標値 : UdmCoordsValue *coords;
        // データ型
        UdmDataType_t datatype;
        archive >> datatype;
        this->coords = UdmCoordsValue::factoryCoordsValue(datatype);
        if (this->coords == NULL) {
            UDM_ERROR_HANDLER(UDM_ERROR_DESERIALIZE, "failure : factoryCoordsValue[data_type=%d].", datatype);
            archive.setstate(std::ios_base::failbit);
            return archive;
        }

#if 0        // tellg, seekg削除
        // XYZ座標値:現在位置まで戻す。
        archive.seekg(cur);
#endif
        // XYZ座標値のデシリアライズ
        archive >> (*(this->coords));
    }
    return archive;
}


/**
 * デシリアライズ(UdmNode::deserialize)にて生成されたオブジェクトの開放を行う.
 * 親要素（セル）はクリアして参照の削除は行わない。
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmNode::freeDeserialize(UdmNode *node)
{
    node->clearParentCells();

    delete node;
    node = NULL;

    return UDM_OK;
}

/**
 * ノード（節点）をコピーする.
 * @param src        ノード（節点）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmNode::cloneNode(const UdmNode& src)
{
    // 物理量データのコピー
    UdmEntity::cloneEntity(src);

    if (src.coords == NULL) return UDM_ERROR;
    UdmDataType_t src_datatype = src.coords->getDataType();
    if (src_datatype == Udm_RealDouble) {
        double x, y, z;
        src.getCoords(x, y, z);
        this->setCoords(x, y, z);
    }
    else {
        float x, y, z;
        src.getCoords(x, y, z);
        this->setCoords(x, y, z);
    }

    return UDM_OK;
}

/**
 * MPIランク番号,ローカルIDが一致する節点（ノード）を取得する.
 * @param inner_nodes        検索節点（ノード）リスト
 * @param  rankno        MPIランク番号
 * @param  localid       ローカルID
 * @return        節点（ノード）
 */
UdmNode* UdmNode::findMpiRankInfo(
                    const std::vector<UdmNode*>& inner_nodes,
                    int rankno,
                    UdmSize_t localid)
{
    if (rankno < 0) return NULL;
    if (localid <= 0) return NULL;

    std::vector<UdmNode*>::const_iterator itr;
    for (itr=inner_nodes.begin(); itr!=inner_nodes.end(); itr++) {
        if ((*itr)->getMyRankno() == rankno && (*itr)->getId() == localid) {
            return (*itr);
        }
        if ((*itr)->existsPreviousRankInfo(rankno, localid)) {
            return (*itr);
        }
        if (((UdmEntity*)(*itr))->existsMpiRankInfo(rankno, localid)) {
            return (*itr);
        }
    }

    return NULL;
}

/**
 * MPIランク番号,ローカルIDが一致する節点（ノード）を取得する.
 * @param inner_nodes        検索節点（ノード）リスト
 * @param  src_node        検索節点（ノード）
 * @return        一致節点（ノード）
 */
UdmNode* UdmNode::findMpiRankInfo(
                    const std::vector<UdmNode*>& inner_nodes,
                    const UdmNode *src_node)
{
    if (src_node == NULL) return NULL;

    int rankno = src_node->getMyRankno();
    UdmSize_t localid = src_node->getId();
    UdmNode *found_node = UdmNode::findMpiRankInfo(inner_nodes, rankno, localid);
    if (found_node != NULL) {
        return found_node;
    }
    int num_mpi = src_node->getNumMpiRankInfos();
    int n;
    for (n=1; n<num_mpi; n++) {
        src_node->getMpiRankInfo(n, rankno, localid);
        found_node = UdmNode::findMpiRankInfo(inner_nodes, rankno, localid);
        if (found_node != NULL) {
            return found_node;
        }
    }

    return NULL;
}


/**
 * 同一節点（ノード）を取得する
 * @return        同一節点（ノード）
 */
UdmNode* UdmNode::getCommonNode() const
{
    return this->common_node;
}

/**
 * 同一節点（ノード）を設定する.
 * @param node        同一節点（ノード）
 */
void UdmNode::setCommonNode(UdmNode* node)
{
    this->common_node = node;
}


/**
 * 節点（ノード）が接続している要素（セル）数を取得する.
 * @return            接続要素（セル）数
 */
UdmSize_t UdmNode::getNumConnectivityCells() const
{
    UdmScannerCells scanner;
    scanner.scannerParents(this);
    UdmSize_t num_cells = scanner.scannerParents(this);
    return num_cells;
}


/**
 * 節点（ノード）が接続している要素（セル）を取得する.
 * @param connectivity_id    接続ID : 1～getNumConnectivityCells()
 * @return            接続要素（セル）
 */
UdmCell* UdmNode::getConnectivityCell(int connectivity_id) const
{
    UdmScannerCells scanner;
    UdmSize_t num_cells = scanner.scannerParents(this);
    if (num_cells <= 0) return NULL;
    const std::vector<std::vector<UdmEntity*> > scan_array = scanner.getScanArrays();
    if (scan_array.size() <= 0) return NULL;
    std::vector<UdmEntity*> cell_array = scan_array[0];

    if (connectivity_id <= 0) return NULL;
    if (connectivity_id > cell_array.size()) return NULL;

    return static_cast<UdmCell*>(cell_array[connectivity_id-1]);
}

/**
 * 節点（ノード）の隣接節点（ノード）数を取得する.
 * @return        隣接節点（ノード）数
 */
UdmSize_t UdmNode::getNumNeighborNodes() const
{
    UdmScannerCells scanner;
    UdmSize_t num_nodes = scanner.scannerNeighborNodes(this);
    return num_nodes;
}

/**
 * 節点（ノード）の隣接節点（ノード）を取得する.
 * @param neighbor_id        隣接ID : 1～getNumNeighborNodes()
 * @return        隣接節点（ノード）
 */
UdmNode* UdmNode::getNeighborNode(int neighbor_id) const
{
    UdmScannerCells scanner;
    UdmSize_t num_nodes = scanner.scannerNeighborNodes(this);
    if (num_nodes <= 0) return 0;
    const std::vector<std::vector<UdmEntity*> > scan_array = scanner.getScanArrays();
    if (scan_array.size() <= 0) return 0;
    std::vector<UdmEntity*> node_array = scan_array[0];

    if (neighbor_id <= 0) return NULL;
    if (neighbor_id > node_array.size()) return NULL;
    return static_cast<UdmNode*>(node_array[neighbor_id-1]);
}


/**
 * 親要素（セル）数を取得する.
 * @return        親要素（セル）数
 */
UdmSize_t UdmNode::getNumParentCells() const
{
    return this->parent_cells.size();
}

/**
 * 親要素（セル）を取得する.
 * @param cell_id        親要素（セル）ID（１～）
 * @return        親要素（セル）
 */
UdmICellComponent* UdmNode::getParentCell(UdmSize_t cell_id) const
{
    if (cell_id <= 0) return NULL;
    if (cell_id > this->parent_cells.size()) return NULL;
    return this->parent_cells[cell_id-1];
}


/**
 * 親要素（セル）リストを設定する.
 * @param parent_cells        親要素（セル）リスト
 * @param size                親要素（セル）サイズ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmNode::setParentCells(UdmICellComponent* parent_cells[], UdmSize_t size)
{
    this->clearParentCells();
    for (UdmSize_t i=0; i<size; i++) {
        if (parent_cells[i] == NULL) return UDM_ERROR;
        this->parent_cells.push_back(parent_cells[i]);
    }
    return UDM_OK;
}

/**
 * 親要素（セル）を追加する.
 * @param parent_cells        親要素（セル）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmNode::insertParentCell(UdmICellComponent* parent_cell)
{
    if (parent_cell == NULL) return UDM_ERROR;

    UdmSize_t cell_id = 0;
    std::vector<UdmICellComponent*>::iterator find_itr;
    find_itr = std::find(this->parent_cells.begin(), this->parent_cells.end(), parent_cell);
    if (find_itr == this->parent_cells.end()) {
        cell_id = parent_cell->getId();
        this->parent_cells.push_back(parent_cell);
    }
    return UDM_OK;
}

/**
 * 親要素（セル）リストをクリアする.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmNode::clearParentCells()
{
    this->parent_cells.clear();
    return UDM_OK;
}

/**
 * 親構成要素から削除親要素（セル）を削除する.
 * @param parent_cell        削除親要素（セル）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmSize_t UdmNode::removeReferencedCell(const UdmICellComponent* parent_cell)
{
    if (parent_cell == NULL) return this->parent_cells.size();

    int i;
    size_t size = this->parent_cells.size();
    if (size > 0) {
        for (i=size-1; i>=0; i--) {
            UdmICellComponent *cell = this->parent_cells.at(i);
            if (cell == parent_cell) {
                this->parent_cells.erase(this->parent_cells.begin() + i);
                break;        // 親要素（セル）は重複はない
            }
        }
    }

    return this->parent_cells.size();
}


/**
 * 親構成要素からこの節点（ノード）の参照を削除する.
 * @param parent_cell        削除親要素（セル）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
void UdmNode::removeReferencedNode()
{
    int size = this->parent_cells.size();
    if (size <= 0) return;

    std::vector<UdmICellComponent*>::iterator itr;
    for (itr=this->parent_cells.begin(); itr!=this->parent_cells.end(); itr++) {
        UdmICellComponent *cell = (*itr);
        cell->removeNode(this);
    }

    return;
}

/**
 * メモリサイズを取得する.
 * @return        メモリサイズ
 */
size_t UdmNode::getMemSize() const
{
    UdmSize_t size = sizeof(*this);
#ifdef _DEBUG
    size_t coords_memseize = this->coords->getMemSize();

    printf("UdmEntity size=%ld\n", UdmEntity::getMemSize());
    printf("this size=%ld\n", sizeof(*this));
    if (this->coords != NULL) {
        printf("coords size=%ld [offset=%ld]\n", this->coords->getMemSize(), offsetof(UdmNode, coords));
    }
    printf("parent_gridcoordinates pointer size=%ld [offset=%ld]\n", sizeof(this->parent_gridcoordinates), offsetof(UdmNode, parent_gridcoordinates));
    printf("common_node common_node size=%ld [offset=%ld]\n", sizeof(this->common_node), offsetof(UdmNode, common_node));

    printf("parent_cells size=%ld [count=%d] [capacity=%d] [offset=%ld] \n",
                        sizeof(this->parent_cells),
                        this->parent_cells.size(),
                        this->parent_cells.capacity(),
                        offsetof(UdmNode, parent_cells));

    size += this->getNumParentCells()*sizeof(UdmCell*);
    size += this->getNumPreviousRankInfos() * sizeof(UdmGlobalRankid);
    size += coords_memseize;
#endif

    return size;
}

/**
 * 節点（セル）の破棄処理を行う.
 * 親要素（セル）のクリアを行う.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
void UdmNode::finalize()
{
    this->parent_cells.clear();

    return;
}
} /* namespace udm */
