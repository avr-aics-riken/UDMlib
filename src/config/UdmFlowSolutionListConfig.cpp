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
 * @file UdmFlowSolutionListConfig.cpp
 * index.dfiファイルのFlowSolutionデータクラスのソースファイル
 */

#include "config/UdmFlowSolutionListConfig.h"

namespace udm {

/**
 * コンストラクタ
 */
UdmFlowSolutionListConfig::UdmFlowSolutionListConfig() : UdmConfigBase(NULL)
{
    this->initialize();
}


/**
 * コンストラクタ : TextParser
 * @param parser        TextParser
 */
UdmFlowSolutionListConfig::UdmFlowSolutionListConfig(TextParser *parser) : UdmConfigBase(parser)
{
    this->initialize();
}

/**
 * デストラクタ
 */
UdmFlowSolutionListConfig::~UdmFlowSolutionListConfig()
{
    // 物理量定義リストをクリアする
    this->clear();
}


/**
 * 初期化を行う.
 */
void UdmFlowSolutionListConfig::initialize()
{
    this->solution_list.clear();
}

/**
 * DFIファイルからFlowSolutionList要素を読みこむ.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutionListConfig::read()
{
    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }

    std::string label, value, solution_name, label_leaf;
    std::vector<std::string> child_list;

    // FlowSolutionList
    label = "/FlowSolutionList";
    if (!this->checkNode(label)) {
        return UDM_WARNING_DFI_NOTFOUND_NODE;
    }
    if ( this->getChildLabels(label, child_list) != UDM_OK) {
        return UDM_ERROR;
    }

    std::vector<std::string>::iterator itr;
    for (itr=child_list.begin(); itr<child_list.end(); itr++) {
        UdmSolutionFieldConfig *solution = new UdmSolutionFieldConfig(this->parser);
        if( solution->read(*itr) == UDM_OK ) {
            this->solution_list.insert(std::make_pair(solution->getSolutionName(), solution));
            solution->setSolutionId(this->solution_list.size());
        }
    }

    return UDM_OK;
}

/**
 * FlowSolutionList要素をDFIファイルに出力する.
 * @param fp        出力ファイルポインタ
 * @param indent    出力インデント
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutionListConfig::write(FILE *fp, unsigned int indent)
{
    std::string label, value;
    UdmError_t error;

    // FlowSolutionList : open
    label = "FlowSolutionList";
    if (this->writeLabel(fp, indent, label) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }
    indent++;

    // FlowSolutionList/[物理量名称]
    std::map<std::string, UdmSolutionFieldConfig*>::const_iterator itr;
    for (itr = this->solution_list.begin(); itr != this->solution_list.end(); itr++) {
        error = itr->second->write(fp, indent);
    }

    // FlowSolutionList : close
    if (this->writeCloseTab(fp, --indent) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }

    return UDM_OK;
}


/**
 * 物理量情報を取得する:const.
 * @param solution_name        物理量変数名称
 * @return        物理量情報
 */
const UdmSolutionFieldConfig* UdmFlowSolutionListConfig::getSolutionFieldConfig(const std::string &solution_name) const
{
    if (this->solution_list.size() <= 0) return NULL;
    std::map<std::string, UdmSolutionFieldConfig*>::const_iterator  itr;

    itr = this->solution_list.find(solution_name);
    if (itr != this->solution_list.end()) {
        return (*itr).second;
    }
    return NULL;
}

/**
 * 物理量情報を取得する.
 * @param solution_name        物理量変数名称
 * @return        物理量情報
 */
UdmSolutionFieldConfig* UdmFlowSolutionListConfig::getSolutionFieldConfig(const std::string &solution_name)
{
    if (this->solution_list.size() <= 0) return NULL;
    std::map<std::string, UdmSolutionFieldConfig*>::iterator  itr;

    itr = this->solution_list.find(solution_name);
    if (itr != this->solution_list.end()) {
        return (*itr).second;
    }
    return NULL;
}

/**
 * 物理量情報を設定する.
 * 同一物理量変数名称が存在していれば置き換える.
 * 同一物理量変数名称が存在していなければ追加する。
 * @param solution        物理量情報
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutionListConfig::setSolutionFieldConfig(const UdmSolutionFieldConfig* solution)
{
    if (solution == NULL) return UDM_ERROR;
    std::string solution_name = solution->getSolutionName();
    if (this->existsSolutionConfig(solution_name)) {
        this->solution_list[solution_name] = new UdmSolutionFieldConfig(*solution);
    }
    else {
        this->solution_list.insert(std::make_pair(solution_name, new UdmSolutionFieldConfig(*solution)));
    }
    std::map<std::string, UdmSolutionFieldConfig*>::iterator itr =  this->solution_list.find(solution_name);
    if (itr == this->solution_list.end()) {
        return UDM_ERROR;
    }
    UdmSize_t solution_id = std::distance(this->solution_list.begin(), itr) + 1;
    this->solution_list[solution_name]->setSolutionId(solution_id);

    return UDM_OK;
}

/**
 * 物理量情報のリスト数を取得する.
 * @return        物理量情報リスト数
 */
UdmSize_t UdmFlowSolutionListConfig::getNumSolutionConfig() const
{
    return this->solution_list.size();
}

/**
 * 物理量名称から物理量IDを取得する.
 * 物理量IDは格納リストインデックス+1とする.
 * @param solution_name        物理量名称
 * @return        物理量ID : 物理量名称が存在しない場合は、0を返す。
 */
UdmSize_t UdmFlowSolutionListConfig::getSolutionId(const std::string &solution_name) const
{
    std::map<std::string, UdmSolutionFieldConfig*>::const_iterator itr =  this->solution_list.find(solution_name);
    if (itr == this->solution_list.end()) {
        return 0;
    }
    UdmSize_t solution_id = std::distance(this->solution_list.begin(), itr) + 1;
    return solution_id;
}

/**
 * 物理量IDから物理量名称を取得する.
 * 物理量IDは格納リストインデックス+1とする.
 * @param [in]  solution_id            物理量ID
 * @param [out] solution_name        物理量名称
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutionListConfig::getSolutionName(UdmSize_t solution_id, std::string &solution_name) const
{
    if (solution_id <= 0) return UDM_ERROR;
    if (solution_id > this->getNumSolutionConfig()) return UDM_ERROR;
    std::map<std::string, UdmSolutionFieldConfig*>::const_iterator itr = this->solution_list.begin();
    std::advance( itr, solution_id-1 );            // イテレータの移動
    solution_name = itr->second->getSolutionName();
    return UDM_OK;
}

/**
 * 物理量情報の物理量変数名称のリストを取得する.
 * @param [out] solution_names        物理量変数名称リスト
 * @return        物理量変数名称のリスト数
 */
UdmSize_t UdmFlowSolutionListConfig::getSolutionNameList(std::vector<std::string> &solution_names) const
{
    unsigned int count = 0;
    std::map<std::string, UdmSolutionFieldConfig*>::const_iterator itr;
    for (itr = this->solution_list.begin(); itr != this->solution_list.end(); itr++) {
        solution_names.push_back(itr->first);
        count++;
    }
    return count;
}


/**
 * 物理量情報の定義位置を取得する.
 * 物理量変数名称が存在しない場合はUdm_GridLocationUnknownを返す.
 * @param solution_name        物理量変数名称
 * @return            物理量の定義位置
 */
UdmGridLocation_t UdmFlowSolutionListConfig::getGridLocation(const std::string &solution_name) const
{
    const UdmSolutionFieldConfig* solution = this->getSolutionFieldConfig(solution_name);
    if (solution == NULL) return Udm_GridLocationUnknown;
    return solution->getGridLocation();
}


/**
 * 物理量情報のデータ型を取得する.
 * 物理量変数名称が存在しない場合はUdm_DataTypeUnknownを返す.
 * @param  solution_name        物理量変数名称
 * @return        データ型
 */
UdmDataType_t UdmFlowSolutionListConfig::getDataType(const std::string &solution_name) const
{
    const UdmSolutionFieldConfig* solution = this->getSolutionFieldConfig(solution_name);
    if (solution == NULL) return Udm_DataTypeUnknown;
    return solution->getDataType();
}


/**
 * 物理量情報のベクトル型を取得する.
 * 物理量変数名称が存在しない場合はUdm_VectorTypeUnknownを返す.
 * @param  solution_name        物理量変数名称
 * @return        ベクトル型
 */
UdmVectorType_t UdmFlowSolutionListConfig::getVectorType(const std::string &solution_name) const
{
    const UdmSolutionFieldConfig* solution = this->getSolutionFieldConfig(solution_name);
    if (solution == NULL) return Udm_VectorTypeUnknown;
    return solution->getVectorType();
}


/**
 * 物理量情報のN成分型の成分数を取得する.
 * 物理量変数名称が存在しない場合は0を返す.
 * @param  solution_name        物理量変数名称
 * @return        N成分数
 */
int UdmFlowSolutionListConfig::getNvectorSize(const std::string &solution_name) const
{
    const UdmSolutionFieldConfig* solution = this->getSolutionFieldConfig(solution_name);
    if (solution == NULL) return 0;
    return solution->getNvectorSize();
}


/**
 * 物理量情報の物理データ固定値フラグを取得する.
 * 物理量変数名称が存在しない場合はfalseを返す.
 * @param  solution_name        物理量変数名称
 * @return        true=固定値
 */
bool UdmFlowSolutionListConfig::isConstantFlag(const std::string &solution_name) const
{
    const UdmSolutionFieldConfig* solution = this->getSolutionFieldConfig(solution_name);
    if (solution == NULL) return false;
    return solution->isConstantFlag();

}


/**
 * 物理量情報を取得する.
 * 物理量変数名称が存在していない場合は、UDM_ERRORを返す.
 * @param [in]  solution_name        物理量変数名称
 * @param [out] grid_location        物理量の定義位置
 * @param [out] data_type            データ型
 * @param [out] vector_type        ベクトル型
 * @param [out] nvector_size        N成分数
 * @param [out] constant_flag        物理データ固定値フラグ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutionListConfig::getSolutionFieldInfo(
                const std::string     &solution_name,
                UdmGridLocation_t    &grid_location,
                UdmDataType_t        &data_type,
                UdmVectorType_t        &vector_type,
                int                    &nvector_size,
                bool            &constant_flag) const
{

    const UdmSolutionFieldConfig* solution = this->getSolutionFieldConfig(solution_name);
    if (solution == NULL) {
        return UDM_ERROR;
    }
    grid_location = solution->getGridLocation();
    data_type = solution->getDataType();
    vector_type = solution->getVectorType();
    nvector_size = solution->getNvectorSize();
    constant_flag = solution->isConstantFlag();

    return UDM_OK;
}

/**
 * 物理量情報を設定する.
 * 物理量変数名称が存在している場合は、上書きする.
 * 存在していない場合は、追加する.
 * @param solution_name        物理量変数名称
 * @param grid_location        物理量の定義位置
 * @param data_type            データ型
 * @param vector_type        ベクトル型
 * @param nvector_size        N成分数
 * @param constant_flag        物理データ固定値フラグ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutionListConfig::setSolutionFieldInfo(
                const std::string     &solution_name,
                UdmGridLocation_t    grid_location,
                UdmDataType_t        data_type,
                UdmVectorType_t        vector_type,
                int                    nvector_size,
                bool                 constant_flag)
{
    UdmSolutionFieldConfig* solution = this->getSolutionFieldConfig(solution_name);
    if (solution == NULL) {
        solution = new UdmSolutionFieldConfig(
                                solution_name,
                                grid_location,
                                data_type,
                                vector_type,
                                nvector_size,
                                constant_flag);
        this->solution_list.insert(std::make_pair(solution_name, solution));
        solution->setSolutionId(this->solution_list.size());
    }
    else {
        solution->setGridLocation(grid_location);
        solution->setDataType(data_type);
        solution->setVectorType(vector_type);
        solution->setNvectorSize(nvector_size);
        solution->setConstantFlag(constant_flag);
    }

    return UDM_OK;
}

/**
 * 物理量情報を設定する:基本情報.
 * 物理量変数名称が存在している場合は、上書きする.
 * 存在していない場合は、追加する.
 * ベクトル型=Udm_Scalar, 初期設定値=0.0, 固定値フラグ=falseにて設定する.
 * @param solution_name        物理量変数名称
 * @param grid_location        物理量の定義位置
 * @param data_type            データ型
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutionListConfig::setSolutionFieldInfo(
                        const std::string& solution_name,
                        UdmGridLocation_t grid_location,
                        UdmDataType_t data_type)
{
    return this->setSolutionFieldInfo(
                            solution_name,
                            grid_location,
                            data_type,
                            Udm_Scalar,
                            1,
                            false);
}
/**
 * 物理量変数名称の物理量情報が存在するかチェックする.
 * @param solution_name        物理量変数名称
 * @return        true=物理量情報が存在する
 */
bool UdmFlowSolutionListConfig::existsSolutionConfig(const std::string &solution_name) const
{
    return (this->getSolutionFieldConfig(solution_name) != NULL);
}

/**
 * 物理量変数名称の物理量情報を削除する.
 * @param solution_name        物理量変数名称
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutionListConfig::removeSolutionConfig(const std::string &solution_name)
{
    std::map<std::string, UdmSolutionFieldConfig*>::iterator itr;
    itr = this->solution_list.find(solution_name);
    if (itr != this->solution_list.end()) {
        if (itr->second != NULL) delete itr->second;
        this->solution_list.erase(itr);
        return UDM_OK;
    }
    return UDM_ERROR;
}


/**
 * 物理量変数名称の物理量情報をすべて削除する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFlowSolutionListConfig::clear()
{
    std::map<std::string, UdmSolutionFieldConfig*>::reverse_iterator ritr = this->solution_list.rbegin();
    while (ritr != this->solution_list.rend()) {
        UdmSolutionFieldConfig* config = (--(ritr.base()))->second;
        if (config != NULL) {
            delete config;
        }
        this->solution_list.erase((++ritr).base());
        ritr = this->solution_list.rbegin();
    }
    this->solution_list.clear();
    return UDM_OK;
}

/**
 * DFIラベルパスのパラメータの設定値を取得する.
 * 数値は文字列に変換して返す。
 * 複数の設定値のあるパラメータの場合はUDM_ERRORを返す。
 * @param [in]  label_path        DFIラベルパス
 * @param [out] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmFlowSolutionListConfig::getDfiValue(const std::string &label_path, std::string &value) const
{
    std::vector<std::string> labels;
    std::string label, name;
    int count;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 2) return UDM_ERROR;
    label = labels[0];
    name = labels[1];
    if (strcasecmp(label.c_str(), UDM_DFI_FLOWSOLUTIONLIST)) {
        return UDM_ERROR;
    }

    // FlowSolutionList/[物理量名称]
    std::map<std::string, UdmSolutionFieldConfig*>::const_iterator itr;
    itr = this->solution_list.find(name);
    if (itr != this->solution_list.end()) {
        return itr->second->getDfiValue(label_path, value);
    }

    return UDM_ERROR;
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
int UdmFlowSolutionListConfig::getNumDfiValue(const std::string &label_path) const
{
    std::vector<std::string> labels;
    std::string label, name;
    int count;
    UdmError_t error = UDM_OK;

    // FlowSolutionList : ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 0) return 0;
    label = labels[0];
    if (strcasecmp(label.c_str(), UDM_DFI_FLOWSOLUTIONLIST)) {
        return 0;
    }
    // FlowSolutionListのパラメータ数を返す
    if (count == 1) {
        return this->solution_list.size();
    }

    name = labels[1];
    // FlowSolutionList/[物理量名称]
    std::map<std::string, UdmSolutionFieldConfig*>::const_iterator itr;
    itr = this->solution_list.find(name);
    if (itr != this->solution_list.end()) {
        return itr->second->getNumDfiValue(label_path);
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
UdmError_t UdmFlowSolutionListConfig::setDfiValue(const std::string &label_path, const std::string &value)
{
    std::vector<std::string> labels;
    std::string label, name;
    int count;
    UdmError_t error = UDM_OK;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 1) return UDM_ERROR;
    label = labels[0];
    name = labels[1];
    if (strcasecmp(label.c_str(), UDM_DFI_FLOWSOLUTIONLIST)) {
        return UDM_ERROR;
    }

    // FlowSolutionList/[物理量名称]
    std::map<std::string, UdmSolutionFieldConfig*>::const_iterator itr;
    itr = this->solution_list.find(name);
    if (itr != this->solution_list.end()) {
        return itr->second->setDfiValue(label_path, value);
    }

    else {
        // 新規に追加する
        UdmSolutionFieldConfig *solution = new UdmSolutionFieldConfig();
        solution->setSolutionName(name);
        error = solution->setDfiValue(label_path, value);
        if (error != UDM_OK) {
            delete solution;
            return error;
        }
        this->solution_list.insert(std::make_pair(name, solution));
        solution->setSolutionId(this->solution_list.size());
    }

    return UDM_OK;
}


/**
 * 物理量IDを再振分を行う.
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmFlowSolutionListConfig::rebuildSolutionList()
{
    std::map<std::string, UdmSolutionFieldConfig*>::iterator itr;
    UdmSize_t solution_id = 0;
    for (itr=this->solution_list.begin(); itr!= this->solution_list.end(); itr++) {
        itr->second->setSolutionId(++solution_id);
    }
    return UDM_OK;
}

/**
 * CGNS:FlowSolutionの物理量名称と一致する[物理量名称]データクラスを取得する.
 * 成分名(Vector, Nvector)を付加（成分prefix）した物理量名称と一致しているかチェックする.
 * @param [in]  cgns_field_name        CGNS:FlowSolutionの物理量名称 (成分prefix付き）
 * @param [out] vector_id            成分ID（１～）
 * @return        一致[物理量名称]データクラス
 */
UdmSolutionFieldConfig* UdmFlowSolutionListConfig::findSolutionFieldByCgnsName(
                                    const std::string& cgns_field_name,
                                    int& vector_id) const
{
    if (cgns_field_name.empty()) return NULL;
    std::map<std::string, UdmSolutionFieldConfig*>::const_iterator itr;
    for (itr=this->solution_list.begin(); itr!= this->solution_list.end(); itr++) {
        std::string field_name = itr->first;
        // 成分prefixなしで文字列一致をチェック
        if (strncasecmp(field_name.c_str(), cgns_field_name.c_str(), field_name.length()) != 0) {
            continue;
        }
        // 成分prefix付き物理量名称を取得
        std::vector<std::string> sol_names;
        int num_name = itr->second->getVectorSolutionNames(sol_names);
        if (num_name == 0) {
            UDM_ERROR_HANDLER(UDM_ERROR_INVALID_FLOWSOLUTION_VECTOR, "num_name is zero");
            return NULL;
        }
        std::vector<std::string>::iterator find_itr;
        find_itr = std::find_if(sol_names.begin(), sol_names.end(), insensitive_compare(cgns_field_name));
        if (find_itr != sol_names.end()) {
            vector_id = std::distance(sol_names.begin(), find_itr) + 1;
            return itr->second;
        }

    }
    return NULL;
}

/**
 * 定義位置が頂点（ノード）の物理量を取得する.
 * @param [out]  list        頂点（ノード）物理量リスト
 * @param [in]   location        取得定義位置
 * @param [in]   constant_flag        true=固定値を取得する
 *
 * @return            頂点（ノード）物理量リスト数
 */
int UdmFlowSolutionListConfig::findSolutionFieldConfigs(std::vector<const UdmSolutionFieldConfig*> &list, UdmGridLocation_t location, bool constant_flag) const
{
    std::map<std::string, UdmSolutionFieldConfig*>::const_iterator itr;
    for (itr=this->solution_list.begin(); itr!= this->solution_list.end(); itr++) {
        std::string field_name = itr->first;
        UdmSolutionFieldConfig* config = itr->second;
        if (config->getGridLocation() != location) continue;
        if (constant_flag != config->isConstantFlag()) continue;
        list.push_back(config);
    }
    return list.size();
}


} /* namespace udm */
