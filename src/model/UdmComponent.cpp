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
 * @file UdmComponent.cpp
 * 要素（セル）の部品要素クラスのソースファイル
 */

#include "model/UdmComponent.h"
#include "model/UdmGeneral.h"
#include "model/UdmCell.h"
#include "model/UdmNode.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmComponent::UdmComponent() : element_type(Udm_ElementTypeUnknown)
{
    this->initialize();
}

/**
 * コンストラクタ
 * @param   type        要素形状タイプ
 */
UdmComponent::UdmComponent(UdmElementType_t type) : element_type(type)
{
    this->initialize();
}

/**
 * デストラクタ
 */
UdmComponent::~UdmComponent()
{
    // この要素を参照しているノード、要素からこの要素を削除する.
    this->removeReferencedCells();
}


/**
 * IDを取得する.
 * @return        ID
 */
UdmSize_t UdmComponent::getId() const
{
    return this->id;
}

/**
 * IDを設定します.
 * @param id    ID
 */
void UdmComponent::setId(UdmSize_t id)
{
    this->id = id;
}

/**
 * 要素クラスタイプを取得する.
 * @return        要素クラスタイプ=Udm_ComponentClass:部品要素クラス
 */
UdmCellClass_t UdmComponent::getCellClass() const
{
    return Udm_ComponentClass;
}

/**
 * 初期化を行う.
 */
void UdmComponent::initialize()
{
    this->nodes.clear();
    this->parent_cells[0] = NULL;
    this->parent_cells[1] = NULL;
    this->remove_entity = false;
    this->id = 0;
}


/**
 * 要素形状タイプを取得する.
 * @return        要素形状タイプ
 */
UdmElementType_t UdmComponent::getElementType() const
{
    return this->element_type;
}

/**
 * 要素形状タイプを設定する.
 * @param element_type        要素形状タイプ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmComponent::setElementType(UdmElementType_t element_type)
{
    this->element_type = element_type;
    return UDM_OK;
}


/**
 * 構成ノード（節点）数を取得する.
 * @return        構成ノード（節点）数
 */
UdmSize_t UdmComponent::getNumNodes() const
{
    return this->nodes.size();
}

/**
 * 構成ノード（節点）を取得する.
 * @param node_id        構成ノードID（１～）
 * @return        構成ノード（節点）
 */
UdmNode* UdmComponent::getNode(UdmSize_t node_id)  const
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
UdmError_t UdmComponent::setNodes(UdmNode* nodes[], UdmSize_t size)
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
UdmError_t UdmComponent::setNodes(const std::vector<UdmNode*> &nodes)
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
const std::vector<UdmNode*>& UdmComponent::getNodes() const
{
    return this->nodes;
}

/**
 * 構成ノード（節点）を挿入する.
 * @param node        構成ノード（節点）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmComponent::insertNode(UdmNode* node)
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
UdmError_t UdmComponent::clearNodes()
{
    this->nodes.clear();
    return UDM_OK;
}

/**
 * 親要素（セル）数を取得する.
 * @return        親要素（セル）数
 */
UdmSize_t UdmComponent::getNumParentCells() const
{
    int size = 0;
    int n;
    for (n=0; n<2; n++) {
        if (this->parent_cells[n] != NULL) {
            size++;
        }
    }
    return size;
}

/**
 * 親要素（セル）を取得する.
 * @param cell_id        親要素（セル）ID（１～）
 * @return        親要素（セル）
 */
UdmCell* UdmComponent::getParentCell(UdmSize_t cell_id) const
{
    if (cell_id <= 0) return NULL;
    if (cell_id > this->getNumParentCells()) return NULL;
    return this->parent_cells[cell_id-1];
}


