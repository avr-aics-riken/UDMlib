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
 * @file UdmZone.cpp
 * ゾーンクラスのソースファイル
 */

#include "model/UdmModel.h"
#include "model/UdmZone.h"
#include "model/UdmGridCoordinates.h"
#include "model/UdmSections.h"
#include "model/UdmFlowSolutions.h"
#include "model/UdmCell.h"
#include "model/UdmNode.h"
#include "model/UdmRankConnectivity.h"
#include "model/UdmUserDefinedDatas.h"
#include "utils/UdmScannerCells.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmZone::UdmZone()
{
    this->initialize();
}

/**
 * コンストラクタ
 * @param parent_model        親ベースモデル
 */
UdmZone::UdmZone(UdmModel* parent_model)
{
    this->initialize();
    this->parent_model = parent_model;
}

/**
 * コンストラクタ
 * @param zone_name        ゾーン名称
 * @param zone_id        ゾーンID
 */
UdmZone::UdmZone(const std::string& zone_name, int zone_id) : UdmGeneral(zone_name, zone_id)
{
    this->initialize();
}

/**
 * デストラクタ
 */
UdmZone::~UdmZone()
{
    if (this->sections != NULL) delete this->sections;
    if (this->solutions != NULL) delete this->solutions;
    if (this->inner_boundary != NULL) delete this->inner_boundary;
    if (this->user_datas != NULL) delete this->user_datas;
    if (this->grid_coordinates != NULL) delete this->grid_coordinates;
    this->sections = NULL;
    this->solutions = NULL;
    this->user_datas = NULL;
    this->grid_coordinates = NULL;
}

/**
 * 初期化を行う
 */
void UdmZone::initialize()
{
    // グリッド座標クラスの生成
    this->grid_coordinates = new UdmGridCoordinates(this);
    // 要素管理クラスの生成
    this->sections = new UdmSections(this);
    // 物理量管理クラスの生成
    this->solutions = new UdmFlowSolutions(this);
    // 内部境界情報
    this->inner_boundary = new UdmRankConnectivity(this);
    // ユーザ定義データクラスの生成
    this->user_datas = new UdmUserDefinedDatas(this);
    // ゾーンタイプ
    this->zone_type = Udm_Unstructured;
    // 親モデル
    this->parent_model = NULL;
    // 分割重み設定フラグ
    this->set_partition_weight = false;
}

/**
 * 物理量管理クラスを取得する.
 * @return        物理量管理クラス
 */
UdmFlowSolutions* UdmZone::getFlowSolutions() const
{
    return this->solutions;
}

/**
 * グリッド座標クラスを取得する.
 * @return        グリッド座標クラス
 */
UdmGridCoordinates* UdmZone::getGridCoordinates() const
{
    return this->grid_coordinates;
}

/**
 * 要素管理クラスを取得する.
 * @return        要素管理クラス
 */
UdmSections* UdmZone::getSections() const
{
    return this->sections;
}



/**
 * ゾーンタイプを取得する.
 * @return        ゾーンタイプ
 */
UdmZoneType_t UdmZone::getZoneType() const
{
    return this->zone_type;
}

/**
 * ゾーンタイプを設定する.
 * Udm_Structured or Udm_Unstructured(default)
 * @param zoneType        ゾーンタイプ
 */
void UdmZone::setZoneType(UdmZoneType_t zone_type)
{
    this->zone_type = zone_type;
}

/**
 * CGNSゾーンタイプからUdmZoneType_tを設定する.
 * @param cgns_zonetype        CGNSゾーンタイプ
 */
UdmZoneType_t UdmZone::setZoneType(ZoneType_t cgns_zonetype)
{
    if (cgns_zonetype == Structured) this->zone_type = Udm_Structured;
    else if (cgns_zonetype == Unstructured) this->zone_type = Udm_Unstructured;
    else this->zone_type = Udm_ZoneTypeUnknown;
    return this->zone_type;
}


/**
 * 親モデルを取得する.
 * @return        親モデル
 */
UdmModel* UdmZone::getParentModel() const
{
    return this->parent_model;
}



/**
 * 親モデルを設定する.
 * @param model        親モデル
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::setParentModel(UdmModel* model)
{
    this->parent_model = model;
    return UDM_OK;
}

/**
 * CGNSファイルからの読込みを行う.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param timeslice_step       CGNS読込ステップ回数 (default=0)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::readCgns(int index_file, int index_base, int index_zone, int timeslice_step)
{
    char zonename[33] = {0x00};
    cgsize_t sizes[9] = {0x00};

    // ゾーン名, ゾーンID
    if (cg_zone_read(index_file, index_base, index_zone, zonename,  sizes) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure:cg_zone_read(index_file=%d,index_base=%d,index_zone=%d)", index_file, index_base, index_zone);
    }
    this->setName(zonename);
    this->setId(index_zone);
    // ノード、セルサイズ
    this->setVertexSize(sizes[0]);
    this->setCellSize(sizes[1]);

    // ゾーンタイプ
    ZoneType_t zonetype;
    if (cg_zone_type(index_file, index_base, index_zone, &zonetype) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure:cg_zone_type");
    }
    this->setZoneType(zonetype);

    // グリッド座標
    if (this->grid_coordinates == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "grid_coordinates is null.");
    }
    if (this->grid_coordinates->readCgns(index_file, index_base, index_zone, timeslice_step) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure:UdmGridCoordinates::readCgns(index_file=%d,index_base=%d,index_zone=%d)", index_file, index_base, index_zone);
    }

    // 要素接続情報
    if (this->sections == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "sections is null.");
    }
    if (this->sections->readCgns(index_file, index_base, index_zone) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "failure:UdmSections::readCgns(index_file=%d,index_base=%d,index_zone=%d)", index_file, index_base, index_zone);
    }

    // 物理量データの読込
    if (this->solutions == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "solutions is null.");
    }
    if (this->solutions->readCgns(index_file, index_base, index_zone, timeslice_step) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure:UdmFlowSolutions::readCgns(index_file=%d,index_base=%d,index_zone=%d)", index_file, index_base, index_zone);
    }

    // ユーザ定義データの読込
    if (this->user_datas == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "user_datas is null.");
    }
    if (this->user_datas->readCgns(index_file, index_base, index_zone) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure:UdmUserDefinedDatas::readCgns(index_file=%d,index_base=%d,index_zone=%d)", index_file, index_base, index_zone);
    }

    return UDM_OK;
}

/**
 * 要素（セル）数を取得する.
 * @return        要素（セル）数
 */
UdmSize_t UdmZone::getCellSize() const
{
    return this->cell_size;
}

/**
 * 要素（セル）数を設定する.
 * @param cell_size        要素（セル）数
 */
void UdmZone::setCellSize(UdmSize_t cell_size)
{
    this->cell_size = cell_size;
}

/**
 * ノード（頂点）数を取得する.
 * @return        ノード（頂点）
 */
UdmSize_t UdmZone::getVertexSize() const
{
    return this->vertex_size;
}

/**
 * ノード（頂点）数を設定する.
 * @param vertex_size        ノード（頂点）
 */
void UdmZone::setVertexSize(UdmSize_t vertex_size)
{
    this->vertex_size = vertex_size;
}

/**
 * クラス情報を文字列出力する:デバッグ用.
 * @param buf        出力文字列
 */
void UdmZone::toString(std::string& buf) const
{
#ifdef _DEBUG_TRACE
    std::stringstream stream;

    // ゾーン名
    stream << "Zone name = " << this->getName()  << std::endl;
    stream << "Zone id = " << this->getId()  << std::endl;
    stream << "ノード（頂点）サイズ = " << this->getVertexSize()  << std::endl;
    stream << "要素（セル）サイズ = " << this->getCellSize()  << std::endl;
    if (this->getZoneType() == Udm_Structured) {
        stream << "ゾーンタイプ = 構造格子(Structured)" << std::endl;
    }
    else if (this->getZoneType() == Udm_Unstructured) {
        stream << "ゾーンタイプ = 非構造格子(Unstructured)" << std::endl;
    }
    else {
        stream << "ゾーンタイプ = 不明" << std::endl;
    }

    // GridCoordinates
    if (this->grid_coordinates != NULL) {
        std::string coords_buf;
        this->grid_coordinates->toString(coords_buf);

        stream << std::endl;
        stream << "GridCoordinates" << std::endl;
        stream << coords_buf;
        stream << std::endl;
    }

    // Elements
    if (this->sections != NULL) {
        std::string elem_buf;
        this->sections->toString(elem_buf);

        stream << std::endl;
        stream << "Elements" << std::endl;
        stream << elem_buf;
        stream << std::endl;

    }

    // UdmRankConnectivity
    if (this->inner_boundary != NULL) {
        std::string inner_buf;
        this->inner_boundary->toString(inner_buf);

        stream << std::endl;
        stream << "RankConnectivity" << std::endl;
        stream << inner_buf;
        stream << std::endl;
    }
    buf += stream.str();
#endif

    return;
}

/**
 * DFI設定情報を取得する.
 * @return        DFI設定情報
 */
UdmDfiConfig* UdmZone::getDfiConfig() const
{
    if (this->getParentModel() == NULL) return NULL;
    return this->getParentModel()->getDfiConfig();
}


