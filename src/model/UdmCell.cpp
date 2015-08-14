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
 * @file UdmCell.cpp
 * 要素（セル）基底クラスのソースファイル
 */

#include "model/UdmCell.h"
#include "model/UdmZone.h"
#include "model/UdmGridCoordinates.h"
#include "model/UdmSections.h"
#include "model/UdmElements.h"
#include "model/UdmNode.h"
#include "model/UdmBar.h"
#include "model/UdmShell.h"
#include "model/UdmSolid.h"
#include "model/UdmComponent.h"
#include "utils/UdmScannerCells.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmCell::UdmCell()
{
    this->initialize();
}

/**
 * コンストラクタ
 * @param   element_type        要素形状タイプ
 */
UdmCell::UdmCell(UdmElementType_t element_type) : UdmEntity(element_type)
{
    this->initialize();
}

/**
 * デストラクタ
 */
UdmCell::~UdmCell()
{
    // この要素を参照しているノード、要素からこの要素を削除する.
    this->removeReferencedCells();
}

/**
 * 初期化を行う.
 */
void UdmCell::initialize()
{
    this->elem_id = 0;
    this->nodes.clear();
    this->components.clear();
    this->parent_elements = NULL;
    this->parent_sections = NULL;
}


/**
 * 管理要素IDを取得する.
 * 管理要素ID = UdmElements::std::map<UdmSize_t, UdmCell*> entity_cellsのキー
 * ゾーン要素ID = cell_id,UdmGeneral::id, ゾーン内での連番ID
 *                 UdmElements::std::map<UdmSize_t, UdmCell*> entity_cellsの格納インデックス+1
 * @return   管理要素ID
 */
UdmSize_t UdmCell::getElemId() const
{
    return this->elem_id;
}


/**
 *
 * 管理要素IDを設定する.
 * 管理要素ID = UdmElements::std::map<UdmSize_t, UdmCell*> entity_cellsのキー
 * ゾーン要素ID = cell_id,UdmGeneral::id, ゾーン内での連番ID
 *                 UdmElements::std::map<UdmSize_t, UdmCell*> entity_cellsの格納インデックス+1
 * @param elemId        管理要素ID
 */
void UdmCell::setElemId(UdmSize_t elem_id)
{
    this->elem_id = elem_id;
}



/**
 * 構成ノード（節点）数を取得する.
 * @return        構成ノード（節点）数
 */
UdmSize_t UdmCell::getNumNodes() const
{
    return this->nodes.size();
}

/**
 * 構成ノード（節点）を取得する.
 * @param node_id        構成ノードID（１～）
 * @return        構成ノード（節点）
 */
UdmNode* UdmCell::getNode(UdmSize_t node_id)  const
{
    if (node_id <= 0) return NULL;
    if (node_id > this->nodes.size()) return NULL;
    return this->nodes[node_id-1];
}


/**
 * 構成ノード（節点）リストを設定する.
 * @param nodes        構成ノード（節点）リスト
 * @param size        構成ノード（節点）数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmCell::setNodes(UdmNode* nodes[], UdmSize_t size)
{
    this->clearNodes();
    for (UdmSize_t i=0; i<size; i++) {
        if (nodes[i] == NULL) return UDM_ERROR;
        this->insertNode(nodes[i]);
    }
    return UDM_OK;
}

/**
 * 構成ノード（節点）リストを設定する.
 * @param nodes        構成ノード（節点）リスト
 * @param size        構成ノード（節点）数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmCell::setNodes(const std::vector<UdmNode*> &nodes)
{
    int n;
    int num_nodes = nodes.size();

    this->clearNodes();
    for (n=0; n<num_nodes; n++) {
        this->insertNode(nodes[n]);
    }
    return UDM_OK;
}

/**
 * 構成ノード（節点）リストを取得する.
 * @return        構成ノード（節点）リスト
 */
const std::vector<UdmNode*>& UdmCell::getNodes() const
{
    return this->nodes;
}

/**
 * 構成ノード（節点）を挿入する.
 * @param node        構成ノード（節点）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmCell::insertNode(UdmNode* node)
{
    if (node == NULL) return UDM_ERROR;
    this->nodes.push_back(node);

    // 親要素（セル）を追加する.
    node->insertParentCell(this);

    return UDM_OK;
}

/**
 * 構成ノード（節点）リストをクリアする.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmCell::clearNodes()
{
    this->nodes.clear();
    return UDM_OK;
}

/**
 * 親セクション（要素構成）情報クラスを取得する.
 * @return        セクション（要素構成）情報クラス
 */
