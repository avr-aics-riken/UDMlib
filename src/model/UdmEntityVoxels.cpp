/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmEntityVoxels.cpp
 * ノード・要素構成クラスのソースファイル
 */

#include "config/UdmSolutionFieldConfig.h"
#include "model/UdmEntityVoxels.h"
#include "model/UdmZone.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmEntityVoxels::UdmEntityVoxels()
{

}

/**
 * デストラクタ
 */
UdmEntityVoxels::~UdmEntityVoxels()
{
}

/**
 * ノード、要素（セル）に物理量データを設定する.
 * @param field_name        物理量フィールド名
 * @param data_size            データ数
 * @param vector_size        Vector成分数
 * @param datatype            データ型 : 物理量データのデータ型
 * @param solution_array                物理量データ
 * @param memtype            物理量データの配列タイプ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmEntityVoxels::setEntitySolutionArray(
                        const std::string &field_name,
                        UdmSize_t data_size,
                        int vector_size,
                        UdmDataType_t datatype,
                        void* solution_array,
                        UdmMemArrayType_t memtype)
{
    UdmError_t error = UDM_OK;
    UdmSize_t n, next_pos, i;
    int sizeofdata = this->sizeofDataType(datatype);

    if (solution_array == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "solution_array is null.");
    }
    if (data_size <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "data_size is zero.");
    }
    if (data_size != this->getNumEntities()) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "data_size[%d] not equal this size[%d].", data_size, this->getNumEntities());
    }
    if (vector_size <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "vector_size is zero.");
    }
    next_pos = 0;
    void *node_datas = this->createDataArray(datatype, vector_size);
    for (n=1; n<=data_size; n++) {
        UdmEntity *node = this->getEntityById(n);
        if (vector_size == 1) {
            error = node->setSolutionScalar(field_name, datatype, (char*)solution_array+next_pos);
            next_pos += sizeofdata;
        }
        else if (vector_size > 1) {
            if (memtype == Udm_MemSequentialArray) {
                for (i=0; i<vector_size; i++) {
                    memcpy((char*)node_datas+i*sizeofdata,
                            (char*)solution_array+next_pos + i*data_size*sizeofdata,
                            sizeofdata);
                }
                error = node->setSolutionVector(field_name, datatype, node_datas, vector_size);
                next_pos += sizeofdata;
            }
            else {
                error = node->setSolutionVector(field_name, datatype, (char*)solution_array+next_pos, vector_size);
                next_pos += sizeofdata*vector_size;
            }
        }
        if (error != UDM_OK) {
            break;
        }
    }
    this->deleteDataArray(node_datas, datatype);
    return error;
}

/**
 * ノード、要素（セル）から物理量データを取得する.
 * @param field_name        物理量名称
 * @param start_id            取得開始インデックス（１～）
 * @param end_id            取得終了インデックス
 * @param vector_size
 * @param datatype
 * @param solution_array
 * @param memtype
 * @return
 */
UdmError_t UdmEntityVoxels::getEntitySolutionArray(
                        const std::string& field_name,
                        UdmSize_t start_id,
                        UdmSize_t end_id,
                        int vector_size,
                        UdmDataType_t datatype,
                        void* solution_array,
                        UdmMemArrayType_t memtype)
{
    UdmError_t error = UDM_OK;
    UdmSize_t n, next_pos, data_size = 0, len;
    int sizeofdata = this->sizeofDataType(datatype);
    int i, config_nvectorsize;

    if (solution_array == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "solution_array is null.");
    }
    if (start_id <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "start_id is zero.");
    }
    if (end_id > this->getNumEntities()) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "end_id[%d] more than this size[%d].", end_id, this->getNumEntities());
    }
    if (start_id > end_id) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "start_id[%d] more than end_id[%d].", start_id, end_id);
    }
    if (vector_size <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "vector_size is zero.");
    }
    // 物理量情報を取得する
    const UdmSolutionFieldConfig *config = this->getSolutionFieldConfig(field_name);
    if (config == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "SolutionFieldConfig is null[name=%s].", field_name.c_str());
    }
    int config_sizeofdata = this->sizeofDataType(config->getDataType());
    config_nvectorsize = config->getNvectorSize();
    UdmDataType_t config_datatype = config->getDataType();
    void *node_datas = this->createDataArray(config_datatype, config_nvectorsize);
    len = end_id-start_id+1;
    next_pos = 0;
    for (n=start_id; n<=end_id; n++) {
        UdmEntity *node = this->getEntityById(n);
        memset(node_datas, 0x00, config_nvectorsize*config_sizeofdata);
        // 物理量データの取得
        node->getSolutionValues(field_name, node_datas);

        if (vector_size == 1 || memtype == Udm_MemIndexesArray) {
            this->copyDataArray((char*)solution_array+next_pos, datatype, node_datas, config_datatype, vector_size);
            next_pos += sizeofdata*vector_size;
        }
        else if (memtype == Udm_MemSequentialArray) {
            for (i=0; i<vector_size; i++) {
                this->copyDataArray(
                                (char*)solution_array+next_pos+i*len*sizeofdata,
                                datatype,
                                (char*)node_datas+i*config_sizeofdata,
                                config_datatype, 1);
            }
            next_pos += sizeofdata;
        }
    }
    this->deleteDataArray(node_datas, datatype);
    return UDM_OK;
}

/**
 * MPIコミュニケータを取得する.
 * @return        MPIコミュニケータ
 */
MPI_Comm UdmEntityVoxels::getMpiComm() const
{
    if (this->getParentZone() == NULL) return MPI_COMM_NULL;
    return this->getParentZone()->getMpiComm();
}



/**
 * MPIランク番号を取得する.
 * @return        MPIランク番号
 */
int UdmEntityVoxels::getMpiRankno() const
{
    if (this->getParentZone() == NULL) return -1;
    return this->getParentZone()->getMpiRankno();
}

/**
 * MPIプロセス数を取得する.
 * @return        MPIプロセス数
 */
int UdmEntityVoxels::getMpiProcessSize() const
{
    if (this->getParentZone() == NULL) return -1;
    return this->getParentZone()->getMpiProcessSize();
}

} /* namespace udm */