/**
 * CGNS:Zoneを出力する:GridCoordinates,Elements,FlowSolution.
 * GridCoordinates,Elements,FlowSolutionを出力する.
 * @param index_file        CGNS:ファイルインデックス
 * @param index_base        CGNS:ベースインデックス
 * @param timeslice_step        時系列ステップ数
 * @param grid_timeslice       CGNS:GridCoordinatesの時系列出力の有無:true=GridCoordinates時系列出力する
 * @param write_constsolutions    固定物理量データの出力フラグ : true=固定物理量データを出力する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::writeCgnsTimeSliceFile(
                        int index_file,
                        int index_base,
                        int timeslice_step,
                        bool grid_timeslice,
                        bool write_constsolutions)
{
    int index_zone = 0;

    // CGNS:Zoneを出力する.
    if (this->writeCgnsZone(index_file, index_base, index_zone) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure : writeCgnsZone");
    }

    // CGNS:GridCoordinatesを書き込む.
    if (this->getGridCoordinates()->writeCgns(index_file, index_base, index_zone, timeslice_step, grid_timeslice) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "getGridCoordinates(index_file=%d, index_base=%d, index_zone=%d, timeslice_step=%d)", index_file, index_base, index_zone, timeslice_step);
    }

    // CGNS:Elementsを書き込む.
    if (this->getSections()->writeCgns(index_file, index_base, index_zone, timeslice_step) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "getSections(index_file=%d, index_base=%d, index_zone=%d, timeslice_step=%d)", index_file, index_base, index_zone, timeslice_step);
    }

    // CGNS:FlowSolutionを書き込む.
    if (this->getFlowSolutions()->writeCgns(index_file, index_base, index_zone, timeslice_step, write_constsolutions) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "getSections(index_file=%d, index_base=%d, index_zone=%d, timeslice_step=%d)", index_file, index_base, index_zone, timeslice_step);
    }

    // ユーザ定義データを出力する.
    // CGNS:UdmUserDefinedDataを書き込む.
    if (this->getUserDefinedDatas()->writeCgns(index_file, index_base, index_zone) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "getSections(index_file=%d, index_base=%d, index_zone=%d)", index_file, index_base, index_zone);
    }

    return UDM_OK;
}


/**
 * CGNS:Zoneを出力する:GridCoordinates,Elements.
 * GridCoordinates,Elementsを出力する.
 * FlowSolutionは出力しない.
 * @param index_file        CGNS:ファイルインデックス
 * @param index_base        CGNS:ベースインデックス
 * @param timeslice_step        時系列ステップ数
 * @param grid_timeslice       CGNS:GridCoordinatesの時系列出力の有無:true=GridCoordinates時系列出力する
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::writeCgnsGridCoordinatesFile(
                        int index_file,
                        int index_base,
                        int timeslice_step,
                        bool grid_timeslice)
{
    int index_zone = 0;

    // CGNS:Zoneを出力する.
    if (this->writeCgnsZone(index_file, index_base, index_zone) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure : writeCgnsZone");
    }

    // CGNS:GridCoordinatesを書き込む.
    if (this->getGridCoordinates()->writeCgns(index_file, index_base, index_zone, timeslice_step, grid_timeslice) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "getGridCoordinates(index_file=%d, index_base=%d, index_zone=%d, timeslice_step=%d)", index_file, index_base, index_zone, timeslice_step);
    }

    // CGNS:Elementsを書き込む.
    if (this->getSections()->writeCgns(index_file, index_base, index_zone, timeslice_step) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "getSections(index_file=%d, index_base=%d, index_zone=%d, timeslice_step=%d)", index_file, index_base, index_zone, timeslice_step);
    }
    return UDM_OK;
}


/**
 * CGNS:Zoneを出力する:Zone作成.
 * Zoneが存在していない場合は作成を行う.
 * 存在している場合は、何もしない.
 * @param [in]  index_file        CGNS:ファイルインデックス
 * @param [in]  index_base        CGNS:ベースインデックス
 * @param [out] index_zone        CGNS:ゾーンインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::writeCgnsZone(int index_file, int index_base, int &index_zone)
{

    char zonename[33] = {0x00}, cgns_zonename[33] = {0x00};
    int num_zones = 0;
    cgsize_t sizes[9] = {0x00};

    if (this->getGridCoordinates() == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "GridCoordinates is null.");
    }
    if (this->getSections() == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "Sections is null.");
    }
    this->setVertexSize(this->getGridCoordinates()->getNumNodes());
    if (this->getVertexSize() <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "vertex size is zero.");
    }
    this->setCellSize(this->getSections()->getNumEntityCells());
    if (this->getCellSize() <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "cell size is zero.");
    }

    // 既にCGNSゾーン名が存在しているかチェックする.
    if (this->getName().empty()) {
        sprintf(zonename, UDM_CGNS_NAME_ZONE, (int)this->getId());
    }
    else {
        sprintf(zonename, "%s", this->getName().c_str());
    }
    index_zone = 0;
    cg_nzones(index_file, index_base, &num_zones);
    for (index_zone=1; index_zone<=num_zones; index_zone++) {
        if (cg_zone_read(index_file, index_base, index_zone, cgns_zonename, sizes) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure : cg_zone_read.");
        }
        if (strcmp(zonename, cgns_zonename) == 0) {
            // 頂点（ノード）、要素（セル）サイズが同じであるかチェックする.
            if (sizes[0] != this->getVertexSize()) {
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "difference of vertex size [cgns:%d != this:%d].", sizes[0], this->getVertexSize());
            }
            if (sizes[1] != this->getCellSize()) {
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "difference of cell size [cgns:%d != this:%d].", sizes[1], this->getCellSize());
            }
            break;
        }
    }
    if (index_zone == 0 || index_zone > num_zones) {
        sizes[0] = this->getVertexSize();
        sizes[1] = this->getCellSize();
        sizes[2] = 0;
        // CGNS:Zoneの作成
        if (cg_zone_write(index_file, index_base, zonename, sizes, Unstructured, &index_zone) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure : cg_zone_write [zone_name=%s].", zonename);
        }
    }

    return UDM_OK;
}


/**
 * CGNS:Zoneを出力する:FlowSolution.
 * FlowSolutionを出力する.
 * GridCoordinates,Elementsは出力しない.
 * @param index_file        CGNS:ファイルインデックス
 * @param index_base        CGNS:ベースインデックス
 * @param timeslice_step        時系列ステップ数
 * @param write_constsolutions    固定物理量データの出力フラグ : true=固定物理量データを出力する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::writeCgnsFlowSolutionFile(
                        int index_file,
                        int index_base,
                        int timeslice_step,
                        bool write_constsolutions)
{
    int index_zone = 0;

    // CGNS:Zoneを出力する.
    if (this->writeCgnsZone(index_file, index_base, index_zone) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure : writeCgnsZone");
    }

    // CGNS:FlowSolutionを書き込む.
    if (this->getFlowSolutions()->writeCgns(index_file, index_base, index_zone, timeslice_step, write_constsolutions) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "getSections(index_file=%d, index_base=%d, index_zone=%d, timeslice_step=%d)", index_file, index_base, index_zone, timeslice_step);
    }

    // ユーザ定義データを出力する.
    // CGNS:UdmUserDefinedDataを書き込む.
    if (this->getUserDefinedDatas()->writeCgns(index_file, index_base, index_zone) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "getSections(index_file=%d, index_base=%d, index_zone=%d)", index_file, index_base, index_zone);
    }

    return UDM_OK;
}

/**
 * CGNS:Zoneを出力する:リンク.
 * リンクを出力する.
 * @param index_file        CGNS:ファイルインデックス
 * @param index_base        CGNS:ベースインデックス
 * @param link_output_path        リンク出力ファイル
 * @param linked_files            リンクファイルリスト
 * @param timeslice_step        時系列ステップ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::writeCgnsLinkFile(
                        int index_file,
                        int index_base,
                        const std::string &link_output_path,
                        const std::vector<std::string> &linked_files,
                        int timeslice_step)
{
    int index_zone = 0;
    char path[256] = {0x00};
    std::vector<std::string>::const_iterator itr;
    std::vector<std::string> solution_paths;
    int n, linked_size;

    if (linked_files.size() <= 0) return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "Link Files size is zero.");

    // CGNS:Zoneを出力する.
    if (this->writeCgnsZone(index_file, index_base, index_zone) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure : writeCgnsZone");
    }

    // CGNS:GridCoordinatesリンクを書き込む.
    if (this->getGridCoordinates()->writeCgnsLinkFile(
                        index_file, index_base, index_zone,
                        link_output_path, linked_files,
                        timeslice_step) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "failure : writeCgnsLinkFile");
    }

    // CGNS:Elementsリンクを書き込む.
    if (this->getSections()->writeCgnsLinkFile(
                        index_file, index_base, index_zone,
                        link_output_path, linked_files,
                        timeslice_step) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "failure : writeCgnsLinkFile");
    }

    // CGNS:FlowSolutionリンクを書き込む.
    if (this->getFlowSolutions()->writeCgnsLinkFile(
                        index_file, index_base, index_zone,
                        link_output_path, linked_files,
                        timeslice_step) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure : writeCgnsLinkFile");
    }

    // CGNS:UdmUserDefinedDataリンクを書き込む.
    if (this->getUserDefinedDatas()->writeCgnsLinkFile(
                        index_file, index_base, index_zone,
                        link_output_path, linked_files) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure : writeCgnsLinkFile");
    }

    return UDM_OK;
}


/**
 * CGNS出力前の初期化を行う.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::initializeWriteCgns()
{
    // CGNS:GridCoordinates出力前の初期化を行う.
    if (this->getGridCoordinates()->initializeWriteCgns() != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure : UdmGridCoordinates::initializeWriteCgns()");
    }

    // CGNS:Elements出力前の初期化を行う.
    if (this->getSections()->initializeWriteCgns() != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure : UdmSections::initializeWriteCgns()");
    }

    // CGNS:FlowSolution出力前の初期化を行う.
    if (this->getFlowSolutions()->initializeWriteCgns() != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure : UdmFlowSolutions::initializeWriteCgns()");
    }
    return UDM_OK;
}


/**
 * 出力ステップ回数を取得する.
 * @return        出力ステップ回数
 */
int UdmZone::getNumCgnsIterativeDatas() const
{
    int count = 0;
    if (this->getGridCoordinates() != NULL) {
        count = this->getGridCoordinates()->getNumCgnsWriteGridCoordnates();
    }
    if (this->getFlowSolutions() != NULL) {
        // CGNS:FlowSolutionの出力数を取得する.
        int sol_count = this->getFlowSolutions()->getNumCgnsWriteSolutions();
        if (count < sol_count) {
            count = sol_count;
        }
    }

    return count;
}


