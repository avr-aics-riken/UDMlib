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
 * @file UdmEntity.cpp
 * 構成単位基底クラスのソースファイル
 */

#include "config/UdmSolutionFieldConfig.h"
#include "model/UdmEntity.h"
#include "model/UdmZone.h"
#include "model/UdmFlowSolutions.h"
#include "model/UdmSolutionData.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmEntity::UdmEntity()
{
    this->initialize();
}

/**
 * コンストラクタ
 * @param element_type        要素形状タイプ
 */
UdmEntity::UdmEntity(UdmElementType_t element_type)
{
    this->initialize();
    this->element_type = element_type;
}


/**
 * デストラクタ
 */
UdmEntity::~UdmEntity()
{
    this->initialize();
}


/**
 * 初期化を行う.
 */
void UdmEntity::initialize()
{
    this->clearSolutionData();
    this->my_rankno = -1;
    this->element_type = Udm_ElementTypeUnknown;
    this->reality_type = Udm_Actual;            // 実体ノード, 要素をデフォルトとする
    this->mpi_rankinfos.clear();
    this->previous_rankinfos.clear();
    this->partition_weight = 0.0;
    this->remove_entity = false;
    this->local_id = 0;
}


/**
 * CGNSノードのIDを取得する.
 * @return        CGNSノードID
 */
UdmSize_t UdmEntity::getId() const
{
    return id;
}

/**
 * CGNSノードのIDを設定します.
 * @param id    CGNSノードID
 */
void UdmEntity::setId(UdmSize_t id)
{
    this->id = id;
}

/**
 * 物理量データ数を取得する.
 * @return        物理量データ数
 */
unsigned int UdmEntity::getNumSolutionDatas() const
{
    return this->solution_fields.size();
}

/**
 * 物理量データのデータ値数を取得する.
 * Scalarデータの場合は1, Vectorデータの場合は3を返す.
 * @param solution_name        物理量データ名称
 * @return        データ値数
 */
unsigned int UdmEntity::getNumSolutionValue(const std::string& solution_name) const
{
    std::vector<UdmSolutionData*>::const_iterator itr;
    for (itr = this->solution_fields.begin(); itr != this->solution_fields.end(); itr++) {
        if ((*itr)->equalsName(solution_name)) {
            return (*itr)->getNumSolutionDatas();
        }
    }
    return 0;
}

/**
 * 物理量データ名称の物理データが存在するかチェックする.
 * @param solution_name        物理量データ名称
 * @return        true=存在する.
 */
bool UdmEntity::existsSolutionData(const std::string &solution_name) const
{
    std::vector<UdmSolutionData*>::const_iterator itr;
    for (itr = this->solution_fields.begin(); itr != this->solution_fields.end(); itr++) {
        if ((*itr)->equalsName(solution_name)) {
            return true;
        }
    }
    return false;
}

/**
 * すべての物理量データを削除する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmEntity::clearSolutionData()
{
    if (this->solution_fields.size() <= 0) return UDM_OK;

/************************
    std::vector<UdmSolutionData*>::reverse_iterator ritr = this->solution_fields.rbegin();
    while (ritr != this->solution_fields.rend()) {
        UdmSolutionData* data = *(--(ritr.base()));
        if (data != NULL) {
            delete data;
        }
        this->solution_fields.erase((++ritr).base());
        ritr = this->solution_fields.rbegin();
    }
*******************************/

    std::vector<UdmSolutionData*>::iterator itr = this->solution_fields.begin();
    for (itr= this->solution_fields.begin(); itr != this->solution_fields.end(); itr++) {
        UdmSolutionData* data = (*itr);
        if (data != NULL) {
            delete data;
        }
    }
    this->solution_fields.clear();
    return UDM_OK;
}

/**
 * 物理量データ名称の物理量データを削除する.
 * @param solution_name        物理量データ名称
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmEntity::removeSolutionData(const std::string &solution_name)
{
    std::vector<UdmSolutionData*>::iterator itr;
    for (itr = this->solution_fields.begin(); itr != this->solution_fields.end(); itr++) {
        if ((*itr)->equalsName(solution_name)) {
            this->solution_fields.erase(itr);
            return UDM_OK;
        }
    }
    return UDM_ERROR;
}


/**
 * 要素形状タイプを取得する.
 * @return        要素形状タイプ
 */