UdmElements* UdmCell::getParentElements() const
{
    return this->parent_elements;
}


/**
 * 親セクション（要素構成）情報クラスを設定する.
 * @param parent_elements        親セクション（要素構成）情報クラス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmCell::setParentElements(UdmElements* parent_elements)
{
    this->parent_elements = parent_elements;
    return UDM_OK;
}

/**
 * 所属ゾーンを取得する.
 * @return        所属ゾーン
 */
UdmZone* UdmCell::getZone() const
{
    if (this->parent_elements != NULL) {
        if (this->parent_elements->getParentSections() == NULL) return NULL;
        return this->parent_elements->getParentSections()->getParentZone();
    }
    if (this->parent_sections != NULL) {
        return this->parent_sections->getParentZone();
    }
    return NULL;
}

/**
 * 要素クラスタイプを取得する.
 * @return        要素クラスタイプ=Udm_ComponentClass:部品要素クラス
 */
UdmCellClass_t UdmCell::getCellClass() const
{
    return Udm_CellClass;
}

/**
 * クラス情報を文字列出力する:デバッグ用.
 * @param buf        出力文字列
 */
void UdmCell::toString(std::string& buf) const
{
#ifdef _DEBUG_TRACE
    std::stringstream stream;
    char fmt_buf[32];
    int count = 0;

    std::vector<UdmNode*>::const_iterator itr;
    if (this->getRealityType() == Udm_Virtual) {
        stream << "VIRTUAL_CELL[ID=" << this->getId() << ",RANK=" << this->getMyRankno() << "] : ";
    }
    else {
        stream << "CELL[" << this->getId() << "] : ";
    }

    std::string type_buf;
    UdmGeneral::toStringElementType(this->getElementType(), type_buf);
    sprintf(fmt_buf, "%-8s", type_buf.c_str());
    stream <<  fmt_buf << " ";

    for (itr=this->nodes.begin(); itr!=this->nodes.end(); itr++) {
        if (itr!=this->nodes.begin()) stream << ", ";
        if ((*itr)->getRealityType() == Udm_Virtual) {
            stream << (*itr)->getId() << "[" << (*itr)->getMyRankno() << "]";
        }
        else {
            stream << (*itr)->getId();
        }
    }
    stream << std::endl;

    // 部品要素の出力
    count = 0;
    std::vector<UdmComponent*>::const_iterator comp_itr;
    for (comp_itr=this->components.begin(); comp_itr!=this->components.end(); comp_itr++) {
        std::string comp_buf;
        (*comp_itr)->toString(comp_buf);
        stream << "    component[" << (++count) << "] " << comp_buf;
    }


    // 物理量の出力
    std::string sol_buf;
    UdmEntity::toString(sol_buf);
    stream << sol_buf;

    buf += stream.str();
#endif
    return;
}

/**
 * 頂点（ノード）構成、IDが同一の要素（セル）であるかチェックする.
 * @param dest_cell        比較対象要素（セル）
 * @return        true=同一要素（セル）
 */
bool UdmCell::equalsNodeIds(const UdmCell* dest_cell) const
{
    if (dest_cell == NULL) return false;
    if (this->nodes.size() != dest_cell->nodes.size()) return false;

    std::set<UdmSize_t> src_ids;
    std::vector<UdmNode*>::const_iterator src_itr;
    std::vector<UdmNode*>::const_iterator dest_itr;

#if 0
    for (dest_itr=dest_cell->nodes.begin(); dest_itr!=dest_cell->nodes.end(); dest_itr++) {
        UdmNode *dest_node = (*dest_itr);
        UdmNode *src_node = NULL;
        for (src_itr=this->nodes.begin(); src_itr!=this->nodes.end(); src_itr++) {
            src_node = (*src_itr);
            if (dest_node->compareGlobalId(src_node) == 0) {
                break;;
            }
            src_node = NULL;
        }
        if (src_node == NULL) {
            return false;
        }
    }
#endif

    UdmGlobalRankidList src_rankids(this->getNumNodes());
    UdmGlobalRankidList dest_rankids(this->getNumNodes());

    for (src_itr=this->nodes.begin(); src_itr!=this->nodes.end(); src_itr++) {
        UdmNode *src_node = (*src_itr);
        src_rankids.addGlobalRankid(src_node->getMyRankno(), src_node->getId());
    }

    for (dest_itr=dest_cell->nodes.begin(); dest_itr!=dest_cell->nodes.end(); dest_itr++) {
        UdmNode *dest_node = (*dest_itr);
        dest_rankids.addGlobalRankid(dest_node->getMyRankno(), dest_node->getId());
    }

    if (!src_rankids.equals(dest_rankids)) {
        return false;
    }

    return true;
}


