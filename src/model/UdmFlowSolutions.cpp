/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmFlowSolutions.cpp
 * 物理量管理クラスのソースファイル
 */

#include "model/UdmFlowSolutions.h"
#include "model/UdmModel.h"
#include "model/UdmZone.h"
#include "model/UdmGridCoordinates.h"
#include "model/UdmSections.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmFlowSolutions::UdmFlowSolutions()
{
    this->initialize();
}

/**
 * コンストラクタ
 * @param  parent_zone    親ゾーン
 */
UdmFlowSolutions::UdmFlowSolutions(UdmZone *parent_zone)
{
    this->initialize();
    this->parent_zone = parent_zone;
}

/**
 * デストラクタ
 */
UdmFlowSolutions::~UdmFlowSolutions()
{
}

/**
 * 初期化を行う.
 */
void UdmFlowSolutions::initialize()
{
    this->parent_zone = NULL;
}

/**
 * 物理量設定情報を取得する.
 * @return        物理量設定情報
 */
UdmFlowSolutionListConfig* UdmFlowSolutions::getFlowSolutionsConfig() const
{
    if (this->getParentZone() == NULL) return NULL;
    if (this->getParentZone()->getParentModel() == NULL) return NULL;
    if (this->getParentZone()->getParentModel()->getDfiConfig() == NULL) return NULL;
    return this->getParentZone()->getParentModel()->getDfiConfig()->getFlowSolutionListConfig();
}

/**
 * 物理量フィールド情報数を取得する.
 * @return        物理量フィールド情報数
 */
UdmSize_t UdmFlowSolutions::getNumSolutionFields() const
{
    if (this->getFlowSolutionsConfig() == NULL) return 0;
    return this->getFlowSolutionsConfig()->getNumSolutionConfig();
}


/**
 * 物理量フィールド情報を取得する:const.
 * @param field_id        物理量フィールドID（１～）
 * @return        物理量フィールド情報
 */
UdmSolutionFieldConfig* UdmFlowSolutions::getSolutionField(UdmSize_t field_id) const
{
    if (this->getFlowSolutionsConfig() == NULL) return NULL;
    if (field_id <= 0) return NULL;
    if (field_id > this->getNumSolutionFields()) return NULL;
    std::string solution_name;
    if (this->getFlowSolutionsConfig()->getSolutionName(field_id, solution_name) != UDM_OK) {
        return NULL;
    }
    return this->getFlowSolutionsConfig()->getSolutionFieldConfig(solution_name);
}

/**
 * 物理量フィールド名称の物理量フィールド情報を取得する:const.
 * @param field_name        物理量フィールド名称
 * @return        物理量フィールド情報
 */
UdmSolutionFieldConfig* UdmFlowSolutions::getSolutionField(const std::string& field_name) const
{
    if (this->getFlowSolutionsConfig() == NULL) return NULL;
    if (field_name.empty()) return NULL;
    if (this->getNumSolutionFields() <= 0) return NULL;
    return this->getFlowSolutionsConfig()->getSolutionFieldConfig(field_name);
}


/**
 * 物理量フィールド名称の物理量フィールド情報を取得する.
 * @param field_name        物理量フィールド名称
 * @return        物理量フィールド情報
 */
UdmSolutionFieldConfig* UdmFlowSolutions::findSolutionFieldByCgnsName(const std::string& cgns_field_name, int &vector_id) const
{
    if (this->getFlowSolutionsConfig() == NULL) return NULL;
    if (cgns_field_name.empty()) return NULL;
    if (this->getNumSolutionFields() <= 0) return NULL;
    return this->getFlowSolutionsConfig()->findSolutionFieldByCgnsName(cgns_field_name, vector_id);
}


/**
 * 物理量フィールド情報を作成する.
 * 既存の物理量フィールド名称は作成不可である.
 * 物理量フィールドタイプ(field_type)=物理量(Udm_FlowSolution)にて作成する.
 * CGNS:FlowSolutionの名称(flowsolution_name)=[なし]にて作成する.
 * @param field_name        物理量フィールド名称
 * @param data_type            データ型
 * @param location            物理量定義位置
 * @return        物理量フィールドID : 作成エラーの場合は、0を返す.
 */
UdmSize_t UdmFlowSolutions::createSolutionField(
                                    const std::string& field_name,
                                    UdmDataType_t data_type,
                                    UdmGridLocation_t location)
{
    if (this->getFlowSolutionsConfig() == NULL) return 0;
    if (this->existsFieldName(field_name)) {
        return 0;
    }
    UdmSolutionFieldConfig *field = new UdmSolutionFieldConfig(field_name, location, data_type);
    this->getFlowSolutionsConfig()->setSolutionFieldConfig(field);
    return this->getNumSolutionFields();
}

/**
 *
 * 物理量フィールド情報を作成する.
 * 既存の物理量フィールド名称は作成不可である.
 * @param field_name        物理量フィールド名称
 * @param data_type            データ型
 * @param grid_location            物理量定義位置
 * @param vector_type        ベクトルタイプ
 * @param nvector_size        N成分数
 * @param constant_flag       固定値フラグ
 * @return        物理量フィールドID : 作成エラーの場合は、0を返す.
 */
UdmSize_t UdmFlowSolutions::createSolutionField(
                        const std::string     &field_name,
                        UdmDataType_t         data_type,
                        UdmGridLocation_t     grid_location,
                        UdmVectorType_t       vector_type,
                        int                   nvector_size,
                        bool                  constant_flag)
{
    if (this->getFlowSolutionsConfig() == NULL) return 0;
    if (this->existsFieldName(field_name)) {
        return 0;
    }
    UdmSolutionFieldConfig *field = new UdmSolutionFieldConfig(
                                        field_name,
                                        grid_location,
                                        data_type,
                                        vector_type,
                                        nvector_size,
                                        constant_flag);
    this->getFlowSolutionsConfig()->setSolutionFieldConfig(field);
    return this->getNumSolutionFields();
}