UdmElementType_t UdmEntity::getElementType() const
{
    return this->element_type;
}

/**
 * 要素形状タイプを設定する.
 * @param element_type        要素形状タイプ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmEntity::setElementType(UdmElementType_t element_type)
{
    this->element_type = element_type;
    return UDM_OK;
}

/**
 * 仮想ノード,要素タイプを取得する.
 * @return        仮想ノード,要素タイプ
 */
UdmRealityType_t UdmEntity::getRealityType() const
{
    return this->reality_type;
}

/**
 * 仮想ノード,要素タイプを設定する.
 * @param reality_type        仮想ノード,要素タイプ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmEntity::setRealityType(UdmRealityType_t reality_type)
{
    this->reality_type = reality_type;
    return UDM_OK;
}

/**
 * 物理量データを設定可能であるかチェックする.
 * 物理量データ名称が物理量管理クラス(UdmFlowSolutions)に設定済みであること.
 *
 * @param solution_name        物理量データ名称
 * @return        true=物理量データを設定可能
 */
bool UdmEntity::validateSolutionData(const std::string& solution_name) const
{
    bool validate = true;
    // 物理量データ名称が物理量管理クラス(UdmFlowSolutions)に設定済みであること.
    const UdmSolutionFieldConfig* field = this->getSolutionFieldConfig(solution_name);
    if (field == NULL) {
        validate = false;
        UDM_WARNING_HANDLER(UDM_ERROR_NOTFOUND_FLOWSOLUTION_NAME, "solution_name=%s", solution_name.c_str());
        return validate;
    }

    // 物理量定義位置が一致していること.
    UdmGridLocation_t location = field->getGridLocation();
    if (location == Udm_Vertex) {
        if (this->element_type != Udm_NODE) {
            validate = false;
            UDM_WARNING_HANDLER(UDM_ERROR_INVALID_FLOWSOLUTION_GRIDLOCATION,
                                "Solution location is Vertex, but this is not node [solution_name=%s].", solution_name.c_str());
            return validate;
        }
    }
    else if (location == Udm_CellCenter) {
        if (this->element_type == Udm_ElementTypeUnknown) validate = false;
        if (this->element_type == Udm_NODE) validate = false;
        if (!validate) {
            UDM_WARNING_HANDLER(UDM_ERROR_INVALID_FLOWSOLUTION_GRIDLOCATION,
                                "Solution location is CellCenter, but this is not cell [solution_name=%s].", solution_name.c_str());
            return validate;
        }
    }

    return validate;
}

/**
 * 物理量フィールド情報を取得する.
 * @param solution_name        物理量データ名称
 * @return        物理量フィールド情報
 */
const UdmSolutionFieldConfig* UdmEntity::getSolutionFieldConfig(const std::string& solution_name) const
{
    const UdmZone* zone = this->getZone();
    if (zone == NULL) return NULL;
    const UdmFlowSolutions* solutions = zone->getFlowSolutions();
    if (solutions == NULL) return NULL;

    return solutions->getSolutionField(solution_name);
}

/**
 * 物理量フィールド情報を取得する.
 * @param solution_name        物理量フィールドID
 * @return        物理量フィールド情報
 */
const UdmSolutionFieldConfig* UdmEntity::getSolutionFieldConfig(UdmSize_t field_id) const
{
    const UdmZone* zone = this->getZone();
    if (zone == NULL) return NULL;
    const UdmFlowSolutions* solutions = zone->getFlowSolutions();
    if (solutions == NULL) return NULL;

    return solutions->getSolutionField(field_id);
}

/**
 * 物理データクラスを生成する.
 * 物理量フィールド情報のデータ型で生成を行う.
 * @param solution_name         物理量データ名称
 * @param vector        ベクトル型
 * @return        生成物理データクラス
 */
