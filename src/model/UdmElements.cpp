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
 * @file UdmElements.cpp
 * セクション（要素構成）情報クラスのソースファイル
 */

#include "model/UdmElements.h"
#include "model/UdmZone.h"
#include "model/UdmGridCoordinates.h"
#include "model/UdmSections.h"
#include "model/UdmCell.h"
#include "model/UdmBar.h"
#include "model/UdmShell.h"
#include "model/UdmSolid.h"
#include "model/UdmNode.h"
#include "model/UdmRankConnectivity.h"
#include "utils/UdmScannerCells.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmElements::UdmElements()
{
    this->initialize();
}

/**
 * コンストラクタ : セクション（要素構成）名称, セクション（要素構成）ID
 * @param section_name        セクション（要素構成）名称
 * @param section_id        セクション（要素構成）ID
 */
UdmElements::UdmElements(const std::string& section_name, UdmSize_t section_id) : UdmGeneral(section_name, section_id)
{
    this->initialize();
}

/**
 * コンストラクタ : 親要素管理クラス
 * @param parent_sections        親要素管理クラス
 */
UdmElements::UdmElements(UdmSections* parent_sections)
{
    this->initialize();
    this->parent_sections = parent_sections;
}


/**
 * デストラクタ
 */
UdmElements::~UdmElements()
{
    this->clearCells();
}

/**
 * 初期化を行う.
 */
void UdmElements::initialize()
{
    this->clearCells();
}

/**
 * 要素形状タイプを取得する.
 * @return        要素形状タイプ
 */
UdmElementType_t UdmElements::getElementType() const
{
    return this->element_type;
}

/**
 * 要素形状タイプを設定する
 * @param element_type        要素形状タイプ
 */
void UdmElements::setElementType(UdmElementType_t element_type)
{
    this->element_type = element_type;
}

/**
 * セクション（要素構成）の要素（セル）数を取得する.
 * @return        要素（セル）数
 */
UdmSize_t UdmElements::getNumCells() const
{
    return this->cell_list.size();
}

/**
 * セルIDの要素（セル）を取得する.
 * @param cell_id        セクション内ローカルセルID（１～）
 * @return        要素（セル）
 */
UdmCell* UdmElements::getCell(UdmSize_t cell_id) const
{
    if (cell_id <= 0) return NULL;
    if (cell_id > this->cell_list.size()) return NULL;
    return this->cell_list[cell_id-1];
}


/**
 * 管理要素（セル）IDの要素（セル）が存在するかチェックする.
 * @param elem_id        管理要素（セル）ID
 * @return        true=要素（セル）が存在する.
 */
bool UdmElements::existsCellByElemId(UdmSize_t elem_id) const
{
    return (this->getCellByElemId(elem_id) != NULL);
}

/**
 * 管理要素（セル）IDの要素（セル）を取得する.
 * @param elem_id        管理要素（セル）ID
 * @return        要素（セル）
 */
UdmCell* UdmElements::getCellByElemId(UdmSize_t elem_id)
{
    if (elem_id <= 0) return NULL;
    std::vector<UdmCell*>::iterator itr;
    for (itr=this->cell_list.begin(); itr!=this->cell_list.end(); itr++) {
        if ((*itr)->getElemId() == elem_id) {
            return (*itr);
        }
    }
    return NULL;
}

/**
 * 管理要素（セル）IDの要素（セル）を取得する.
 * @param elem_id        管理要素（セル）ID
 * @return        要素（セル）
 */
const UdmCell* UdmElements::getCellByElemId(UdmSize_t elem_id) const
{
    if (elem_id <= 0) return NULL;
    std::vector<UdmCell*>::const_iterator itr;
    for (itr=this->cell_list.begin(); itr!=this->cell_list.end(); itr++) {
        if ((*itr)->getElemId() == elem_id) {
            return (*itr);
        }
    }
    return NULL;
}