/**
 * 要素（セル）を生成する.
 * @param elem_type        要素タイプ
 * @return        生成要素（セル）
 */
UdmCell* UdmCell::factoryCell(UdmElementType_t elem_type)
{

    int num_vertex = UdmGeneral::getNumVertexOfElementType(elem_type);
    if (num_vertex <= 0) return NULL;

    UdmCell* cell = NULL;
    if (elem_type == Udm_BAR_2) {
        cell = new UdmBar(elem_type);
    }
    else if (elem_type == Udm_TRI_3 || elem_type == Udm_QUAD_4) {
        cell = new UdmShell(elem_type);
    }
    else if (elem_type == Udm_TETRA_4
            || elem_type == Udm_PYRA_5
            || elem_type == Udm_PENTA_6
            || elem_type == Udm_HEXA_8) {
        cell = new UdmSolid(elem_type);
    }
    else {
        return NULL;
    }

    return cell;
}


/**
 * 部品要素（セル）を生成する.
 * @param elem_type        要素タイプ
 * @return        生成部品要素（セル）
 */
UdmComponent* UdmCell::factoryComponentCell(UdmElementType_t elem_type)
{

    int num_vertex = UdmGeneral::getNumVertexOfElementType(elem_type);
    if (num_vertex <= 0) return NULL;

    UdmComponent *component = new UdmComponent(elem_type);

    return component;
}


/**
 * 部品要素（セル）の数を取得する.
 * @return        部品要素（セル）数
 */
UdmSize_t UdmCell::getNumComponentCells() const
{
    return this->components.size();
}

/**
 * 部品要素（セル）を取得する.
 * @param components_id        部品要素（セル）ID （１～）
 * @return        部品要素（セル）
 */
UdmComponent* UdmCell::getComponentCell(UdmSize_t components_id) const
{
    if (components_id <= 0) return NULL;
    if (components_id > this->components.size()) return NULL;
    return this->components[components_id-1];
}


/**
 * 部品要素（セル）を生成して、UdmSectionsクラスに格納を行う.
 * @param element_type        要素形状タイプ
 * @param num_nodes            頂点（ノード）数
 * @param face_nodes        面-頂点テーブル
 * @return        部品要素（セル）ID  : 追加失敗、エラーの場合は0を返す。
 */
UdmSize_t UdmCell::createComponentCell(UdmElementType_t element_type, unsigned int num_nodes, UdmSize_t face_nodes[])
{
    unsigned int i;
    UdmSections* sections = this->getParentSections();
    if (sections == NULL) return 0;

    // 部品要素（セル）の生成
    UdmComponent *component = this->factoryComponentCell(element_type);
    for (i=0; i<num_nodes; i++) {
        UdmNode* node = this->getNode(face_nodes[i]);
        if (node == NULL) {
            delete component;
            return 0;
        }
        component->insertNode(node);
    }

    // 同一部品要素（セル）が追加済みであるかチェックする.
    UdmComponent* found_cell = sections->findComponentCell(component);
    if (found_cell == NULL) {
        UdmSize_t cell_id = sections->insertComponentCell(component);
        if (cell_id <= 0) {
            delete component;
            return 0;
        }
        found_cell = component;
    }
    else {
        // 追加済みの部品要素（セル）を用いる
        delete component;
    }


    // 親要素を追加
    found_cell->insertParentCell(this);
    // 部品要素（セル）を追加
    this->components.push_back(found_cell);
    return this->components.size();
}

/**
 * 要素（セル）の頂点（ノード）の接続情報を収集する.
 * @param scanner        収集クラス
 * @return
 */
UdmSize_t UdmCell::scannerCells(UdmScannerCells* scanner)
{
    return scanner->scannerElements(this);
}

/**
 * 要素（セル）の隣接要素の接続情報を収集する.
 * @param scanner        収集クラス
 * @return
 */