/**
 * 物理量フィールドIDの物理量フィールド名称を取得する.
 * @param field_id        物理量フィールドID
 * @param        物理量フィールド名称
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutions::getFieldName(UdmSize_t field_id, std::string& field_name) const
{
    const UdmSolutionFieldConfig* field = this->getSolutionField(field_id);
    if (field == NULL) return UDM_ERROR;
    field_name = field->getSolutionName();
    return UDM_OK;
}

/**
 * 物理量フィールドIDのデータ型を取得する.
 * @param field_id        物理量フィールドID
 * @return        データ型
 */
UdmDataType_t UdmFlowSolutions::getFieldDataType(UdmSize_t field_id) const
{
    const UdmSolutionFieldConfig* field = this->getSolutionField(field_id);
    if (field == NULL) return Udm_DataTypeUnknown;
    return field->getDataType();
}

/**
 * 物理量フィールドIDの物理量定義位置を取得する.
 * @param field_id        物理量フィールドID
 * @return        物理量定義位置
 */
UdmGridLocation_t UdmFlowSolutions::getFieldGridLocation(UdmSize_t field_id) const
{
    const UdmSolutionFieldConfig* field = this->getSolutionField(field_id);
    if (field == NULL) return Udm_GridLocationUnknown;
    return field->getGridLocation();
}

/**
 * 物理量フィールドIDのCGNS:FlowSolution名称を取得する.
 * @param [in] field_id        物理量フィールドID
 * @param [out] cgns_name        CGNS:FlowSolution名称
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutions::getCgnsFlowSolutionName(UdmSize_t field_id, std::string &cgns_name) const
{
    std::string field_name;
    if (this->getFieldName(field_id, field_name) != UDM_OK) {
        return UDM_ERROR;
    }
    return this->getCgnsFlowSolutionName(field_name, cgns_name);
}

/**
 * 物理量フィールド名称のCGNS:FlowSolution名称を取得する.
 * @param [in] field_name        物理量フィールド名称
 * @param [out] cgns_name        CGNS:FlowSolution名称
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutions::getCgnsFlowSolutionName(const std::string& field_name, std::string &cgns_name) const
{
    const UdmSolutionFieldConfig* field = this->getSolutionField(field_name);
    if (field == NULL) return UDM_ERROR;

    UdmGridLocation_t location = field->getGridLocation();
    // ノード（頂点）
    if (location == Udm_Vertex) {
        cgns_name = UDM_CGNS_NAME_FLOWSOLUTION_VERTEX;
    }
    // 要素（セル）センター
    else if (location == Udm_CellCenter) {
        cgns_name = UDM_CGNS_NAME_FLOWSOLUTION_CELLCENTER;
    }
    else {
        return UDM_ERROR;
    }
    return UDM_OK;
}


/**
 * 物理量フィールド名称のCGNS:Field名称リストを取得する.
 * @param [in] field_name        物理量フィールド名称
 * @param [out] cgns_names        CGNS:FlowSolution/Field名称リスト
 * @return        Field名称リスト数
 */
int UdmFlowSolutions::getCgnsSolutionFieldNames(UdmSize_t field_id, std::vector<std::string> &cgns_names) const
{
    std::string field_name;
    if (this->getFieldName(field_id, field_name) != UDM_OK) {
        return UDM_ERROR;
    }
    return this->getCgnsSolutionFieldNames(field_name, cgns_names);
}


/**
 * 物理量フィールド名称のCGNS:Field名称リストを取得する.
 * @param [in] field_name        物理量フィールド名称
 * @param [out] cgns_names        CGNS:FlowSolution/Field名称リスト
 * @return        Field名称リスト数
 */
int UdmFlowSolutions::getCgnsSolutionFieldNames(const std::string& field_name, std::vector<std::string> &cgns_names) const
{
    const UdmSolutionFieldConfig* field = this->getSolutionField(field_name);
    if (field == NULL) return 0;
    return field->getVectorSolutionNames(cgns_names);;
}


/**
 * 物理量フィールド名称の物理量フィールドIDを取得する.
 * @param field_name        物理量フィールド名称
 * @return        物理量フィールドID : 存在しない場合は0を返す
 */
UdmSize_t UdmFlowSolutions::getFieldId(const std::string &field_name) const
{
    if (this->getFlowSolutionsConfig() == NULL) return 0;
    if (field_name.empty()) return 0;
    return this->getFlowSolutionsConfig()->getSolutionId(field_name);
}

/**
 * 物理量フィールド名称のデータ型を取得する.
 * @param field_name        物理量フィールド名称
 * @return        データ型 : 物理量フィールド名称が存在しない場合は、Udm_DataTypeUnknownを返す.
 */
UdmDataType_t UdmFlowSolutions::getFieldDataType(const std::string& field_name) const
{
    UdmDataType_t type = Udm_DataTypeUnknown;
    if (this->getFlowSolutionsConfig() == NULL) return type;
    return this->getFlowSolutionsConfig()->getDataType(field_name);
}

/**
 * 物理量フィールド名称の物理量定義位置を取得する.
 * @param field_name        物理量フィールド名称
 * @return        物理量定義位置 : 物理量フィールド名称が存在しない場合は、Udm_GridLocationUnknownを返す.
 */
UdmGridLocation_t UdmFlowSolutions::getFieldGridLocation(const std::string& field_name) const
{
    UdmGridLocation_t type = Udm_GridLocationUnknown;
    if (this->getFlowSolutionsConfig() == NULL) return type;
    return this->getFlowSolutionsConfig()->getGridLocation(field_name);
}

/**
 * 物理量フィールド名称の物理量フィールド情報が存在するかチェックする.
 * @param field_name        物理量フィールド名称
 * @return        true=物理量フィールド情報が存在する
 */