/**
 * 要素（セル）をセクション（要素構成）に追加する.
 * セクション（要素構成）:UdmSectionへの追加と要素管理:UdmElementsへ追加する。
 * @param elem_id        管理要素（セル）ID（ゾーン内の一意(任意)の要素ID）
 * @param cell        要素（セル）
 * @return        ゾーン要素(セル)ID（１～）: 0の場合は挿入エラー
 */
UdmSize_t UdmElements::setCellByElemId(UdmSize_t elem_id, UdmCell* cell)
{
    if (cell == NULL) return 0;
    if (this->parent_sections == NULL) return 0;
    UdmSize_t cell_id = 0;
    // 既存要素（セル）であるかチェックする.
    std::vector<UdmCell*>::const_iterator itr = this->searchElemId(this->cell_list, elem_id);
    if (itr != this->cell_list.end()) {
        std::vector<UdmCell*>::iterator rm_itr = this->cell_list.begin() + (itr - this->cell_list.begin());
        this->cell_list.erase(rm_itr);
    }

    // セクション（要素構成）に追加する.
    cell->setElemId(elem_id);
    this->insertCell(cell);

    return cell_id;
}

/**
 * 要素（セル）をセクション（要素構成）に追加する.
 * @param cell        要素（セル）
 * @return        セクション（要素構成）内ID（１～）: 0の場合は挿入エラー
 */
UdmSize_t UdmElements::insertCell(UdmCell* cell)
{
    UdmSize_t elem_id = cell->getElemId();
    UdmSize_t last_elem_id = 0;
    if (this->cell_list.size() > 0) {
        UdmCell *last = *(this->cell_list.end()-1);
        last_elem_id = last->getElemId();
    }
    if (elem_id == 0 || last_elem_id < elem_id) {
        // 最終に追加する.
        this->cell_list.push_back(cell);
        cell->setId(this->cell_list.size());
    }
    else {
        std::vector<UdmCell*>::const_iterator itr;
        std::vector<UdmCell*>::iterator ins_itr;
        itr = this->searchUpperElemId(this->cell_list, elem_id);
        ins_itr = this->cell_list.begin() + (itr - this->cell_list.begin());
        ins_itr = this->cell_list.insert(ins_itr, cell);
        UdmSize_t cell_id = std::distance(this->cell_list.begin(), ins_itr) + 1;
        cell->setId(cell_id);
    }
    if (elem_id == 0) {
        cell->setElemId(cell->getId());
    }

    // 親セクション（要素構成）を設定する.
    cell->setParentElements(this);

    // 要素管理クラスへ追加する
    UdmSize_t cell_id = this->parent_sections->setEntityCell(cell->getElemId(), cell);

    return cell->getId();
}

/**
 * 要素（セル）をすべて削除する.
 * 要素（セル）のオブジェクトの削除はUdmSections::clearEntityCells()で行う。
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmElements::clearCells()
{
    this->cell_list.clear();
    return UDM_OK;
}

/**
 * 親要素管理クラスを取得する.
 * @return        親要素管理クラス
 */
UdmSections* UdmElements::getParentSections()
{
    return this->parent_sections;
}

/**
 * 親要素管理クラスを取得する:const.
 * @return        親要素管理クラス
 */
const UdmSections* UdmElements::getParentSections() const
{
    return this->parent_sections;
}

