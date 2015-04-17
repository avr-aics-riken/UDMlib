/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmElements.cpp
 * 要素管理クラスのソースファイル
 */

#include "model/UdmSections.h"
#include "model/UdmModel.h"
#include "model/UdmZone.h"
#include "model/UdmElements.h"
#include "model/UdmNode.h"
#include "model/UdmCell.h"
#include "model/UdmComponent.h"
#include "model/UdmFlowSolutions.h"
#include "model/UdmRankConnectivity.h"
#include "utils/UdmScannerCells.h"
#include "utils/UdmSearchTable.h"


namespace udm
{

/**
 * コンストラクタ
 */
UdmSections::UdmSections()
{
    this->initialize();
}


/**
 * コンストラクタ
 * @param zone        ゾーン
 */
UdmSections::UdmSections(UdmZone* zone)
{
    this->initialize();
    this->parent_zone = zone;
}


/**
 * デストラクタ
 */
UdmSections::~UdmSections()
{
    this->finalize();
}


/**
 * 初期化を行う.
 */
void UdmSections::initialize()
{
    this->section_list.clear();
    this->entity_cells.clear();
    this->virtual_cells.clear();
    this->component_cells.clear();
    this->parent_zone = NULL;
}

/**
 * セクション（要素構成）を作成する.
 * セクション（要素構成）を作成して、セクション（要素構成）リストに追加する.
 * 既定の命名規約に従いセクション（要素構成）名称を作成する.
 * @param element_type            要素タイプ
 * @return        生成セクション（要素構成）
 */
UdmElements * UdmSections::createSection(UdmElementType_t element_type)
{
    char elem_name[33] = {0x00};
    std::string element_typename;
    if (this->toStringElementType(element_type, element_typename) != UDM_OK) {
        UDM_ERROR_HANDLER(UDM_ERROR_INVALID_ELEMENTTYPE, "element_type=%d", element_type);
        return NULL;
    }

    sprintf(elem_name, UDM_CGNS_FORMAT_ELEMENTS, element_typename.c_str());
    if (this->existsSection(elem_name)) return NULL;

    /// セクション（要素構成）名称、セクション（要素構成）IDでセクション（要素構成）の生成を行う.
    UdmElements* section = new UdmElements(elem_name, this->section_list.size());
    section->setElementType(element_type);
    if (this->setSection(section) == 0) {
        return NULL;
    }
    return section;
}

/**
 * セクション（要素構成）を作成する.
 * セクション（要素構成）を作成して、セクション（要素構成）リストに追加する.
 * 既存セクション（要素構成）名称は、作成できない。（NULLを返す）
 * @param section_name        セクション（要素構成）名称
 * @param element_type            要素タイプ
 * @return        生成セクション（要素構成）
 */
UdmElements * UdmSections::createSection(const std::string &section_name, UdmElementType_t element_type)
{
    if (this->existsSection(section_name)) return NULL;
    /// セクション（要素構成）名称、セクション（要素構成）IDでセクション（要素構成）の生成を行う.
    UdmElements* section = new UdmElements(section_name, this->section_list.size());
    section->setElementType(element_type);
    if (this->setSection(section) == 0) {
        return NULL;
    }
    return section;
}

/**
 * セクション（要素構成）数を取得する.
 * @return        セクション（要素構成）数
 */
int UdmSections::getNumSections() const
{
    return this->section_list.size();
}

/**
 * セクション（要素構成）名称が存在するかチェックする.
 * @param section_name        セクション（要素構成）名称
 * @return        true=セクション（要素構成）が存在する.
 */
bool UdmSections::existsSection(const std::string& section_name) const
{
    return (this->getSection(section_name) != NULL);
}

/**
 * セクションIDからセクション（要素構成）を取得する
 * @param section_id        セクションID（1～）
 * @return        セクション（要素構成）
 */
UdmElements* UdmSections::getSection(int section_id)
{
    if (section_id <= 0) return NULL;
    if (section_id > this->section_list.size()) return NULL;
    return this->section_list[section_id-1];
}

/**
 * セクションIDからセクション（要素構成）を取得する:const
 * @param section_id        セクションID（1～）
 * @return        セクション（要素構成）
 */
const UdmElements* UdmSections::getSection(int section_id) const
{
    if (section_id <= 0) return NULL;
    if (section_id > this->section_list.size()) return NULL;
    return this->section_list[section_id-1];
}

/**
 * セクション名称からセクション（要素構成）を取得する.
 * @param section_name        セクション名称
 * @return        セクション（要素構成）
 */
UdmElements* UdmSections::getSection(const std::string& section_name)
{
    std::vector<UdmElements*>::iterator itr;
    for (itr = this->section_list.begin(); itr != this->section_list.end(); itr++) {
        if ((*itr)->equalsName(section_name)) {
            return (*itr);
        }
    }
    return NULL;
}

/**
 * セクション名称からセクション（要素構成）を取得する:const.
 * @param section_name        セクション名称
 * @return        セクション（要素構成）
 */
const UdmElements* UdmSections::getSection(const std::string& section_name) const
{
    std::vector<UdmElements*>::const_iterator itr;
    for (itr = this->section_list.begin(); itr != this->section_list.end(); itr++) {
        if ((*itr)->equalsName(section_name)) {
            return (*itr);
        }
    }
    return NULL;
}

/**
 * セクション（要素構成）を追加する.
 * @param section        セクション（要素構成）
 * @return        挿入セクションID
 */
int UdmSections::setSection(UdmElements* section)
{
    int section_id = 0;
    if (section == NULL) return 0;

    // セクション（要素構成）名称と同名のセクション（要素構成）を検索する.
    UdmElements *found_section = this->getSection(section->getName());
    if (found_section == NULL) {
        this->section_list.push_back(section);
        section_id = this->section_list.size();
        section->setId(section_id);
    }
    else {
        section_id = found_section->getId();
        if (found_section != section) {
            delete found_section;
        }
        this->section_list[section_id-1] = section;
        section->setId(section_id);
    }
    section->setParentSections(this);

    return section_id;
}

/**
 * セクションIDのセクション（要素構成）を削除する.
 * @param section_id        セクションID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::removeSection(int section_id)
{
    if (section_id <= 0) return UDM_ERROR;
    if (section_id > this->section_list.size()) return UDM_ERROR;

    if (this->section_list[section_id-1] != NULL) delete this->section_list[section_id-1];
    std::vector<UdmElements*>::iterator itr = this->section_list.begin();
    std::advance( itr, section_id-1 );            // イテレータの移動
    this->section_list.erase(itr);

    // セクションIDの再振分
    this->rebuildSections();

    return UDM_OK;
}

/**
 * セクション名称のセクション（要素構成）を削除する.
 * @param section_name        セクション名称
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::removeSection(const std::string& section_name)
{
    std::vector<UdmElements*>::iterator itr;
    for (itr = this->section_list.begin(); itr != this->section_list.end(); itr++) {
        if ((*itr)->equalsName(section_name)) {
            if ((*itr) != NULL) delete (*itr);
            this->section_list.erase(itr);
            // セクションIDの再振分
            this->rebuildSections();
            return UDM_OK;
        }
    }
    return UDM_ERROR;
}

/**
 * すべてのセクション（要素構成）を削除する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::clearSections()
{
/************
    std::vector<UdmElements*>::reverse_iterator ritr = this->section_list.rbegin();
    while (ritr != this->section_list.rend()) {
        UdmElements* section = *(--(ritr.base()));
        if (section != NULL) {
            delete section;
        }
        this->section_list.erase((++ritr).base());
        ritr = this->section_list.rbegin();
    }
********************/