/**
 * CGNS:ZoneIterativeDataを出力する.
 * ZoneIterativeDataが存在していなければ作成する。
 * 存在していれば、既存時系列に追加を行う.
 * @param index_file            CGNSファイルインデックス
 * @param index_base            CGNSベースインデックス
 * @param timeslice_step        時系列ステップ番号
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::writeCgnsIterativeDatas(int index_file, int index_base, int timeslice_step)
{
    int index_zone = 0;
    int narrays = 0;
    int i, n;
    char name[33] = {0x00};
    DataType_t cgns_datatype;
    int data_dimension;
    cgsize_t dimension_vector[3];
    char* gridcoordinates_pointers = NULL, *flowsolution_pointers = NULL;
    int before_grid_count = 0, before_solution_count = 0;
    UdmError_t error;
    int count;
    int solution_count = 0;
    int grid_count = 0;
    bool exists_solution = false;
    bool exists_grid = false;
    std::vector<int> iterative_steps;
    std::vector<float> iterative_times;
    std::vector<std::string> iterative_grids;
    std::vector<std::string> iterative_solutions;

    // CGNS:Zoneを出力する.
    if (this->writeCgnsZone(index_file, index_base, index_zone) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure : writeCgnsZone");
    }

    // CGNS:ゾーン時系列データの存在チェック : cg_ziter_write
    if (!this->existsCgnsNode(index_file,index_base,index_zone,"ZoneIterativeData")) {
        // CGNS:ゾーン時系列データの書込 : cg_ziter_write
        if (cg_ziter_write(index_file,index_base,index_zone,"ZoneIterativeData") != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure : cg_ziter_write");
        }
    }

    // GridCoordinatesの出力済みチェック
    grid_count = 0;
    cg_ngrids(index_file,index_base, index_zone, &grid_count);

    // FlowSolution_tの出力済みチェック
    solution_count = 0;
    cg_nsols(index_file,index_base, index_zone, &solution_count);

    // ベースの時系列ステップ番号を取得する.
    int steps = this->getParentModel()->readCgnsBaseIterativeDatas(index_file, index_base, iterative_steps, iterative_times);

    // ゾーンの時系列データ(ZoneIterativeData)を取得する.
    error = this->readCgnsZoneIterativeDatas(index_file, index_base, index_zone, iterative_grids, iterative_solutions);
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }
    if (iterative_grids.size() > 0 && iterative_solutions.size() > 0) {
        if (iterative_grids.size() != iterative_solutions.size()) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "not equals GridCoordinatesPointers size(=%d) and FlowSolutionPointers size(=%d).", iterative_grids.size(), iterative_solutions.size());
        }
    }
    if (iterative_grids.size() > 0) {
        if (iterative_steps.size() != iterative_grids.size()) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "not equals BaseIterativeData size(=%d) and GridCoordinatesPointers size(=%d).", iterative_steps.size(), iterative_grids.size());
        }
    }
    if (iterative_solutions.size() > 0) {
        if (iterative_steps.size() != iterative_solutions.size()) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "not equals BaseIterativeData size(=%d) and FlowSOlutionPointers size(=%d).", iterative_steps.size(), iterative_solutions.size());
        }
    }

    // 時系列ステップ番号のゾーンの時系列データ（名称）を削除する.
    for (n=iterative_steps.size()-1; n>=0; n--) {
        if (iterative_steps[n] == timeslice_step) {
            std::vector<std::string>::iterator del_itr;
            if (iterative_grids.size() > n) {
                del_itr = iterative_grids.begin()+n;
                iterative_grids.erase(del_itr);
                }
            if (iterative_solutions.size() > n) {
                del_itr = iterative_solutions.begin()+n;
                iterative_solutions.erase(del_itr);
            }
        }
    }

    // 出力FlowSolution名を追加する.
    if (this->getFlowSolutions() != NULL && solution_count > 0) {
        const std::vector<std::string> solution_names = this->getFlowSolutions()->getCgnsWriteSolutions();
        if (solution_names.size() > 0) {
            int previous_size = iterative_solutions.size();
            for (n=0; n<iterative_steps.size(); n++) {
                // 時系列ステップ番号位置にFlowSolution名を追加する.
                if (iterative_steps[n] >= timeslice_step) {
                    std::vector<std::string>::const_iterator itr;
                    std::vector<std::string>::iterator ins_itr;
                    ins_itr = iterative_solutions.begin()+n;
                    for (itr=solution_names.begin(); itr!=solution_names.end(); itr++) {
                        iterative_solutions.insert(ins_itr++, (*itr));
                    }
                    break;
                }
            }
            // 未挿入の場合は、最後に挿入する
            if (previous_size == iterative_solutions.size()) {
                std::vector<std::string>::const_iterator itr;
                for (itr=solution_names.begin(); itr!=solution_names.end(); itr++) {
                    iterative_solutions.push_back(*itr);
                }
            }
        }
        solution_count = solution_names.size();
    }

    // 出力GridCoordinates名を追加する
    if (this->getGridCoordinates() != NULL && grid_count > 0) {
        count = this->getGridCoordinates()->getNumCgnsWriteGridCoordnates();
        if (solution_count > 0) {
            count = solution_count;
        }
        if (count > 0) {
            const std::string grid_name = this->getGridCoordinates()->getCgnsWriteGridCoordnates();
            std::string write_name = grid_name;
            if (write_name.empty()) {
                write_name = UDM_CGNS_NAME_GRIDCOORDINATES;
            }
            int previous_size = iterative_grids.size();
            for (n=0; n<iterative_steps.size(); n++) {
                // 時系列ステップ番号位置にGridCoordinates名を追加する.
                if (iterative_steps[n] >= timeslice_step) {
                    std::vector<std::string>::iterator ins_itr;
                    ins_itr = iterative_grids.begin()+n;
                    for (i=0; i<count; i++) {
                        iterative_grids.insert(ins_itr++, write_name);
                    }
                    break;
                }
            }
            // 未挿入の場合は、最後に挿入する
            if (previous_size == iterative_grids.size()) {
                for (i=0; i<count; i++) {
                    iterative_grids.push_back(write_name);
                }
            }
        }
    }

    if (cg_goto(index_file,index_base,"Zone_t",index_zone,"ZoneIterativeData",0,"end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure : cg_goto(Zone_t/ZoneIterativeData)");
    }

    // FlowSolution出力
    if (iterative_solutions.size() > 0) {
        flowsolution_pointers = new char[iterative_solutions.size()*32];
        memset(flowsolution_pointers, ' ', iterative_solutions.size()*32*sizeof(char));
        for (n=0; n<iterative_solutions.size(); n++) {
            memcpy(flowsolution_pointers+n*32, iterative_solutions[n].c_str(), iterative_solutions[n].length());
        }
        dimension_vector[0] = 32;
        dimension_vector[1] = iterative_solutions.size();
        // 時系列FlowSolutionの書込 : "FlowSolutionPointers"
        if (cg_array_write("FlowSolutionPointers",Character,2,dimension_vector,flowsolution_pointers) != CG_OK) {
            error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "failue : cg_array_write(FlowSolutionPointers).");
        }
    }

    // GridCoordinatesPointersの出力
    if (iterative_grids.size() > 0) {
        gridcoordinates_pointers = new char[iterative_grids.size()*32];
        memset(gridcoordinates_pointers, ' ', iterative_grids.size()*32*sizeof(char));
        for (n=0; n<iterative_grids.size(); n++) {
            memcpy(gridcoordinates_pointers+n*32, iterative_grids[n].c_str(), iterative_grids[n].length());
        }
        dimension_vector[0] = 32;
        dimension_vector[1] = iterative_grids.size();
        // 時系列GridCoordinates名の書込 : "GridCoordinatesPointers"
        if (cg_array_write("GridCoordinatesPointers",Character,2,dimension_vector,gridcoordinates_pointers) != CG_OK) {
            error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "failue : cg_array_write(GridCoordinatesPointers).");
        }
    }

    if (gridcoordinates_pointers != NULL) delete []gridcoordinates_pointers;
    if (flowsolution_pointers != NULL) delete []flowsolution_pointers;

    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    return UDM_OK;
}

/**
 * CGNS:Zone配下にCGNSノード名が存在するかチェックする.
 * @param index_file            CGNSファイルインデックス
 * @param index_base            CGNSベースインデックス
 * @param index_zone            CGNSゾーンインデックス
 * @param node_name                CGNSノード名
 * @return            true=CGNSノード名が存在する.
 */
bool UdmZone::existsCgnsNode(int index_file, int index_base, int index_zone, const std::string& node_name) const
{
    if (cg_goto(index_file, index_base, "Zone_t", index_zone, node_name.c_str(), 0, "end") != CG_OK) {
        return false;
    }

    return true;
}

/**
 * MPIコミュニケータを取得する.
 * @return        MPIコミュニケータ
 */
MPI_Comm UdmZone::getMpiComm() const
{
    if (this->getParentModel() == NULL) return MPI_COMM_NULL;
    return this->getParentModel()->getMpiComm();
}


/**
 * MPIランク番号を取得する.
 * @return        MPIランク番号
 */
int UdmZone::getMpiRankno() const
{
    if (this->getParentModel() == NULL) return -1;
    return this->getParentModel()->getMpiRankno();
}

/**
 * MPIプロセス数を取得する.
 * @return        MPIプロセス数
 */
int UdmZone::getMpiProcessSize() const
{
    if (this->getParentModel() == NULL) return -1;
    return this->getParentModel()->getMpiProcessSize();
}


