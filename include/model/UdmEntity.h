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

#ifndef _UDMENTITY_H_
#define _UDMENTITY_H_

/**
 * @file UdmEntity.h
 * 構成単位基底クラスのヘッダーファイル
 */

#include "model/UdmSolutionData.h"
#include "model/UdmGlobalRankid.h"
#include "utils/UdmSerialization.h"

namespace udm
{
class UdmZone;
class UdmNode;
class UdmCell;
class UdmSolutionFieldConfig;

/**
 * 構成単位基底クラス.
 * 節点（ノード）、要素（セル）の最小単位の基底クラスです.
 */
class UdmEntity: public UdmBase, public UdmISerializable
{
private:

    /**
     * ID : CGNSノードID、要素ローカルID, ノードローカルID
     */
    UdmSize_t id;

    /**
     * 物理量データリスト
     * キー : 物理量名称
     */
    std::vector<UdmSolutionData*> solution_fields;

    /**
     * グローバルID : 未使用
     * UdmSize_t global_id;
     */

    /**
     * 自ランク番号.
     * 節点（ノード）、要素（セル）の自分のランク番号
     */
    int my_rankno;

    /**
     * 要素形状タイプ
     */
    UdmElementType_t element_type;

    /**
     * 仮想ノード,要素タイプ
     */
    UdmRealityType_t reality_type;

    /**
     * MPI:接続ランク番号リスト
     */
    UdmGlobalRankidList mpi_rankinfos;

    /**
     * 以前の自グローバルIDリスト
     */
    UdmGlobalRankidList previous_rankinfos;

    /**
     * ローカルID.
     * 構成節点＋仮想節点、構成要素＋仮想要素の連番
     * 構成節点, 構成要素ではIDと同じ。
     * 仮想節点, 仮想要素は構成節点, 構成要素の最大IDからの連番
     */
    UdmSize_t local_id;

    /**
     * 分割重み
     */
    float partition_weight;

    /**
     * 節点（ノード）、要素（セル）の削除フラグ
     */
    bool remove_entity;

public:
    UdmEntity();
    UdmEntity(UdmElementType_t element_type);
    virtual ~UdmEntity();

    UdmSize_t getId() const;
    void setId(UdmSize_t id);
    // UdmSolutionData
    unsigned int getNumSolutionDatas() const;
    UdmError_t clearSolutionData();
    unsigned int getNumSolutionValue(const std::string &solution_name) const;
    UdmError_t getSolutionValues(const std::string &solution_name, void *values) const;
    template <class VALUE_TYPE> UdmError_t getSolutionScalar(const std::string &solution_name, VALUE_TYPE &value) const;
    template <class VALUE_TYPE> unsigned int getSolutionVector(const std::string &solution_name, VALUE_TYPE *value)  const;
    template <class VALUE_TYPE> UdmError_t setSolutionScalar(const std::string &solution_name, VALUE_TYPE value);
    template <class VALUE_TYPE> UdmError_t setSolutionVector(const std::string &solution_name, const VALUE_TYPE* values, unsigned int size = 3);
    template <class VALUE_TYPE> UdmError_t initializeValue(const std::string &solution_name, VALUE_TYPE value);
    UdmError_t setSolutionScalar(const std::string &solution_name, UdmDataType_t datatype, const void* value);
    UdmError_t setSolutionVector(const std::string &solution_name, UdmDataType_t datatype, const void* values, unsigned int size = 3);
    bool existsSolutionData(const std::string &solution_name) const;
    UdmError_t removeSolutionData(const std::string &solution_name);

    // global_id
    UdmSize_t getGlobalId() const;
    UdmError_t setGlobalId(UdmSize_t global_id);

    UdmElementType_t getElementType() const;
    UdmError_t setElementType(UdmElementType_t element_type);
    UdmRealityType_t getRealityType() const;
    UdmError_t setRealityType(UdmRealityType_t reality_type);

    // for debug
    void toString(std::string &buf) const;
    // memory size
    size_t getMemSize() const;

    // MPI:ランク番号
    int getNumMpiRankInfos() const;
    UdmError_t getMpiRankInfo(int info_id, int &rankno, UdmSize_t &localid) const;
    UdmError_t setMpiRankInfo(int rankno, UdmSize_t localid);
    UdmError_t addMpiRankInfo(int rankno, UdmSize_t localid);
    UdmError_t removeMpiRankInfo(int rankno, UdmSize_t localid);
    bool existsMpiRankInfo(int rankno, UdmSize_t localid) const;
    UdmError_t updateMpiRankInfo(int old_rankno, UdmSize_t old_localid, int new_rankno, UdmSize_t new_localid);
    const UdmGlobalRankidList& getMpiRankInfos() const;
    void clearMpiRankInfos();
    void eraseInvalidMpiRankInfos();

    // 自ランク番号
    int getMyRankno() const;
    void setMyRankno(int rankno);

    // 以前のグローバルIDリスト
    void addPreviousRankInfo(int rankno, UdmSize_t localid);
    void clearPreviousInfos();
    const UdmGlobalRankidList& getPreviousRankInfos() const;
    bool existsPreviousRankInfo(int rankno, UdmSize_t localid) const;
    int getNumPreviousRankInfos() const;
    UdmError_t getPreviousRankInfo(int info_id, int &rankno, UdmSize_t &localid) const;

    // シリアライズ
    UdmSerializeArchive& serialize(UdmSerializeArchive &archive) const;
    UdmSerializeArchive& deserialize(UdmSerializeArchive &archive);