    std::vector<UdmElements*>::iterator itr = this->section_list.begin();
    for (itr=this->section_list.begin(); itr!=this->section_list.end(); itr++) {
        UdmElements* section = (*itr);
        if (section != NULL) {
            delete section;
        }
    }

    this->section_list.clear();
    return UDM_OK;
}

/**
 * セクション（要素構成）IDを再設定を行う.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::rebuildSections()
{
    UdmSize_t section_id = 0;
    std::vector<UdmElements*>::iterator itr;
    for (itr = this->section_list.begin(); itr != this->section_list.end(); itr++) {
        if ((*itr) != NULL) (*itr)->setId(++section_id);
    }
    return UDM_OK;
}

/**
 * 実体要素（セル）数を取得する.
 * @return        実体要素（セル）数
 */
UdmSize_t UdmSections::getNumEntityCells() const
{
    return this->entity_cells.size();
}


/**
 * 要素IDから実体要素（セル）を取得する:const.
 * 要素IDは実体要素（セル）リストの格納インデックスである。（キーではない）
 * @param cell_id        要素ID（１～getNumEntityCells）
 * @return        実体要素（セル）
 */
UdmCell* UdmSections::getEntityCell(UdmSize_t cell_id) const
{
    if (cell_id <= 0) return NULL;
    if (cell_id > this->entity_cells.size()) return NULL;

    return this->entity_cells[cell_id-1];
}

/**
 * 管理IDから実体要素（セル）を取得する.
 * 管理IDは実体要素（セル）リストのキーである。（要素IDは実体要素（セル）リストの格納インデックスである）
 * @param elem_id        管理ID
 * @return        実体要素（セル）
 */
UdmCell* UdmSections::getEntityCellByElemId(UdmSize_t elem_id)
{
    std::vector<UdmCell*>::const_iterator itr = this->searchElemId(this->entity_cells, elem_id);
    if (itr != this->entity_cells.end()) {
        return (*itr);
    }
    return NULL;
}

/**
 * 管理IDから実体要素（セル）を取得する:const.
 * 管理IDは実体要素（セル）リストのキーである。（要素IDは実体要素（セル）リストの格納インデックスである）
 * @param elem_id        管理ID
 * @return        実体要素（セル）
 */
const UdmCell* UdmSections::getEntityCellByElemId(UdmSize_t elem_id) const
{
    std::vector<UdmCell*>::const_iterator itr = this->searchElemId(this->entity_cells, elem_id);
    if (itr != this->entity_cells.end()) {
        return (*itr);
    }
    return NULL;
}

/**
 * 実体要素（セル）を設定する.
 * 要素（セル）IDが既に存在している場合は上書きする.
 * 連続した要素（セル）IDにて挿入を行えば、要素（セル）ID=挿入要素（セル）IDとなる。
 * @param section_name        セクション名
 * @param elem_id        要素管理ID：ゾーン内で一意の管理ID
 * @param cell        実体要素（セル）
 * @return        挿入要素（セル）ID
 */
UdmSize_t UdmSections::setEntityCellByElemId(const std::string& section_name, UdmSize_t elem_id, UdmCell* cell)
{
    if (cell == NULL) return 0;
    if (elem_id <= 0) return 0;
    UdmElements* section = this->getSection(section_name);
    if (section == NULL)  return 0;
    UdmSize_t new_id = section->setCellByElemId(elem_id, cell);
    return new_id;
}

/**
 * 実体要素（セル）を設定する.
 * 要素（セル）IDが既に存在している場合は上書きする.
 * 連続した要素（セル）IDにて挿入を行えば、要素（セル）ID=挿入要素（セル）IDとなる。
 * @param section_id        セクションID
 * @param cell_id        要素（セル）ID ( 管理ID )
 * @param cell        実体要素（セル）
 * @return        挿入要素（セル）ID
 */
UdmSize_t UdmSections::setEntityCellByElemId(UdmSize_t section_id, UdmSize_t elem_id, UdmCell* cell)
{
    if (cell == NULL) return 0;
    if (elem_id <= 0) return 0;
    UdmElements* section = this->getSection(section_id);
    if (section == NULL)  return 0;
    UdmSize_t new_id = section->setCellByElemId(elem_id, cell);
    return new_id;
}

/**
 * 実体要素（セル）を設定する.
 * 要素（セル）IDが既に存在している場合は上書きする.
 * 連続した要素（セル）IDにて挿入を行えば、要素（セル）ID=挿入要素（セル）IDとなる。
 * @param elem_id        要素（セル）ID ( 管理ID )
 * @param cell        実体要素（セル）
 * @return        挿入要素（セル）ID
 */
UdmSize_t UdmSections::setEntityCell(UdmSize_t elem_id, UdmCell* cell)
{
    UdmSize_t new_id = 0;
    if (elem_id <= 0) return 0;
    if (cell == NULL) return 0;
    std::vector<UdmCell*>::const_iterator itr =  this->searchElemId(this->entity_cells, elem_id);
    if (itr != this->entity_cells.end()) {
        // 既存の実体要素（セル）を削除する.
        if ((*itr) != NULL) delete (*itr);
        std::vector<UdmCell*>::iterator rm_itr;
        rm_itr = this->entity_cells.begin() + (itr - this->entity_cells.begin());
        this->entity_cells.erase(rm_itr);
    }

    // 管理要素(セル)IDを設定する.
    cell->setElemId(elem_id);

    // 実体要素リストに挿入する
    new_id = this->insertEntityCell(cell);
    return new_id;
}


/**
 * 実体要素（セル）を削除する.
 * @param cell_id        実体要素（セル）ID（１～getNumEntityCells）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::removeEntityCell(UdmSize_t cell_id)
{
    if (cell_id <= 0) return UDM_ERROR;
    if (cell_id > this->entity_cells.size()) return UDM_ERROR;

    std::vector<UdmCell*>::const_iterator itr;
    std::vector<UdmCell*>::iterator rm_itr;

    // 実体要素リストから検索
    itr = this->searchEntityId(this->entity_cells, cell_id);
    if (itr != this->entity_cells.end()) {
        delete (*itr);
        rm_itr = this->entity_cells.begin() + (itr - this->entity_cells.begin());
        this->entity_cells.erase(rm_itr);
        return UDM_OK;
    }

    return UDM_OK;
}

/**
 * すべての実体要素（セル）を削除する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::clearEntityCells()
{
    std::vector<UdmCell*>::const_iterator itr;
    for (itr=this->entity_cells.begin(); itr!=this->entity_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        if (cell != NULL) {
            delete cell;
        }
    }
    this->entity_cells.clear();
    return UDM_OK;
}

/**
 * 管理要素IDから実体要素（セル）IDを取得する.
 * 管理要素IDは実体要素（セル）リストのキー、実体要素（セル）IDは格納インデックス+1である.
 * @param elem_id        管理要素ID
 * @return            実体要素（セル）ID
 */
UdmSize_t UdmSections::getCellId(UdmSize_t elem_id)
{
    UdmSize_t cell_id = 0;
    std::vector<UdmCell*>::const_iterator itr = this->searchElemId(this->entity_cells, elem_id);
    if (itr != this->entity_cells.end()) {
        cell_id = (*itr)->getId();
    }
    return cell_id;
}