bool UdmFlowSolutions::existsFieldName(const std::string& field_name) const
{
    if (this->getFlowSolutionsConfig() == NULL) return false;
    const UdmSolutionFieldConfig* field = this->getSolutionField(field_name);
    return (field != NULL);
}


/**
 * 物理量フィールドIDの物理量フィールドを削除する.
 * @param field_id        物理量フィールドID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutions::removeSolutionField(UdmSize_t field_id)
{
    if (this->getFlowSolutionsConfig() == NULL) return UDM_ERROR;
    if (field_id <= 0) return UDM_ERROR;
    if (field_id > this->getNumSolutionFields()) return UDM_ERROR;
    std::string field_name;
    if (this->getFieldName(field_id, field_name) != UDM_OK) {
        return UDM_ERROR;
    }
    if (this->getFlowSolutionsConfig()->removeSolutionConfig(field_name) != UDM_OK) {
        return UDM_ERROR;
    }

    // 物理量フィールドIDの再振分
    this->rebuildSolutionFields();

    return UDM_OK;

}

/**
 * 物理量フィールド名称の物理量フィールドを削除する.
 * @param field_name        物理量フィールド名称
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutions::removeSolutionField(const std::string& field_name)
{
    if (this->getFlowSolutionsConfig() == NULL) return UDM_ERROR;
    if (this->getFlowSolutionsConfig()->removeSolutionConfig(field_name) != UDM_OK) {
        return UDM_ERROR;
    }

    // 物理量フィールドIDの再振分
    this->rebuildSolutionFields();
    return UDM_OK;
}

/**
 * 物理量ベクトルデータタイプを取得する:物理量フィールドID.
 * @param field_id        物理量フィールドID
 * @return        物理量ベクトルデータタイプ
 */
UdmVectorType_t UdmFlowSolutions::getFieldVectorType(UdmSize_t field_id) const
{
    UdmVectorType_t type = Udm_VectorTypeUnknown;
    if (this->getFlowSolutionsConfig() == NULL) return type;
    std::string field_name;
    if (this->getFieldName(field_id, field_name) != UDM_OK) {
        return type;
    }
    return this->getFieldVectorType(field_name);
}

/**
 * 物理量N成分数を取得する:物理量フィールドID.
 * @param field_id        物理量フィールドID
 * @return        物理量N成分数
 */
int UdmFlowSolutions::getFieldNvectorSize(UdmSize_t field_id) const
{
    if (this->getFlowSolutionsConfig() == NULL) return 0;
    std::string field_name;
    if (this->getFieldName(field_id, field_name) != UDM_OK) {
        return 0;
    }
    return this->getFieldNvectorSize(field_name);
}


/**
 * 物理量固定値であるかチェックする:物理量フィールドID.
 * @param field_id        物理量フィールドID
 * @return        true=物理量固定値
 */
bool UdmFlowSolutions::isFieldConstant(UdmSize_t field_id) const
{
    std::string field_name;
    if (this->getFieldName(field_id, field_name) != UDM_OK) {
        return false;
    }
    return this->isFieldConstant(field_name);

}

/**
 * 物理量ベクトルデータタイプを取得する:物理量フィールド名.
 * @param field_name        物理量フィールド名
 * @return        物理量ベクトルデータタイプ
 */
UdmVectorType_t UdmFlowSolutions::getFieldVectorType(const std::string& field_name) const
{
    UdmVectorType_t type = Udm_VectorTypeUnknown;
    if (this->getFlowSolutionsConfig() == NULL) return type;
    return this->getFlowSolutionsConfig()->getVectorType(field_name);
}

/**
 * 物理量N成分数を取得する:物理量フィールド名.
 * @param field_name        物理量フィールド名
 * @return        物理量N成分数
 */
int UdmFlowSolutions::getFieldNvectorSize(const std::string& field_name) const
{
    if (this->getFlowSolutionsConfig() == NULL) return 0;
    return this->getFlowSolutionsConfig()->getNvectorSize(field_name);
}


/**
 * 物理量固定値であるかチェックする:物理量フィールド名.
 * @param field_name        物理量フィールド名
 * @return        true=物理量固定値
 */
bool UdmFlowSolutions::isFieldConstant(const std::string& field_name) const
{
    if (this->getFlowSolutionsConfig() == NULL) return false;
    return this->getFlowSolutionsConfig()->isConstantFlag(field_name);
}


/**
 * 物理量フィールドIDの再振分を行う.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutions::rebuildSolutionFields()
{
    if (this->getFlowSolutionsConfig() == NULL) return UDM_ERROR;
    return this->getFlowSolutionsConfig()->rebuildSolutionList();
}

/**
 * 物理量フィールド情報をクリアする.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutions::clearSolutionFields()
{
    if (this->getFlowSolutionsConfig() == NULL) return UDM_ERROR;
    return this->getFlowSolutionsConfig()->clear();
}

/**
 * 親ゾーンを取得する.
 * @return        親ゾーン
 */
UdmZone* UdmFlowSolutions::getParentZone()
{
    return this->parent_zone;
}

/**
 * 親ゾーンを取得する:const.
 * @return        親ゾーン
 */
const UdmZone* UdmFlowSolutions::getParentZone() const
{
    return this->parent_zone;
}