/**
 * 親要素管理クラスを設定する.
 * @param zone        親要素管理クラス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmElements::setParentSections(UdmSections* sections)
{
    this->parent_sections = sections;
    return UDM_OK;
}


/**
 * CGNSファイルからElements（要素接続情報）の読込みを行う.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param index_section     CGNSセクションインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmElements::readCgns(int index_file, int index_base, int index_zone, int index_section)
{
    UdmError_t error = UDM_OK;
    char section_name[33] = {0x00};
    ElementType_t cgns_elemtype;
    UdmElementType_t elem_type;
    cgsize_t start, end, cell_size;
    int nbndry, parent_flag;
    cgsize_t element_size;
    cgsize_t n;
    int next_vertex = 0;
    std::string buf;

    // セクションの読込
    if (cg_section_read(
                index_file, index_base,index_zone,
                index_section,section_name,
                &cgns_elemtype, &start, &end, &nbndry,&parent_flag) != CG_OK) {
        return UDM_ERROR_CGNS_INVALID_ELEMENTS;
    }
    this->setName(section_name);
    this->setId(index_section);
    elem_type = this->setElementType(cgns_elemtype);
    cell_size = end - start + 1;

    // 接続情報データサイズ
    if (cg_ElementDataSize(index_file, index_base,index_zone, index_section, &element_size) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "failure:cg_ElementDataSize");
    }
    if (element_size <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "element_size is zero.");
    }
    // 接続情報の読込
    cgsize_t *elements = new cgsize_t[element_size];
    memset(elements, 0x00, sizeof(cgsize_t)*element_size);
    if (elements == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "memory allocate error. element_size=%ld", element_size);
    }
    if (cg_elements_read(index_file, index_base,index_zone, index_section, elements, NULL) != CG_OK) {
        if (elements != NULL) delete[]elements;
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "failure:cg_elements_read");
    }

    // Udm_TETRA_4 = 5,               //!< Solid:四面体要素
    // Udm_PYRA_5 = 6,                //!< Solid:ピラミッド要素
    // Udm_PENTA_6 = 7,               //!< Solid:五面体要素
    // Udm_HEXA_8 = 8,                //!< Solid:六面体要素
    if (elem_type != Udm_MIXED && this->isSupportElementType(elem_type)) {
        for (n=0; n<cell_size; n++) {
            UdmCell *cell = this->factoryCell(elem_type, elements+next_vertex);
            if (cell == NULL) {
                if (elements != NULL) delete[]elements;
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_CREATE_CELL, "elem_type=%d", (int)elem_type);
            }
            next_vertex += cell->getNumNodes();
            // 作成セルの格納
            this->setCellByElemId(start+n, cell);
        }
    }
    // Udm_MIXED = 9,                 //!< 混合要素
    else if (elem_type == Udm_MIXED) {
        for (n=0; n<cell_size; n++) {
            ElementType_t cgns_cell_type = (ElementType_t)elements[next_vertex];
            UdmElementType_t cell_type = this->toElementType(cgns_cell_type);

            // 要素タイプチェック
            // modify by @hira at 2015/08/04
            // if (elem_type == Udm_MIXED || !this->isSupportElementType(elem_type)) {
            if (cell_type == Udm_MIXED || !this->isSupportElementType(cell_type)) {
                this->toStringElementType(cell_type, buf);
                error = UDM_WARNING_HANDLER(UDM_WARNING_CGNS_NOTSUPPORT_ELEMENTTYPE, "read CGNS ElementType=%s", buf.c_str());
                break;
            }

            next_vertex++;
            UdmCell *cell = this->factoryCell(cell_type, elements+next_vertex);
            if (cell == NULL) {
                if (elements != NULL) delete[]elements;
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_CREATE_CELL,  "elem_type=%d", (int)elem_type);
            }
            next_vertex += cell->getNumNodes();
            // 作成セルの格納
            this->setCellByElemId(start+n, cell);
        }
    }
    else {
        this->toStringElementType(elem_type, buf);
        error = UDM_WARNING_HANDLER(UDM_WARNING_CGNS_NOTSUPPORT_ELEMENTTYPE, "read CGNS ElementType=%s", buf.c_str());
    }

    if (elements != NULL) delete[]elements;
    return error;
}

/**
 * CGNS:要素形状タイプをUDMlib:要素形状タイプとして設定する.
 * @param cgns_elementtype        CGNS:要素形状タイプ
 * @return        UDMlib:要素形状タイプ
 */
UdmElementType_t UdmElements::setElementType(ElementType_t cgns_elementtype)
{
    this->element_type = this->toElementType(cgns_elementtype);
    return this->element_type;
}

/**
 * 要素（セル）を生成する.
 * @param elem_type        要素タイプ
 * @param elements        要素接続情報（ノードリスト）
 * @return        生成要素（セル）
 */
