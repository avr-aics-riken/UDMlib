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
 * @file UdmSolutionData.cpp
 * 物理量データクラスのソースファイル
 */

#include "model/UdmSolutionData.h"

namespace udm
{

/**
 * 物理量データオブジェクトの生成を行う.
 * @param name            物理量名称
 * @param vector        ベクトルタイプ
 * @param datatype        データ型
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmSolutionData* UdmSolutionData::factorySolutionData(const std::string &name, UdmVectorType_t vector, UdmDataType_t datatype)
{
    if (datatype == Udm_Integer) {
        return new UdmSolutionDataImpl<int>(name, vector);
    }
    else if (datatype == Udm_LongInteger) {
        return new UdmSolutionDataImpl<long long>(name, vector);
    }
    else if (datatype == Udm_RealSingle) {
        return new UdmSolutionDataImpl<float>(name, vector);
    }
    else if (datatype == Udm_RealDouble) {
        return new UdmSolutionDataImpl<double>(name, vector);
    }
    return NULL;
}

/**
 * 物理量データオブジェクトの生成を行う.
 * @param name            物理量名称
 * @param datatype        データ型
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmSolutionData* UdmSolutionData::factorySolutionData(const std::string &name, UdmDataType_t datatype)
{
    UdmSolutionData* solution = NULL;
    if (datatype == Udm_Integer) {
        solution = new UdmSolutionDataImpl<int>(name);
    }
    else if (datatype == Udm_LongInteger) {
        solution =  new UdmSolutionDataImpl<long long>(name);
    }
    else if (datatype == Udm_RealSingle) {
        solution =  new UdmSolutionDataImpl<float>(name);
    }
    else if (datatype == Udm_RealDouble) {
        solution =  new UdmSolutionDataImpl<double>(name);
    }
    if (solution != NULL) {
        solution->setDataType(datatype);
    }

    return solution;
}


/**
 * シリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmSolutionData::serialize(UdmSerializeArchive& archive) const
{
    unsigned int size, n;
    UdmVectorType_t vector_type;
    UdmDataType_t data_type;

    size = this->getNumSolutionDatas();
    vector_type = this->getVectorType();
    data_type = this->getDataType();

    // Genaral基本情報
    this->serializeGeneralBase(archive, this->getId(), data_type, this->getName());
    // this->serializeGeneralBase(archive, data_type, this->getName());

    // 物理量ベクトルデータタイプ
    archive << vector_type;
    // 物理量データサイズ.
    archive << size;

    if (data_type == Udm_Integer) {
        int *values = new int[size];
        this->getSolutionDatas(values);
        for (n = 0; n<size; n++) archive << values[n];
        delete values;
    }
    else if (data_type == Udm_LongInteger) {
        long long *values = new long long[size];
        this->getSolutionDatas(values);
        for (n = 0; n<size; n++) archive << values[n];
        delete values;
    }
    else if (data_type == Udm_RealSingle) {
        float *values = new float[size];
        this->getSolutionDatas(values);
        for (n = 0; n<size; n++) archive << values[n];
        delete values;
    }
    else if (data_type == Udm_RealDouble) {
        double *values = new double[size];
        this->getSolutionDatas(values);
        for (n = 0; n<size; n++) archive << values[n];
        delete values;
    }

    return archive;
}

/**
 * デシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
inline UdmSerializeArchive& UdmSolutionData::deserialize(UdmSerializeArchive& archive)
{
    // 物理量名、データ型、ベクトルタイプを取得する.
    std::string field_name;
    UdmDataType_t data_type;
    UdmVectorType_t vector_type;
    unsigned int size, n;
    UdmSize_t sol_id;

    // ストリームステータスチェック
    if (archive.rdstate() != std::ios_base::goodbit) {
        UDM_ERROR_HANDLER(UDM_ERROR_DESERIALIZE, "rdstate is not goodbit.");
        return archive;
    }

#if 0        // tellg, seekg削除 : deserializeGeneralBaseは読込済み
    // Genaral基本情報
    // 物理量データ型
    // 物理量名称
    // this->deserializeGeneralBase(archive, data_type, field_name);
    this->deserializeGeneralBase(archive, sol_id, data_type, field_name);
    this->setId(sol_id);
    this->setDataType(data_type);
    this->setName(field_name);
#endif
    sol_id = this->getId();
    data_type = this->getDataType();
    field_name = this->getName();

    // 物理量ベクトルデータタイプ
    archive.read(vector_type, sizeof(vector_type));
    this->setVectorType(vector_type);

    // 物理量データサイズ.
    archive >> size;
    if (data_type == Udm_Integer) {
        int *values = new int[size];
        for (n = 0; n<size; n++) archive >> values[n];
        this->setSolutionDatas(values, size);
        delete values;
    }
    else if (data_type == Udm_LongInteger) {
        long long *values = new long long[size];
        for (n = 0; n<size; n++) archive >> values[n];
        this->setSolutionDatas(values, size);
        delete values;
    }
    else if (data_type == Udm_RealSingle) {
        float *values = new float[size];
        for (n = 0; n<size; n++) archive >> values[n];
        this->setSolutionDatas(values, size);
        delete values;
    }
    else if (data_type == Udm_RealDouble) {
        double *values = new double[size];
        for (n = 0; n<size; n++) archive >> values[n];
        this->setSolutionDatas(values, size);
        delete values;
    }

    return archive;
}

/**
 * データ型を設定する:float.
 */
template <>
void UdmSolutionDataImpl<float>::setDataType()
{
    this->setDataType(Udm_RealSingle);
}


/**
 * データ型を設定する:double.
 */
template <>
void UdmSolutionDataImpl<double>::setDataType()
{
    this->setDataType(Udm_RealDouble);
}


/**
 * データ型を設定する:int.
 */
template <>
void UdmSolutionDataImpl<int>::setDataType()
{
    this->setDataType(Udm_Integer);
}


/**
 * データ型を設定する:long long.
 */
template <>
void UdmSolutionDataImpl<long long>::setDataType()
{
    this->setDataType(Udm_LongInteger);
}



} /* namespace udm */