/**
 * 親ゾーンを設定する.
 * @param zone        親ゾーン
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutions::setParentZone(UdmZone* zone)
{
    this->parent_zone = zone;
    return UDM_OK;
}

/**
 * CGNSから物理量データを読み込む.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param timeslice_step       CGNS読込ステップ回数 (default=-1)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutions::readCgns(int index_file, int index_base, int index_zone, int timeslice_step)
{
    int index_dfi_field, index_field;
    int num_fields = 0;
    char cgns_name[33], field_name[33];
    DataType_t cgns_datatype;
    UdmDataType_t field_datatype;
    GridLocation_t cgns_location;
    UdmGridLocation_t grid_location;
    UdmSize_t field_size = 0;
    cgsize_t range_min, range_max;
    std::string dif_filed_name;

    num_fields = this->getNumSolutionFields();
    for (index_dfi_field=1; index_dfi_field<=num_fields; index_dfi_field++) {
        // 物理量名称
        this->getFieldName(index_dfi_field, dif_filed_name);

        // 取得対象のCGNS:FlowSolutionとCGNS:SolutionFieldのIDを取得する.
        int index_solution = 0;
        std::vector<int> index_fields;
        if (this->getCgnsFlowSolutionIds(index_dfi_field, index_solution, index_fields, index_file, index_base, index_zone, timeslice_step) != UDM_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "index_dfi_field=%d,name=%s", index_dfi_field, dif_filed_name.c_str());
        }
        if (index_solution <= 0 || index_fields.size() <= 0) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "index_dfi_field=%d,name=%s", index_dfi_field, dif_filed_name.c_str());
        }

        // FlowSolution情報の取得
        if (cg_sol_info(index_file, index_base, index_zone, index_solution, cgns_name, &cgns_location) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure:cg_sol_info,name=%s", dif_filed_name.c_str());
        }
        // 物理量定義位置
        grid_location = this->toGridLocation(cgns_location);
        if (grid_location == Udm_Vertex) {
            field_size = this->getParentZone()->getVertexSize();
        }
        else if (grid_location == Udm_CellCenter) {
            field_size = this->getParentZone()->getCellSize();
        }
        else {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "invalid GridLocation[=%d],name=%s", (int)grid_location, dif_filed_name.c_str());
        }
        // データ型
        field_datatype = this->getFieldDataType(index_dfi_field);
        if (field_datatype == Udm_DataTypeUnknown) {
            // 最初の物理量フィールドから取得する.
            index_field = index_fields[0];
            if (cg_field_info(index_file, index_base, index_zone, index_solution,
                    index_field, &cgns_datatype, field_name) != CG_OK) {
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure:cg_field_info,name=%s", dif_filed_name.c_str());
            }
            field_datatype = this->toUdmDataType(cgns_datatype);
        }
        unsigned int num_vector = index_fields.size();
        unsigned int sizeofdata = this->sizeofDataType(field_datatype);
        // データサイズ = フィールドサイズ*成分数
        void *field_datas = this->createDataArray(field_datatype, field_size*num_vector);

        // CGNS:FlowSolution/Filedを読込
        std::vector<int>::const_iterator field_itr;
        UdmSize_t mempos = 0;
        for (field_itr=index_fields.begin(); field_itr!=index_fields.end(); field_itr++) {
            index_field = (*field_itr);
            if (cg_field_info(index_file, index_base, index_zone, index_solution,
                    index_field, &cgns_datatype, field_name) != CG_OK) {
                this->deleteDataArray(field_datas, field_datatype);
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure:cg_field_info,name=%s", dif_filed_name.c_str());
            }

            range_min = 1;
            range_max = field_size;
            cgns_datatype = this->toCgnsDataType(field_datatype);
            if (cg_field_read(index_file, index_base, index_zone, index_solution,
                    field_name, cgns_datatype, &range_min, &range_max, (char*)field_datas+mempos) != CG_OK) {
                this->deleteDataArray(field_datas, field_datatype);
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure:cg_field_read,name=%s", dif_filed_name.c_str());
            }
            mempos += field_size*sizeofdata;
        }
        // 読込物理量データをセットする.
        if (grid_location == Udm_Vertex) {
            UdmGridCoordinates *grid = this->getParentZone()->getGridCoordinates();
            if (grid == NULL) {
                this->deleteDataArray(field_datas, field_datatype);
                return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "UdmGridCoordinates is null.[name=%s]", dif_filed_name.c_str());
            }
            if (grid->setEntitySolutionArray(
                            dif_filed_name,
                            field_size,
                            num_vector,
                            field_datatype,
                            field_datas,
                            Udm_MemSequentialArray) != UDM_OK) {
                this->deleteDataArray(field_datas, field_datatype);
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "invalid setEntitySolutionArray.[name=%s]", dif_filed_name.c_str());
            }
        }
        else if (grid_location == Udm_CellCenter) {
            UdmSections *sections = this->getParentZone()->getSections();
            if (sections == NULL) {
                this->deleteDataArray(field_datas, field_datatype);
                return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "UdmSections is null.");
            }
            if (sections->setEntitySolutionArray(
                            dif_filed_name,
                            field_size,
                            num_vector,
                            field_datatype,
                            field_datas,
                            Udm_MemSequentialArray) != UDM_OK) {
                this->deleteDataArray(field_datas, field_datatype);
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "invalid setEntityCellSolutionArray.[name=%s]", dif_filed_name.c_str());
            }
        }

        this->deleteDataArray(field_datas, field_datatype);
    }

    return UDM_OK;
}

/**
 * CGNSファイルからDFI:物理量フィールドIDの読込対象となるCGNS:FlowSolutionIDとCGNS:FieldIDを取得する.
 * @param [in]  index_dfi_field        DFI:物理量フィールドID
 * @param [out] index_solution        CGNS:FlowSolutionID（１～）
 * @param [out] index_fields        CGNS:FlowSolution/FieldID（１～）
 * @param [in]  index_file            CGNS:ファイルインデックス
 * @param [in]  index_base            CGNS:ベースインデックス
 * @param [in]  index_zone            CGNS:ゾーンインデックス
 * @param [in]  timeslice_step        読込ステップ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutions::getCgnsFlowSolutionIds(
                        int index_dfi_field,
                        int &index_solution, std::vector<int>& index_fields,
                        int index_file, int index_base, int index_zone,
                        int timeslice_step)
{
    int index, num_fields, num_solutions;
    int n;
    GridLocation_t cgns_location;
    UdmGridLocation_t grid_location;
    DataType_t cgns_datatype;
    char cgns_name[33];
    std::vector<int> found_fieldids;
    bool constant = false;

    const UdmSolutionFieldConfig* field_config = this->getSolutionField(index_dfi_field);
    if (field_config == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "not found UdmSolutionFieldConfig[field_id=%d].", index_dfi_field);
    }

    // 定義位置
    grid_location = this->getFieldGridLocation(index_dfi_field);

    // 読込フィールド名の取得：[物理量名]X,[物理量名]Y,[物理量名]Z
    std::vector<std::string> cgns_fieldnames;
    int num_cgns_field = this->getCgnsSolutionFieldNames(index_dfi_field, cgns_fieldnames);
    if (num_cgns_field <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_VARIABLE, "num_cgns_field is zero");
    }
    constant = this->isFieldConstant(index_dfi_field);

    // 読込CGNS:FlowSolutionの取得
    std::vector<std::string> solution_names;
    if (!constant) {
        UdmError_t error = this->getCgnsIterativeFlowSolutionNames(
                                            solution_names,
                                            index_file, index_base, index_zone,
                                            timeslice_step);
        if (error == UDM_WARNING_CGNS_NOTEXISTS_ITERATIVEDATA) {
            // 警告メッセージ:CGNS:BaseIterativeData, ZoneIterativeDataが存在しない。
            UDM_WARNINGNO_HANDLER(UDM_WARNING_CGNS_NOTEXISTS_ITERATIVEDATA);

            // 物理量の命名規約から取得する.
            index = this->findCgnsFlowSolution(
                                    index_file,
                                    index_base,
                                    index_zone,
                                    field_config,
                                    timeslice_step);
            if (index > 0) {
                // FlowSolution名称, 定義位置の取得
                if (cg_sol_info(index_file, index_base, index_zone, index, cgns_name, &cgns_location) != CG_OK) {
                    return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure:cg_sol_info");
                }
                solution_names.push_back(std::string(cgns_name));
            }
            else {
                solution_names.clear();
                num_solutions = findCgnsSolutionFieldName(
                                            index_file,
                                            index_base,
                                            index_zone,
                                            grid_location,
                                            cgns_fieldnames,
                                            solution_names);
                if (num_solutions > 1) {
                    return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "can not found unique CGNS:FlowSolution[num_solutions=%d].", num_solutions);
                }
            }
        }
        else if (error != UDM_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure:getCgnsIterativeFlowSolutionNames");
        }
    }
    else {
        // Number of FlowSolution_t nodes
        if (cg_nsols(index_file, index_base, index_zone, &num_solutions) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure:cg_nsols");
        }

        // 固定値の場合は、すべてのCGNS:FlowSolutionから検索する.
        for (index=1; index<=num_solutions; index++) {
            // FlowSolution名称, 定義位置の取得
            if (cg_sol_info(index_file, index_base, index_zone, index, cgns_name, &cgns_location) != CG_OK) {
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure:cg_sol_info");
            }
            if (grid_location != this->toGridLocation(cgns_location)) {
                // GridLocationが異なる。
                continue;
            }
            solution_names.push_back(std::string(cgns_name));
        }
    }
    if (solution_names.size() <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "solution_names size is zero");
    }

    // CGNS:FlowSolution名と読込フィールド名の検索
    std::vector<std::string>::const_iterator itr;
    for (itr=solution_names.begin(); itr!=solution_names.end(); itr++) {
        index = this->getCgnsFlowSolutionId(index_file, index_base, index_zone, (*itr));
        if (index <= 0) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "not found CGNS:FlowSolution name[%s].", (*itr).c_str());
        }
        // FlowSolution名称, 定義位置の取得
        if (cg_sol_info(index_file, index_base, index_zone, index, cgns_name, &cgns_location) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure:cg_sol_info");
        }
        if (grid_location != this->toGridLocation(cgns_location)) {
            // GridLocationが異なる。
            continue;
        }

        // SolutionFieldの読込
        if (cg_nfields(index_file, index_base, index_zone, index, &num_fields) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure:cg_nfields");
        }
        found_fieldids.clear();
        for (n = 1; n<=num_fields; n++) {
            if (cg_field_info(index_file, index_base, index_zone, index, n, &cgns_datatype, cgns_name) != CG_OK) {
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure:cg_field_info");
            }
            // 読込フィールド名と同一のCGNS:Fieldの検索
            std::vector<std::string>::iterator find_itr;
            find_itr = std::find_if(cgns_fieldnames.begin(), cgns_fieldnames.end(), insensitive_compare(std::string(cgns_name)));
            if (find_itr != cgns_fieldnames.end()) {
                found_fieldids.push_back(n);
            }
        }
        if (num_cgns_field == (int)found_fieldids.size()) {
            index_solution = index;
            std::copy(found_fieldids.begin(), found_fieldids.end(), std::back_inserter(index_fields) );
            return UDM_OK;
        }
    }
    return UDM_ERROR_HANDLER(UDM_ERROR, "not found index_dfi_field=%d", index_dfi_field);
}

/**
 * CGNS:FlowSolutionを出力する.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param timeslice_step       CGNS読込ステップ回数
 * @param write_constsolutions    固定物理量データの出力フラグ : true=固定物理量データを出力する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutions::writeCgns(
                        int index_file,
                        int index_base,
                        int index_zone,
                        int timeslice_step,
                        bool write_constsolutions)
{
    UdmSize_t field_id, num_fields, n;
    int num_config;
    std::vector<const UdmSolutionFieldConfig*> vertex_configs;
    std::vector<const UdmSolutionFieldConfig*> cell_configs;
    std::vector<const UdmSolutionFieldConfig*> vertex_const_configs;
    std::vector<const UdmSolutionFieldConfig*> cell_const_configs;
    std::vector<const UdmSolutionFieldConfig*>::const_iterator itr;
    bool write_const = false;

    // 出力CGNS:FlowSolution名リストのクリア
    this->clearCgnsWriteSolutions();

    if (this->getFlowSolutionsConfig() == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "flowsolutions_config is null.");
    }

    // Vertex, CellCenter, 固定値毎に取得する.
    num_config = this->getFlowSolutionsConfig()->findSolutionFieldConfigs(vertex_configs, Udm_Vertex, false);
    num_config = this->getFlowSolutionsConfig()->findSolutionFieldConfigs(cell_configs, Udm_CellCenter, false);
    if (write_constsolutions) {
        num_config = this->getFlowSolutionsConfig()->findSolutionFieldConfigs(vertex_const_configs, Udm_Vertex, true);
        num_config = this->getFlowSolutionsConfig()->findSolutionFieldConfigs(cell_const_configs, Udm_CellCenter, true);
    }

    // Vertex:Const出力
    if (vertex_const_configs.size() > 0) {
        // 出力済みであるかチェックする.
        write_const = false;
        if (this->findCgnsFlowSolution(index_file, index_base, index_zone, vertex_const_configs[0], timeslice_step) <= 0) {
            write_const = true;        // 未出力であるので出力する.
        }
        if (write_const) {
            for (itr=vertex_const_configs.begin(); itr!=vertex_const_configs.end(); itr++) {
                const UdmSolutionFieldConfig* config = (*itr);
                if (this->writeCgnsSolutionField(config, index_file, index_base, index_zone, timeslice_step) != UDM_OK) {
                    return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "config name=%s", config->getSolutionName().c_str());
                }
            }
        }
    }

    // CellCenter:Const出力
    if (cell_const_configs.size() > 0) {
        // 出力済みであるかチェックする.
        write_const = false;
        if (this->findCgnsFlowSolution(index_file, index_base, index_zone, cell_const_configs[0], timeslice_step) <= 0) {
            write_const = true;        // 未出力であるので出力する.
        }
        if (write_const) {
            for (itr=cell_const_configs.begin(); itr!=cell_const_configs.end(); itr++) {
                const UdmSolutionFieldConfig* config = (*itr);
                // 出力済みであるかチェックする.
                if (this->findCgnsFlowSolution(index_file, index_base, index_zone, config, timeslice_step) <= 0) {
                    if (this->writeCgnsSolutionField(config, index_file, index_base, index_zone, timeslice_step) != UDM_OK) {
                        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "config name=%s", config->getSolutionName().c_str());
                    }
                }
            }
        }
    }

    // Vertex出力
    for (itr=vertex_configs.begin(); itr!=vertex_configs.end(); itr++) {
        const UdmSolutionFieldConfig* config = (*itr);
        if (this->writeCgnsSolutionField(config, index_file, index_base, index_zone, timeslice_step) != UDM_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "config name=%s", config->getSolutionName().c_str());
        }
    }

    // CellCenter出力
    for (itr=cell_configs.begin(); itr!=cell_configs.end(); itr++) {
        const UdmSolutionFieldConfig* config = (*itr);
        if (this->writeCgnsSolutionField(config, index_file, index_base, index_zone, timeslice_step) != UDM_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "config name=%s", config->getSolutionName().c_str());
        }
    }

    return UDM_OK;
}

/**
 * CGNS:FlowSolutionのリンクを出力する.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param link_output_path        リンク出力ファイル
 * @param linked_files            リンクファイルリスト
 * @param timeslice_step       CGNS出力ステップ回数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutions::writeCgnsLinkFile(
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
    std::vector<std::string>::const_iterator solution_itr;
    char path[256] = {0x00};
    std::string filename;
    int cell_dim, phys_dim;
    cgsize_t sizes[9] = {0x00};
    int n, linked_size;
    std::vector<std::string> solution_names;
    std::vector<std::string> solution_paths;
    std::vector<std::string> solution_files;
    std::vector<std::string> link_names;

    // 出力CGNS:FlowSolution名リストのクリア
    this->clearCgnsWriteSolutions();

    // リンクファイルにCGNS:FlowSolutionが存在するかチェックする.
    filename.clear();
    solution_paths.clear();
    solution_files.clear();

    for (itr=linked_files.begin(); itr!= linked_files.end(); itr++) {
        filename = (*itr);
        // リンクファイルオープン
        if (cg_open(filename.c_str(), CG_MODE_READ, &index_linkfile) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_OPENERROR, "filename=%s, cgns_error=%s", filename.c_str(), cg_get_error());
        }
        // 出力リンクファイルと同じCGNSベースインデックス, CGNSゾーンインデックスとする.
        cg_base_read(index_linkfile, index_base, basename, &cell_dim, &phys_dim);
        cg_zone_read(index_linkfile, index_base, index_zone, zonename, sizes);

        // CGNS:FlowSolution名称の取得
        solution_names.clear();
        if (this->getCgnsIterativeFlowSolutionNames(
                                            solution_names,
                                            index_linkfile, index_base, index_zone,
                                            timeslice_step) != UDM_OK) {
            cg_close(index_linkfile);
            continue;
        }

        // リンク元パスの作成
        for (solution_itr=solution_names.begin(); solution_itr!=solution_names.end(); solution_itr++) {
            sprintf(path, "/%s/%s/%s", basename, zonename, solution_itr->c_str());
            solution_paths.push_back(std::string(path));
            solution_files.push_back(filename);
            link_names.push_back(*solution_itr);
        }
        cg_close(index_linkfile);
    }

    // リンク元CGNS:FlowSolution名称
    linked_size = solution_paths.size();
    if (linked_size <= 0) {
        // CGNS:FlowSolutionはリンクしない。
        return UDM_OK;
    }

    // CGNS:FlowSolutionリンクの作成:CGNS:FlowSolutionリンクは同名リンクとする.
    for (n=0; n<linked_size; n++) {

        // 同名のCGNS:FlowSolutionが存在しないこと。
        if (cg_goto(index_file, index_base, "Zone_t", index_zone, link_names[n].c_str(), 0, "end") == CG_OK) {
            continue;        // 作成済み
        }

        // CGNSリンクの作成
        if (cg_goto(index_file, index_base, "Zone_t", index_zone, "end") != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO, "index_file=%d,index_base=%d,index_zone=%d", index_file, index_base, index_zone);
        }

        // リンクファイルをリンク出力ファイルからの相対パスに変換する.
        std::string linked_relativepath;
        this->getLinkedRelativePath(link_output_path, solution_files[n], linked_relativepath);

        // CGNSリンク出力
        if (cg_link_write(link_names[n].c_str(), linked_relativepath.c_str(), solution_paths[n].c_str()) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "failure:cg_link_write(%s,%s,%s)",link_names[n].c_str(), solution_files[n].c_str(), solution_paths[n].c_str());
        }
        // CGNS:FlowSolution出力名を追加する.
        this->addCgnsWriteSolutions(link_names[n]);
    }

    return UDM_OK;
}


/**
 * CGNS出力前の初期化を行う.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutions::initializeWriteCgns()
{
    this->clearCgnsWriteSolutions();
    return UDM_OK;
}


/**
 * 出力結果CGNS:FlowSolution名リストを取得する.
 * @return        出力結果CGNS:FlowSolution名リスト
 */