UdmSolutionData* UdmEntity::factorySolutionData(const std::string& solution_name, UdmVectorType_t vector)
{
    const UdmSolutionFieldConfig* field = this->getSolutionFieldConfig(solution_name);
    if (field == NULL) return NULL;

    UdmDataType_t datatype = field->getDataType();
    UdmSolutionData* solution_data = UdmSolutionData::factorySolutionData(solution_name, vector, datatype);
    return solution_data;
}

/**
 * 物理量データ値を設定する.
 * 物理量データ名称が存在しない場合は、追加する.
 * @param solution_name        物理量データ名称
 * @param datatype        物理データ型
 * @param value        物理データ値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmEntity::setSolutionScalar(
                        const std::string& solution_name,
                        UdmDataType_t datatype,
                        const void* value)
{
    UdmError_t error;
    if (datatype == Udm_Integer) {
        int sol_value = ((int*)value)[0];
        error = this->setSolutionScalar<int>(solution_name, sol_value);
    }
    else if (datatype == Udm_LongInteger) {
        long long sol_value = ((long long*)value)[0];
        error = this->setSolutionScalar<long long>(solution_name, sol_value);
    }
    else if (datatype == Udm_RealSingle) {
        float sol_value = ((float*)value)[0];
        error = this->setSolutionScalar<float>(solution_name, sol_value);
    }
    else if (datatype == Udm_RealDouble) {
        double sol_value = ((double*)value)[0];
        error = this->setSolutionScalar<double>(solution_name, sol_value);
    }
    else {
        return UDM_ERROR;
    }
    return error;
}

/**
 * 物理量データ値リストを設定する.
 * 物理量データ名称が存在しない場合は、追加する.
 * @param solution_name        物理量データ名称
 * @param datatype        物理データ型
 * @param values        物理量データ値リスト
 * @param size            物理量データ数 (default = 3)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmEntity::setSolutionVector(
                        const std::string& solution_name,
                        UdmDataType_t datatype,
                        const void* values,
                        unsigned int size)
{
    UdmError_t error;
    if (datatype == Udm_Integer) {
        const int *sol_values = (const int*)values;
        error = this->setSolutionVector<int>(solution_name, sol_values, size);
    }
    else if (datatype == Udm_LongInteger) {
        const long long *sol_values = (const long long*)values;
        error = this->setSolutionVector<long long>(solution_name, sol_values, size);
    }
    else if (datatype == Udm_RealSingle) {
        const float *sol_values = (const float*)values;
        error = this->setSolutionVector<float>(solution_name, sol_values, size);
    }
    else if (datatype == Udm_RealDouble) {
        const double *sol_values = (const double*)values;
        error = this->setSolutionVector<double>(solution_name, sol_values, size);
    }
    else {
        return UDM_ERROR;
    }
    return error;
}


/**
 * クラス情報を文字列出力する:デバッグ用.
 * @param buf        出力文字列
 */
void UdmEntity::toString(std::string& buf) const
{
#ifdef _DEBUG_TRACE
    std::stringstream stream;

    std::vector<UdmSolutionData*>::const_iterator itr;
    for (itr = this->solution_fields.begin(); itr != this->solution_fields.end(); itr++) {
        std::string field_buf;
        (*itr)->toString(field_buf);
        stream << field_buf << std::endl;
    }

    buf += stream.str();
#endif
    return;
}