/**
 * Zoltan分割によるインポート要素（セル）を追加する.
 * @param [in] import_cells        インポート要素（セル）リスト
 * @param [out] import_nodes        インポート節点（ノード）リスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::importCells(
                const std::vector<UdmCell*>& import_cells,
                std::vector<UdmNode*>& import_nodes)
{
#ifdef _DEBUG_TRACE
    UDM_DEBUG_PRINTF("%s:%d [%s]",  __FILE__, __LINE__, __FUNCTION__);
#endif
#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START(__FUNCTION__);
#endif

    UdmError_t error = UDM_OK;

    if (this->getSections() == NULL) return UDM_ERROR;
    if (this->getGridCoordinates() == NULL) return UDM_ERROR;
    if (this->getRankConnectivity() == NULL) return UDM_ERROR;

#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        std::vector<UdmCell*>::const_iterator cell_itr;
        int n = 0;
        int count = 0;
        for (cell_itr=import_cells.begin(); cell_itr!=import_cells.end(); cell_itr++) {
            UdmCell* cell = (*cell_itr);
            count++;
            UDM_DEBUG_PRINTF("[rank:%d] Import Cell %d [cell_id=%d, rankno=%d] \n", this->getMpiRankno(), count, cell->getId(), cell->getMyRankno());
            unsigned int num_node = cell->getNumNodes();
            for (n=1; n<=num_node; n++) {
                UdmNode *node = cell->getNode(n);
                UDM_DEBUG_PRINTF("    [rank:%d] Import Node %d [node_id=%d, rankno=%d] \n", this->getMpiRankno(), n, node->getId(), node->getMyRankno());
            }
        }
    }
#endif

    // ランク番号,ID検索テーブルをクリアする。
    this->getRankConnectivity()->clearSearchTable();

    // インポート要素（セル）の節点（ノード）リストの取得：GridCoordinatesに追加するリスト
    unsigned int n;
    UdmRankConnectivity *inner = this->getRankConnectivity();
    std::vector<UdmCell*>::const_iterator itr;
    std::vector<UdmNode*>::const_iterator find_itr;
    std::vector<UdmNode*>::iterator ins_itr;
    UdmNode *dest_node = NULL;
    UdmGlobalRankidList import_globalids;
    UdmSize_t array_size = 0;
    for (itr=import_cells.begin(); itr!=import_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        unsigned int num_node = cell->getNumNodes();
        array_size += num_node;
    }
    import_nodes.reserve(array_size);
    import_globalids.reserve(array_size);

    // 内部境界節点（ノード）のランク番号、ID検索テーブルを作成する。
    inner->createSearchTable();

    for (itr=import_cells.begin(); itr!=import_cells.end(); itr++) {
        UdmCell* cell = (*itr);
        unsigned int num_node = cell->getNumNodes();
        for (n=1; n<=num_node; n++) {
            UdmNode *import_node = cell->getNode(n);
            UdmSize_t node_id = import_node->getId();
            int rankno = import_node->getMyRankno();

            // 未追加であれば挿入する。
            // UdmNode *find_node = UdmNode::findMpiRankInfo(import_nodes, import_node->getMyRankno(), import_node->getId());
            // if (find_node != NULL) continue;
            if (import_globalids.existsGlobalRankid(import_node->getMyRankno(), import_node->getId())) {
                continue;
            }
            // 共有節点（ノード）を検索する
            dest_node = inner->findMpiRankInfo(import_node);
            if (dest_node != NULL) {
                // 未追加であれば挿入する。
                // find_node = UdmNode::findMpiRankInfo(import_nodes, dest_node->getMyRankno(), dest_node->getId());
                // if (find_node != NULL) continue;
                if (import_globalids.existsGlobalRankid(dest_node->getMyRankno(), dest_node->getId())) {
                    continue;
                }
            }

            if (dest_node == NULL) {
                // インポート節点（ノード）は使用せず、コピーを作成する.
                dest_node = new UdmNode();
                dest_node->cloneNode(*import_node);
            }
            // 接続MPIランク情報を削除する
            //dest_node->clearMpiRankInfos();

            // ID,ランク番号を以前のID,ランク番号に設定する.
            dest_node->addMpiRankInfo(import_node->getMyRankno(), import_node->getId());
            dest_node->addPreviousRankInfo(import_node->getMyRankno(), import_node->getId());

            // グローバルID（ID+ランク番号）順番に挿入する.
            find_itr = this->searchUpperGlobalId(import_nodes, dest_node);
            if (find_itr == import_nodes.end()) {
                import_nodes.push_back(dest_node);
            }
            else {
                ins_itr = import_nodes.begin() + (find_itr - import_nodes.begin());
                import_nodes.insert(ins_itr, dest_node);
            }

            // インポートグローバルIDリストに挿入する
          this->insertImportGlobalRankids(import_globalids, dest_node);
        }
    }

#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        int count = 0;
        std::vector<UdmNode*>::const_iterator trace_itr;
        for (trace_itr=import_nodes.begin(); trace_itr!=import_nodes.end(); trace_itr++) {
            UdmNode* node = (*trace_itr);
            count++;
            UDM_DEBUG_PRINTF("[rank:%d] Insert Node %d [node_id=%d, rankno=%d] \n", this->getMpiRankno(), count, node->getId(), node->getMyRankno());
        }
    }
#endif

    // 要素（セル）の追加
    error = this->getSections()->importCells(import_cells, import_nodes);
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    // 節点（ノード）の追加
    error = this->getGridCoordinates()->importNodes(import_nodes);
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }
#ifdef _DEBUG
    printf("[rank=%d] UdmZone::importCells getGridCoordinates()->importNodes(import_nodes_size=%d) = node_size=%ld \n",
            this->getMpiRankno(),
            import_nodes.size(),
            this->getGridCoordinates()->getNumNodes()
            );
#endif

    // 内部境界の追加：インポート節点（ノード）はすべてが内部境界ではないが、一時的に内部境界として追加する.
    // UdmRankConnectivity::migrationBoundary()にて不要節点（ノード）は削除する.
    std::vector<UdmNode*>::iterator node_itr;
    for (node_itr=import_nodes.begin(); node_itr!=import_nodes.end(); node_itr++) {
        if ((*node_itr)->getNumMpiRankInfos() > 0) {
            this->getRankConnectivity()->insertRankConnectivityNode(*node_itr);
        }
    }

    // 部品要素を作成して、内部境界ではない節点（ノード）を削除する.
    // 部品要素の作成
    error = this->createComponentCells(import_nodes);
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    // 内部境界ではない節点（ノード）を削除する.
    inner->eraseRankConnectivity();

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_STOP(__FUNCTION__);
#endif
    return UDM_OK;
}

/**
 * 仮想要素（セル）、仮想節点（ノード）のインポートを行う.
 * @param [in]  import_virtuals        仮想要素（セル）
 * @param [out] import_nodes        インポート節点（ノード）リスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::importVirturalCells(
                const std::vector<UdmCell*>& import_virtuals,
                std::vector<UdmNode*>& import_nodes)
{

    UdmError_t error = UDM_OK;
    int myrank = this->getMpiRankno();

    if (this->getSections() == NULL) return UDM_ERROR;
    if (this->getGridCoordinates() == NULL) return UDM_ERROR;
    if (this->getRankConnectivity() == NULL) return UDM_ERROR;

    if (import_virtuals.size() <= 0) return UDM_OK;

    // インポート仮想要素（セル）の仮想節点（ノード）リストの取得：GridCoordinatesに追加するリスト
    unsigned int n;
    std::vector<UdmNode*> virtual_nodes;
    UdmRankConnectivity *inner = this->getRankConnectivity();
    std::vector<UdmCell*>::const_iterator itr;
    std::vector<UdmNode*>::const_iterator find_itr;
    std::vector<UdmNode*>::iterator ins_itr;
    const std::vector<UdmNode*> current_virtual_nodes = this->getGridCoordinates()->getVirtualNodes();
    UdmNode *dest_node = NULL;
    for (itr=import_virtuals.begin(); itr!=import_virtuals.end(); itr++) {
        UdmCell* virtual_cell = (*itr);
        unsigned int num_node = virtual_cell->getNumNodes();
        for (n=1; n<=num_node; n++) {
            dest_node = NULL;
            UdmNode *virtual_node = virtual_cell->getNode(n);
            int virtual_rankno = virtual_node->getMyRankno();
            UdmSize_t virtual_nodeid = virtual_node->getId();
            if (myrank == virtual_rankno) {
                // 自身の節点（ノード）
                dest_node = this->getGridCoordinates()->findNodeByGlobalId(virtual_node->getMyRankno(), virtual_node->getId());
                if (dest_node != NULL) {
                    // 未追加であれば挿入する。
                    find_itr = this->searchGlobalId(virtual_nodes, dest_node);
                    if (find_itr != virtual_nodes.end()) continue;
                }
            }

            if (dest_node == NULL) {
                // 未追加であれば挿入する。
                find_itr = this->searchGlobalId(virtual_nodes, virtual_node);
                if (find_itr != virtual_nodes.end()) continue;

                // 既存仮想節点（ノード）が存在するかチェックする.
                find_itr  = this->searchGlobalId(current_virtual_nodes, virtual_node);
                if (find_itr != current_virtual_nodes.end()) {
                    dest_node = *find_itr;
                    dest_node->cloneNode(*virtual_node);
                    dest_node->setRealityType(Udm_Virtual);
                }
                if (dest_node == NULL) {
                    // インポート節点（ノード）のrankno, idと一致する内部境界節点（ノード）を検索する
                    dest_node = inner->findMpiRankInfo(virtual_node->getMyRankno(), virtual_node->getId());
                }
                if (dest_node == NULL) {
                    // インポート節点（ノード）は使用せず、コピーを作成する.
                    dest_node = new UdmNode();
                    dest_node->cloneNode(*virtual_node);
                    // 内部境界節点（ノード）ではないので、仮想節点（ノード）である。
                    dest_node->setRealityType(Udm_Virtual);
                }
                else {
                    // 未追加であれば挿入する。
                    find_itr = this->searchGlobalId(virtual_nodes, dest_node);
                    if (find_itr != virtual_nodes.end()) continue;
                }
            }

            // グローバルID（ID+ランク番号）順番に挿入する.
            find_itr = this->searchUpperGlobalId(virtual_nodes, dest_node);
            if (find_itr == virtual_nodes.end()) {
                virtual_nodes.push_back(dest_node);
            }
            else {
                ins_itr = virtual_nodes.begin() + (find_itr - virtual_nodes.begin());
                virtual_nodes.insert(ins_itr, dest_node);
            }
        }
    }

    // 仮想要素（セル）の追加
    error = this->getSections()->importVirtualCells(import_virtuals, virtual_nodes);
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    // 仮想節点（ノード）の追加
    error = this->getGridCoordinates()->importVirtualNodes(virtual_nodes);
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    import_nodes = virtual_nodes;

    return UDM_OK;
}


/**
 * Zoltan分割によるエクスポート要素（セル）を削除する.
 * @param [in]  export_cells        エクスポート要素（セル）リスト
 * @param [out] export_nodes        エクスポート節点（ノード）リスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::exportCells(
                        const std::vector<UdmCell*>& export_cells,
                        std::vector<UdmEntity*>& export_nodes)
{
    if (this->getSections() == NULL) return UDM_ERROR;
    if (this->getGridCoordinates() == NULL) return UDM_ERROR;
    if (this->getRankConnectivity() == NULL) return UDM_ERROR;

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START(__FUNCTION__);
#endif

    // ランク番号,ID検索テーブルをクリアする。
    this->getRankConnectivity()->clearSearchTable();

    // 部品要素の削除
    this->getSections()->clearComponentCells();

    // 削除要素（セル）の節点（ノード）リストの取得
    UdmScannerCells scanner_cells;
    scanner_cells.scannerElements(export_cells);
    UdmSize_t node_size = scanner_cells.getScanEntities(export_nodes);

    // エクスポート要素（セル）の削除
    this->getSections()->removeCells(export_cells);

    // ノードの削除
    std::vector<UdmEntity*>::iterator node_itr;
    std::vector<UdmNode*> remove_node;
    std::vector<UdmEntity*> inner_node;
    for (node_itr=export_nodes.begin(); node_itr!=export_nodes.end(); node_itr++) {
        UdmNode* node = static_cast<UdmNode*>(*node_itr);
        if (node != NULL) {
            if (node->getNumParentCells() <= 0) {
                remove_node.push_back(node);    // 削除ノード
            }
            else {
                inner_node.push_back(node);        // 内部境界ノード
            }
        }
    }
    // グリッド座標クラスから節点（ノード）を削除する.
    this->getGridCoordinates()->removeNodes(remove_node);
    export_nodes.clear();
    export_nodes = inner_node;

#if 0
    if (node_size > 0) {
        long long i;
        for (i=node_size-1; i>=0; i--) {
            const UdmEntity* node = export_nodes[i];
            if (node->getNumParentCells() <= 0) {
                // 節点（ノード）の削除
                this->getGridCoordinates()->removeNode(node);
                export_nodes.erase(export_nodes.begin()+i);
            }
        }
    }
#endif

    // 内部境界の追加：エクスポート節点（ノード）の削除残は内部境界である.
    for (node_itr=export_nodes.begin(); node_itr!=export_nodes.end(); node_itr++) {
        UdmNode* node = static_cast<UdmNode*>(*node_itr);
        // 自ランク番号,IDをMPI情報に追加する
        node->addMpiRankInfo(node->getMyRankno(), node->getId());
        // 内部境界に追加
        this->getRankConnectivity()->insertRankConnectivityNode(node);
    }

    // 部品要素の作成
    this->getSections()->createComponentCells();

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_STOP(__FUNCTION__);
#endif
    return UDM_OK;
}

/**
 * ゾーンの基本情報のシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmZone::serialize(UdmSerializeArchive& archive) const
{
    // UdmGeneral
    // ゾーンID
    // データ型
    // ゾーン名
    this->serializeGeneralBase(archive, this->getId(), this->getDataType(), this->getName());

    // ゾーンタイプ
    archive.write(this->getZoneType(), sizeof(UdmZoneType_t));

    return archive;
}


/**
 * ゾーンの基本情報のデシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmZone::deserialize(UdmSerializeArchive& archive)
{
    UdmSize_t zone_id;
    UdmDataType_t zone_datatype;
    std::string zone_name;

    // UdmGeneral
    // ゾーンID
    // データ型
    // ゾーン名
    this->deserializeGeneralBase(archive, zone_id, zone_datatype, zone_name);
    // ID : ゾーンID
    this->setId(zone_id);
    // データ型
    this->setDataType(zone_datatype);
    // ゾーン名
    this->setName(zone_name);

    // ゾーンタイプ
    archive.read(this->zone_type, sizeof(UdmZoneType_t));

    return archive;
}

/**
 * ゾーンの基本情報を全プロセスに送信する.
 * すべてのプロセスにて同一モデル、ゾーンが存在するかチェックする.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::broadcastZone()
{
    int n;
    int mpi_flag;
    UdmError_t error = UDM_OK;

    // MPI初期化済みであるかチェックする.
    udm_mpi_initialized(&mpi_flag);
    if (!mpi_flag) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_MPI, "Please execute MPI_Init beforehand.");
    }
    if (!(this->getMpiRankno() >= 0 && this->getMpiProcessSize() >= 1)) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_MPI, "invalid mpi process [mpi_rankno=%d,mpi_num_process=%d].", this->getMpiRankno(), this->getMpiProcessSize());
    }

    int mpi_rankno = this->getMpiRankno();
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
    UdmZone mpi_zone;
    streamBuffer.initialize(buf, buf_size);
    archive >> mpi_zone;
    if (buf != NULL) {
        delete []buf;
        buf = NULL;
    }

    // ゾーンの作成済みであるかチェックする.
    if (this->getId() == 0) {
        // ゾーン未作成であるので、ゾーンを作成する.
        error = this->cloneZone(mpi_zone);
        if (error != UDM_OK) {
            UDM_ERRORNO_HANDLER(error);
        }
    }
    // ゾーンが存在するので、ゾーンが同一であるかチェックする.
    else if (!this->equalsZone(mpi_zone)) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "not equals zone(CGNS:ZONE).");
    }

    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }

    // グリッド座標情報の基本情報を全プロセスに送信する.
    error = this->grid_coordinates->brodcastGridCoordinates();
    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }

    // セクション（要素構成）情報の基本情報を全プロセスに送信する.
    error = this->sections->brodcastSections();
    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }

    return UDM_OK;
}

/**
 * ゾーンをコピーする.
 * @param src        コピー元ゾーン
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::cloneZone(const UdmZone& src)
{
    // ID : ゾーンID
    this->setId(src.getId());
    // データ型
    this->setDataType(src.getDataType());
    // ゾーン名
    this->setName(src.getName());
    // ゾーンタイプ
    this->setZoneType(src.getZoneType());

    return UDM_OK;
}

/**
 * 同一ゾーンであるかチェックする.
 * ID,ゾーン名,ゾーンタイプ等の基本情報が同じであるかチェックする.
 * @param model        チェック対象ゾーン
 * @return            true=同一
 */