/**
 * 仮想要素（セル）数を取得する.
 * @return        仮想要素（セル）数
 */
UdmSize_t UdmSections::getNumVirtualCells() const
{
    return this->virtual_cells.size();
}

/**
 * 仮想要素（セル）を取得する.
 * 仮想要素（セル）IDは、仮想要素（セル）リスト内の格納インデックス＋１である.
 * @param virtual_cell_id        仮想要素（セル）ID（１～）
 * @return        仮想要素（セル）
 */
UdmCell* UdmSections::getVirtualCell(UdmSize_t virtual_cell_id) const
{
    if (virtual_cell_id <= 0) return NULL;
    if (virtual_cell_id > this->virtual_cells.size()) return NULL;
    return this->virtual_cells[virtual_cell_id-1];
}


/**
 * 仮想要素（セル）を挿入する.
 * 仮想要素（セル）はID, 自ランク番号は、実体ランクのID,ランク番号とする.
 * @param virtual_cell        仮想要素（セル）
 * @return        仮想要素（セル）数
 */
UdmSize_t UdmSections::insertVirtualCell(UdmCell* virtual_cell)
{
    if (virtual_cell == NULL) return 0;

    std::vector<UdmCell*>::const_iterator itr;
    std::vector<UdmCell*>::iterator ins_itr;
    itr = this->searchUpperGlobalId(this->virtual_cells, virtual_cell);
    if (itr == this->virtual_cells.end()) {
        this->virtual_cells.push_back(virtual_cell);
    }
    else {
        ins_itr = this->virtual_cells.begin() + (itr - this->virtual_cells.begin());
        ins_itr = this->virtual_cells.insert(ins_itr, virtual_cell);
    }
    // 要素管理クラスを設定
    virtual_cell->setParentSections(this);
    virtual_cell->setRealityType(Udm_Virtual);

    return this->virtual_cells.size();
}

/**
 * 仮想要素（セル）を削除する.
 * @param virtual_cell_id        仮想要素（セル）ID（１～）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::removeVirtualCell(UdmSize_t virtual_cell_id)
{
    if (virtual_cell_id <= 0) return UDM_ERROR;
    if (virtual_cell_id > this->virtual_cells.size()) return UDM_ERROR;
    if (this->virtual_cells[virtual_cell_id-1] != NULL) delete this->virtual_cells[virtual_cell_id-1];
    std::vector<UdmCell*>::iterator itr = this->virtual_cells.begin();
    std::advance( itr, virtual_cell_id-1 );            // イテレータの移動
    this->virtual_cells.erase(itr);

    return UDM_OK;
}

/**
 * 仮想要素（セル）をすべて削除する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::clearVirtualCells()
{
    std::vector<UdmCell*>::const_iterator itr;
    for (itr=this->virtual_cells.begin(); itr!=this->virtual_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        if (cell != NULL) {
            delete cell;
        }
    }
    this->virtual_cells.clear();

    return UDM_OK;
}

/**
 * 親ゾーンを取得する.
 * @return        親ゾーン
 */
UdmZone* UdmSections::getParentZone() const
{
    return this->parent_zone;
}


/**
 * 親ゾーンを設定する.
 * @param zone        親ゾーン
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::setParentZone(UdmZone* zone)
{
    this->parent_zone = zone;
    return UDM_OK;
}

/**
 * CGNSファイルからElements（要素接続情報）の読込みを行う.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::readCgns(int index_file, int index_base, int index_zone)
{
    int num_sections = 0;
    int index_section;
    UdmError_t error = UDM_OK;

    // セクションの読込
    if (cg_nsections(index_file,index_base,index_zone,&num_sections) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "failure:cg_nsections");
    }
    for (index_section=1; index_section <= num_sections; index_section++) {
        UdmElements *section = new UdmElements(this);
        error = section->readCgns(index_file, index_base,index_zone, index_section);
        if (error == UDM_OK) {
            this->setSection(section);
        }
        else if (error == UDM_WARNING_CGNS_NOTSUPPORT_ELEMENTTYPE) {
            delete section;
        }
        else {
            delete section;
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "index_file=%d,index_base=%d,index_zone=%d,index_section=%d", index_file, index_base,index_zone, index_section);
        }
    }

    return UDM_OK;
}

/**
 * 部品要素（セル）を作成する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::createComponentCells()
{
    UdmError_t error = UDM_OK;
    std::vector<UdmCell*>::iterator itr;
    for (itr=this->entity_cells.begin(); itr!=this->entity_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        if ((error = cell->createComponentCells()) != UDM_OK) {
            return error;
        }
    }

    for (itr=this->virtual_cells.begin(); itr!=this->virtual_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        if ((error = cell->createComponentCells()) != UDM_OK) {
            return error;
        }
    }
    return UDM_OK;
}

/**
 * 部品要素（セル）数を取得する.
 * @return        部品要素（セル）数
 */
UdmSize_t UdmSections::getNumComponentCells() const
{
    return this->component_cells.size();
}

/**
 * 部品要素（セル）を取得する:const.
 * @param component_id        部品要素（セル）ID （１～）
 * @return        部品要素（セル）
 */
UdmComponent* UdmSections::getComponentCell(UdmSize_t component_id) const
{
    if (component_id <= 0) return NULL;
    if (component_id > this->component_cells.size()) return NULL;
    return this->component_cells[component_id];
}


/**
 * 部品要素（セル）を追加する.
 * IDを設定する.IDは最終ID+1とする.
 * @param component_cell        部品要素（セル）
 * @return        部品要素（セル）ID （１～）
 */
UdmSize_t UdmSections::insertComponentCell(UdmComponent* component_cell)
{

    if (component_cell == NULL) return 0;
    UdmSize_t size = this->component_cells.size();
    UdmSize_t cell_id = size;
    if (size > 0) {
        cell_id = this->component_cells[size-1]->getId();
        cell_id++;
    }
    else {
        cell_id = 1;
    }
    // ID
    component_cell->setId(cell_id);

    // 部品要素（セル）を追加する.
    this->component_cells.push_back(component_cell);

    return cell_id;
}

/**
 * 部品要素（セル）を削除する.
 * @param component_id        部品要素（セル）ID（１～）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::removeComponentCell(UdmSize_t component_id)
{
    if (component_id <= 0) return UDM_ERROR;

    // 部品要素リストから検索
    std::vector<UdmComponent*>::const_iterator itr;
    std::vector<UdmComponent*>::iterator rm_itr;
    itr = this->searchEntityId(this->component_cells, component_id);
    if (itr != this->component_cells.end()) {
        delete (*itr);
        rm_itr = this->component_cells.begin() + (itr - this->component_cells.begin());
        this->component_cells.erase(rm_itr);
        return UDM_OK;
    }
}

/**
 * 部品要素（セル）をすべて削除する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::clearComponentCells()
{
    // 部品要素の削除
    std::vector<UdmComponent*>::const_iterator itr;
    for (itr=this->component_cells.begin(); itr!=this->component_cells.end(); itr++) {
        UdmComponent* cell = (*itr);
        if (cell != NULL) {
            delete cell;
        }
    }
    this->component_cells.clear();
    return UDM_OK;
}

/**
 * 同一部品要素（セル）を検索する.
 * 同一頂点（ノード）構成（ノードID）であるか検索する.
 * @param dest_cell        検索要素（セル）
 * @return            同一部品要素（セル）：同一部品要素（セル）が存在しない場合はNULLを返す。
 */