/**
 * 物理量データを取得する:void*
 * @param [in]  solution_name        物理量名
 * @param [out] values            データ値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmEntity::getSolutionValues(const std::string& solution_name, void* values) const
{
    std::vector<UdmSolutionData*>::const_iterator itr;
    for (itr = this->solution_fields.begin(); itr != this->solution_fields.end(); itr++) {
        if ((*itr)->equalsName(solution_name)) {
            if ((*itr)->getDataType() == Udm_Integer) {
                (*itr)->getSolutionDatas((int*)values);
            }
            else if ((*itr)->getDataType() == Udm_LongInteger) {
                (*itr)->getSolutionDatas((long long*)values);
            }
            else if ((*itr)->getDataType() == Udm_RealSingle) {
                (*itr)->getSolutionDatas((float*)values);
            }
            else if ((*itr)->getDataType() == Udm_RealDouble) {
                (*itr)->getSolutionDatas((double*)values);
            }
            return UDM_OK;
        }
    }
    return UDM_ERROR;
}

/**
 * 物理量データ初期値を設定する.
 * ベクトル型はすべての成分に対して初期設定値を設定する.
 * @param solution_name        物理量データ名称
 * @param value        物理データ初期設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmEntity::initializeValue(const std::string& solution_name, VALUE_TYPE value)
{
    int n;
    if (!this->validateSolutionData(solution_name)) {
        return UDM_ERROR;
    }
    std::vector<UdmSolutionData*>::iterator itr;
    for (itr = this->solution_fields.begin(); itr != this->solution_fields.end(); itr++) {
        if ((*itr)->equalsName(solution_name)) {
            (*itr)->initializeValue<VALUE_TYPE>(value);
            return UDM_OK;
        }
    }

    const UdmSolutionFieldConfig* field = this->getSolutionFieldConfig(solution_name);
    if (field == NULL) return UDM_ERROR;
    UdmVectorType_t vector = field->getVectorType();
    int nvector = field->getNvectorSize();
    UdmSolutionData *solution = this->factorySolutionData(solution_name, vector);
    VALUE_TYPE* values = new VALUE_TYPE[nvector];
    for (n=0; n<nvector; n++) values[n] = value;
    solution->setSolutionDatas(values, nvector);
    this->insertSolutionData(solution);

    return UDM_OK;
}
template UdmError_t UdmEntity::initializeValue<int>(const std::string& solution_name, int value);
template UdmError_t UdmEntity::initializeValue<long long>(const std::string& solution_name, long long value);
template UdmError_t UdmEntity::initializeValue<float>(const std::string& solution_name, float value);
template UdmError_t UdmEntity::initializeValue<double>(const std::string& solution_name, double value);

/**
 * MPIランク番号リスト数を取得する.
 * @return        MPIランク番号リスト数
 */
int UdmEntity::getNumMpiRankInfos() const
{
    return this->mpi_rankinfos.size();
}


/**
 * MPIランク番号リスト数を取得する.
 * @return        MPIランク番号リスト数
 */
const UdmGlobalRankidList& UdmEntity::getMpiRankInfos() const
{
    return this->mpi_rankinfos;
}

/**
 * 以前グローバルIDリストを取得する.
 * @return        以前グローバルIDリスト
 */
const UdmGlobalRankidList& UdmEntity::getPreviousRankInfos() const
{
    return this->previous_rankinfos;
}

/**
 * 以前グローバルIDリスト数を取得する.
 * @return        以前グローバルIDリスト数
 */
int UdmEntity::getNumPreviousRankInfos() const
{
    return this->previous_rankinfos.size();
}

/**
 * MPIランク番号,ローカルIDを取得する.
 * @param [in]  info_id        MPIランク番号リストID（１～）
 * @param [out] rankno         MPIランク番号
 * @param [out] localid        ローカルID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmEntity::getMpiRankInfo(int info_id, int &rankno, UdmSize_t &localid) const
{
    if (info_id <= 0) return UDM_ERROR;
    if (info_id > this->mpi_rankinfos.size()) return UDM_ERROR;

    return this->mpi_rankinfos.getGlobalRankid(info_id, rankno, localid);
}

/**
 * MPIランク番号を追加する.
 * @param  rankno        MPIランク番号
 * @param  localid       ローカルID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmEntity::addMpiRankInfo(int rankno, UdmSize_t localid)
{
    if (rankno < 0) return UDM_ERROR;

    UdmGlobalRankid info(rankno, localid);
    return this->mpi_rankinfos.addGlobalRankid(rankno, localid);
}

/**
 * MPIランク番号を削除する.
 * @param  rankno        MPIランク番号
 * @param  localid       ローカルID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmEntity::removeMpiRankInfo(int rankno, UdmSize_t localid)
{
    if (rankno < 0) return UDM_ERROR;
    return this->mpi_rankinfos.removeGlobalRankid(rankno, localid);
}



/**
 * MPIランク番号を設定する.
 * @param  rankno        MPIランク番号
 * @param  localid       ローカルID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmEntity::setMpiRankInfo(int rankno, UdmSize_t localid)
{
    if (rankno < 0) return UDM_ERROR;

    this->clearMpiRankInfos();
    return this->addMpiRankInfo(rankno, localid);
}


/**
 * MPIランク番号をクリアする.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
void UdmEntity::clearMpiRankInfos()
{
    this->mpi_rankinfos.clear();
}


/**
 * MPIランク番号,ローカルIDが存在するかチェックする.
 * @param  rankno        MPIランク番号
 * @param  localid       ローカルID
 * @return        true=存在する.
 */