bool UdmZone::equalsZone(const UdmZone &zone) const
{
    if (this->getId() != zone.getId()) return false;
    if (this->getName() != zone.getName()) return false;
    if (this->getZoneType() != zone.getZoneType()) return false;

    return true;
}

/**
 * ノード（頂点）サイズを設定し直す.
 * GrdiCoordinatesから再取得する.
 */
void UdmZone::updateVertexSize()
{
    if (this->getGridCoordinates() == NULL) {
        this->vertex_size = 0;
    }
    this->vertex_size = this->getGridCoordinates()->getNumNodes();
}

/**
 * 要素（セル）サイズを設定し直す.
 * Elementsから再取得する.
 */
void UdmZone::updateCellSize()
{
    if (this->getSections() == NULL) {
        this->cell_size = 0;
    }
    this->cell_size = this->getSections()->getNumEntityCells();
}

/**
 * 内部境界情報を取得する:const.
 * @return        内部境界情報
 */
UdmRankConnectivity* UdmZone::getRankConnectivity() const
{
    return this->inner_boundary;
}

/**
 * ゾーンの再構成、部品要素の再作成を行う.
 * @param import_nodes        再構成対象節点（ノード）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::rebuildZone(const std::vector<UdmNode*> import_nodes)
{
#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START("UdmZone::rebuildZone");
#endif

    UdmError_t error;
    if (import_nodes.size() > 0) {
        // 再構成対象節点（ノード）の親要素（セル）を探索し、部品要素を再作成する.
        error = this->createComponentCells(import_nodes);
    }
    else {
        // 部品要素を作成する.
        error = this->createComponentCells();
    }
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    // 節点（ノード）IDを再構成する.
    if (this->getGridCoordinates() != NULL) {
        error = this->getGridCoordinates()->rebuildNodes();
        if (error != UDM_OK) {
            return UDM_ERRORNO_HANDLER(error);
        }
        error = this->getGridCoordinates()->rebuildVirtualNodes();
        if (error != UDM_OK) {
            return UDM_ERRORNO_HANDLER(error);
        }
    }

    // 要素（セル）IDを再構成する.
    if (this->getSections() != NULL) {
        error = this->getSections()->rebuildCellId();
        if (error != UDM_OK) {
            return UDM_ERRORNO_HANDLER(error);
        }
    }

    // 変更されたIDをすべてのプロセスに転送する
    if (this->getRankConnectivity() != NULL) {
        // 内部境界節点（ノード）をソートする.
        error = this->getRankConnectivity()->rebuildRankConnectivity();

        // 変更されたIDの転送する
        error = this->getRankConnectivity()->transferUpdatedIds();
        if (error != UDM_OK) {
            return UDM_ERRORNO_HANDLER(error);
        }
    }

    // 節点（ノード）数、要素（セル）数を更新する.
    this->updateVertexSize();
    this->updateCellSize();

    // 以前のランクID、IDをクリアする.
    this->clearPreviousInfos();

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_STOP("UdmZone::rebuildZone");
#endif
    return UDM_OK;
}

/**
 * CGNS:要素（セル）次元数を取得する.
 * @return        要素（セル）次元数
 */
int UdmZone::getCellDimension() const
{
    if (this->getParentModel() == NULL) return 0;
    return this->getParentModel()->getCellDimension();
}


/**
 * ユーザ定義データ管理クラスを取得する.
 * @return        ユーザ定義データ管理クラス
 */
UdmUserDefinedDatas* UdmZone::getUserDefinedDatas() const
{
    return this->user_datas;
}

