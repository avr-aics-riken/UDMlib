/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmSolutionFieldConfig.cpp
 * [物理量名称]データクラスのソースファイル
 */

#include "config/UdmSolutionFieldConfig.h"

namespace udm {


/**
 * コンストラクタ
 */
UdmSolutionFieldConfig::UdmSolutionFieldConfig() : UdmConfigBase(NULL)
{
    this->initialize();
}

/**
 * コンストラクタ : TextParser
 * @param parser        TextParser
 */
UdmSolutionFieldConfig::UdmSolutionFieldConfig(TextParser *parser) : UdmConfigBase(parser)
{
    this->initialize();
}

/**
 * コピーコンストラクタ
 * @param src_solution_config        コピー元物理量情報
 */
UdmSolutionFieldConfig::UdmSolutionFieldConfig(const UdmSolutionFieldConfig &src_solution_config) : UdmConfigBase(NULL)
{
    this->setSolutionId(src_solution_config.solution_id);        ///< 物理量ID
    this->setSolutionName(src_solution_config.solution_name);        ///< 物理量名称
    this->setGridLocation(src_solution_config.grid_location);            ///< 物理量適用位置
    this->setDataType(src_solution_config.data_type);            ///< データ型
    this->setVectorType(src_solution_config.vector_type);            ///< ベクトル型
    this->setNvectorSize(src_solution_config.nvector_size);            ///< N成分データ数
    this->setConstantFlag(src_solution_config.constant_flag);            ///< 固定値フラグ
}

/**
 * コンストラクタ : 物理量情報
 * @param solution_name        物理量名称
 * @param grid_location        物理量適用位置
 * @param data_type            データ型
 * @param vector_type        ベクトル型
 * @param nvector_size            N成分数
 * @param constant_flag        固定値フラグ
 */
UdmSolutionFieldConfig::UdmSolutionFieldConfig(
                    const std::string     &solution_name,
                    UdmGridLocation_t    grid_location,
                    UdmDataType_t        data_type,
                    UdmVectorType_t        vector_type,
                    int                   nvector_size,
                    bool                  constant_flag)
{
    this->setSolutionName(solution_name);        ///< 物理量名称
    this->setGridLocation(grid_location);            ///< 物理量適用位置
    this->setDataType(data_type);            ///< データ型
    this->setVectorType(vector_type);            ///< ベクトル型
    this->setNvectorSize(nvector_size);            ///< N成分数
    this->setConstantFlag(constant_flag);        ///< 固定値フラグ
}

/**
 * コンストラクタ : 物理量情報
 * @param solution_name        物理量名称
 * @param grid_location        物理量適用位置
 * @param data_type            データ型
 */
UdmSolutionFieldConfig::UdmSolutionFieldConfig(
                    const std::string     &solution_name,
                    UdmGridLocation_t    grid_location,
                    UdmDataType_t        data_type)
{
    this->setSolutionName(solution_name);        ///< 物理量名称
    this->setGridLocation(grid_location);            ///< 物理量適用位置
    this->setDataType(data_type);            ///< データ型
}

/**
 * デストラクタ
 */
UdmSolutionFieldConfig::~UdmSolutionFieldConfig()
{
}


/**
 * 初期化を行う.
 */
void UdmSolutionFieldConfig::initialize()
{
    this->solution_id = 0;
    this->solution_name.clear();
    this->grid_location = Udm_Vertex;
    this->data_type = Udm_DataTypeUnknown;
    this->vector_type = Udm_Scalar;
    this->nvector_size = 1;
    this->constant_flag = false;
}

/**
 * DFIファイルから/FlowSolutionList/[物理量名称]要素を読みこむ.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSolutionFieldConfig::read(const std::string& solution_label)
{
    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }

    std::string label, value, label_leaf;
    UdmError_t ierror = UDM_OK;
    std::vector<std::string> child_list;

    // FlowSolutionList/[物理量名称]
    std::vector<std::string> path_list;
    int count = this->split(solution_label, '/', path_list);
    if (count != 2) {
        return UDM_ERROR_INVALID_DFI_ELEMENTPATH;
    }
    if (strcasecmp(path_list[0].c_str(), UDM_DFI_FLOWSOLUTIONLIST)) {
        return UDM_ERROR_INVALID_DFI_ELEMENTPATH;
    }
    this->solution_name = path_list[1];

    // FlowSolutionList/[物理量名称]/GridLocation
    label = solution_label + "/" + UDM_DFI_GRIDLOCATION;
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->grid_location = this->convertGridLocation(value, &ierror);
    }

    // FlowSolutionList/[物理量名称]/DataType
    label = solution_label + "/" + UDM_DFI_DATATYPE;
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->data_type = this->convertDataType(value, &ierror);
    }

    // FlowSolutionList/[物理量名称]/VectorType
    label = solution_label + "/" + UDM_DFI_VECTORTYPE;
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->vector_type = this->convertVectorType(value, &ierror);
    }

    // FlowSolutionList/[物理量名称]/NvectorSize
    label = solution_label + "/" + UDM_DFI_NVECTORSIZE;
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->nvector_size = this->convertLong(value, &ierror);
    }

    // FlowSolutionList/[物理量名称]/Constant
    label = solution_label + "/" + UDM_DFI_CONSTANT;
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->constant_flag = this->convertBool(value, &ierror);
    }

    return UDM_OK;
}

/**
 * FlowSolutionList/[物理量名称]要素をDFIファイルに出力する.
 * @param fp        出力ファイルポインタ
 * @param indent    出力インデント
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSolutionFieldConfig::write(FILE *fp, unsigned int indent)
{
    std::string label, value;
    UdmError_t error;

    // FlowSolution/[物理量名称] : open
    label = this->solution_name;
    if (this->writeLabel(fp, indent, label) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }
    indent++;

    // FlowSolutionList/[物理量名称]/GridLocation
    label = UDM_DFI_GRIDLOCATION;
    error = this->toString(this->grid_location, value);
    this->writeNodeValue(fp, indent, label, value);

    // FlowSolutionList/[物理量名称]/DataType
    label = UDM_DFI_DATATYPE;
    error = this->toString(this->data_type, value);
    this->writeNodeValue(fp, indent, label, value);

    // FlowSolutionList/[物理量名称]/VectorType
    label = UDM_DFI_VECTORTYPE;
    error = this->toString(this->vector_type, value);
    this->writeNodeValue(fp, indent, label, value);

    // FlowSolutionList/[物理量名称]/NvectorSize
    label = UDM_DFI_NVECTORSIZE;
    error = this->toString((long long)this->nvector_size, value);
    this->writeNodeValue(fp, indent, label, value);

    // FlowSolutionList/[物理量名称]/Constant
    label = UDM_DFI_CONSTANT;
    error = this->toString(this->constant_flag, value);
    this->writeNodeValue(fp, indent, label, value);

    // FlowSolution/[物理量名称] : close
    if (this->writeCloseTab(fp, --indent) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }

    return UDM_OK;
}

/**
 * 物理量IDを取得する.
 * @return        物理量ID
 */
UdmSize_t UdmSolutionFieldConfig::getSolutionId() const
{
    return this->solution_id;
}

/**
 * 物理量IDを設定する.
 * @param solution_id        物理量ID
 */
void UdmSolutionFieldConfig::setSolutionId(UdmSize_t solution_id)
{
    this->solution_id = solution_id;
}


/**
 * 物理量名称を取得する.
 * @return        物理量名称
 */
const std::string& UdmSolutionFieldConfig::getSolutionName() const
{
    return this->solution_name;
}

/**
 * 物理量名称を設定する.
 * @param solution_name        物理量名称
 */
void UdmSolutionFieldConfig::setSolutionName(const std::string& solution_name)
{
    this->solution_name = solution_name;
}

/**
 * 物理量適用位置を取得する.
 * @return    物理量適用位置
 */
UdmGridLocation_t UdmSolutionFieldConfig::getGridLocation() const
{
    return this->grid_location;
}

/**
 * 物理量適用位置を設定する.
 * @param grid_location        物理量適用位置
 */
void UdmSolutionFieldConfig::setGridLocation(UdmGridLocation_t grid_location)
{
    this->grid_location = grid_location;
}


/**
 * データ型を取得する.
 * @return        データ型
 */
UdmDataType_t UdmSolutionFieldConfig::getDataType() const
{
    return this->data_type;
}

/**
 * データ型を設定する.
 * @param data_type        データ型
 */
void UdmSolutionFieldConfig::setDataType(UdmDataType_t data_type)
{
    this->data_type = data_type;
}

/**
 * ベクトル型を取得する.
 * @return        ベクトル型
 */
UdmVectorType_t UdmSolutionFieldConfig::getVectorType() const
{
    return this->vector_type;
}

/**
 * ベクトル型を設定する.
 * @param vector_type        ベクトル型
 */
void UdmSolutionFieldConfig::setVectorType(UdmVectorType_t vector_type)
{
    this->vector_type = vector_type;
}

/**
 * Nデータ成分数を取得する.
 * @return        Nデータ成分数
 */
int UdmSolutionFieldConfig::getNvectorSize() const
{
    return this->nvector_size;
}

/**
 * Nデータ成分数を設定する.
 * @param nvector_size        Nデータ成分数
 */
void UdmSolutionFieldConfig::setNvectorSize(int nvector_size)
{
    this->nvector_size = nvector_size;
}

/**
 * 固定値フラグを取得する.
 * @return        固定値フラグ
 */
bool UdmSolutionFieldConfig::isConstantFlag() const
{
    return this->constant_flag;
}

/**
 * 固定値フラグを設定する.
 * @param constant_flag        固定値フラグ
 */
void UdmSolutionFieldConfig::setConstantFlag(bool constant_flag)
{
    this->constant_flag = constant_flag;
}



/**
 * DFIラベルパスのパラメータの設定値を取得する.
 * 数値は文字列に変換して返す。
 * 複数の設定値のあるパラメータの場合はUDM_ERRORを返す。
 * @param [in]  label_path        DFIラベルパス
 * @param [out] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmSolutionFieldConfig::getDfiValue(const std::string &label_path, std::string &value) const
{
    std::vector<std::string> labels;
    std::string label, solution_name, name;
    int count;
    UdmError_t error = UDM_OK;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count != 3) return UDM_ERROR;
    label = labels[0];
    solution_name = labels[1];
    name = labels[2];
    if (strcasecmp(label.c_str(), UDM_DFI_FLOWSOLUTIONLIST)) {
        return UDM_ERROR;
    }
    if (strcasecmp(solution_name.c_str(), this->solution_name.c_str())) {
        return UDM_ERROR;
    }

    // FlowSolutionList/[物理量名称]/GridLocation
    if (!strcasecmp(name.c_str(), UDM_DFI_GRIDLOCATION)) {
        error = this->toString(this->grid_location, value);
        return error;
    }
    // FlowSolutionList/[物理量名称]/DataType
    else if (!strcasecmp(name.c_str(), UDM_DFI_DATATYPE)) {
        error = this->toString(this->data_type, value);
        return error;
    }
    // FlowSolutionList/[物理量名称]/VectorType
    else if (!strcasecmp(name.c_str(), UDM_DFI_VECTORTYPE)) {
        error = this->toString(this->vector_type, value);
        return error;
    }
    // FlowSolutionList/[物理量名称]/NvectorSize
    else if (!strcasecmp(name.c_str(), UDM_DFI_NVECTORSIZE)) {
        error = this->toString((long long)this->nvector_size, value);
        return error;
    }
    // FlowSolutionList/[物理量名称]/Constant
    else if (!strcasecmp(name.c_str(), UDM_DFI_CONSTANT)) {
        error = this->toString(this->constant_flag, value);
        return error;
    }
    else {
        return UDM_ERROR;
    }

}

/**
 * DFIラベルパスのパラメータの設定値数を取得する.
 * 複数の設定値のあるパラメータの場合は設定値数を返す。
 * １つのみ設定値のパラメータの場合は1を返す。
 * 設定値されていないパラメータの場合は0を返す。
 * 子ノードが存在している場合は、ノード数又はパラメータ数を返す.
 * @param [in]  label_path        DFIラベルパス
 * @param [out] value                設定値
 * @return        パラメータの設定値数
 */
int UdmSolutionFieldConfig::getNumDfiValue(const std::string &label_path) const
{
    std::vector<std::string> labels;
    std::string label, solution_name, name;
    int count;
    UdmError_t error = UDM_OK;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 1) return 0;
    label = labels[0];
    solution_name = labels[1];
    if (strcasecmp(label.c_str(), UDM_DFI_FLOWSOLUTIONLIST)) {
        return 0;
    }
    if (strcasecmp(solution_name.c_str(), this->solution_name.c_str())) {
        return 0;
    }
    // FlowSolution[物理量名称]のパラメータ数を返す
    if (count == 2) {
        return  ( 5 );
    }
    if (count != 3) return 0;
    name = labels[2];