bool UdmEntity::existsMpiRankInfo(int rankno, UdmSize_t localid) const
{
    return this->mpi_rankinfos.existsGlobalRankid(rankno, localid);
}

/**
 * MPIランク番号,ローカルIDを検証して不正データを削除する.
 */
void UdmEntity::eraseInvalidMpiRankInfos()
{
    int n;
    bool validate = true;
    int my_rankno = this->getMyRankno();
    this->mpi_rankinfos.eraseInvalidGlobalRankids(my_rankno);

    return;
}

/**
 * 以前のグローバルIDが存在するかチェックする.
 * @param  rankno        以前ランク番号
 * @param  localid       以前ローカルID
 * @return        true=存在する.
 */
bool UdmEntity::existsPreviousRankInfo(int rankno, UdmSize_t localid) const
{
    return this->previous_rankinfos.existsGlobalRankid(rankno, localid);
}

/**
 * 以前のグローバルIDを取得する.
 * @param [in]  info_id        以前のグローバルIDのリストID（１～）
 * @param [out] rankno         以前のランク番号
 * @param [out] localid        以前のローカルID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmEntity::getPreviousRankInfo(int info_id, int &rankno, UdmSize_t &localid) const
{
    if (info_id <= 0) return UDM_ERROR;
    if (info_id > this->previous_rankinfos.size()) return UDM_ERROR;

    return this->previous_rankinfos.getGlobalRankid(info_id, rankno, localid);
}

/**
 * シリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmEntity::serialize(UdmSerializeArchive& archive) const
{
    int n;

    // UdmEntity基本情報
    // ID : CGNSノードID、要素ローカルID, ノードローカルID
    // CGNSノードのデータ型
    // 要素形状タイプ
    this->serializeEntityBase(archive, this->getId(), this->getElementType());

    // 自ランク番号
    archive << this->my_rankno;

    // 以前のID、ランク番号
    archive << this->previous_rankinfos;

    // 仮想ノード,要素タイプ
    archive << this->getRealityType();
    // 物理量データリスト
    int num_fields = this->getNumSolutionDatas();
    if (num_fields > 0) {
        archive << num_fields;        // 物理量データ数
        std::vector<UdmSolutionData*>::const_iterator itr;
        for (itr = this->solution_fields.begin(); itr != this->solution_fields.end(); itr++) {
            const UdmSolutionData* field = (*itr);
            archive << *field;
        }
    }
    else {
        archive << (int)0;
    }
    // MPI:ランク番号リスト
    archive << this->mpi_rankinfos;

    // 分割重み
    archive << this->partition_weight;

    return archive;
}

/**
 * デシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmEntity::deserialize(UdmSerializeArchive& archive)
{
    int n;
    UdmSize_t id;
    UdmElementType_t element_type;
    int my_rankno = -1;
    UdmRealityType_t reality_type;
    int num_fields = 0;
    UdmSize_t previous_id;
    int previous_rankno;

    // ストリームステータスチェック
    if (archive.rdstate() != std::ios_base::goodbit) {
        UDM_ERROR_HANDLER(UDM_ERROR_DESERIALIZE, "rdstate is not goodbit.");
        return archive;
    }

    // UdmEntity基本情報
    this->deserializeEntityBase(archive, id,  element_type);

    // ID : CGNSノードID、要素ローカルID, ノードローカルID
    this->setId(id);
    // 要素形状タイプ
    this->setElementType(element_type);

    // 自ランク番号
    archive >> my_rankno;
    this->setMyRankno(my_rankno);

    // 以前のID、ランク番号
    archive >> this->previous_rankinfos;

    // 仮想ノード,要素タイプ
    archive.read(reality_type, sizeof(reality_type));
    this->setRealityType(reality_type);
    // 物理量データリスト
    archive >> num_fields;        // 物理量データ数
    for (n=1; n<=num_fields; n++) {

#if 0        // tellg, seekg削除
        // 物理量:現在位置
        // std::iostream::pos_type cur = archive.tellg();
        size_t cur = archive.tellg();
#endif
        // 物理量名、データ型、ベクトルタイプを取得する.
        std::string field_name;
        UdmDataType_t field_data_type;
        UdmSize_t sol_id;
        // 物理量名称
        // 物理量データ型
        UdmGeneral::deserializeGeneralBase(archive, sol_id, field_data_type, field_name);

        // 物理量データを作成する.
        UdmSolutionData *field = UdmSolutionData::factorySolutionData(field_name, field_data_type);
        if (field == NULL) {
            UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure : factorySolutionData[name=%s,data_type=%d].", field_name.c_str(), field_data_type);
            return archive;
        }
        field->setId(sol_id);

#if 0        // tellg, seekg削除
        // 物理量:現在位置まで戻す。
        archive.seekg(cur);
#endif

        archive >> *field;

        // 作成物理量データを挿入する.
        this->insertSolutionData(field);

        // ストリームステータスチェック
        if (archive.rdstate() != std::ios_base::goodbit) {
            UDM_ERROR_HANDLER(UDM_ERROR_DESERIALIZE, "rdstate is not goodbit[field_id=%d].", n);
            return archive;
        }
    }

    // MPI:ランク番号リスト
    archive >> this->mpi_rankinfos;

    // 分割重み
    float weight = 0.0;
    archive >> weight;
    this->setPartitionWeight(weight);

    return archive;
}

/**
 * ID,ランク番号での比較を行う.
 * src.rankno < dest.rankno
 * src.id < dest.id
 * @param src        比較UdmEntity:src
 * @param dest        比較UdmEntity:dest
 * @return        true=src < dest
 */