/**
 * ドメイン、ランク情報を更新する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::updateProcessConfigs()
{
    if (this->getGridCoordinates() == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "GridCoordinates is null.");
    }
    if (this->getSections() == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "Sections is null.");
    }
    if (this->getDfiConfig() == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "DfiConfig is null.");
    }

    int my_rankno = this->getMpiRankno();
    int process_size = this->getMpiProcessSize();

    // 自節点（ノード）数＋自ランク番号以上と接続している節点（ノード）数を取得する
    UdmSize_t num_overnodes = this->getGridCoordinates()->getNumNodesWithoutLessRankno();
    // 要素数を取得する.
    UdmSize_t num_elems = this->getSections()->getNumEntityCells();

    // ドメイン全体の節点（ノード）数を取得する : ランク番号０に収集する
    UdmSize_t num_domain_nodes = 0;
    UdmSize_t send_buf = num_overnodes;
    udm_mpi_reduce(&send_buf, &num_domain_nodes, 1, udm_udmSize_to_mpiType(), MPI_SUM, 0, this->getMpiComm());

    // ドメイン全体の要素（セル）数を取得する : ランク番号０に収集する
    UdmSize_t num_domain_cells = 0;
    send_buf = num_elems;
    udm_mpi_reduce(&send_buf, &num_domain_cells, 1, udm_udmSize_to_mpiType(), MPI_SUM, 0, this->getMpiComm());

    // ランク別の節点（ノード）数
    UdmSize_t num_nodes = this->getGridCoordinates()->getNumNodes();
    UdmSize_t *rank_nodes = new UdmSize_t[process_size];
    memset(rank_nodes, 0x00, process_size*sizeof(UdmSize_t));
    send_buf = num_nodes;
    udm_mpi_gather(&send_buf, 1, udm_udmSize_to_mpiType(), rank_nodes, 1, udm_udmSize_to_mpiType(), 0, this->getMpiComm());

    // ランク別の要素（セル）数
    UdmSize_t *rank_elems = new UdmSize_t[process_size];
    memset(rank_elems, 0x00, process_size*sizeof(UdmSize_t));
    send_buf = num_elems;
    udm_mpi_gather(&send_buf, 1, udm_udmSize_to_mpiType(), rank_elems, 1, udm_udmSize_to_mpiType(), 0, this->getMpiComm());

    // 並列情報：MPI
    MPI_Group mpi_group;
    int mpi_group_size = 0;
    udm_mpi_comm_group(this->getMpiComm(), &mpi_group );
    udm_mpi_group_size(mpi_group, &mpi_group_size);

    // ランク番号０のみ設定値を更新する.
    if (my_rankno == 0) {
        // ドメイン情報：Domain
        this->getDfiConfig()->setDomainConfig(this->getCellDimension(), num_domain_nodes, num_domain_cells);

        // 並列情報：MPI
        this->getDfiConfig()->setMpiConfig(process_size, mpi_group_size);

        // プロセス情報：Process
        this->getDfiConfig()->setProcessConfig(process_size, rank_nodes, rank_elems);
    }

    delete []rank_nodes;
    delete []rank_elems;

    return UDM_OK;
}

/**
 * 物理量情報を取得する
 * @param [in]  solution_name        物理量名称
 * @param [out] data_type
 * @param [out] grid_location
 * @param [out] vector_type
 * @param [out] nvector_size
 * @param [out] constant_flag
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::getSolutionInfo(
                        const std::string& solution_name,
                        UdmDataType_t& data_type,
                        UdmGridLocation_t& grid_location,
                        UdmVectorType_t& vector_type,
                        int& nvector_size,
                        bool& constant_flag) const
{
    if (this->getFlowSolutions() == NULL) return UDM_ERROR;
    const UdmFlowSolutions* solutions = this->getFlowSolutions();
    const UdmSolutionFieldConfig* config = solutions->getSolutionField(solution_name);
    if (config == NULL) return UDM_ERROR;

    data_type = config->getDataType();
    grid_location = config->getGridLocation();
    vector_type = config->getVectorType();
    nvector_size = config->getNvectorSize();
    constant_flag = config->isConstantFlag();

    return UDM_OK;
}

/**
 * 節点（ノード）数を取得する.
 * @return        節点（ノード）数
 */
UdmSize_t UdmZone::getNumNodes() const
{
    if (this->getGridCoordinates() == NULL) return 0;
    return this->getGridCoordinates()->getNumNodes();
}

/**
 * 要素（セル）数を取得する.
 * @return        要素（セル）数
 */
UdmSize_t UdmZone::getNumCells() const
{
    if (this->getSections() == NULL) return 0;
    return this->getSections()->getNumEntityCells();
}

