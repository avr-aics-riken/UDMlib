/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmCoordsValue.cpp
 * 座標値クラスのソースファイル
 */

#include "model/UdmCoordsValue.h"

namespace udm
{

/*****************************************************/
/****  UdmCoordsValue                             ****/
/*****************************************************/

/**
 * 座標データクラスを生成する.
 * @param datatype        生成データ型
 * @return        生成座標データクラス
 */
UdmCoordsValue* UdmCoordsValue::factoryCoordsValue(UdmDataType_t datatype)
{
    UdmCoordsValue* coords = NULL;
    if (datatype == Udm_Integer) {
        coords = new UdmCoordsValueImpl<int>();
    }
    else if (datatype == Udm_LongInteger) {
        coords = new UdmCoordsValueImpl<long long>();
    }
    else if (datatype == Udm_RealSingle) {
        coords = new UdmCoordsValueImpl<float>();
    }
    else if (datatype == Udm_RealDouble) {
        coords = new UdmCoordsValueImpl<double>();
    }
    if (coords != NULL) {
        coords->setDataType(datatype);
    }
    return coords;
}


/**
 * 座標データを比較する.
 * 比較座標１ - 比較座標２の結果を返す.
 * Z座標から比較する. Z座標 -> Y座標 -> X座標
 * @param    coords2            比較座標２
 * @return    -1 = 比較座標１(=this) < 比較座標２
 *              0 = 比較座標１(=this) = 比較座標２
 *             +1 = 比較座標１(=this) > 比較座標２
 */
int UdmCoordsValue::compareCoords(const UdmCoordsValue& coords2) const
{
    UdmDataType_t datatype = this->getDataType();
    if (datatype == Udm_Integer) {
        const UdmCoordsValueImpl<int>* this_impl = dynamic_cast<const UdmCoordsValueImpl<int>*>(this);
        const UdmCoordsValueImpl<int>* coords2_impl = dynamic_cast<const UdmCoordsValueImpl<int>*>(&coords2);
        if (this_impl == NULL || coords2_impl == NULL) return -1;
        return this_impl->compareCoords(*coords2_impl);
    }
    else if (datatype == Udm_LongInteger) {
        const UdmCoordsValueImpl<long long>* this_impl = dynamic_cast<const UdmCoordsValueImpl<long long>*>(this);
        const UdmCoordsValueImpl<long long>* coords2_impl = dynamic_cast<const UdmCoordsValueImpl<long long>*>(&coords2);
        if (this_impl == NULL || coords2_impl == NULL) return -1;
        return this_impl->compareCoords(*coords2_impl);
    }
    else if (datatype == Udm_RealSingle) {
        const UdmCoordsValueImpl<float>* this_impl = dynamic_cast<const UdmCoordsValueImpl<float>*>(this);
        const UdmCoordsValueImpl<float>* coords2_impl = dynamic_cast<const UdmCoordsValueImpl<float>*>(&coords2);
        if (this_impl == NULL || coords2_impl == NULL) return -1;
        return this_impl->compareCoords(*coords2_impl);
    }
    else if (datatype == Udm_RealDouble) {
        const UdmCoordsValueImpl<double>* this_impl = dynamic_cast<const UdmCoordsValueImpl<double>*>(this);
        const UdmCoordsValueImpl<double>* coords2_impl = dynamic_cast<const UdmCoordsValueImpl<double>*>(&coords2);
        if (this_impl == NULL || coords2_impl == NULL) return -1;
        return this_impl->compareCoords(*coords2_impl);
    }
    return -1;
}


/**
 * シリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmCoordsValue::serialize(UdmSerializeArchive& archive) const
{
    // 座標データ : DATA_TYPE coords[3];
    // データ型
    UdmDataType_t datatype = this->getDataType();
    archive << datatype;
    if (datatype == Udm_Integer) {
        int x, y, z;
        this->getCoords(x, y, z);
        archive << x << y << z;
    }
    else if (datatype == Udm_LongInteger) {
        long long x, y, z;
        this->getCoords(x, y, z);
        archive << x << y << z;
    }
    else if (datatype == Udm_RealSingle) {
        float x, y, z;
        this->getCoords(x, y, z);
        archive << x << y << z;
    }
    else if (datatype == Udm_RealDouble) {
        double x, y, z;
        this->getCoords(x, y, z);
        archive << x << y << z;
    }

    return archive;
}

/**
 * デシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmCoordsValue::deserialize(UdmSerializeArchive& archive)
{
    // ストリームステータスチェック
    if (archive.rdstate() != std::ios_base::goodbit) {
        UDM_ERROR_HANDLER(UDM_ERROR_DESERIALIZE, "rdstate is not goodbit.");
        return archive;
    }

    // 座標データ : DATA_TYPE coords[3];
    // データ型
    UdmDataType_t datatype = this->getDataType();

#if 0        // tellg, seekg削除 : 座標データタイプは読込済み
    archive >> datatype;
#endif
    if (datatype == Udm_Integer) {
        int x, y, z;
        archive >> x >> y >> z;
        this->setCoords(x, y, z);
    }
    else if (datatype == Udm_LongInteger) {
        long long x, y, z;
        archive >> x >> y >> z;
        this->setCoords(x, y, z);
    }
    else if (datatype == Udm_RealSingle) {
        float x, y, z;
        archive >> x >> y >> z;
        this->setCoords(x, y, z);
    }
    else if (datatype == Udm_RealDouble) {
        double x, y, z;
        archive >> x >> y >> z;
        this->setCoords(x, y, z);
    }

    return archive;
}

/*****************************************************/
/****  UdmCoordsValueImpl                         ****/
/*****************************************************/
/**
 * データ型を設定する:float.
 */
template <>
void UdmCoordsValueImpl<float>::setDataType()
{
    this->setDataType(Udm_RealSingle);
}


/**
 * データ型を設定する:double.
 */
template <>
void UdmCoordsValueImpl<double>::setDataType()
{
    this->setDataType(Udm_RealDouble);
}


/**
 * データ型を設定する:int.
 */
template <>
void UdmCoordsValueImpl<int>::setDataType()
{
    this->setDataType(Udm_Integer);
}


/**
 * データ型を設定する:long long.
 */
template <>
void UdmCoordsValueImpl<long long>::setDataType()
{
    this->setDataType(Udm_LongInteger);
}


} /* namespace udm */