bool UdmEntity::compareIds(const UdmEntity* src, const UdmEntity* dest)
{

    UdmSize_t src_id = 0, dest_id = 0;
    int src_rankno = 0, dest_rankno = 0;
    src_id = src->getId();
    src_rankno = src->getMyRankno();
    dest_id = dest->getId();
    dest_rankno = dest->getMyRankno();

    if (src_rankno > dest_rankno) return false;
    else if (src_rankno < dest_rankno) return true;

    if (src_id > dest_id) return false;
    return true;
}

/**
 * UdmEntityの基本情報をシリアライズを行う.
 * @param [inout] archive        シリアライズ・デシリアライズクラス
 * @param  entity_id            ID
 * @param  entity_elementtype        要素（セル）タイプ
 * @return        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmEntity::serializeEntityBase(
                            UdmSerializeArchive& archive,
                            UdmSize_t entity_id,
                            UdmElementType_t entity_elementtype)
{
    std::string name;

    // ID : CGNSノードID、要素ローカルID, ノードローカルID
    archive << entity_id;                // ID

    // 要素形状タイプ
    archive.write(entity_elementtype, sizeof(entity_elementtype));

    return archive;
}

/**
 * UdmEntityの基本情報をデシリアライズを行う.
 * デシリアライズによって取得した値を返す。
 * @param [inout] archive        シリアライズ・デシリアライズクラス
 * @param [out] entity_id            ID
 * @param [out] entity_elementtype        要素（セル）タイプ
 * @return        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmEntity::deserializeEntityBase(
                            UdmSerializeArchive& archive,
                            UdmSize_t& entity_id,
                            UdmElementType_t &entity_elementtype)
{
    std::string name;

    archive >> entity_id;            // ID

    // 要素形状タイプ
    archive.read(entity_elementtype, sizeof(entity_elementtype));

    return archive;
}


/**
 * 自ランク番号を取得する.
 * @return        自ランク番号.
 */