template<class DATA_TYPE>
UdmCell* UdmElements::factoryCell(UdmElementType_t elem_type, DATA_TYPE *elements)
{
    int n;

    int num_vertex = this->getNumVertexOfElementType(elem_type);
    if (num_vertex <= 0) return NULL;

    UdmCell* cell = UdmCell::factoryCell(elem_type);
    if (cell == NULL) return NULL;

    if (elements != NULL) {
        UdmGridCoordinates *grid = this->getGridCoordinates();
        if (grid == NULL) {
            delete cell;
            return NULL;
        }
        for (n=0; n<num_vertex; n++) {
            UdmSize_t node_id = (UdmSize_t)(elements[n]);
            UdmNode* node = grid->getNodeById(node_id);
            if (node == NULL) {
                delete cell;
                return NULL;
            }
            cell->insertNode(node);
        }
    }

    return cell;
}

/**
 * 所属ゾーンを取得する.
 * @return        所属ゾーン
 */
UdmZone* UdmElements::getZone() const
{
    if (this->parent_sections == NULL) return NULL;
    return this->parent_sections->getParentZone();
}

/**
 * GridCoordinatesクラスを取得する.
 * @return        GridCoordinatesクラス
 */
UdmGridCoordinates* UdmElements::getGridCoordinates() const
{
    if (this->getZone() == NULL) return NULL;
    return this->getZone()->getGridCoordinates();
}

/**
 * クラス情報を文字列出力する:デバッグ用.
 * @param buf        出力文字列
 */
void UdmElements::toString(std::string& buf) const
{
#ifdef _DEBUG_TRACE
    std::stringstream stream;

    stream << std::endl;
    stream << "Section : " << this->getName() << std::endl;
    std::string type_buf;
    this->toStringElementType(this->getElementType(), type_buf);
    stream << "ElementType : " << type_buf << std::endl;

    std::vector<UdmCell*>::const_iterator itr;
    for (itr=this->cell_list.begin(); itr!=this->cell_list.end(); itr++) {
        std::string cell_buf;
        (*itr)->toString(cell_buf);
        stream << cell_buf << std::endl;
    }

    buf += stream.str();
#endif

    return;
}