const std::vector<std::string>& UdmFlowSolutions::getCgnsWriteSolutions() const
{
    return this->cgns_writesolutions;
}

/**
 * 出力結果CGNS:FlowSolution名をリストに追加する.
 * 追加済みであれば追加しない。
 * @param solution_name        出力結果CGNS:FlowSolution名
 * @return        出力結果CGNS:FlowSolution名リスト数
 */
int UdmFlowSolutions::addCgnsWriteSolutions(const std::string solution_name)
{
    // 追加済みかチェックする.
    std::vector< std::string >::iterator itr = std::find(
            this->cgns_writesolutions.begin(),
            this->cgns_writesolutions.end() , solution_name );
    if( itr != this->cgns_writesolutions.end() ) return this->cgns_writesolutions.size();

    // 追加を行う
    this->cgns_writesolutions.push_back(solution_name);
    return this->cgns_writesolutions.size();
}

/**
 * 出力結果CGNS:FlowSolution名リストをクリアする.
 */
void UdmFlowSolutions::clearCgnsWriteSolutions()
{
    this->cgns_writesolutions.clear();
}

/**
 * 出力結果CGNS:FlowSolution名リスト数を取得する.
 * @return        出力結果CGNS:FlowSolution名リスト
 */
int UdmFlowSolutions::getNumCgnsWriteSolutions() const
{
    return this->cgns_writesolutions.size();
}