int UdmEntity::getMyRankno() const
{
    return this->my_rankno;
}

/**
 * 自ランク番号を設定する.
 * @param rankno        自ランク番号
 */
void UdmEntity::setMyRankno(int rankno)
{
    this->my_rankno = rankno;
}

/**
 * 物理量データをコピーする.
 * @param src        元構成単位基底クラス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmEntity::cloneEntity(const UdmEntity& src)
{

    // ID : CGNSノードID、要素ローカルID, ノードローカルID
    this->id = src.id;

    // 物理量データをコピーする.
    this->clearSolutionData();

    // 物理量データリスト
    std::vector<UdmSolutionData*>::const_iterator sol_itr;
    for (sol_itr=src.solution_fields.begin(); sol_itr!=src.solution_fields.end(); sol_itr++) {
        const UdmSolutionData* src_solution = (*sol_itr);
        UdmSolutionData* dest_solution = UdmSolutionData::factorySolutionData(
                                            src_solution->getName(),
                                            src_solution->getVectorType(),
                                            src_solution->getDataType());
        dest_solution->cloneSolutionData(*src_solution);
        // 物理量データを追加する
        this->insertSolutionData(dest_solution);
    }
    // 自ランク番号
    this->my_rankno = src.my_rankno;
    // 要素形状タイプ
    this->element_type = src.element_type;
    // 仮想ノード,要素タイプ
    this->reality_type = src.reality_type;

    // 親構成要素はコピーしない

    // MPI:ランク番号リスト
    this->clearMpiRankInfos();
    this->mpi_rankinfos.cloneGlobalRankidList(src.mpi_rankinfos);

    return UDM_OK;
}

/**
 * 物理量データを追加する.
 * @param field        物理量データ
 * @return        物理量データ数
 */
unsigned int UdmEntity::insertSolutionData(UdmSolutionData* field)
{
    this->solution_fields.push_back(field);
    return this->solution_fields.size();
}

/**
 * グローバルID(ID+myrankno)で比較を行う.
 * ranknoを先に比較を行う.
 * @param src        比較対象
 * @return            -1=thisが小さい,0=同じ,+1=thisが大きい
 */
int UdmEntity::compareGlobalId(const UdmEntity* src)
{
    if (src == NULL) return -1;

    // 差分
    int diff_rank =  this->getMyRankno() - src->getMyRankno();
    long long diff_id =  (unsigned long long)this->getId() - (unsigned long long)src->getId();

    // ランク番号比較
    if (diff_rank != 0) return diff_rank;

    // ID比較
    if (diff_id != 0) return diff_id;

    return 0;

}

/**
 * グローバルID(ID+myrankno)で比較を行う.
 * ranknoを先に比較を行う.
 * @param src_rankno        比較対象ランク番号
 * @param src_id        比較対象ID
 * @return            -1=thisが小さい,0=同じ,+1=thisが大きい
 */
int UdmEntity::compareGlobalId(int src_rankno, UdmSize_t src_id)
{
    // 差分
    int diff_rank =  this->getMyRankno() - src_rankno;
    long long diff_id =  (unsigned long long)this->getId() - (unsigned long long)src_id;

    // ランク番号比較
    if (diff_rank != 0) return diff_rank;
    // ID比較
    if (diff_id != 0) return diff_id;

    return 0;
}


/**
 * 以前の自ランク番号を追加する.
 * @param rankno        以前のランク番号
 * @param localid        以前のID
 */
void UdmEntity::addPreviousRankInfo(int rankno, UdmSize_t localid)
{
    this->previous_rankinfos.addGlobalRankid(rankno, localid);
}


/**
 * MPI:ランク番号、ローカル番号を変更する
 * @param old_rankno        以前のランク番号
 * @param old_localid        以前のID
 * @param new_rankno        新しいランク番号
 * @param new_localid        新しいID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmEntity::updateMpiRankInfo(
                int old_rankno, UdmSize_t old_localid,
                int new_rankno, UdmSize_t new_localid)
{
    if (new_rankno < 0) return UDM_ERROR;
    if (new_localid <= 0) return UDM_ERROR;

    return this->mpi_rankinfos.updateGlobalRankid(old_rankno, old_localid, new_rankno, new_localid);
}

/**
 * 以前のID、ランク番号をクリアする.
 */