UdmSize_t UdmCell::scannerGraph(UdmScannerCells* scanner) const
{
    return scanner->scannerGraph(this);
}


/**
 * シリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmCell::serialize(UdmSerializeArchive& archive) const
{
    UdmSize_t n, num_nodes;

    // UdmEntity
    UdmEntity::serialize(archive);

#if 0
    {
        char buf[32] = {0x00};
        sprintf(buf, "UdmCell:%d[%d]", this->getId(), this->getMyRankno());
        archive.write(buf, 32);
    }
#endif

    // セクション（要素構成）情報 : IDと要素形状タイプのみ
    // セクション（要素構成）情報はすべてのプロセスにて作成済みとする.
    if (this->parent_elements != NULL) {
        archive << this->parent_elements->getId();
        archive << (int)this->parent_elements->getElementType();
    }
    else {
        // 仮想セルの場合、セクション（要素構成）情報はない
        archive << (UdmSize_t)0;
        archive << (int)Udm_ElementTypeUnknown;
    }

#if 0
    {
        char buf[32] = {0x00};
        sprintf(buf, "UdmCell:%d[%d]-num_nodes=%d",
                    this->getId(),
                    this->getMyRankno(),
                    this->getNumNodes() );
        archive.write(buf, 32);
    }
#endif

    // 構成ノード : std::vector<UdmNode*>  nodes
    num_nodes = this->getNumNodes();
    archive << num_nodes;
    for (n=1; n<=num_nodes; n++) {
        const UdmNode*  node = this->getNode(n);
        archive << (*node);
    }

    return archive;
}

/**
 * デシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmCell::deserialize(UdmSerializeArchive& archive)
{
    UdmSize_t n, num_nodes;
    UdmSize_t elements_id;
    UdmElementType_t elements_type;

    // ストリームステータスチェック
    if (archive.rdstate() != std::ios_base::goodbit) {
        UDM_ERROR_HANDLER(UDM_ERROR_DESERIALIZE, "rdstate is not goodbit.");
        return archive;
    }

    // UdmEntity
    UdmEntity::deserialize(archive);

#if 0
    {
        char buf[32] = {0x00};
        archive.read(buf, 32);
        printf("%s\n", buf);
    }
#endif

    // UdmElements
    archive >> elements_id;
    archive.read(elements_type, sizeof(UdmElementType_t));
    if (elements_id > 0) {
        UdmElements *elements = new UdmElements();
        elements->setId(elements_id);
        elements->setElementType(elements_type);
        this->parent_elements = elements;
    }
    else {
        this->parent_elements = NULL;
    }

#if 0
    {
        char buf[32] = {0x00};
        archive.read(buf, 32);
        printf("%s\n", buf);
    }
#endif

    // 構成ノード : std::vector<UdmNode*>  nodes
    archive >> num_nodes;
    for (n=1; n<=num_nodes; n++) {
        UdmNode*  node = new UdmNode();
        archive >> (*node);
        // ノードの挿入
        this->insertNode(node);

        // ストリームステータスチェック
        if (archive.rdstate() != std::ios_base::goodbit) {
            UDM_ERROR_HANDLER(UDM_ERROR_DESERIALIZE, "rdstate is not goodbit[node_id=%d].", n);
            return archive;
        }
    }

    return archive;
}

/**
 * デシリアライズ(UdmCell::deserialize)にて生成されたオブジェクトの開放を行う.
 * UdmElements(parent_elements)を開放する.
 * 要素（セル）を開放する.
 * 構成ノードを削除(delete)する。
 * 通常、構成ノード(UdmNode)はグリッド座標クラス(UdmGridCoordinates)にて管理（削除）を行う。
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmCell::freeDeserialize(UdmCell *cell)
{
    // UdmElements
    if (cell->parent_elements != NULL) {
        delete cell->parent_elements;
        cell->parent_elements = NULL;
    }

    int n = 0;
    // デシリアライズにて生成された要素（セル）を削除する.
    std::vector<UdmNode*>::iterator itr = cell->nodes.begin();
    for (itr=cell->nodes.begin(); itr!=cell->nodes.end(); itr++) {
        UdmNode* node = (*itr);
        if (node != NULL) {
            UdmNode::freeDeserialize(node);
        }
    }
    cell->nodes.clear();

    delete cell;
    cell = NULL;

    return UDM_OK;
}

/**
 * この要素（セル）を親に持つ節点（ノード）、要素（セル）からこの要素（セル）を削除する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmCell::removeReferencedCells()
{
    UdmSize_t num_parents;
    int i, size;

    // セクション（要素構成）情報のUdmElements::cell_listから削除する.
    // std::vectorから削除するのみ.
    if (this->parent_elements != NULL) {
        this->parent_elements->removeCell(this);
    }

    // 部品要素（セル）から親要素（セル）を削除する.
    size = this->components.size();
    if (size > 0) {
        for (i=size-1; i>=0; i--) {
            UdmComponent* component = this->components[i];
            // 親要素（セル）を削除する.
            num_parents = component->removeReferencedCell(this);

            // 部品要素（セル）から要素（セル）を削除(free)する.
            // 親要素（セル）が存在しない独立要素（セル）は、要素（セル）自身を削除する.
            this->removeComponentCell(component);
        }
    }

    // 構成節点（ノード）から親要素（セル）を削除する.
    std::vector<UdmNode*>::iterator  itr_node;
    for (itr_node=this->nodes.begin(); itr_node!=this->nodes.end(); itr_node++) {
        UdmNode* node = (*itr_node);
        // 親要素（セル）を削除する.
        num_parents = node->removeReferencedCell(this);
    }

    return UDM_OK;
}

/**
 * 部品要素（セル）を削除する.
 * 削除要素（セル）が親要素を持たない場合は、要素（セル）自身を削除する.
 * @param component            削除美品要素（セル）
 * @return            部品要素（セル）数
 */