    static UdmSerializeArchive& serializeEntityBase(
                        UdmSerializeArchive& archive,
                        UdmSize_t entity_id,
                        UdmElementType_t entity_elementtype);
    static UdmSerializeArchive& deserializeEntityBase(
                        UdmSerializeArchive &archive,
                        UdmSize_t &entity_id,
                        UdmElementType_t &entity_elementtype);

    // ソート
    static bool compareIds(const UdmEntity *src, const UdmEntity *dest);
    int compareGlobalId(const UdmEntity *src);
    int compareGlobalId(int src_rankno, UdmSize_t src_id);

    // ローカルID
    UdmSize_t getLocalId() const;
    void setLocalId(UdmSize_t localId);

    // 分割重み
    float getPartitionWeight() const;
    void setPartitionWeight(float partition_weight);
    void clearPartitionWeight();

    // 削除フラグ
    bool isRemoveEntity() const;
    void setRemoveEntity(bool remove);

protected:
    UdmError_t cloneEntity(const UdmEntity &src);
    unsigned int insertSolutionData(UdmSolutionData *field);
    virtual UdmZone* getZone() const = 0;

private:
    void initialize();
    bool validateSolutionData(const std::string &solution_name) const;
    const UdmSolutionFieldConfig* getSolutionFieldConfig(const std::string &solution_name) const;
    const UdmSolutionFieldConfig* getSolutionFieldConfig(UdmSize_t field_id) const;
    UdmSolutionData* factorySolutionData(const std::string &name, UdmVectorType_t vector);

    /**
     * コピーオペレータ：コピー禁止
     * @param src        コピー元オブジェクト
     */
    void operator =(const UdmEntity& src) {}

    /**
     * コピーコンストラクタ：コピー禁止
     * @param src        コピー元オブジェクト
     */
    UdmEntity(const UdmEntity& src) {}
};

/**
 * 物理量データ値を取得する.
 * @param [in]  solution_name        物理量データ名称
 * @param [out] value        取得物理データ値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmEntity::getSolutionScalar(const std::string& solution_name, VALUE_TYPE& value) const
{
    std::vector<UdmSolutionData*>::const_iterator itr;
    for (itr = this->solution_fields.begin(); itr != this->solution_fields.end(); itr++) {
        if ((*itr)->equalsName(solution_name)) {
            value = (VALUE_TYPE)(*itr)->getSolutionData<VALUE_TYPE>();
            return UDM_OK;
        }
    }
    return UDM_ERROR;
}

/**
 * 物理量データ値リストを取得する.
 * @param [in]  solution_name        物理量データ名称
 * @param [out] values        取得物理データ値リスト
 * @return        取得データ数
 */
template<class VALUE_TYPE>
unsigned int UdmEntity::getSolutionVector(const std::string& solution_name, VALUE_TYPE* values) const
{
    std::vector<UdmSolutionData*>::const_iterator itr;
    for (itr = this->solution_fields.begin(); itr != this->solution_fields.end(); itr++) {
        if ((*itr)->equalsName(solution_name)) {
            unsigned int len = (*itr)->getNumSolutionDatas();
            for (unsigned int i=0; i<len; i++) {
                values[i] = (VALUE_TYPE)(*itr)->getSolutionData<VALUE_TYPE>(i+1);
            }
            return len;
        }
    }
    return 0;
}

/**
 * 物理量データ値を設定する.
 * 物理量データ名称が存在しない場合は、追加する.
 * @param solution_name        物理量データ名称
 * @param value        物理データ値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmEntity::setSolutionScalar(const std::string& solution_name, VALUE_TYPE value)
{
    if (!this->validateSolutionData(solution_name)) {
        return UDM_ERROR;
    }
    std::vector<UdmSolutionData*>::iterator itr;
    for (itr = this->solution_fields.begin(); itr != this->solution_fields.end(); itr++) {
        if ((*itr)->equalsName(solution_name)) {
            (*itr)->setDataSize(1);
            (*itr)->setSolutionData<VALUE_TYPE>(value);
            return UDM_OK;
        }
    }
    UdmSolutionData *solution = this->factorySolutionData(solution_name, Udm_Scalar);
    solution->setSolutionData<VALUE_TYPE>(value);
    this->insertSolutionData(solution);

    return UDM_OK;
}

/**
 * 物理量データ値リストを設定する.
 * 物理量データ名称が存在しない場合は、追加する.
 * @param solution_name        物理量データ名称
 * @param values        物理量データ値リスト
 * @param size            物理量データ数 (default = 3)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmEntity::setSolutionVector(const std::string& solution_name, const VALUE_TYPE* values, unsigned int size)
{
    if (!this->validateSolutionData(solution_name)) {
        return UDM_ERROR;
    }
    std::vector<UdmSolutionData*>::iterator itr;
    for (itr = this->solution_fields.begin(); itr != this->solution_fields.end(); itr++) {
        if ((*itr)->equalsName(solution_name)) {
            (*itr)->setSolutionDatas<VALUE_TYPE>(values, size);
            return UDM_OK;
        }
    }

    UdmSolutionData *solution = this->factorySolutionData(solution_name, Udm_Vector);
    solution->setSolutionDatas(values, size);
    this->insertSolutionData(solution);

    return UDM_OK;
}


} /* namespace udm */

#endif /* _UDMENTITY_H_ */