/**
 * 親要素（セル）リストを設定する.
 * @param parent_cells        親要素（セル）リスト
 * @param size                親要素（セル）サイズ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmComponent::setParentCells(UdmCell* parent_cells[], UdmSize_t size)
{
    this->clearParentCells();
    for (UdmSize_t i=0; i<size; i++) {
        if (parent_cells[i] == NULL) return UDM_ERROR;
        this->insertParentCell(parent_cells[i]);
    }
    return UDM_OK;
}

/**
 * 親要素（セル）を追加する.
 * @param parent_cells        親要素（セル）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmComponent::insertParentCell(UdmCell* parent_cell)
{
    if (parent_cell == NULL) return UDM_ERROR;

    int n;
    for (n=0; n<2; n++) {
        if (this->parent_cells[n] == parent_cell) {
            return UDM_OK;
        }
        else if (this->parent_cells[n] == NULL) {
            this->parent_cells[n] = parent_cell;
            return UDM_OK;
        }
    }

    return UDM_ERROR;
}


/**
 * 親要素（セル）リストをクリアする.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmComponent::clearParentCells()
{
    this->parent_cells[0] = NULL;
    this->parent_cells[1] = NULL;
    return UDM_OK;
}

/**
 * 親構成要素から削除親要素（セル）を削除する.
 * @param parent_cell        削除親要素（セル）
 * @return        親構成要素数
 */
UdmSize_t UdmComponent::removeReferencedCell(const UdmCell* parent_cell)
{
    if (parent_cell == NULL) return this->getNumParentCells();

    int count = 0;
    int n;
    for (n=0; n<2; n++) {
        if (this->parent_cells[n] == parent_cell) {
            this->parent_cells[n] = NULL;
        }
        else if (this->parent_cells[n] != NULL) {
            count++;
        }
    }

    // 親要素が存在しないので、構成節点（ノード）からこの部品要素を削除する.
    if (count == 0) {
        // 構成節点（ノード）から親要素（セル）を削除する.
        std::vector<UdmNode*>::iterator  itr_node;
        for (itr_node=this->nodes.begin(); itr_node!=this->nodes.end(); itr_node++) {
            UdmNode* node = (*itr_node);
            // 親要素（セル）を削除する.
            node->removeReferencedCell(this);
        }
    }

    return this->getNumParentCells();
}

/**
 * 親構成要素からすべての削除親要素（セル）を削除する.
 * @return        親構成要素数
 */
UdmSize_t UdmComponent::removeReferencedCells()
{

    int count = 0;
    int n;
    for (n=0; n<2; n++) {
        if (this->parent_cells[n] != NULL) {
            this->parent_cells[n]->removeComponentCell(this);
            this->parent_cells[n] = NULL;
        }
    }

    // 親要素が存在しないので、構成節点（ノード）からこの部品要素を削除する.
    // 構成節点（ノード）から親要素（セル）を削除する.
    std::vector<UdmNode*>::iterator  itr_node;
    for (itr_node=this->nodes.begin(); itr_node!=this->nodes.end(); itr_node++) {
        UdmNode* node = (*itr_node);
        // 親要素（セル）を削除する.
        node->removeReferencedCell(this);
    }

    return 0;
}

/**
 * 節点（ノード）、要素（セル）の削除フラグを取得する.
 * @return        true=節点（ノード）、要素（セル）オブジェクトを削除
 */
bool UdmComponent::isRemoveEntity() const
{
    return this->remove_entity;
}

/**
 * 節点（ノード）、要素（セル）の削除フラグを設定する.
 * @param remove        true=節点（ノード）、要素（セル）オブジェクトを削除
 */
void UdmComponent::setRemoveEntity(bool remove)
{
    this->remove_entity = remove;
}


/**
 * クラス情報を文字列出力する:デバッグ用.
 * @param buf        出力文字列
 */
void UdmComponent::toString(std::string& buf) const
{
#ifdef _DEBUG_TRACE
    std::stringstream stream;
    char fmt_buf[32];
    int count = 0;

    std::string type_buf;
    UdmGeneral::toStringElementType(this->getElementType(), type_buf);
    sprintf(fmt_buf, "%-8s", type_buf.c_str());
    stream <<  fmt_buf << " ";

    std::vector<UdmNode*>::const_iterator itr;
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

    buf += stream.str();
#endif

    return;
}

/**
 * 構成節点（ノード）から削除節点（ノード）を削除する.
 * 構成節点（ノード）リストから削除節点（ノード）と一致するiteratorをeraseするのみ。deleteは行わない。
 * @param node        削除節点（ノード）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
void UdmComponent::removeNode(const UdmNode* node)
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
 * 部品要素（セル）の破棄処理を行う.
 * 部品要素リストのクリア、構成節点（ノード）のクリアを行う.
 */
void UdmComponent::finalize()
{
    int n = 0;
    for (n=0; n<2; n++) {
        this->parent_cells[n] = NULL;
    }
    this->nodes.clear();
    return;
}
} /* namespace udm */