UdmSize_t UdmCell::removeComponentCell(const UdmComponent* component)
{
    if (component == NULL) return this->components.size();

    std::vector<UdmComponent*>::iterator itr;
    for (itr=this->components.begin(); itr!=this->components.end(); itr++) {
        if (*itr == component) {
            this->components.erase(itr);
            break;
        }
    }

    // 削除要素（セル）の親要素（セル）が存在しければ、削除要素（セル）自身を削除する.
    if (component->getNumParentCells() <= 0) {
        if (this->getZone() != NULL && this->getZone()->getSections() != NULL) {
            this->getZone()->getSections()->removeComponentCell(component);
        }
    }

    return this->components.size();
}

/**
 * 部品要素をクリアする.
 * @return
 */
UdmError_t UdmCell::clearComponentCells()
{
    if (this->components.size() <= 0) return UDM_OK;

    std::vector<UdmComponent*>::reverse_iterator ritr;
    for (ritr = this->components.rbegin(); ritr != this->components.rend(); ++ritr) {
        UdmComponent* cell = (*ritr);
        this->removeComponentCell(cell);
    }

    return UDM_OK;
}


/**
 * 要素（セル）をコピーする.
 * @param src        コピー元要素（セル）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmCell::cloneCell(const UdmCell& src)
{
    // 物理量データのコピー
    UdmEntity::cloneEntity(src);

    // 管理CGNS要素ID.
    this->elem_id = src.elem_id;

    // 構成ノードはコピーしない。
    // 親セクション（要素構成）情報クラスはコピーしない
    // 部品要素（セル）はコピーしない

    return UDM_OK;
}

/**
 * エクスポート先のランク番号を設定する.
 * 構成節点（ノード）に対してエクスポート先ランク番号を設定する.
 * エクスポート先ローカルIDは、未設定(=0)とする.
 * @param rankno        エクスポート先ランク番号
 */
void UdmCell::setExportRankno(int rankno)
{
    // 要素（セル）に対してエクスポート先ランク番号を設定する.
    this->addMpiRankInfo(rankno, 0);

    std::vector<UdmNode*>::const_iterator itr;
    for (itr=this->nodes.begin(); itr!=this->nodes.end(); itr++) {
        (*itr)->addMpiRankInfo(rankno, 0);
    }
}

/**
 * 親セクション管理クラスを取得する.
 * @return        親セクション管理クラス
 */
UdmSections* UdmCell::getParentSections() const
{
    if (this->parent_sections != NULL) {
        return this->parent_sections;
    }
    if (this->parent_elements != NULL) {
        return this->parent_elements->getParentSections();
    }
    return NULL;
}

/**
 * 親セクション管理クラスを設定する.
 * @param sections        親セクション管理クラス
 */