UdmComponent* UdmSections::findComponentCell(const UdmComponent* dest_cell)
{
    if (dest_cell == NULL) return NULL;

    // 先頭の１つの節点（ノード）の取得
    UdmNode *node = dest_cell->getNode(1);

    // 同一部品要素（セル）が存在していれば、節点（ノード）の親要素（セル）として登録済みである。
    int num_parent_cells = node->getNumParentCells();
    int n;
    UdmICellComponent* src_cell = NULL;
    for (n=1; n<=num_parent_cells; n++) {
        src_cell = node->getParentCell(n);
        if (src_cell->getId() <= 0) continue;
        if (src_cell->getCellClass() != Udm_ComponentClass) continue;
        if (!src_cell->equalsNodeIds(dest_cell)) continue;
        return dynamic_cast<UdmComponent*>(src_cell);
    }

    return NULL;
}



/**
 * FlowSolutionクラスを取得する.
 * @return        FlowSolutionクラス
 */
UdmFlowSolutions* UdmSections::getFlowSolutions()
{
    if (this->parent_zone == NULL) return NULL;
    return this->parent_zone->getFlowSolutions();
}


/**
 * クラス情報を文字列出力する:デバッグ用.
 * @param buf        出力文字列
 */
void UdmSections::toString(std::string& buf) const
{
#ifdef _DEBUG_TRACE
    std::stringstream stream;

    std::vector<UdmElements*>::const_iterator itr;
    for (itr = this->section_list.begin(); itr != this->section_list.end(); itr++) {
        std::string section_buf;
        (*itr)->toString(section_buf);
        stream << section_buf << std::endl;
    }

    stream << "Virtual Cells" << std::endl;
    std::vector<UdmCell*>::const_iterator cell_itr;
    for (cell_itr=this->virtual_cells.begin(); cell_itr!=this->virtual_cells.end(); cell_itr++) {
        std::string cell_buf;
        (*cell_itr)->toString(cell_buf);
        stream << cell_buf << std::endl;
    }

    buf += stream.str();
#endif

    return;
}


/**
 * 実体要素（セル）数を取得する.
 * @return        実体要素（セル）数
 */
UdmSize_t UdmSections::getNumEntities() const
{
    return this->getNumEntityCells();
}

/**
 * 要素IDから実体要素（セル）を取得する.
 * @param cell_id        要素ID（１～）
 * @return        実体要素（セル）
 */
UdmEntity* UdmSections::getEntityById(UdmSize_t entity_id)
{
    return this->getEntityCell(entity_id);
}

/**
 * 要素IDから実体要素（セル）を取得する:const.
 * @param cell_id        要素ID（１～）
 * @return        実体要素（セル）
 */
const UdmEntity* UdmSections::getEntityById(UdmSize_t entity_id) const
{
    return this->getEntityCell(entity_id);
}


/**
 * CGNS:Elementsを出力する.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param timeslice_step       CGNS出力ステップ回数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::writeCgns(int index_file, int index_base, int index_zone, int timeslice_step)
{
    UdmSize_t cell_size = 1;
    std::vector<UdmElements*>::iterator itr;
    for (itr = this->section_list.begin(); itr != this->section_list.end(); itr++) {
        if ((*itr)->writeCgns(index_file, index_base, index_zone, cell_size) != UDM_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "writeCgns(index_file=%d, index_base=%d, index_zone=%d, timeslice_step=%d)", index_file, index_base, index_zone, timeslice_step);
        }
        cell_size += (*itr)->getNumCells();
    }

    return UDM_OK;
}

/**
 * CGNS:Elementsのリンクを出力する.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param link_output_path        リンク出力ファイル
 * @param linked_files            リンクファイルリスト
 * @param timeslice_step       CGNS出力ステップ回数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::writeCgnsLinkFile(
                        int index_file,
                        int index_base,
                        int index_zone,
                        const std::string &link_output_path,
                        const std::vector<std::string> &linked_files,
                        int timeslice_step)
{
    std::vector<UdmElements*>::iterator itr;
    for (itr = this->section_list.begin(); itr != this->section_list.end(); itr++) {
        if ((*itr)->writeCgnsLinkFile(index_file, index_base, index_zone, link_output_path, linked_files) != UDM_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "writeCgns(index_file=%d, index_base=%d, index_zone=%d, timeslice_step=%d)", index_file, index_base, index_zone, timeslice_step);
        }
    }

    return UDM_OK;
}

/**
 * CGNS出力前の初期化を行う.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::initializeWriteCgns()
{
    std::vector<UdmElements*>::iterator itr;
    for (itr = this->section_list.begin(); itr != this->section_list.end(); itr++) {
        if ((*itr)->initializeWriteCgns() != UDM_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "failure : UdmElements::initializeWriteCgns()");
        }
    }

    return UDM_OK;
}

/**
 * 物理量フィールド情報を取得する.
 * @param solution_name        物理量データ名称
 * @return        物理量フィールド情報
 */
const UdmSolutionFieldConfig* UdmSections::getSolutionFieldConfig(const std::string& solution_name) const
{
    const UdmZone* zone = this->getParentZone();
    if (zone == NULL) return NULL;
    const UdmFlowSolutions* solutions = zone->getFlowSolutions();
    if (solutions == NULL) return NULL;

    return solutions->getSolutionField(solution_name);
}