void UdmEntity::clearPreviousInfos()
{
    this->previous_rankinfos.clear();
    return;
}


/**
 * ローカルID（構成ノード、セルID＋仮想ノード、セルIDの連番）を取得する.
 * @return        ローカルID
 */
UdmSize_t UdmEntity::getLocalId() const
{
    return this->local_id;
}

/**
 * ローカルID（構成ノード、セルID＋仮想ノード、セルIDの連番）を設定する.
 * @param vid        ローカルID
 */
void UdmEntity::setLocalId(UdmSize_t vid)
{
    this->local_id = vid;
}

/**
 * 分割重みを取得する.
 * @return        分割重み
 */
float UdmEntity::getPartitionWeight() const
{
    return this->partition_weight;
}

/**
 * 分割重みを設定する.
 * @param weight        分割重み
 */
void UdmEntity::setPartitionWeight(float weight)
{
    this->partition_weight = weight;
    if (weight != 0.0) {
        if (this->getZone() != NULL) {
            this->getZone()->setPartitionWeight(true);
        }
    }
}

/**
 * 分割重みをクリアする.
 */
void UdmEntity::clearPartitionWeight()
{
    this->partition_weight = 0.0;
}

/**
 * 節点（ノード）、要素（セル）の削除フラグを取得する.
 * @return        true=節点（ノード）、要素（セル）オブジェクトを削除
 */
bool UdmEntity::isRemoveEntity() const
{
    return this->remove_entity;
}

/**
 * 節点（ノード）、要素（セル）の削除フラグを設定する.
 * @param remove        true=節点（ノード）、要素（セル）オブジェクトを削除
 */
void UdmEntity::setRemoveEntity(bool remove)
{
    this->remove_entity = remove;
}


/**
 * メモリサイズを取得する.
 * @return        メモリサイズ
 */
size_t UdmEntity::getMemSize() const
{
    UdmSize_t size = sizeof(*this);
#ifdef _DEBUG
    printf("solution_fields size=%ld [offset=%ld] \n", sizeof(this->solution_fields), offsetof(UdmEntity, solution_fields));
    printf("my_rankno size=%ld [offset=%ld] \n", sizeof(this->my_rankno), offsetof(UdmEntity, my_rankno));
    printf("element_type size=%ld [offset=%ld] \n", sizeof(this->element_type), offsetof(UdmEntity, element_type));
    printf("reality_type size=%ld [offset=%ld] \n", sizeof(this->reality_type), offsetof(UdmEntity, reality_type));
    printf("mpi_rankinfos size=%ld [count=%d] [offset=%ld] \n",
                        sizeof(this->mpi_rankinfos),
                        this->mpi_rankinfos.size(),
                        offsetof(UdmEntity, mpi_rankinfos));
    printf("previous_rankinfos size=%ld [count=%d] [offset=%ld] \n",
                        sizeof(this->previous_rankinfos),
                        this->previous_rankinfos.size(),
                        offsetof(UdmEntity, previous_rankinfos));
    printf("virtual_id size=%ld [offset=%ld] \n", sizeof(this->local_id), offsetof(UdmEntity, local_id));
    printf("partition_weight size=%ld [offset=%ld] \n", sizeof(this->partition_weight), offsetof(UdmEntity, partition_weight));
    printf("remove_entity size=%ld [offset=%ld] \n", sizeof(this->remove_entity), offsetof(UdmEntity, remove_entity));

    size_t rankinfos_memsize = this->previous_rankinfos.getMemSize();
    printf("previous_rankinfos getMemSize=%ld [count=%d] \n",rankinfos_memsize, this->previous_rankinfos.size());

    size += this->previous_rankinfos.size() * sizeof(UdmGlobalRankid);
#endif

    return size;
}
} /* namespace udm */