/**
 * CGNS:FlowSolutionを出力する.
 * @param config            物理量情報
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param timeslice_step       CGNS出力ステップ番号
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutions::writeCgnsSolutionField(
                        const UdmSolutionFieldConfig* config,
                        int index_file,
                        int index_base,
                        int index_zone,
                        int timeslice_step)
{
    int index_solution, num_solutions, index_field;
    std::string flowsolution_name;
    char name[33];
    int vector_size, n, next_pos;
    GridLocation_t cgns_location;
    UdmDataType_t datatype;
    DataType_t cgns_datatype;
    UdmSize_t data_size;
    UdmEntityVoxels* voxels = NULL;
    int sizeofdata;

    // CGNS:FlowSolution名
    if (config->generateCgnsFlowSolutionName(flowsolution_name, timeslice_step) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure : generateCgnsFlowSolutionName(config name=%s)", config->getSolutionName().c_str());
    }

    // CGNS:SolutionField出力の物理量名:物理量名称+'X','Y',Z'、物理量名称+'1'-'N'
    std::vector<std::string> vector_names;
    config->getVectorSolutionNames(vector_names);
    vector_size = config->getNvectorSize();
    if (vector_names.size() != vector_size) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_VARIABLE, "not equal vector size(name size %d != vector size %d)", vector_names.size(), vector_size);
    }

    // CGNS:FlowSolution名が作成済みであるかチェックする
    index_solution = this->findCgnsFlowSolution(index_file, index_base, index_zone, config, timeslice_step);
    if (index_solution == 0) {
        // CGNS:FlowSolutionを作成する.
        GridLocation_t location = this->toCgnsGridLocation(config->getGridLocation());
        if (cg_sol_write(index_file, index_base, index_zone, flowsolution_name.c_str(), location, &index_solution) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure : cg_sol_write(flowsolution name=%s), cg_error=%s", flowsolution_name.c_str(), cg_get_error());
        }
    }

    datatype = config->getDataType();
    cgns_datatype = this->toCgnsDataType(datatype);
    sizeofdata = this->sizeofDataType(datatype);
    voxels = NULL;
    if (config->getGridLocation() == Udm_Vertex) {
        voxels = this->getParentZone()->getGridCoordinates();
    }
    else if (config->getGridLocation() == Udm_CellCenter) {
        voxels = this->getParentZone()->getSections();
    }
    if (voxels == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "UdmEntityVoxels is null.[GridLocation=%d]", config->getGridLocation());
    }
    data_size = voxels->getNumEntities();

    // データサイズ = フィールドサイズ*成分数
    void *field_datas = this->createDataArray(datatype, data_size*vector_size);
    // データの取得
    if (voxels->getEntitySolutionArray(
                            config->getSolutionName(),
                            1, data_size,
                            vector_size,
                            datatype,
                            field_datas,
                            Udm_MemSequentialArray) != UDM_OK) {
        this->deleteDataArray(field_datas, datatype);
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure : getEntitySolutionArray(field name=%s)", config->getSolutionName().c_str());
    }

    // CGNS:SolutionFieldの出力
    next_pos = 0;
    for (n=0; n<vector_size; n++) {
        if (cg_field_write(index_file, index_base, index_zone, index_solution,
                            cgns_datatype,
                            vector_names[n].c_str(),
                            (char*)field_datas+next_pos,
                            &index_field) != CG_OK) {
            this->deleteDataArray(field_datas, datatype);
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure : cg_field_write(field name=%s)", vector_names[n].c_str());
        }
        next_pos += data_size*sizeofdata;
    }

    this->deleteDataArray(field_datas, datatype);

    // CGNS:FlowSolution出力名を追加する.
    this->addCgnsWriteSolutions(flowsolution_name);

    return UDM_OK;
}


/**
 * 固定値物理量が出力済みであるかチェックする.
 * @param filename        CGNSファイル名
 * @param index_base        CGNSベースインデックス：デフォルト=1
 * @param index_zone        CGNSゾーンインデックス：デフォルト=1
 * @return        true=CGNS:GridCoordinatesが出力済み
 */