/**
 * 物理量データの初期化を行う.
 * @param solution_name        物理量名称
 * @param value                初期設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmSections::initializeValueEntities(const std::string& solution_name, VALUE_TYPE value)
{
    // 物理量名称が存在するか？セル定義物理量であるかチェックする.
    const UdmZone* zone = this->getParentZone();
    if (zone == NULL) return UDM_ERROR;
    const UdmFlowSolutions* solutions = zone->getFlowSolutions();
    if (solutions == NULL) return UDM_ERROR;
    const UdmSolutionFieldConfig* config = solutions->getSolutionField(solution_name);
    if (config == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "not found solution_name=%s", solution_name.c_str());
    }
    if (config->getGridLocation() != Udm_CellCenter) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "solution_name[%s] is not CellCenter.", solution_name.c_str());
    }

    // セルの初期化を行う
    std::vector<UdmCell*>::iterator itr;
    for (itr=this->entity_cells.begin(); itr!=this->entity_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        cell->initializeValue(solution_name, value);
    }

    return UDM_OK;
}
template UdmError_t UdmSections::initializeValueEntities<int>(const std::string& solution_name, int value);
template UdmError_t UdmSections::initializeValueEntities<long long>(const std::string& solution_name, long long value);
template UdmError_t UdmSections::initializeValueEntities<float>(const std::string& solution_name, float value);
template UdmError_t UdmSections::initializeValueEntities<double>(const std::string& solution_name, double value);



/**
 * 削除要素（セル）を削除する.
 * 実体要素リスト,仮想要素リスト,部品要素リストから削除要素（セル）を削除する.
 * セクション（要素構成）情報(UdmElements)からの削除はUdmCellのデストラクタで行う.
 * @param cell        削除要素（セル）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::removeCell(const UdmCell* cell)
{
    if (cell == NULL) return UDM_ERROR;

    UdmSize_t cell_id = cell->getId();
    std::vector<UdmCell*>::const_iterator itr;
    std::vector<UdmCell*>::iterator rm_itr;

    // 実体要素リストから検索
    itr = this->searchGlobalId(this->entity_cells, cell);
    if (itr != this->entity_cells.end()) {
        if ((*itr) == cell) {
            delete cell;
            rm_itr = this->entity_cells.begin() + (itr - this->entity_cells.begin());
            this->entity_cells.erase(rm_itr);
            return UDM_OK;
        }
    }

    // 仮想要素リストから検索
    itr = this->searchGlobalId(this->virtual_cells, cell);
    if (itr != this->virtual_cells.end()) {
        if ((*itr) == cell) {
            delete cell;
            rm_itr = this->virtual_cells.begin() + (itr - this->virtual_cells.begin());
            this->virtual_cells.erase(rm_itr);
            return UDM_OK;
        }
    }

    // cell_idが存在しない
    UDM_WARNING_HANDLER(UDM_WARNING_INVALID_CELL, "not found cell[cell_id=%ld,type=%d]", cell_id, cell->getElementType());

    return UDM_ERROR;
}


/**
 * 削除部品要素（セル）を削除する.
 * 実体要素リスト,仮想要素リストリストから削除要素（セル）を削除する.
 * セクション（要素構成）情報(UdmElements)からの削除はUdmCellのデストラクタで行う.
 * @param component        削除部品要素（セル）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::removeComponentCell(const UdmComponent* component)
{
    if (component == NULL) return UDM_ERROR;

    UdmSize_t component_id = component->getId();
    std::vector<UdmComponent*>::const_iterator itr;
    std::vector<UdmComponent*>::iterator rm_itr;

    // 部品要素リストから検索
    itr = this->searchEntityId(this->component_cells, component_id);
    if (itr != this->component_cells.end()) {
        if ((*itr) == component) {
            delete component;
            rm_itr = this->component_cells.begin() + (itr - this->component_cells.begin());
            this->component_cells.erase(rm_itr);
            return UDM_OK;
        }
    }

    // cell_idが存在しない
    UDM_WARNING_HANDLER(UDM_WARNING_INVALID_CELL, "not found cell[cell_id=%ld,type=%d]", component_id, component->getElementType());

    return UDM_ERROR;
}


/**
 * 実体要素リストに要素（セル）を挿入する.
 * 挿入位置は管理要素ID(elem_id)順に挿入する.
 * @param cell        追加要素（セル）
 * @return        要素（セル）ID （１～）
 */
UdmSize_t UdmSections::insertEntityCell(UdmCell* cell)
{
    UdmSize_t elem_id = cell->getElemId();
    UdmSize_t last_elem_id = 0;
    UdmSize_t last_cell_id = 0;
    if (this->entity_cells.size() > 0) {
        UdmCell *last = *(this->entity_cells.end()-1);
        last_elem_id = last->getElemId();
        last_cell_id = last->getId();
    }
    if (last_elem_id < elem_id) {
        // 最終に追加する.
        this->entity_cells.push_back(cell);
        cell->setId(last_cell_id+1);
    }
    else {
        std::vector<UdmCell*>::const_iterator itr;
        std::vector<UdmCell*>::iterator ins_itr;
        itr = this->searchUpperElemId(this->entity_cells, elem_id);
        ins_itr = this->entity_cells.begin() + (itr - this->entity_cells.begin());
        ins_itr = this->entity_cells.insert(ins_itr, cell);
        UdmSize_t cell_id = std::distance(this->entity_cells.begin(), ins_itr) + 1;
        cell->setId(cell_id);
    }

    cell->setParentSections(this);
    // MPIランク番号
    cell->setMyRankno(this->getMpiRankno());

    return cell->getId();
}

/**
 * セクション（要素構成）情報の基本情報のシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmSections::serialize(UdmSerializeArchive& archive) const
{
    int n;
    // セクション数
    int num_sections = this->getNumSections();
    archive.write(this->getNumSections(), sizeof(int));

    // セクション（要素構成）情報
    for (n=1; n<=num_sections; n++) {
        this->getSection(n)->serialize(archive);
    }

    return archive;
}


/**
 * セクション（要素構成）情報の基本情報のデシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmSections::deserialize(UdmSerializeArchive& archive)
{
    int n;
    // セクション数
    int num_sections = 0;
    archive.read(num_sections, sizeof(int));

    // セクション（要素構成）情報
    for (n=1; n<=num_sections; n++) {
        UdmElements *elements = new UdmElements();
        elements->deserialize(archive);
        this->setSection(elements);
    }

    return archive;
}

/**
 * セクション（要素構成）情報の基本情報を全プロセスに送信する.
 * すべてのプロセスにて同一セクション（要素構成）情報が存在するかチェックする.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::brodcastSections()
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
    UdmSections mpi_sections;
    streamBuffer.initialize(buf, buf_size);
    archive >> mpi_sections;
    if (buf != NULL) {
        delete []buf;
        buf = NULL;
    }

    // セクション（要素構成）情報の作成済みであるかチェックする.
    if (this->getNumEntityCells() == 0) {
        // セクション（要素構成）情報未作成であるので、セクション（要素構成）情報を作成する.
        error = this->cloneSections(mpi_sections);
        if (error != UDM_OK) {
            UDM_ERRORNO_HANDLER(error);
        }
    }
    // セクション（要素構成）情報が存在するので、セクション（要素構成）情報が同一であるかチェックする.
    else if (!this->equalsSections(mpi_sections)) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "not equals sections(CGNS:ELEMENTS).");
    }

    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }

    return UDM_OK;
}


/**
 * セクション（要素構成）の基本情報をコピーする.
 * @param src        コピー元セクション（要素構成）情報
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::cloneSections(const UdmSections& src)
{
    // ID
    this->setId(src.getId());
    // データ型
    this->setDataType(src.getDataType());
    // 名前
    this->setName(src.getName());

    // セクション（要素構成）情報のクリア
    this->clearSections();

    int n;
    int num_sections = src.getNumSections();
    for (n=1; n<=num_sections; n++) {
        UdmElements *elements = new UdmElements();
        elements->cloneElements(*src.getSection(n));
        this->setSection(elements);
    }

    return UDM_OK;
}

/**
 * 同一セクション（要素構成）情報であるかチェックする.
 * セクション（要素構成）情報の基本情報が同じであるかチェックする.
 * @param model        チェック対象セクション（要素構成）情報
 * @return            true=同一
 */
bool UdmSections::equalsSections(const UdmSections& sections) const
{
    if (this->getId() != sections.getId()) return false;
    if (this->getNumSections() != sections.getNumSections())  return false;
    int n;
    int num_sections = this->getNumSections();
    for (n=1; n<=num_sections; n++) {
        if (!this->getSection(n)->equalsElements(*sections.getSection(n))) {
            return false;
        }
    }

    return true;
}