/**
 * CGNS:Elementsを出力する.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param index_cell        CGNSセル開始インデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmElements::writeCgns(int index_file, int index_base, int index_zone, int index_cell)
{
    int index_section;
    cgsize_t start, end;
    char elem_name[33] = {0x00};
    int nbndry = 0;
    std::string element_typename;
    ElementType_t cgns_elemtype;

    if (this->toStringElementType(this->getElementType(), element_typename) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_ELEMENTTYPE, "element_type=%d", this->getElementType());
    }

    if (this->getName().empty()) {
        sprintf(elem_name, UDM_CGNS_FORMAT_ELEMENTS, element_typename.c_str());
    }
    else {
        sprintf(elem_name, "%s", this->getName().c_str());
    }
    // CGNS:Elemetsが既に出力済みであるかチェックする。
    if (cg_goto(index_file, index_base, "Zone_t", index_zone, elem_name, 0, "end") == CG_OK) {
        return UDM_OK;
    }

    // 最終要素番号
    if (this->getNumCells() <= 0) {
        // 出力要素なし
        UDM_WARNING_HANDLER(UDM_WARNING_CGNS_EMPTYCELL, "CGNS:Elements=%s", this->getName().c_str());
        return UDM_OK;
    }

    start=index_cell;
    end=index_cell + this->getNumCells() - 1;
    cgns_elemtype = this->toCgnsElementType(this->getElementType());
    if (cgns_elemtype == ElementTypeNull) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_ELEMENTTYPE, "cgns_elemtype=%d", cgns_elemtype);
    }

#if 0
    {
        UdmSize_t num_nodes = 0;
        std::vector<UdmCell*>::const_iterator itr;
        for (itr=this->cell_list.begin(); itr!=this->cell_list.end();itr++) {
            UdmCell *cell = (*itr);
            // DEBUG
            printf("[rank=%d] UdmElements::writeCgns : scannerElements cell_id[%ld]=num_node=%d \n",
                    this->getParentSections()->getMpiRankno(),
                    cell->getId(),
                    cell->getNumNodes());
            num_nodes += cell->getNumNodes();
        }

        // DEBUG
        printf("[rank=%d] UdmElements::writeCgns : scannerElements end num_nodes=%ld \n",
                this->getParentSections()->getMpiRankno(), num_nodes);
    }
#endif

    UdmScannerCells scanner_cells;
    scanner_cells.scannerElements(this->cell_list);

    size_t data_size = scanner_cells.getNumElementsDatas(this->element_type);
    cgsize_t *element_ids = new cgsize_t[data_size];
    scanner_cells.getScanCgnsElementsIds<cgsize_t>(this->element_type, element_ids);

    // 要素の接続情報の設定 : cg_section_write
    if (cg_section_write(index_file,index_base,index_zone, elem_name, cgns_elemtype,start,end,nbndry,element_ids, &index_section) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "failure : cg_section_write, cgns_error=%s", cg_get_error());
    }

    delete []element_ids;

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
UdmError_t UdmElements::writeCgnsLinkFile(
                        int index_file,
                        int index_base,
                        int index_zone,
                        const std::string &link_output_path,
                        const std::vector<std::string> &linked_files)
{
    UdmError_t error = UDM_OK;
    char basename[33] = {0x00};
    char zonename[33] = {0x00};
    char element_name[33] = {0x00};
    int index_linkfile;
    std::vector<std::string>::const_iterator itr;
    char path[256] = {0x00};
    std::string filename;
    int cell_dim, phys_dim;
    cgsize_t sizes[9] = {0x00};
    cgsize_t start, end;
    int nsections;
    int n;
    ElementType_t type;
    int nbndry, parent_flag;
    std::vector<std::string> linked_paths;
    std::vector<std::string> link_names;
    std::vector<std::string> elements_files;
    int linked_size;

    // リンクファイルにElementsが存在するかチェックする.
    filename.clear();
    for (itr=linked_files.begin(); itr!= linked_files.end(); itr++) {
        filename = (*itr);
        // リンクファイルオープン
        if (cg_open(filename.c_str(), CG_MODE_READ, &index_linkfile) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_OPENERROR, "filename=%s, cgns_error=%s", filename.c_str(), cg_get_error());
        }
        // 出力リンクファイルと同じCGNSベースインデックス, CGNSゾーンインデックスとする.
        cg_base_read(index_linkfile, index_base, basename, &cell_dim, &phys_dim);
        cg_zone_read(index_linkfile, index_base, index_zone, zonename, sizes);

        nsections = 0;
        cg_nsections(index_linkfile, index_base, index_zone, &nsections);
        if (nsections <= 0) continue;

        for (n=1; n<=nsections; n++) {
            if (cg_section_read(
                        index_linkfile, index_base, index_zone, n,
                        element_name, &type, &start, &end, &nbndry, &parent_flag) != CG_OK) {
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "failure : cg_section_read.");
            }
            if (strcmp(element_name, this->getName().c_str()) == 0) {
                // CGNS:Elementsのリンクパスの作成
                sprintf(path, "/%s/%s/%s", basename, zonename, element_name);
                linked_paths.push_back(std::string(path));
                link_names.push_back(element_name);
                elements_files.push_back(filename);
                break;
            }
        }

        cg_close(index_linkfile);
        break;
    }
    // CGNS:Elementsリンク数
    linked_size = linked_paths.size();
    if (linked_size <= 0 || filename.empty()) {
        // CGNS:Elementsが存在しないのでリンクしない。
        return UDM_OK;
    }

    // CGNS:Elementsリンクの作成:CGNS:Elementsリンクは同名リンクとする.
    for (n=0; n<linked_size; n++) {
        // 同名のCGNS:Elementsが存在しないこと。
        if (cg_goto(index_file, index_base, "Zone_t", index_zone, link_names[n].c_str(), 0, "end") == CG_OK) {
            continue;        // 作成済み
        }

        // CGNSリンクの作成
        if (cg_goto(index_file, index_base, "Zone_t", index_zone, "end") != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO, "index_file=%d,index_base=%d,index_zone=%d", index_file, index_base, index_zone);
        }

        // リンクファイルをリンク出力ファイルからの相対パスに変換する.
        std::string linked_relativepath;
        this->getLinkedRelativePath(link_output_path, elements_files[n], linked_relativepath);

        // CGNSリンク出力
        if (cg_link_write(link_names[n].c_str(), linked_relativepath.c_str(), linked_paths[n].c_str()) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "failure:cg_link_write(%s,%s,%s), cgns_error=%s",
                                    link_names[n].c_str(), filename.c_str(), linked_paths[n].c_str(), cg_get_error());
        }
    }

    return UDM_OK;
}

/**
 * CGNS出力前の初期化を行う.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmElements::initializeWriteCgns()
{
    return UDM_OK;
}

/**
 * 削除要素（セル）を削除する.
 * 実体要素リスト,仮想要素リスト,部品要素リストから削除要素（セル）を削除する.
 * @param cell        削除要素（セル）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmElements::removeCell(const UdmCell* cell)
{
    if (cell == NULL) return UDM_ERROR;
    if (this->cell_list.size() <= 0) return UDM_OK;

    UdmSize_t cell_id = cell->getId();
    std::vector<UdmCell*>::const_iterator itr = this->searchGlobalId(this->cell_list, cell);
    if (itr != this->cell_list.end()) {
        if ((*itr) == cell) {
            std::vector<UdmCell*>::iterator rm_itr = this->cell_list.begin() + (itr - this->cell_list.begin());
            this->cell_list.erase(rm_itr);
            return UDM_OK;
        }
    }

    return UDM_ERROR;
}


/**
 * セクション（要素構成）情報の基本情報のシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmElements::serialize(UdmSerializeArchive& archive) const
{
    // Genaral基本情報
    // ID
    // データ型
    // 名前
    UdmGeneral::serializeGeneralBase(archive, this->getId(), this->getDataType(), this->getName());

    // 要素形状タイプ
    archive.write(this->element_type, sizeof(UdmElementType_t ));

    return archive;
}


/**
 * セクション（要素構成）情報の基本情報のデシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmElements::deserialize(UdmSerializeArchive& archive)
{
    UdmSize_t elements_id;
    std::string elements_name;
    UdmDataType_t elements_datatype;

    // Genaral基本情報
    // ID
    // データ型
    // 名前
    UdmGeneral::deserializeGeneralBase(archive, elements_id, elements_datatype, elements_name);
    this->setId(elements_id);
    this->setName(elements_name);
    this->setDataType(elements_datatype);

    // 要素形状タイプ
    archive.read(this->element_type, sizeof(UdmElementType_t ));

    return archive;
}

/**
 * セクション（要素構成）情報をコピーする.
 * @param src        コピー元セクション（要素構成）情報
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmElements::cloneElements(const UdmElements& src)
{
    // ID : セクションID
    this->setId(src.getId());
    // データ型
    this->setDataType(src.getDataType());
    // セクション名
    this->setName(src.getName());
    // 要素形状タイプ
    this->setElementType(src.getElementType());

    return UDM_OK;
}

/**
 * 同一セクション（要素構成）情報であるかチェックする.
 * セクション（要素構成）情報の基本情報が同じであるかチェックする.
 * @param model        チェック対象セクション（要素構成）情報
 * @return            true=同一
 */