/**
 * 節点（ノード）の物理量データ値を取得する:スカラデータ.
 * @param [in]  node_id        節点（ノード）ID（=１～）
 * @param [in]  solution_name        物理量名
 * @param [out]  value                物理量データ値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmZone::getSolutionScalarInNode(
                    UdmSize_t node_id,
                    const std::string& solution_name,
                    VALUE_TYPE& value) const
{
    if (this->getGridCoordinates() == NULL) return UDM_ERROR;
    UdmEntity *entity = this->getGridCoordinates()->getNodeById(node_id);
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
unsigned int UdmZone::getSolutionVectorinNode(
                    UdmSize_t node_id,
                    const std::string& solution_name,
                    VALUE_TYPE* values) const
{
    if (this->getGridCoordinates() == NULL) return 0;
    UdmEntity *entity = this->getGridCoordinates()->getNodeById(node_id);
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
UdmError_t UdmZone::setSolutionScalarInNode(
                        UdmSize_t node_id,
                        const std::string& solution_name,
                        VALUE_TYPE value)
{
    if (this->getGridCoordinates() == NULL) return UDM_ERROR;
    UdmEntity *entity = this->getGridCoordinates()->getNodeById(node_id);
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
UdmError_t UdmZone::setSolutionVectorInNode(
                        UdmSize_t node_id,
                        const std::string& solution_name,
                        const VALUE_TYPE* values,
                        unsigned int size)
{
    if (this->getGridCoordinates() == NULL) return UDM_ERROR;
    UdmEntity *entity = this->getGridCoordinates()->getNodeById(node_id);
    if (entity == NULL) return UDM_ERROR;

    return entity->setSolutionVector<VALUE_TYPE>(solution_name, values, size);
}


/**
 * 要素（セル）の物理量データ値を取得する:スカラデータ.
 * @param [in]  cell_id        要素（セル）ID（=１～）
 * @param [in]  solution_name        物理量名
 * @param [out]  value                物理量データ値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmZone::getSolutionScalarInCell(
                        UdmSize_t cell_id,
                        const std::string& solution_name,
                        VALUE_TYPE& value) const
{
    if (this->getSections() == NULL) return UDM_ERROR;
    UdmEntity *entity = this->getSections()->getEntityCell(cell_id);
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
unsigned int UdmZone::getSolutionVectorInCell(
                        UdmSize_t cell_id,
                        const std::string& solution_name,
                        VALUE_TYPE* values) const
{
    if (this->getSections() == NULL) return 0;
    UdmEntity *entity = this->getSections()->getEntityCell(cell_id);
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
UdmError_t UdmZone::setSolutionScalarInCell(
                        UdmSize_t cell_id,
                        const std::string& solution_name,
                        VALUE_TYPE value)
{
    if (this->getSections() == NULL) return UDM_ERROR;
    UdmEntity *entity = this->getSections()->getEntityCell(cell_id);
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
UdmError_t UdmZone::setSolutionVectorInCell(
                        UdmSize_t cell_id,
                        const std::string& solution_name,
                        const VALUE_TYPE* values,
                        unsigned int size)
{
    if (this->getSections() == NULL) return UDM_ERROR;
    UdmEntity *entity = this->getSections()->getEntityCell(cell_id);
    if (entity == NULL) return UDM_ERROR;

    return entity->setSolutionVector<VALUE_TYPE>(solution_name, values, size);
}

template UdmError_t UdmZone::getSolutionScalarInNode<int>(UdmSize_t node_id, const std::string& solution_name, int& value) const;
template UdmError_t UdmZone::getSolutionScalarInNode<long long>(UdmSize_t node_id, const std::string& solution_name, long long& value) const;
template UdmError_t UdmZone::getSolutionScalarInNode<float>(UdmSize_t node_id, const std::string& solution_name, float& value) const;
template UdmError_t UdmZone::getSolutionScalarInNode<double>(UdmSize_t node_id, const std::string& solution_name, double& value) const;

template unsigned int UdmZone::getSolutionVectorinNode<int>(UdmSize_t node_id, const std::string& solution_name, int* values) const;
template unsigned int UdmZone::getSolutionVectorinNode<long long>(UdmSize_t node_id, const std::string& solution_name, long long* values) const;
template unsigned int UdmZone::getSolutionVectorinNode<float>(UdmSize_t node_id, const std::string& solution_name, float* values) const;
template unsigned int UdmZone::getSolutionVectorinNode<double>(UdmSize_t node_id, const std::string& solution_name, double* values) const;

template UdmError_t UdmZone::setSolutionScalarInNode<int>(UdmSize_t node_id, const std::string& solution_name, int value);
template UdmError_t UdmZone::setSolutionScalarInNode<long long>(UdmSize_t node_id, const std::string& solution_name, long long value);
template UdmError_t UdmZone::setSolutionScalarInNode<float>(UdmSize_t node_id, const std::string& solution_name, float value);
template UdmError_t UdmZone::setSolutionScalarInNode<double>(UdmSize_t node_id, const std::string& solution_name, double value);

template UdmError_t UdmZone::setSolutionVectorInNode<int>(UdmSize_t node_id, const std::string& solution_name, const int* values, unsigned int size);
template UdmError_t UdmZone::setSolutionVectorInNode<long long>(UdmSize_t node_id, const std::string& solution_name, const long long* values, unsigned int size);
template UdmError_t UdmZone::setSolutionVectorInNode<float>(UdmSize_t node_id, const std::string& solution_name, const float* values, unsigned int size);
template UdmError_t UdmZone::setSolutionVectorInNode<double>(UdmSize_t node_id, const std::string& solution_name, const double* values, unsigned int size);

template UdmError_t UdmZone::getSolutionScalarInCell<int>(UdmSize_t cell_id, const std::string& solution_name, int& value) const;
template UdmError_t UdmZone::getSolutionScalarInCell<long long>(UdmSize_t cell_id, const std::string& solution_name, long long& value) const;
template UdmError_t UdmZone::getSolutionScalarInCell<float>(UdmSize_t cell_id, const std::string& solution_name, float& value) const;
template UdmError_t UdmZone::getSolutionScalarInCell<double>(UdmSize_t cell_id, const std::string& solution_name, double& value) const;

template unsigned int UdmZone::getSolutionVectorInCell<int>(UdmSize_t cell_id, const std::string& solution_name, int* values) const;
template unsigned int UdmZone::getSolutionVectorInCell<long long>(UdmSize_t cell_id, const std::string& solution_name, long long* values) const;
template unsigned int UdmZone::getSolutionVectorInCell<float>(UdmSize_t cell_id, const std::string& solution_name, float* values) const;
template unsigned int UdmZone::getSolutionVectorInCell<double>(UdmSize_t cell_id, const std::string& solution_name, double* values) const;

template UdmError_t UdmZone::setSolutionScalarInCell<int>(UdmSize_t cell_id, const std::string& solution_name, int value);
template UdmError_t UdmZone::setSolutionScalarInCell<long long>(UdmSize_t cell_id, const std::string& solution_name, long long value);
template UdmError_t UdmZone::setSolutionScalarInCell<float>(UdmSize_t cell_id, const std::string& solution_name, float value);
template UdmError_t UdmZone::setSolutionScalarInCell<double>(UdmSize_t cell_id, const std::string& solution_name, double value);

template UdmError_t UdmZone::setSolutionVectorInCell<int>(UdmSize_t cell_id, const std::string& solution_name, const int* values, unsigned int size);
template UdmError_t UdmZone::setSolutionVectorInCell<long long>(UdmSize_t cell_id, const std::string& solution_name, const long long* values, unsigned int size);
template UdmError_t UdmZone::setSolutionVectorInCell<float>(UdmSize_t cell_id, const std::string& solution_name, const float* values, unsigned int size);
template UdmError_t UdmZone::setSolutionVectorInCell<double>(UdmSize_t cell_id, const std::string& solution_name, const double* values, unsigned int size);

/**
 * CGNS:ZoneIterativeDataを取得する.
 * @param [in]  index_file        CGNSファイルインデックス
 * @param [in] index_base        CGNSベースインデックス
 * @param [in] index_zone        CGNSゾーンインデックス
 * @param [out] iterative_grids        CGNS:GridCoordinatesPointersの名前リスト
 * @param [out] iterative_solutions        CGNS:FlowSolutionPointersの名前リスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::readCgnsZoneIterativeDatas(
                        int index_file,
                        int index_base,
                        int index_zone,
                        std::vector<std::string>& iterative_grids,
                        std::vector<std::string>& iterative_solutions)
{
    UdmError_t error;
    int n, narrays;
    DataType_t cgns_datatype;
    int data_dimension;
    cgsize_t dimension_vector[3];
    char* gridcoordinates_pointers = NULL, *flowsolution_pointers = NULL;
    char array_name[33] = {0x00};
    char pointer_name[33] = {0x00};
    int i;

    if (cg_goto(index_file,index_base,"Zone_t",index_zone,"ZoneIterativeData",0,"end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure : cg_goto(Zone_t/ZoneIterativeData)");
    }
    narrays = 0;
    cg_narrays(&narrays);
    error = UDM_OK;
    for (n=1; n<=narrays; n++) {
        cg_array_info(n, array_name, &cgns_datatype, &data_dimension, dimension_vector);
        if (data_dimension != 2) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "data_dimension not equal 2.");
        }
        if (dimension_vector[0] != 32
            || dimension_vector[1] <= 0) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "dimension_vector[0]=%d,dimension_vector[1]=%d.", dimension_vector[0], dimension_vector[1]);
        }
        if (strcmp(array_name, "GridCoordinatesPointers") == 0) {
            gridcoordinates_pointers = new char[dimension_vector[0] *dimension_vector[1]];
            memset(gridcoordinates_pointers, ' ', dimension_vector[0] *dimension_vector[1]*sizeof(char));
            cg_array_read(n, gridcoordinates_pointers);
            for (i=0; i<dimension_vector[1]; i++) {
                memset(pointer_name, 0x00, 33);
                strncpy(pointer_name,
                        gridcoordinates_pointers+i*dimension_vector[0],
                        dimension_vector[0]);
                this->trim(pointer_name);
                iterative_grids.push_back(std::string(pointer_name));
            }
        }
        if (strcmp(array_name, "FlowSolutionPointers") == 0) {
            flowsolution_pointers = new char[dimension_vector[0] *dimension_vector[1]];
            memset(flowsolution_pointers, ' ', dimension_vector[0] *dimension_vector[1]*sizeof(char));
            cg_array_read(n, flowsolution_pointers);
            for (i=0; i<dimension_vector[1]; i++) {
                memset(pointer_name, 0x00, 33);
                strncpy(pointer_name,
                        flowsolution_pointers+i*dimension_vector[0],
                        dimension_vector[0]);
                this->trim(pointer_name);
                iterative_solutions.push_back(std::string(pointer_name));
            }
        }
    }

    if (gridcoordinates_pointers != NULL) delete []gridcoordinates_pointers;
    if (flowsolution_pointers != NULL) delete []flowsolution_pointers;

    return UDM_OK;
}

/**
 * 仮想要素（セル）、節点（ノード）を削除する。
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::clearVertialCells()
{
    UdmError_t error = UDM_OK;

    // 仮想要素（セル）を削除する.
    if (this->getSections()->clearVirtualCells() != UDM_OK) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "do not clear virtual cells.");
    }

    // 仮想節点（ノード）を削除する.
    if (this->getGridCoordinates()->clearVirtualNodes() != UDM_OK) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_NODE, "do not clear virtual nodes.");
    }

    return error;
}

/**
 * UdmRankConnectivityの接続面をCGNSファイル出力する.
 * UdmRankConnectivityの検証用関数.
 * @param index_file        CGNS:ファイルインデックス
 * @param index_base        CGNS:ベースインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::writeRankConnectivity(
                        int index_file,
                        int index_base)
{
    int index_zone = 0;
    int timeslice_step = 0;
    bool grid_timeslice = false;

    // CGNS:Zoneを出力する.
    if (this->writeCgnsZone(index_file, index_base, index_zone) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure : writeCgnsZone");
    }

    // CGNS:GridCoordinatesを書き込む.
    if (this->getGridCoordinates()->writeCgns(index_file, index_base, index_zone, timeslice_step, grid_timeslice) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "getGridCoordinates(index_file=%d, index_base=%d, index_zone=%d, timeslice_step=%d)", index_file, index_base, index_zone, timeslice_step);
    }

    // UdmRankConnectivityをCGNS:Elementsを書き込む.
    if (this->getRankConnectivity()->writeRankConnectivity(index_file, index_base, index_zone) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "UdmRankConnectivity::writeRankConnectivity(index_file=%d, index_base=%d, index_zone=%d)", index_file, index_base, index_zone);
    }

    return UDM_OK;
}


/**
 * 仮想セルをCGNSファイル出力する.
 * 仮想セルの検証用関数.
 * @param index_file        CGNS:ファイルインデックス
 * @param index_base        CGNS:ベースインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::writeVirtualCells(int index_file, int index_base)
{
    int index_zone = 0;
    int timeslice_step = 0;
    bool grid_timeslice = false;

    // CGNS:Zoneを出力する.
    if (this->writeVirtualZone(index_file, index_base, index_zone) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure : writeCgnsZone");
    }

    // CGNS:GridCoordinatesを書き込む.
    if (this->getGridCoordinates()->writeVirtualNodes(index_file, index_base, index_zone) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "getGridCoordinates(index_file=%d, index_base=%d, index_zone=%d, timeslice_step=%d)", index_file, index_base, index_zone, timeslice_step);
    }

    // UdmSection::仮想セルをCGNS:Elementsに書き込む.
    if (this->getSections()->writeVirtualCells(index_file, index_base, index_zone) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "UdmRankConnectivity::writeRankConnectivity(index_file=%d, index_base=%d, index_zone=%d)", index_file, index_base, index_zone);
    }

    return UDM_OK;
}

/**
 * 仮想セルのCGNS:Zoneを出力する
 * Zoneが存在していない場合は作成を行う.
 * 存在している場合は、何もしない.
 * @param [in]  index_file        CGNS:ファイルインデックス
 * @param [in]  index_base        CGNS:ベースインデックス
 * @param [out] index_zone        CGNS:ゾーンインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::writeVirtualZone(int index_file, int index_base, int &index_zone)
{

    char zonename[33] = {0x00}, cgns_zonename[33] = {0x00};
    int num_zones = 0;
    cgsize_t sizes[9] = {0x00};
    UdmSize_t vertex_size;

    if (this->getGridCoordinates() == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "GridCoordinates is null.");
    }
    if (this->getSections() == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "Sections is null.");
    }
    vertex_size = this->getGridCoordinates()->getNumNodes();
    vertex_size += this->getGridCoordinates()->getNumVirtualNodes();
    this->setVertexSize(vertex_size);
    if (this->getVertexSize() <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "vertex size is zero.");
    }
    this->setCellSize(this->getSections()->getNumVirtualCells());
    if (this->getCellSize() <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "cell size is zero.");
    }

    // 既にCGNSゾーン名が存在しているかチェックする.
    if (this->getName().empty()) {
        sprintf(zonename, UDM_CGNS_NAME_ZONE, (int)this->getId());
    }
    else {
        sprintf(zonename, "%s", this->getName().c_str());
    }
    index_zone = 0;

    sizes[0] = this->getVertexSize();
    sizes[1] = this->getCellSize();
    sizes[2] = 0;
    // CGNS:Zoneの作成
    if (cg_zone_write(index_file, index_base, zonename, sizes, Unstructured, &index_zone) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "failure : cg_zone_write [zone_name=%s].", zonename);
    }

    return UDM_OK;
}

/**
 * CGNSゾーンを結合する.
 * GridCoordinates,Elements,FlowSolutionsを追加する.
 * @param dest_zone        結合ゾーン
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::joinCgnsZone(const UdmZone* dest_zone)
{
    UdmError_t error = UDM_OK;

    // 部品要素を削除する
    UdmSections *dest_sections = dest_zone->getSections();
    dest_sections->clearComponentCells();

    // 節点（ノード）を追加する
    UdmGridCoordinates *dest_grid = dest_zone->getGridCoordinates();
    error = this->getGridCoordinates()->joinCgnsGridCoordinates(dest_grid);
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    // 要素（セル）の接続情報を追加する.
    error = this->getSections()->joinCgnsSections(dest_sections);
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    // 物理量:UdmFlowSolutionsは追加の必要はない。
    // UdmFlowSolutionsは節点（ノード）,要素（セル）に設定された物理量とのインターフェイスである。

    // 接続情報:UdmRankConnectivityへの節点（ノード）の追加は行わない。UdmGridCoordinatesで行う。

    // ユーザ定義データ
    const UdmUserDefinedDatas *dest_user_datas = dest_zone->getUserDefinedDatas();
    error = this->getUserDefinedDatas()->joinCgnsUserDefinedDatas(dest_user_datas);
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    return UDM_OK;
}

/**
 * 仮想セルの転送を行う.
 * @param [out] virtual_nodes        転送節点（ノード）リスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::transferVirtualCells(std::vector<UdmNode*>& virtual_nodes)
{
    if (this->getRankConnectivity() == NULL) return UDM_ERROR;
    return this->getRankConnectivity()->transferVirtualCells(virtual_nodes);
}


/**
 * 内部CGNSデータの検証を行う.
 * @return        true=検証OK
 */