/**
 * 要素（セル）をインポートする.
 * @param cell        インポート要素（セル）
 * @param import_nodes        インポート節点（ノード）リスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::importCells(
                const std::vector<UdmCell*>& import_cells,
                const std::vector<UdmNode*>& import_nodes)
{
    UdmError_t error = UDM_OK;
    int count = 0;
    int n;

    // ランク番号、ID検索テーブルを作成する.
    UdmSearchTable search_table(import_nodes);

    std::vector<UdmNode*> nodes;
    std::vector<UdmCell*>::const_iterator itr;
    for (itr=import_cells.begin(); itr != import_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        // ElementsID
        UdmSize_t elements_id = 0;
        if (cell->getParentElements() == NULL) {
            return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "parent_elements is null.");
        }
        elements_id = cell->getParentElements()->getId();
        UdmElements *elements = this->getSection(elements_id);
        if (elements == NULL) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "not found elements of elements_id[=%d].", elements_id);
        }

        // 要素（セル）の構成ノードを検索する.
        UdmNode* find_node = NULL;
        int num_nodes = cell->getNumNodes();
        nodes.clear();
        nodes.reserve(num_nodes);
        for (n=1; n<=num_nodes; n++) {
            const UdmNode *node = cell->getNode(n);
            find_node = search_table.findMpiRankInfo(node);
            if (find_node != NULL) {
                nodes.push_back(find_node);
            }
            else {
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS,  "not found node[id=%d,my_rankno=%d] in cell[id=%d,my_rankno=%d].", node->getId(), node->getMyRankno(), cell->getId(), cell->getMyRankno());
            }
        }
        // 要素（セル）の追加
        error = elements->importCell(cell, nodes);
        if (error != UDM_OK) {
            return UDM_ERROR_HANDLER(error, "elements_id=%d,cell_id=%d[%d], n=%d", elements_id, cell->getId(), cell->getMyRankno(), count);
        }
        count++;
    }

    return UDM_OK;
}

/**
 * 仮想要素（セル）をインポートする.
 * @param import_virtuals        インポート仮想要素（セル）
 * @param virtual_nodes        インポート仮想節点（ノード）リスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::importVirtualCells(
                        const std::vector<UdmCell*>& import_virtuals,
                        const std::vector<UdmNode*>& virtual_nodes)
{
    UdmError_t error = UDM_OK;
    std::vector<UdmCell*> insert_cells;

    // ランク番号、ID検索テーブルを作成する.
    UdmSearchTable search_table(virtual_nodes);

    std::vector<UdmCell*>::const_iterator itr;
    std::vector<UdmCell*>::const_iterator found_itr;
    for (itr=import_virtuals.begin(); itr != import_virtuals.end(); itr++) {
        UdmCell* src_cell = (*itr);
        UdmSize_t src_cell_id = src_cell->getId();
        int src_rankno = src_cell->getMyRankno();

        // 同一の仮想要素（セル）が存在するかチェックする.
        found_itr = this->searchGlobalId(this->virtual_cells, src_cell);
        if (found_itr != this->virtual_cells.end()) {
            // 同一の仮想要素（セル）が存在するので、物理量のコピーを行う.
            UdmCell *dest_cell = *found_itr;
            dest_cell->cloneCell(*src_cell);
            dest_cell->setRealityType(Udm_Virtual);
            continue;
        }

        // 要素（セル）の作成
        UdmCell *dest_cell = UdmCell::factoryCell(src_cell->getElementType());
        if (dest_cell == NULL) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_CREATE_CELL,  "elem_type=%d", (int)src_cell->getElementType());
        }
        // 要素（セル）のコピー
        dest_cell->cloneCell(*src_cell);
        // 仮想要素（セル）
        dest_cell->setRealityType(Udm_Virtual);

        // 節点（ノード）を追加する
        int n;
        std::vector<UdmNode*>::const_iterator find_itr;
        UdmNode* find_node = NULL;
        int num_nodes = src_cell->getNumNodes();
        for (n=1; n<=num_nodes; n++) {
            const UdmNode *src_node = src_cell->getNode(n);

            // 節点（ノード）リストから検索
            find_node = search_table.findMpiRankInfo(src_node);

#if 0
            find_itr = this->searchGlobalId(virtual_nodes, src_node);
            if (find_itr != virtual_nodes.end()) {
                find_node = *find_itr;
            }
            else {
                // 共有節点（ノード）を検索する
                find_node = UdmNode::findMpiRankInfo(virtual_nodes, src_node->getMyRankno(), src_node->getId());
            }
#endif
            if (find_node == NULL) {
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_NODE,  "not found node[id=%d,my_rankno=%d", src_node->getId(), src_node->getMyRankno());
            }
            // 構成節点（ノード）の追加
            dest_cell->insertNode(find_node);
        }

        // 仮想要素（セル）の追加
        this->insertVirtualCell(dest_cell);
        insert_cells.push_back(dest_cell);
    }

    // 部品要素の作成
    for (itr=insert_cells.begin(); itr != insert_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        UdmSize_t cell_id = cell->getId();
        // 部品要素の作成
        cell->createComponentCells();
    }

    // 仮想要素の接続情報を取得する.
    UdmScannerCells scanner;
    UdmSize_t count = scanner.scannerGraph(insert_cells);
    const std::vector<std::vector<UdmEntity*> > graph_cells = scanner.getScanArrays();

    std::vector< std::vector<UdmEntity*> >::const_iterator graph_itr;
    std::vector<UdmEntity*>::const_iterator vertex_itr;
    std::vector<UdmCell*> delete_cells;
    UdmSize_t n = 0;
    int virtual_count = 0;
    for (graph_itr=graph_cells.begin(); graph_itr!=graph_cells.end(); graph_itr++) {
        if (graph_itr->size() <= 1) continue;        // 接続要素が存在していること
        // ０はinsert_cellsの要素（セル）
        UdmEntity *center_cell = graph_itr->at(0);
        if (center_cell == NULL) continue;
        virtual_count = 0;
        for (vertex_itr=graph_itr->begin(); vertex_itr!=graph_itr->end(); vertex_itr++) {
            if ((*vertex_itr)->getRealityType() == Udm_Virtual) {
                virtual_count++;
            }
        }
        if (virtual_count == graph_itr->size()) {
            // すべての接続が仮想要素であるので、要素（セル）を削除する.
            delete_cells.push_back(static_cast<UdmCell*>(graph_itr->at(0)));
        }
    }

    // すべての接続が仮想要素の削除
    for (itr=delete_cells.begin(); itr != delete_cells.end(); itr++) {
        UdmCell *cell = (*itr);
        this->removeCell(cell);
    }

    // 仮想要素（セル）ローカルIDの再構築
    this->rebuildVirtualCells();

    return UDM_OK;
}

/**
 * 最大の要素管理ID(elem_id)を取得する.
 * 最終の要素管理ID(elem_id)を取得する.
 * @return        最大要素管理ID(elem_id)
 */
UdmSize_t UdmSections::getMaxEntityElemId() const
{
    if (this->entity_cells.size() <= 0) return 0;
    std::vector<UdmCell*>::const_iterator itr;
    itr = this->entity_cells.end() - 1;
    return (*itr)->getElemId();
}