bool UdmElements::equalsElements(const UdmElements& elements) const
{
    if (this->getId() != elements.getId()) return false;
    if (this->getName() != elements.getName()) return false;
    if (this->getElementType() != elements.getElementType()) return false;

    return true;
}

/**
 * 要素（セル）をインポートする.
 * @param src_cell        インポート要素（セル）
 * @param import_nodes        インポート節点（ノード）リスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmElements::importCell(UdmCell* src_cell, const std::vector<UdmNode*>& import_nodes)
{
    UdmError_t error = UDM_OK;
    if (src_cell == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS,  "src_cell is null");
    }
    UdmSections* sections = this->getParentSections();
    if (sections == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE,  "Sections is null");
    }

    // 要素（セル）の作成
    UdmCell *dest_cell = UdmCell::factoryCell(src_cell->getElementType());
    if (dest_cell == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_CREATE_CELL,  "elem_type=%d", (int)src_cell->getElementType());
    }
    dest_cell->setParentElements(this);

    // 要素（セル）のコピー
    dest_cell->cloneCell(*src_cell);

    // 節点（ノード）を追加する
    int n;
    std::vector<UdmNode*>::const_iterator find_itr;
    UdmNode* find_node = NULL;
    int num_nodes = src_cell->getNumNodes();
    for (n=1; n<=num_nodes; n++) {
        const UdmNode *src_node = src_cell->getNode(n);
        int src_rankno = src_node->getMyRankno();
        UdmSize_t src_id = src_node->getId();

        // 節点（ノード）リストから検索
        find_node = NULL;
        find_itr = this->searchGlobalId(import_nodes, src_node);
        if (find_itr != import_nodes.end()) {
            find_node = *find_itr;
        }
        else {
            // 共有節点（ノード）を検索する
            find_node = UdmNode::findMpiRankInfo(import_nodes, src_node);
        }
        if (find_node == NULL) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS,  "not found node id=%d,my_rankno=%d [n=%d/%d].", src_node->getId(), src_node->getMyRankno(), n, num_nodes);
        }
        // 構成節点（ノード）の追加
        dest_cell->insertNode(find_node);
    }

    // 要素（セル）の追加
    // 管理ID(elem_id)は最大ID+1とする
    dest_cell->setElemId(sections->getMaxEntityElemId()+1);
    this->insertCell(dest_cell);

    return UDM_OK;
}

/**
 * 要素（セル）をセクション（要素構成）に追加する.
 * @param elem_type        要素タイプ
 * @param node_ids        要素接続情報（ノードリスト）
 * @return                要素（セル）ID（１～）: 0の場合は挿入エラー
 */