    // FlowSolutionList/[物理量名称]/GridLocation
    if (!strcasecmp(name.c_str(), UDM_DFI_GRIDLOCATION)) {
        return 1;
    }
    // FlowSolutionList/[物理量名称]/DataType
    else if (!strcasecmp(name.c_str(), UDM_DFI_DATATYPE)) {
        return 1;
    }
    // FlowSolutionList/[物理量名称]/VectorType
    else if (!strcasecmp(name.c_str(), UDM_DFI_VECTORTYPE)) {
        return 1;
    }
    // FlowSolutionList/[物理量名称]/NvectorSize
    else if (!strcasecmp(name.c_str(), UDM_DFI_NVECTORSIZE)) {
        return 1;
    }
    // FlowSolutionList/[物理量名称]/Constant
    else if (!strcasecmp(name.c_str(), UDM_DFI_CONSTANT)) {
        return 1;
    }

    return 0;
}

/**
 * DFIラベルパスのパラメータの設定値を設定する.
 * 数値は文字列から変換して設定する。
 * 複数の設定値を持つことの可能なパラメータの場合は１つのみ設定する.
 * @param [in]  label_path        DFIラベルパス
 * @param [in] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmSolutionFieldConfig::setDfiValue(const std::string &label_path, const std::string &value)
{
    std::vector<std::string> labels;
    std::string label, solution_name, name;
    int count;
    UdmError_t error = UDM_OK;

    // FlowSolutionList
    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count != 3) return UDM_ERROR;
    label = labels[0];
    solution_name = labels[1];
    name = labels[2];
    if (strcasecmp(label.c_str(), UDM_DFI_FLOWSOLUTIONLIST)) {
        return UDM_ERROR;
    }
    if (strcasecmp(solution_name.c_str(), this->solution_name.c_str())) {
        return UDM_ERROR;
    }

    // FlowSolutionList/[物理量名称]/GridLocation
    if (!strcasecmp(name.c_str(), UDM_DFI_GRIDLOCATION)) {
        this->grid_location = this->convertGridLocation(value, &error);
        return error;
    }
    // FlowSolutionList/[物理量名称]/DataType
    else if (!strcasecmp(name.c_str(), UDM_DFI_DATATYPE)) {
        this->data_type = this->convertDataType(value, &error);
        return error;
    }
    // FlowSolutionList/[物理量名称]/VectorType
    else if (!strcasecmp(name.c_str(), UDM_DFI_VECTORTYPE)) {
        this->vector_type = this->convertVectorType(value, &error);
        return error;
    }
    // FlowSolutionList/[物理量名称]/NvectorSize
    else if (!strcasecmp(name.c_str(), UDM_DFI_NVECTORSIZE)) {
        this->nvector_size = this->convertLong(value, &error);
        return error;
    }
    // FlowSolutionList/[物理量名称]/Constant
    else if (!strcasecmp(name.c_str(), UDM_DFI_CONSTANT)) {
        this->constant_flag = this->convertBool(value, &error);
        return error;
    }
    else {
        return UDM_ERROR;
    }

    return UDM_OK;
}

/**
 * 成分prefix付きの物理量名称リストを取得する.
 * Scalarデータ : 物理量名称と同じ、１つのみ
 * Vectorデータ : 物理量名称 + 'X','Y',Z'、3ヶ
 * Nvectorデータ : 物理量名称 + '1'-'N'、Nヶ
 * @param [out] vector_names        物理量名称リスト
 * @return        物理量名称リスト数
 */