/**
 * 要素（セル）のIDの再構築を行う。
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::rebuildCellId()
{
    UdmSize_t cell_id = 0;
    std::vector<UdmCell*>::iterator itr;
    for (itr=this->entity_cells.begin(); itr!=this->entity_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        // 現在のIDを以前のIDに設定する.
        cell->addPreviousRankInfo(cell->getMyRankno(), cell->getId());
        cell->setId(++cell_id);            // IDをインクリメントして設定する
        cell->setLocalId(cell_id);         // ローカルIDを設定する
    }

    // 仮想要素（セル）のIDは実体ランクのIDであるので、再構築しない。ローカルIDを設定する
    for (itr=this->virtual_cells.begin(); itr!=this->virtual_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        cell->setLocalId(++cell_id);         // ローカルIDをインクリメントして設定する
    }

    // 部品要素はIDが意味を持っていないが、ID検索、削除の為に再構築を行う。
    cell_id = 0;
    std::vector<UdmComponent*>::iterator comp_itr;
    for (comp_itr=this->component_cells.begin(); comp_itr!=this->component_cells.end(); comp_itr++) {
        UdmComponent* cell = (*comp_itr);
        // 部品要素は現在のIDは設定しない
        cell->setId(++cell_id);            // IDをインクリメントして設定する
    }


    return UDM_OK;
}


/**
 * 仮想要素（セル）のIDの再構築を行う。
 * ローカルIDに構成要素（セル）＋連番を設定する
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::rebuildVirtualCells()
{
    UdmSize_t cell_id = this->entity_cells.size();
    std::vector<UdmCell*>::iterator itr;
    for (itr=this->virtual_cells.begin(); itr!= this->virtual_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        cell->setLocalId(++cell_id);         // ローカルIDをインクリメントして設定する
    }

    return UDM_OK;
}


/**
 * 要素（セル）の物理量データ値を取得する:スカラデータ.
 * @param [in]  cell_id        要素（セル）ID（=１～）
 * @param [in]  solution_name        物理量名
 * @param [out]  value                物理量データ値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmSections::getSolutionScalar(
                        UdmSize_t cell_id,
                        const std::string& solution_name,
                        VALUE_TYPE& value) const
{
    UdmEntity *entity = this->getEntityCell(cell_id);
    if (entity == NULL) return UDM_ERROR;

    return entity->getSolutionScalar<VALUE_TYPE>(solution_name, value);
}

/**
 * 要素（セル）の物理量データ値を取得する:ベクトルデータ.
 * @param [in]  cell_id        要素（セル）ID（=１～）
 * @param [in]  solution_name        物理量名
 * @param [out]  value                物理量データリスト
 * @return        物理量データ数
 */
template<class VALUE_TYPE>
unsigned int UdmSections::getSolutionVector(
                        UdmSize_t cell_id,
                        const std::string& solution_name,
                        VALUE_TYPE* values) const
{
    UdmEntity *entity = this->getEntityCell(cell_id);
    if (entity == NULL) return 0;

    return entity->getSolutionVector<VALUE_TYPE>(solution_name, values);
}

/**
 * 要素（セル）の物理量データ値を設定する:スカラデータ.
 * @param   cell_id        要素（セル）ID（=１～）
 * @param   solution_name        物理量名
 * @param   value                物理量データ値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmSections::setSolutionScalar(
                        UdmSize_t cell_id,
                        const std::string& solution_name,
                        VALUE_TYPE value)
{
    UdmEntity *entity = this->getEntityCell(cell_id);
    if (entity == NULL) return UDM_ERROR;

    return entity->setSolutionScalar<VALUE_TYPE>(solution_name, value);
}

/**
 * 要素（セル）の物理量データ値を設定する:ベクトルデータ.
 * @param   cell_id        要素（セル）ID（=１～）
 * @param   solution_name        物理量名
 * @param   values                物理量データリスト
 * @param   size                物理量データ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmSections::setSolutionVector(
                        UdmSize_t cell_id,
                        const std::string& solution_name,
                        const VALUE_TYPE* values,
                        unsigned int size)
{
    UdmEntity *entity = this->getEntityCell(cell_id);
    if (entity == NULL) return UDM_ERROR;

    return entity->setSolutionVector<VALUE_TYPE>(solution_name, values, size);
}

template UdmError_t UdmSections::getSolutionScalar<int>(UdmSize_t cell_id, const std::string& solution_name, int& value) const;
template UdmError_t UdmSections::getSolutionScalar<long long>(UdmSize_t cell_id, const std::string& solution_name, long long& value) const;
template UdmError_t UdmSections::getSolutionScalar<float>(UdmSize_t cell_id, const std::string& solution_name, float& value) const;
template UdmError_t UdmSections::getSolutionScalar<double>(UdmSize_t cell_id, const std::string& solution_name, double& value) const;

template unsigned int UdmSections::getSolutionVector<int>(UdmSize_t cell_id, const std::string& solution_name, int* values) const;
template unsigned int UdmSections::getSolutionVector<long long>(UdmSize_t cell_id, const std::string& solution_name, long long* values) const;
template unsigned int UdmSections::getSolutionVector<float>(UdmSize_t cell_id, const std::string& solution_name, float* values) const;
template unsigned int UdmSections::getSolutionVector<double>(UdmSize_t cell_id, const std::string& solution_name, double* values) const;

template UdmError_t UdmSections::setSolutionScalar<int>(UdmSize_t cell_id, const std::string& solution_name, int value);
template UdmError_t UdmSections::setSolutionScalar<long long>(UdmSize_t cell_id, const std::string& solution_name, long long value);
template UdmError_t UdmSections::setSolutionScalar<float>(UdmSize_t cell_id, const std::string& solution_name, float value);
template UdmError_t UdmSections::setSolutionScalar<double>(UdmSize_t cell_id, const std::string& solution_name, double value);

template UdmError_t UdmSections::setSolutionVector<int>(UdmSize_t cell_id, const std::string& solution_name, const int* values, unsigned int size);
template UdmError_t UdmSections::setSolutionVector<long long>(UdmSize_t cell_id, const std::string& solution_name, const long long* values, unsigned int size);
template UdmError_t UdmSections::setSolutionVector<float>(UdmSize_t cell_id, const std::string& solution_name, const float* values, unsigned int size);
template UdmError_t UdmSections::setSolutionVector<double>(UdmSize_t cell_id, const std::string& solution_name, const double* values, unsigned int size);


/**
 * 仮想セルをCGNSファイル出力する.
 * 仮想セルの検証用関数.
 * @param index_file        CGNS:ファイルインデックス
 * @param index_base        CGNS:ベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::writeVirtualCells(int index_file, int index_base, int index_zone)
{
    int index_section;
    cgsize_t start, end;
    char elem_name[33] = "Elements_VirtualCells";
    int nbndry = 0;
    std::string element_typename;
    UdmElementType_t elemtype = Udm_ElementTypeUnknown;
    ElementType_t cgns_elemtype;
    cgsize_t *element_ids = NULL;
    UdmSize_t data_size = 0;
    int n;
    UdmSize_t count = 0;

    // 仮想セルの要素タイプを探索
    std::vector<UdmCell*>::iterator itr;
    for (itr=this->virtual_cells.begin(); itr!=this->virtual_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        if (elemtype == Udm_ElementTypeUnknown) {
            elemtype = cell->getElementType();
        }
        else if (elemtype != cell->getElementType()) {
            elemtype = Udm_MIXED;
        }
        data_size += cell->getNumNodes();
    }
    // 最終要素番号
    start=1;
    end=this->virtual_cells.size();
    cgns_elemtype = this->toCgnsElementType(elemtype);
    if (cgns_elemtype == ElementTypeNull) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_ELEMENTTYPE, "cgns_elemtype=%d", cgns_elemtype);
    }
    // ノードリスト
    if (elemtype == Udm_MIXED) {
        data_size += this->virtual_cells.size();
    }
    element_ids = new cgsize_t[data_size];
    memset(element_ids, 0x00, sizeof(cgsize_t)*data_size);
    count = 0;
    for (itr=this->virtual_cells.begin(); itr!=this->virtual_cells.end(); itr++) {
        UdmCell* cell = (*itr);
#ifdef _DEBUG
        UdmSize_t cell_id = cell->getId();
        int rankno =- cell->getMyRankno();
#endif
        if (elemtype == Udm_MIXED) {
            element_ids[count++] = (cgsize_t)this->toCgnsElementType(cell->getElementType());
        }
        int num_nodes = cell->getNumNodes();
        for (n=1; n<=num_nodes; n++) {
            const UdmNode *node = cell->getNode(n);
            if (node->getRealityType() != Udm_Virtual) {
                element_ids[count++] = node->getId();
            }
            else {
                element_ids[count++] = node->getLocalId();
            }
        }
    }

    // 要素の接続情報の設定 : cg_section_write
    if (cg_section_write(index_file,index_base,index_zone, elem_name, cgns_elemtype,start,end,nbndry,element_ids, &index_section) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "failure : cg_section_write cgns_error=%s", cg_get_error());
    }

    delete []element_ids;

    return UDM_OK;
}


/**
 * CGNS:Elementsを結合する.
 * @param dest_sections        結合Elements
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::joinCgnsSections(UdmSections* dest_sections)
{
    if (dest_sections == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "dest_sections is null.");
    }

    UdmError_t error = UDM_OK;
    int n;
    int num_elements = dest_sections->getNumSections();
    for (n=1; n<=num_elements; n++) {
        UdmElements *dest_elements = dest_sections->getSection(n);
        UdmElements *src_elements = this->getSection(dest_elements->getName());
        if (src_elements == NULL) {
            return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_ELEMENTTYPE, "not found CGNS:Elements name[=%s].", dest_elements->getName().c_str());
        }
        // Elementsを結合する.
        error = src_elements->joinCgnsElements(dest_elements);
        if (error != UDM_OK) {
            return UDM_ERRORNO_HANDLER(error);
        }
    }

    // UdmZoneのdeleteにてUdmCellがdeleteされない様にclearする
    dest_sections->entity_cells.clear();

    return UDM_OK;
}

/**
 * 要素（セル）リストを削除する.
 * @param remove_cells        削除要素（セル）リスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSections::removeCells(const std::vector<UdmCell*>& remove_cells)
{
    UdmSize_t erase_size = 0;
    std::vector<UdmCell*>::const_iterator itr;
    for (itr=remove_cells.begin(); itr!=remove_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        if (cell != NULL) {
            cell->setRemoveEntity(true);
        }
    }

    // セクション（要素構成）情報クラスから削除を行う.
    std::vector<UdmElements*>::iterator elems_itr;
    for (elems_itr = this->section_list.begin(); elems_itr != this->section_list.end(); elems_itr++) {
        (*elems_itr)->removeCells(remove_cells);
    }

    erase_size += this->entity_cells.size();
    this->entity_cells.erase(
            std::remove_if(
                    this->entity_cells.begin(),
                    this->entity_cells.end(),
                    std::mem_fun(&UdmEntity::isRemoveEntity)),
            this->entity_cells.end());
    erase_size -= this->entity_cells.size();

    if (erase_size < remove_cells.size()) {
        erase_size += this->virtual_cells.size();
        this->virtual_cells.erase(
                std::remove_if(
                        this->virtual_cells.begin(),
                        this->virtual_cells.end(),
                        std::mem_fun(&UdmEntity::isRemoveEntity)),
                this->virtual_cells.end());
        erase_size -= this->virtual_cells.size();
    }

    // 部品要素リストから削除
    if (erase_size < remove_cells.size()) {
        erase_size += this->component_cells.size();
        this->component_cells.erase(
                std::remove_if(
                        this->component_cells.begin(),
                        this->component_cells.end(),
                        std::mem_fun(&UdmComponent::isRemoveEntity)),
                this->component_cells.end());
        erase_size -= this->component_cells.size();
    }

    for (itr=remove_cells.begin(); itr!=remove_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        if (cell != NULL) {
            delete cell;
        }
    }

    return UDM_OK;
}

/**
 * 分割重みをクリアする.
 * 分割重みフラグをクリアする.
 * 要素（セル）に設定している重み値をクリアする.
 */