UdmSize_t UdmElements::insertCellConnectivity(
                        UdmElementType_t elem_type,
                        UdmSize_t* node_ids)
{
    UdmCell *cell = this->factoryCell<UdmSize_t>(elem_type, node_ids);
    if (cell == NULL) return 0;

    return this->insertCell(cell);
}

/**
 * 要素（セル）リストをセクション（要素構成）に追加する.
 * @param elem_type            要素タイプ
 * @param cell_size            要素（セル）数
 * @param elements            要素接続情報リスト（ノードリスト）
 * @return                    追加要素（セル）数
 */
UdmSize_t UdmElements::setCellsConnectivity(
                        UdmElementType_t elem_type,
                        UdmSize_t cell_size,
                        UdmSize_t* elements)
{
    UdmSize_t n;
    UdmSize_t count = 0;
    UdmSize_t cell_id;

    if (elem_type != Udm_MIXED) {
        int num_vertex = this->getNumVertexOfElementType(elem_type);
        for (n=0; n<cell_size; n++) {
            cell_id = this->insertCellConnectivity(elem_type, elements+n*num_vertex);
            if (cell_id > 0) {
                count++;
            }
        }
    }
    else if (elem_type == Udm_MIXED) {
        UdmSize_t next_vertex = 0;
        for (n=0; n<cell_size; n++) {
            UdmElementType_t cell_type = (UdmElementType_t)(*(elements+next_vertex));
            next_vertex++;
            cell_id = this->insertCellConnectivity(cell_type, elements+next_vertex);
            if (cell_id > 0) {
                count++;
            }
            next_vertex += this->getNumVertexOfElementType(cell_type);
        }
    }
    return count;
}