bool UdmZone::validateCgns() const
{
    bool validate = true;

    UdmRankConnectivity *rank_boundary = this->getRankConnectivity();
    if (rank_boundary == NULL) {
        UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "UdmRankConnectivity is null.");
        return false;
    }

    // 内部境界の整合性をチェックする
    if (!rank_boundary->mpi_validateRankConnectivity()) {
        validate = false;
    }

    return validate;
}

/**
 * 分割重みが設定されているかチェックする.
 * @return        true=分割重みが設定されている.
 */
bool UdmZone::isSetPartitionWeight() const
{
    return this->set_partition_weight;
}

/**
 * 分割重み設定フラグを設定する.
 * @param set_weight        分割重み設定フラグ : true=分割重みが設定されている.
 */
void UdmZone::setPartitionWeight(bool set_weight)
{
    this->set_partition_weight = set_weight;
}

/**
 * 節点（ノード）が接続している要素（セル）数を取得する.
 * @param node_id        節点（ノード）ID
 * @return            接続要素（セル）数
 */
int UdmZone::getNumConnectivityCells(UdmSize_t node_id) const
{
    if (this->getGridCoordinates() == NULL) return 0;
    UdmNode *node = this->getGridCoordinates()->getNodeById(node_id);
    if (node == NULL) return 0;
    return node->getNumConnectivityCells();
}

/**
 * 節点（ノード）が接続している要素（セル）を取得する.
 * @param  node_id        節点（ノード）ID
 * @param  connectivity_id        接続ID : 1～getNumConnectivityCells(node_id)
 * @return        接続要素（セル）数
 */
UdmCell* UdmZone::getConnectivityCell(UdmSize_t node_id, int connectivity_id) const
{
    if (this->getGridCoordinates() == NULL) return 0;
    UdmNode *node = this->getGridCoordinates()->getNodeById(node_id);
    if (node == NULL) return 0;
    return node->getConnectivityCell(connectivity_id);
}

/**
 * 要素（セル）を構成する節点（ノード）数を取得する.
 * @param cell_id        要素（セル）ID
 * @return        構成節点（ノード）数
 */
int UdmZone::getNumConnectivityNodes(UdmSize_t cell_id) const
{
    if (this->getSections() == NULL) return 0;
    UdmCell *cell = this->getSections()->getEntityCell(cell_id);
    if (cell == NULL) return 0;
    return cell->getNumNodes();
}

/**
 * 要素（セル）を構成する節点（ノード）を取得する.
 * @param  cell_id        要素（セル）ID
 * @param  connectivity_id       接続ID : 1～getNumConnectivityNodes(cell_id)
 * @return        構成節点（ノード）数
 */
UdmNode* UdmZone::getConnectivityNode(UdmSize_t cell_id, int connectivity_id) const
{
    if (this->getSections() == NULL) return 0;
    UdmCell *cell = this->getSections()->getEntityCell(cell_id);
    if (cell == NULL) return 0;
    return cell->getNode(connectivity_id);;
}

/**
 * 要素（セル）の隣接要素（セル）数を取得する.
 * @param cell_id        要素（セル）ID
 * @return        隣接要素（セル）数
 */
int UdmZone::getNumNeighborCells(UdmSize_t cell_id) const
{
    if (this->getSections() == NULL) return 0;
    UdmCell *cell = this->getSections()->getEntityCell(cell_id);
    if (cell == NULL) return 0;
    return cell->getNumNeighborCells();
}

/**
 * 要素（セル）の隣接要素（セル）を取得する.
 * @param  cell_id        要素（セル）ID
 * @param  neighbor_id        隣接ID : 1～getNumNeighborCells(cell_id)
 * @return        隣接要素（セル）数
 */
UdmCell* UdmZone::getNeighborCell(UdmSize_t cell_id, int neighbor_id) const
{
    if (this->getSections() == NULL) return 0;
    UdmCell *cell = this->getSections()->getEntityCell(cell_id);
    if (cell == NULL) return 0;
    return cell->getNeighborCell(neighbor_id);
}


/**
 * 節点（ノード）の隣接節点（ノード）数を取得する.
 * @param node_id        節点（ノード）ID
 * @return        隣接節点（ノード）数
 */
int UdmZone::getNumNeighborNodes(UdmSize_t node_id) const
{
    if (this->getGridCoordinates() == NULL) return 0;
    UdmNode *node = this->getGridCoordinates()->getNodeById(node_id);
    if (node == NULL) return 0;
    return node->getNumNeighborNodes();
}

/**
 * 節点（ノード）の隣接節点（ノード）を取得する.
 * @param  node_id        節点（ノード）ID
 * @param  neighbor_id        隣接ID : 1～getNumNeighborCells(cell_id)
 * @return        隣接節点（ノード）数
 */
UdmNode* UdmZone::getNeighborNode(UdmSize_t node_id, int neighbor_id) const
{
    if (this->getGridCoordinates() == NULL) return 0;
    UdmNode *node = this->getGridCoordinates()->getNodeById(node_id);
    if (node == NULL) return 0;
    return node->getNeighborNode(neighbor_id);
}

/**
 * 節点（ノード）を取得する.
 * @param node_id        節点（ノード）ID：1～getNumNodes()
 * @return        節点（ノード）
 */
UdmNode* UdmZone::getNode(UdmSize_t node_id) const
{
    if (this->getGridCoordinates() == NULL) return 0;
    return this->getGridCoordinates()->getNodeById(node_id);
}

/**
 * 要素（セル）を取得する.
 * @param cell_id        要素（セル）ID：1～getNumCells()
 * @return            要素（セル）
 */
UdmCell* UdmZone::getCell(UdmSize_t cell_id) const
{
    if (this->getSections() == NULL) return 0;
    return this->getSections()->getEntityCell(cell_id);
}


/**
 * 以前のID、ランク番号をクリアする.
 */
void UdmZone::clearPreviousInfos()
{
    if (this->grid_coordinates == NULL) return;
    if (this->getRankConnectivity() == NULL) return;

    // 以前のID、ランク番号をクリアする.
    this->grid_coordinates->clearPreviousInfos();

    // ランク番号、ID検索テーブルを再作成する。
    this->getRankConnectivity()->createSearchTable();

    return;
}


/**
 * インポートグローバルIDリストに追加節点（ノード）を追加する。
 * インポートグローバルIDリストに追加節点（ノード）のランク番号,ID, 以前のランク番号,ID, MPI接続のランク番号,IDを追加する.
 * @param [out] import_globalids            インポートグローバルIDリスト
 * @param [in] import_node            追加節点（ノード）
 * @return            追加したインポート節点（ノード）
 */
void UdmZone::insertImportGlobalRankids(
                UdmGlobalRankidList &import_globalids,
                const UdmNode* import_node) const
{
    if (import_node == NULL) return;

    UdmSize_t src_id = import_node->getId();
    int src_rankno = import_node->getMyRankno();

    import_globalids.addGlobalRankid(src_rankno, src_id);

    const UdmGlobalRankidList previous_infos = import_node->getPreviousRankInfos();
    import_globalids.addGlobalRankids(previous_infos);

    const UdmGlobalRankidList& mpi_infos = import_node->getMpiRankInfos();
    import_globalids.addGlobalRankids(mpi_infos);

    return;
}

/**
 * 再構成対象節点（ノード）の親要素（セル）を探索し、部品要素を作成する.
 * @param import_nodes            部品要素の作成対象節点（ノード）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::createComponentCells(const std::vector<UdmNode*> import_nodes)
{
    UdmError_t error = UDM_OK;
    // 再構成対象節点（ノード）の親要素（セル）を探索する.
    UdmScannerCells scanner;
    UdmSize_t num_parents = scanner.scannerParents(import_nodes);
    if (num_parents > 0) {
        std::vector<UdmEntity*> parent_cells;
        std::vector<UdmEntity*>::iterator cell_itr;
        scanner.getScanEntities(parent_cells);
        for (cell_itr=parent_cells.begin(); cell_itr!=parent_cells.end(); cell_itr++) {
            // 部品要素を再作成する.
            UdmCell *cell = static_cast<UdmCell*>(*cell_itr);
            if (cell == NULL) continue;
            error = cell->createComponentCells();
            if (error != UDM_OK) {
                return UDM_ERRORNO_HANDLER(error);
            }
        }
    }
    return error;
}


/**
 * 部品要素の作成を行う.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmZone::createComponentCells()
{
    UdmError_t error = UDM_OK;
    if (this->getSections() == NULL) return UDM_ERROR;
    error = this->getSections()->createComponentCells();
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    return error;
}

/**
 * 分割重みをクリアする.
 * 分割重みフラグをクリアする.
 * 要素（セル）に設定している重み値をクリアする.
 */
void UdmZone::clearPartitionWeight()
{
    this->setPartitionWeight(false);
    if (this->getSections() == NULL) return;
    this->getSections()->clearPartitionWeight();

}

} /* namespace udm */