void UdmCell::setParentSections(UdmSections* sections)
{
    this->parent_sections = sections;
}



/**
 * 要素（セル）の隣接要素（セル）数を取得する.
 * @return        隣接要素（セル）数
 */
UdmSize_t UdmCell::getNumNeighborCells() const
{
    UdmScannerCells scanner;
    UdmSize_t num_cells = scanner.scannerNeighborCells(this);
    return num_cells;
}

/**
 * 要素（セル）の隣接要素（セル）を取得する.
 * @param neighbor_id        隣接ID : 1～getNumNeighborCells()
 * @return        隣接要素（セル）
 */
UdmCell* UdmCell::getNeighborCell(int neighbor_id) const
{
    UdmScannerCells scanner;
    UdmSize_t num_cells = scanner.scannerNeighborCells(this);
    if (num_cells <= 0) return 0;
    const std::vector<std::vector<UdmEntity*> > scan_array = scanner.getScanArrays();
    if (scan_array.size() <= 0) return 0;
    std::vector<UdmEntity*> cell_array = scan_array[0];

    if (neighbor_id <= 0) return NULL;
    if (neighbor_id > cell_array.size()) return NULL;
    return static_cast<UdmCell*>(cell_array[neighbor_id-1]);
}

/**
 * メモリサイズを取得する.
 * @return        メモリサイズ
 */
size_t UdmCell::getMemSize() const
{
    UdmSize_t size = sizeof(*this);
#ifdef _DEBUG
    printf("UdmEntity size=%ld\n", UdmEntity::getMemSize());
    printf("this size=%ld\n", sizeof(*this));
    printf("elem_id size=%ld [offset=%ld]\n", sizeof(this->elem_id), offsetof(UdmCell, elem_id));
    printf("nodes size=%ld [count=%d] [offset=%ld]\n", sizeof(this->nodes), this->nodes.size(), offsetof(UdmCell, nodes));
    printf("parent_elements pointer size=%ld [offset=%ld]\n", sizeof(this->parent_elements), offsetof(UdmCell, parent_elements));
    printf("parent_sections pointer size=%ld [offset=%ld]\n", sizeof(this->parent_sections), offsetof(UdmCell, parent_sections));
    printf("components size=%ld [count=%d] [offset=%ld]\n", sizeof(this->components), this->components.size(), offsetof(UdmCell, components));

    size += this->getNumParentCells()*sizeof(UdmCell*);
    size += this->nodes.size()*sizeof(UdmNode*) + this->components.size()*sizeof(UdmCell*);
#endif

    return size;
}

/**
 * 親要素（セル）数を取得する.
 * 要素（セル）クラスには親要素（セル）はいない。
 * @return        親要素（セル）数：0固定
 */
UdmSize_t UdmCell::getNumParentCells() const
{
    return 0;
}

/**
 * 親要素（セル）を取得する.
 * 要素（セル）クラスには親要素（セル）はいない。
 * @param cell_id        親要素（セル）ID（１～）
 * @return        親要素（セル）:NULL固定
 */
UdmCell* UdmCell::getParentCell(UdmSize_t cell_id) const
{
    return NULL;
}

/**
 * CGNSノードのIDを取得する.
 * @return        CGNSノードID
 */
UdmSize_t UdmCell::getId() const
{
    return UdmEntity::getId();
}


/**
 * 要素形状タイプを取得する.
 * @return        要素形状タイプ
 */
UdmElementType_t UdmCell::getElementType() const
{
    return UdmEntity::getElementType();
}

/**
 * 構成節点（ノード）から削除節点（ノード）を削除する.
 * 構成節点（ノード）リストから削除節点（ノード）と一致するiteratorをeraseするのみ。deleteは行わない。
 * @param node        削除節点（ノード）
 */
void UdmCell::removeNode(const UdmNode* node)
{
    std::vector<UdmNode*>::iterator itr;
    for (itr=this->nodes.begin(); itr!=this->nodes.end(); itr++) {
        if ((*itr) == node) {
            this->nodes.erase(itr);
            break;
        }
    }

    return;
}


/**
 * 要素（セル）の破棄処理を行う.
 * 部品要素リストのクリア、構成節点（ノード）のクリアを行う.
 */
void UdmCell::finalize()
{
    this->nodes.clear();
    this->components.clear();
    this->parent_elements = NULL;
    this->parent_sections = NULL;

    return;
}
} /* namespace udm */