bool UdmFlowSolutions::existsCgnsConstantSolutions(
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

    // UDM_CGNS_NAME_FLOWSOLUTION_VERTEX_CONST = "UdmSol_Node_Const"が存在するかチェックする.
    if (cg_goto(index_file, index_base, "Zone_t", index_zone, UDM_CGNS_NAME_FLOWSOLUTION_VERTEX_CONST, 0, "end") == CG_OK) {
        return true;
    }
    // UDM_CGNS_NAME_FLOWSOLUTION_CELLCENTER_CONST = "UdmSol_Cell_Const"が存在するかチェックする.
    if (cg_goto(index_file, index_base, "Zone_t", index_zone, UDM_CGNS_NAME_FLOWSOLUTION_CELLCENTER_CONST, 0, "end") == CG_OK) {
        return true;
    }

    return false;
}

/**
 * 物理量情報出力のCGNS:FlowSolutionインデックスを取得する.
 * 物理量情報から出力されるCGNS:FlowSolution名の既存インデックスを取得する.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param config            物理量情報
 * @param timeslice_step       CGNS出力ステップ番号
 * @return            CGNS:FlowSolutionインデックス（存在しない場合は, 0を返す）
 */
int UdmFlowSolutions::findCgnsFlowSolution(
                        int index_file,
                        int index_base,
                        int index_zone,
                        const UdmSolutionFieldConfig* config,
                        int timeslice_step)
{
    int index_solution, num_solutions;
    std::string flowsolution_name;
    char cgns_name[33];
    GridLocation_t cgns_location;

    if (config == NULL) return 0;

    // CGNS:FlowSolution名
    if (config->generateCgnsFlowSolutionName(flowsolution_name, timeslice_step) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure : generateCgnsFlowSolutionName(config name=%s)", config->getSolutionName().c_str());
    }

    // CGNS:FlowSolution名が作成済みであるかチェックする
    num_solutions = 0;
    if (cg_nsols(index_file, index_base, index_zone, &num_solutions) != CG_OK) {
        num_solutions = 0;
    }
    for (index_solution=1; index_solution<=num_solutions; index_solution++) {
        cg_sol_info(index_file, index_base, index_zone, index_solution, cgns_name, &cgns_location);
        if (strcmp(cgns_name, flowsolution_name.c_str()) == 0) {
            return index_solution;
        }
    }

    return 0;
}