void UdmSections::clearPartitionWeight()
{
    std::vector<UdmCell*>::const_iterator itr;
    for (itr=this->entity_cells.begin(); itr!=this->entity_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        cell->clearPartitionWeight();
    }

    return;
}

/**
 * メモリサイズを取得する.
 * @return        メモリサイズ
 */
size_t UdmSections::getMemSize() const
{
    size_t size = sizeof(*this);

#ifdef _DEBUG
    printf("this size=%ld\n", sizeof(*this));
    printf("section_list size=%ld [offset=%ld]\n", sizeof(this->section_list), offsetof(UdmSections, section_list));
    printf("entity_cells size=%ld [count=%ld] [offset=%ld]\n",
                            sizeof(this->entity_cells),
                            this->entity_cells.size(),
                            offsetof(UdmSections, entity_cells));
    printf("virtual_cells size=%ld [count=%ld] [offset=%ld]\n",
                            sizeof(this->virtual_cells),
                            this->virtual_cells.size(),
                            offsetof(UdmSections, virtual_cells));
    printf("component_cells size=%ld [count=%ld] [offset=%ld]\n",
                            sizeof(this->component_cells),
                            this->component_cells.size(),
                            offsetof(UdmSections, component_cells));

    printf("parent_zone pointer size=%ld [offset=%ld]\n", sizeof(this->parent_zone), offsetof(UdmSections, parent_zone));

    size += this->entity_cells.size()*sizeof(UdmCell*);
    size += this->virtual_cells.size()*sizeof(UdmCell*);
    size += this->component_cells.size()*sizeof(UdmCell*);

    std::vector<UdmElements*>::const_iterator itr;
    for (itr = this->section_list.begin(); itr != this->section_list.end(); itr++) {
        size += (*itr)->getMemSize();
    }
#endif

    return size;
}


/**
 * ローカルセルIDの要素（セル）を取得する.
 * 構成要素（セル）と仮想要素（セル）から要素（セル）を取得する.
 * @param node_id        ノードID（１～getNumNodes()+getNumVirtualNodes()）
 * @return        要素（セル）
 */
UdmCell* UdmSections::getCellByLocalId(UdmSize_t cell_id) const
{
    if (cell_id <= 0) return NULL;
    if (cell_id <= this->entity_cells.size()) {
        return this->getEntityCell(cell_id);
    }
    else if (cell_id > this->entity_cells.size()) {
        return this->getVirtualCell(cell_id - this->entity_cells.size());
    }
    return NULL;
}


/**
 * 要素（セル）の破棄処理を行う.
 */
void UdmSections::finalize()
{
    std::vector<UdmCell*>::const_iterator itr;
    for (itr=this->entity_cells.begin(); itr!=this->entity_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        if (cell != NULL) {
            cell->finalize();
            delete cell;
        }
    }
    this->entity_cells.clear();

    for (itr=this->virtual_cells.begin(); itr!=this->virtual_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        if (cell != NULL) {
            cell->finalize();
            delete cell;
        }
    }
    this->virtual_cells.clear();

    // 部品要素の削除
    std::vector<UdmComponent*>::const_iterator comp_itr;
    for (comp_itr=this->component_cells.begin(); comp_itr!=this->component_cells.end(); comp_itr++) {
        UdmComponent* cell = (*comp_itr);
        if (cell != NULL) {
            cell->finalize();
            delete cell;
        }
    }
    this->component_cells.clear();

    // UdmElementsのクリア
    this->clearSections();

    return;
}

} /* namespace udm */