int UdmSolutionFieldConfig::getVectorSolutionNames(std::vector<std::string> &vector_names) const
{
    if (this->vector_type == Udm_Scalar) {
        vector_names.push_back(this->solution_name);
        return 1;
    }
    else if (this->vector_type == Udm_Vector) {
        vector_names.push_back(this->solution_name + "X");
        vector_names.push_back(this->solution_name + "Y");
        vector_names.push_back(this->solution_name + "Z");
        return 3;
    }
    else if (this->vector_type == Udm_Nvector) {
        for (int i=1; i<=this->nvector_size; i++) {
            char buf[128] = {0x00};
            sprintf(buf, "%s%d", this->solution_name.c_str(), i);
            vector_names.push_back(std::string(buf));
        }
        return this->nvector_size;
    }
    return 0;
}

/**
 * CGNS:FlowSolution名称を作成する.
 * @param [out]  cgns_name                CGNS:FlowSolution名称
 * @param [in]   timeslice_step            ステップ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmSolutionFieldConfig::generateCgnsFlowSolutionName(std::string& cgns_name, int timeslice_step) const
{
    char name[33] = {0x00};
    if (this->constant_flag) {
        if (this->grid_location == Udm_Vertex) cgns_name = UDM_CGNS_NAME_FLOWSOLUTION_VERTEX_CONST;
        else if (this->grid_location == Udm_CellCenter) cgns_name = UDM_CGNS_NAME_FLOWSOLUTION_CELLCENTER_CONST;
        else return UDM_ERROR;
    }
    else {
        if (this->grid_location == Udm_Vertex) {
            sprintf(name, UDM_CGNS_FORMAT_FLOWSOLUTION_VERTEX, timeslice_step);
            cgns_name = name;
        }
        else if (this->grid_location == Udm_CellCenter) {
            sprintf(name, UDM_CGNS_FORMAT_FLOWSOLUTION_CELLCENTER, timeslice_step);
            cgns_name = name;
        }
        else return UDM_ERROR;
    }
    return UDM_OK;
}

} /* namespace udm */