/**
 * 物理量情報出力のCGNS:FlowSolutionインデックスを取得する.
 * 物理量情報から出力されるCGNS:FlowSolution名の既存インデックスを取得する.
 * @param [in] index_file        CGNSファイルインデックス
 * @param [in] index_base        CGNSベースインデックス
 * @param [in] index_zone        CGNSゾーンインデックス
 * @param [in] grid_location      物理量定義位置
 * @param [out] solution_names     CGNS:FlowSolution名リスト
 * @return            CGNS:FlowSolution名リスト数
 */
int UdmFlowSolutions::findCgnsSolutionFieldName(
                        int index_file,
                        int index_base,
                        int index_zone,
                        UdmGridLocation_t grid_location,
                        const std::vector<std::string> &cgns_fieldnames,
                        std::vector<std::string> &solution_names) const
{
    int index_solution, num_solutions;
    std::string flowsolution_name;
    char cgns_solutionname[33] = {0x00};
    char cgns_fieldname[33];
    GridLocation_t cgns_location;
    GridLocation_t find_location = this->toCgnsGridLocation(grid_location);
    int index_field, num_fields = 0;
    DataType_t cgns_datatype;

    solution_names.clear();
    num_solutions = 0;
    if (cg_nsols(index_file, index_base, index_zone, &num_solutions) != CG_OK) {
        num_solutions = 0;
    }
    for (index_solution=1; index_solution<=num_solutions; index_solution++) {
        cg_sol_info(index_file, index_base, index_zone, index_solution, cgns_solutionname, &cgns_location);
        if (find_location != cgns_location) continue;

        // SolutionFieldの読込
        num_fields = 0;
        if (cg_nfields(index_file, index_base, index_zone, index_solution, &num_fields) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure:cg_nfields");
        }
        for (index_field = 1; index_field<=num_fields; index_field++) {
            if (cg_field_info(index_file, index_base, index_zone, index_solution, index_field, &cgns_datatype, cgns_fieldname) != CG_OK) {
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure:cg_field_info");
            }
            // 読込フィールド名と同一のCGNS:Fieldの検索
            std::vector<std::string>::const_iterator find_itr;
            find_itr = std::find_if(cgns_fieldnames.begin(), cgns_fieldnames.end(), insensitive_compare(std::string(cgns_fieldname)));
            if (find_itr != cgns_fieldnames.end()) {
                solution_names.push_back(std::string(cgns_solutionname));
                break;
            }
        }
    }

    return solution_names.size();
}


} /* namespace udm */