/**
 * CGNS:要素（セル）を結合する.
 * @param dest_elements        結合要素（セル）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmElements::joinCgnsElements(UdmElements* dest_elements)
{
    if (dest_elements == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "dest_elements is null.");
    }
    UdmGridCoordinates *grid = this->getGridCoordinates();
    if (grid == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "UdmGridCoordinates is null.");
    }

    // CGNS:Elements名が同じであるかチェックする.
    if (this->getName() != dest_elements->getName()) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_ELEMENTTYPE, "not equals CGNS:Elements name [src=%s,dest=%s].", this->getName().c_str(), dest_elements->getName().c_str());
    }
    // 要素タイプが同じであるかチェックする.
    if (this->getElementType() != dest_elements->getElementType()) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_ELEMENTTYPE, "not equals ElementType [src=%d,dest=%d].", this->getElementType(), dest_elements->getElementType());
    }

    int n = 0, i = 0;
    int num_cells = dest_elements->getNumCells();
    for (n=1; n<=num_cells; n++) {
        UdmCell *dest_cell = dest_elements->getCell(n);

        // 構成ノードを再作成する.
        std::vector<UdmNode*>  dest_nodes;
        int num_nodes = dest_cell->getNumNodes();
        for (i=1; i<=num_nodes; i++) {
            UdmNode *dest_node = dest_cell->getNode(i);
            if (dest_node->getCommonNode() == NULL) {
                dest_nodes.push_back(dest_node);
            }
            else {
                // 同一節点（ノード）を追加する.
                dest_nodes.push_back(dest_node->getCommonNode());
            }
        }

        dest_cell->setNodes(dest_nodes);
        dest_cell->setElemId(0);            // elem_idをクリアする
        // 要素（セル）を挿入する.
        this->insertCell(dest_cell);
    }
    dest_elements->clearCells();

    return UDM_OK;;
}


/**
 * 要素（セル）リストを削除する.
 * @param remove_cells        削除要素（セル）リスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmElements::removeCells(const std::vector<UdmCell*>& remove_cells)
{
    if (remove_cells.size() <= 0) return UDM_OK;
    if (this->cell_list.size() <= 0) return UDM_OK;

    std::vector<UdmCell*>::const_iterator itr;
    for (itr=remove_cells.begin(); itr!=remove_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        if (cell != NULL) {
            cell->setParentElements(NULL);
            cell->setRemoveEntity(true);
        }
    }

    UdmSize_t erase_size = this->cell_list.size();
    this->cell_list.erase(
            std::remove_if(
                    this->cell_list.begin(),
                    this->cell_list.end(),
                    std::mem_fun(&UdmEntity::isRemoveEntity)),
            this->cell_list.end());
    erase_size -= this->cell_list.size();

    return UDM_OK;
}


/**
 * メモリサイズを取得する.
 * @return        メモリサイズ
 */
size_t UdmElements::getMemSize() const
{
    UdmSize_t size = sizeof(*this);
#ifdef _DEBUG
    printf("this size=%ld\n", sizeof(*this));
    printf("parent_sections pointer size=%ld [offset=%ld]\n", sizeof(this->parent_sections), offsetof(UdmElements, parent_sections));
    printf("cell_list size=%ld [count=%ld] [offset=%ld]\n",
                            sizeof(this->cell_list),
                            this->cell_list.size(),
                            offsetof(UdmElements, cell_list));
    printf("element_type size=%ld [offset=%ld]\n", sizeof(this->element_type), offsetof(UdmElements, element_type));
    printf("start_elemid size=%ld [offset=%ld]\n", sizeof(this->start_elemid), offsetof(UdmElements, start_elemid));
    printf("end_elemid size=%ld [offset=%ld]\n", sizeof(this->end_elemid), offsetof(UdmElements, end_elemid));

    size += this->cell_list.size()*sizeof(UdmCell*);
#endif

    return size;
}


} /* namespace udm */
