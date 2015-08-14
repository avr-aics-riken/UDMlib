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

#ifndef _UDMSOLUTIONDATA_H_
#define _UDMSOLUTIONDATA_H_

/**
 * @file UdmSolutionData.h
 * 物理量データクラスのヘッダーファイル
 */

#include "model/UdmGeneral.h"
#include "utils/UdmSerialization.h"

namespace udm
{
/**
 * 物理量データクラスの抽象クラス
 * 物理量データクラスのラップクラス、物理量データクラスのオブジェクト生成を行う。
 */
class UdmSolutionData: public UdmGeneral, public UdmISerializable
{
public:
    virtual unsigned int getNumSolutionDatas() const = 0;
    virtual UdmVectorType_t getVectorType() const = 0;
    virtual unsigned int getDataSize() const = 0;
    virtual UdmError_t setVectorType(UdmVectorType_t vectortype) = 0;
    virtual UdmError_t setDataSize(unsigned int size) = 0;
    virtual void toString(std::string &buf) const = 0;
    static UdmSolutionData* factorySolutionData(const std::string &name, UdmVectorType_t vector, UdmDataType_t datatype);
    static UdmSolutionData* factorySolutionData(const std::string &name, UdmDataType_t datatype);
    template<class VALUE_TYPE> VALUE_TYPE getSolutionData(unsigned int value_id = 1) const;
    template<class VALUE_TYPE> unsigned int getSolutionDatas(VALUE_TYPE* values) const;
    template<class VALUE_TYPE> UdmError_t setSolutionDatas(const VALUE_TYPE *data, unsigned int size);
    template<class VALUE_TYPE> UdmError_t setSolutionData(VALUE_TYPE data, unsigned int value_id = 1);
    template<class VALUE_TYPE> UdmError_t initializeValue(VALUE_TYPE data);
    virtual UdmError_t cloneSolutionData(const UdmSolutionData &src) = 0;

    /**
     * シリアライズを行う.
     * @param archive        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& serialize(UdmSerializeArchive &archive) const;

    /**
     * デシリアライズを行う.
     * @param archive        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& deserialize(UdmSerializeArchive &archive);

};


/**
 * 物理量データクラス.
 *
 */
template <class DATA_TYPE>
class UdmSolutionDataImpl: public UdmSolutionData
{
private:
    /**
     * 物理量データ
     */
    DATA_TYPE *datas;

    /**
     * 物理量データサイズ.
     * vectortype=Udm_Scalarの場合は1, Udm_Vectorの場合は3とする.
     *
     */
    unsigned int data_size;

    /**
     * 物理量ベクトルデータタイプ
     */
    UdmVectorType_t vectortype;

public:
    UdmSolutionDataImpl(const std::string &name);
    UdmSolutionDataImpl(const std::string &name, UdmVectorType_t vector);
    UdmSolutionDataImpl(const UdmSolutionDataImpl &solution);
    virtual ~UdmSolutionDataImpl();
    unsigned int getSolutionDatas(DATA_TYPE* values) const;
    DATA_TYPE getSolutionData(unsigned int value_id = 1) const;
    unsigned int getNumSolutionDatas() const;
    UdmError_t setSolutionDatas(const DATA_TYPE *data, unsigned int size);
    UdmError_t setSolutionData(DATA_TYPE data, unsigned int value_id = 1);
    UdmError_t initializeValue(DATA_TYPE data);
    UdmVectorType_t getVectorType() const;
    unsigned int getDataSize() const;
    UdmError_t setVectorType(UdmVectorType_t vectortype);
    UdmError_t setDataSize(unsigned int size);
    UdmError_t cloneSolutionData(const UdmSolutionData &src);

    void toString(std::string &buf) const;

private:
    /**
     * CGNSノードのデータ型を設定する.
     * @param dataType        データ型
     */
    void setDataType(UdmDataType_t datatype)
    {
        UdmGeneral::setDataType(datatype);
        return;
    }
    void setDataType();
    void initialize(void);

};

template <> void UdmSolutionDataImpl<double>::setDataType();
template <> void UdmSolutionDataImpl<float>::setDataType();
template <> void UdmSolutionDataImpl<int>::setDataType();
template <> void UdmSolutionDataImpl<long long>::setDataType();


/**
 * 物理量データを取得する.
 * データインデックスが不正な場合は、0を返す。
 * @param id        データインデックス (デフォルト=1) : (1～)
 * @return        物理量データ
 */
template<class VALUE_TYPE>
VALUE_TYPE UdmSolutionData::getSolutionData(unsigned int value_id) const
{
    if (value_id <= 0) return 0;
    UdmDataType_t datatype = this->getDataType();
    if (datatype == Udm_Integer) {
        const UdmSolutionDataImpl<int>* data = dynamic_cast<const UdmSolutionDataImpl<int>*>(this);
        return data->getSolutionData(value_id);
    }
    else if (datatype == Udm_LongInteger) {
        const UdmSolutionDataImpl<long long>* data = dynamic_cast<const UdmSolutionDataImpl<long long>*>(this);
        return data->getSolutionData(value_id);
    }
    else if (datatype == Udm_RealSingle) {
        const UdmSolutionDataImpl<float>* data = dynamic_cast<const UdmSolutionDataImpl<float>*>(this);
        return data->getSolutionData(value_id);
    }
    else if (datatype == Udm_RealDouble) {
        const UdmSolutionDataImpl<double>* data = dynamic_cast<const UdmSolutionDataImpl<double>*>(this);
        return data->getSolutionData(value_id);
    }
    return 0;
}


/**
 * 物理量データリストを取得する.
 * @param [out]  values        物理量データリスト
 * @return        物理量データ数
 */
template<class VALUE_TYPE>
unsigned int UdmSolutionData::getSolutionDatas(VALUE_TYPE* values) const
{
    UdmDataType_t datatype = this->getDataType();
    if (datatype == Udm_Integer) {
        const UdmSolutionDataImpl<int>* data = dynamic_cast<const UdmSolutionDataImpl<int>*>(this);
        return data->getSolutionDatas((int*)values);
    }
    else if (datatype == Udm_LongInteger) {
        const UdmSolutionDataImpl<long long>* data = dynamic_cast<const UdmSolutionDataImpl<long long>*>(this);
        return data->getSolutionDatas((long long*)values);
    }
    else if (datatype == Udm_RealSingle) {
        const UdmSolutionDataImpl<float>* data = dynamic_cast<const UdmSolutionDataImpl<float>*>(this);
        return data->getSolutionDatas((float*)values);
    }
    else if (datatype == Udm_RealDouble) {
        const UdmSolutionDataImpl<double>* data = dynamic_cast<const UdmSolutionDataImpl<double>*>(this);
        return data->getSolutionDatas((double*)values);
    }
    return 0;
}

/**
 * 物理量データリストを設定する.
 * @param values        データリスト
 * @param size       データ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmSolutionData::setSolutionDatas(const VALUE_TYPE *values, unsigned int size)
{
    UdmDataType_t datatype = this->getDataType();
    if (datatype == Udm_Integer) {
        UdmSolutionDataImpl<int>* data = dynamic_cast<UdmSolutionDataImpl<int>*>(this);
        // int sol_datas[size];
        int *sol_datas = new int[size];
        for (unsigned int i=0; i<size; i++) sol_datas[i] = values[i];
        // return data->setSolutionDatas(sol_datas, size);
        UdmError_t error = data->setSolutionDatas(sol_datas, size);
        delete []sol_datas;
        return error;
    }
    else if (datatype == Udm_LongInteger) {
        UdmSolutionDataImpl<long long>* data = dynamic_cast<UdmSolutionDataImpl<long long>*>(this);
        // long long sol_datas[size];
        long long *sol_datas = new long long[size];
        for (unsigned int i=0; i<size; i++) sol_datas[i] = values[i];
        // return data->setSolutionDatas(sol_datas, size);
        UdmError_t error = data->setSolutionDatas(sol_datas, size);
        delete []sol_datas;
        return error;
    }
    else if (datatype == Udm_RealSingle) {
        UdmSolutionDataImpl<float>* data = dynamic_cast<UdmSolutionDataImpl<float>*>(this);
        // float sol_datas[size];
        float *sol_datas = new float[size];
        for (unsigned int i=0; i<size; i++) sol_datas[i] = values[i];
        // return data->setSolutionDatas(sol_datas, size);
        UdmError_t error = data->setSolutionDatas(sol_datas, size);
        delete []sol_datas;
        return error;
    }
    else if (datatype == Udm_RealDouble) {
        UdmSolutionDataImpl<double>* data = dynamic_cast<UdmSolutionDataImpl<double>*>(this);
        // double sol_datas[size];
        double *sol_datas = new double[size];
        for (unsigned int i=0; i<size; i++) sol_datas[i] = values[i];
        // return data->setSolutionDatas(sol_datas, size);
        UdmError_t error = data->setSolutionDatas(sol_datas, size);
        delete []sol_datas;
        return error;
    }

    return UDM_ERROR;
}

/**
 * 物理量データを設定する.
 * @param value        データ値
 * @param value_id        設定インデックス (デフォルト=1) : (1～)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmSolutionData::setSolutionData(VALUE_TYPE value, unsigned int value_id)
{
    if (value_id <= 0) return UDM_ERROR;

    UdmDataType_t datatype = this->getDataType();
    if (datatype == Udm_Integer) {
        UdmSolutionDataImpl<int>* data = dynamic_cast<UdmSolutionDataImpl<int>*>(this);
        return data->setSolutionData((int)value, value_id);
    }
    else if (datatype == Udm_LongInteger) {
        UdmSolutionDataImpl<long long>* data = dynamic_cast<UdmSolutionDataImpl<long long>*>(this);
        return data->setSolutionData((long long)value, value_id);
    }
    else if (datatype == Udm_RealSingle) {
        UdmSolutionDataImpl<float>* data = dynamic_cast<UdmSolutionDataImpl<float>*>(this);
        return data->setSolutionData((float)value, value_id);
    }
    else if (datatype == Udm_RealDouble) {
        UdmSolutionDataImpl<double>* data = dynamic_cast<UdmSolutionDataImpl<double>*>(this);
        return data->setSolutionData((double)value, value_id);
    }
    return UDM_ERROR;

}

/**
 * 物理量初期設定値を設定する.
 * @param data        初期設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class VALUE_TYPE>
UdmError_t UdmSolutionData::initializeValue(VALUE_TYPE value)
{
    UdmDataType_t datatype = this->getDataType();
    if (datatype == Udm_Integer) {
        UdmSolutionDataImpl<int>* data = dynamic_cast<UdmSolutionDataImpl<int>*>(this);
        return data->initializeValue((int)value);
    }
    else if (datatype == Udm_LongInteger) {
        UdmSolutionDataImpl<long long>* data = dynamic_cast<UdmSolutionDataImpl<long long>*>(this);
        return data->initializeValue((long long)value);
    }
    else if (datatype == Udm_RealSingle) {
        UdmSolutionDataImpl<float>* data = dynamic_cast<UdmSolutionDataImpl<float>*>(this);
        return data->initializeValue((float)value);
    }
    else if (datatype == Udm_RealDouble) {
        UdmSolutionDataImpl<double>* data = dynamic_cast<UdmSolutionDataImpl<double>*>(this);
        return data->initializeValue((double)value);
    }
    return UDM_ERROR;
}


/**
 * コンストラクタ
 * @param  name        物理量名称
 */
template <class DATA_TYPE>
UdmSolutionDataImpl<DATA_TYPE>::UdmSolutionDataImpl(const std::string &name)
{
    this->initialize();
    this->setName(name);
}


/**
 * コンストラクタ
 * @param  name        物理量名称
 * @param  vectortype    ベクトルタイプ
 */
template <class DATA_TYPE>
UdmSolutionDataImpl<DATA_TYPE>::UdmSolutionDataImpl(const std::string &name, UdmVectorType_t vectortype)
{
    this->initialize();
    this->setName(name);
    this->setVectorType(vectortype);
}

/**
 * コピーコンストラクタ
 * @param solution    物理量データ
 */
template <class DATA_TYPE>
UdmSolutionDataImpl<DATA_TYPE>::UdmSolutionDataImpl(const UdmSolutionDataImpl &solution)
{
    this->initialize();
    // 物理データコピー
    this->setName(solution.getName());
    this->setVectorType(solution.vectortype);
    this->setSolutionDatas(solution.datas, solution.data_size);
}

/**
 * デストラクタ
 */
template <class DATA_TYPE>
UdmSolutionDataImpl<DATA_TYPE>::~UdmSolutionDataImpl()
{
    if (this->datas != NULL) delete this->datas;
}

/**
 * 物理量データをコピーする.
 * @param src        元物理量データ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class DATA_TYPE>
inline UdmError_t UdmSolutionDataImpl<DATA_TYPE>::cloneSolutionData(const UdmSolutionData& src)
{
    UdmGeneral::cloneGeneral(src);

    this->vectortype = src.getVectorType();

    unsigned int size = src.getDataSize();
    DATA_TYPE *values = new DATA_TYPE[size];
    src.getSolutionDatas(values);

    // 物理量データ
    this->setSolutionDatas(values, size);

    delete[] values;
    return UDM_OK;
}

/**
 * データサイズを取得する.
 * @return        データサイズ
 */
template<class DATA_TYPE>
inline unsigned int UdmSolutionDataImpl<DATA_TYPE>::getDataSize() const
{
    return this->data_size;
}

/**
 * 初期化を行う.
 */
template <class DATA_TYPE>
void UdmSolutionDataImpl<DATA_TYPE>::initialize()
{
    this->setDataType();
    this->datas = NULL;
    this->data_size = 0;
    this->vectortype = Udm_VectorTypeUnknown;
}

/**
 * 物理量データリストを取得する.
 * @param [out]  values        物理量データリスト
 * @return        物理量データ数
 */
template<class DATA_TYPE>
inline unsigned int UdmSolutionDataImpl<DATA_TYPE>::getSolutionDatas(DATA_TYPE* values) const
{
    if (this->datas == NULL) return  UDM_ERROR;
    if (this->data_size <= 0) return UDM_ERROR;
    memcpy(values, this->datas, sizeof(DATA_TYPE)*this->data_size);
    return UDM_OK;
}

/**
 * 物理量データを取得する.
 * データインデックスが不正な場合は、0を返す。
 * @param value_id        データインデックス (デフォルト=1) : (1～)
 * @return        物理量データ
 */
template<class DATA_TYPE>
inline DATA_TYPE UdmSolutionDataImpl<DATA_TYPE>::getSolutionData(unsigned int value_id) const
{
    if (this->datas == NULL) return 0;
    if (value_id <= 0) return 0;
    if (value_id > this->data_size) return 0;
    return this->datas[value_id-1];
}

/**
 * 物理量データ数を取得する.
 * @return        物理量データ数
 */
template<class DATA_TYPE>
inline unsigned int UdmSolutionDataImpl<DATA_TYPE>::getNumSolutionDatas() const
{
    return this->data_size;
}

/**
 * 物理量データリストを設定する.
 * @param datas        データリスト
 * @param size       データ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class DATA_TYPE>
inline UdmError_t UdmSolutionDataImpl<DATA_TYPE>::setSolutionDatas(const DATA_TYPE *datas, unsigned int size)
{
    this->setDataSize(size);
    if (this->datas == NULL) {
        return UDM_ERROR;
    }
    if (size > 0) {
        memcpy(this->datas, datas, sizeof(DATA_TYPE)*size);
    }

    return UDM_OK;
}

/**
 * 物理量データを設定する.
 * @param data        データ値
 * @param id        設定インデックス (デフォルト=1) : (1～)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class DATA_TYPE>
inline UdmError_t UdmSolutionDataImpl<DATA_TYPE>::setSolutionData(DATA_TYPE data, unsigned int value_id)
{
    if (this->datas == NULL) {
        return UDM_ERROR;
    }
    if (value_id <= 0) return UDM_ERROR;
    if (value_id > this->data_size) return UDM_ERROR;
    this->datas[value_id-1] = data;

    return UDM_OK;
}


/**
 * 物理量初期設定値を設定する.
 * @param data        初期設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class DATA_TYPE>
inline UdmError_t UdmSolutionDataImpl<DATA_TYPE>::initializeValue(DATA_TYPE data)
{
    if (this->datas == NULL) {
        return UDM_ERROR;
    }
    unsigned int n;
    for (n=0; n<this->data_size; n++) {
        this->datas[n] = data;
    }

    return UDM_OK;
}


/**
 * ベクトル型を取得する.
 * @return            ベクトル型
 */
template<class DATA_TYPE>
inline UdmVectorType_t UdmSolutionDataImpl<DATA_TYPE>::getVectorType() const
{
    return this->vectortype;
}


/**
 * 物理量データのベクトルタイプを設定する.
 * 物理量データはクリアする.
 * @param vectortype        ベクトルタイプ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class DATA_TYPE>
UdmError_t UdmSolutionDataImpl<DATA_TYPE>::setVectorType(UdmVectorType_t vectortype)
{
    this->vectortype = vectortype;
    if (this->vectortype == Udm_Scalar) return this->setDataSize(1);
    else if (this->vectortype == Udm_Vector) return this->setDataSize(3);
    else return this->setDataSize(0);
}

/**
 * 物理量データサイズを設定する.
 * 物理量データは設定サイズにより再アロケートする.
 * @param size        物理量データサイズ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
template<class DATA_TYPE>
inline UdmError_t UdmSolutionDataImpl<DATA_TYPE>::setDataSize(unsigned int size)
{
    if (this->data_size != size) {
        if (this->datas != NULL) {
            delete this->datas;
            this->datas = NULL;
        }
    }
    if (this->datas == NULL && size > 0) {
        this->datas = new DATA_TYPE[size];
        memset(this->datas, 0x00, sizeof(DATA_TYPE)*size);
    }
    this->data_size = size;

    if (this->data_size == 1) this->vectortype = Udm_Scalar;
    else if (this->data_size == 3) this->vectortype = Udm_Vector;
    else if (this->data_size > 0) this->vectortype = Udm_Nvector;
    else this->vectortype = Udm_VectorTypeUnknown;

    return UDM_OK;
}


/**
 * 物理量データを文字列出力する:デバッグ用.
 * @param buf        出力文字列
 */
template<class DATA_TYPE>
void UdmSolutionDataImpl<DATA_TYPE>::toString(std::string& buf) const
{
    buf.clear();

    char tmp[128];
    sprintf(tmp, "name = %s, ", this->getName().c_str());
    buf += tmp;
    if (this->getDataType() == Udm_Integer) sprintf(tmp, "[int] : ");
    else if (this->getDataType() == Udm_LongInteger) sprintf(tmp, "[long long] : ");
    else if (this->getDataType() == Udm_RealSingle) sprintf(tmp, "[float] : ");
    else if (this->getDataType() == Udm_RealDouble) sprintf(tmp, "[double] : ");
    buf += tmp;

    for (unsigned int i=0; i<this->data_size; i++) {
        if (this->getDataType() == Udm_Integer || this->getDataType() == Udm_LongInteger) {
            sprintf(tmp, "data[%d]=%lld ", i, (long long)this->datas[i]);
        }
        else {
            sprintf(tmp, "data[%d]=%e ", i, (float)this->datas[i]);
        }
        buf += tmp;
    }
    return;
}


} /* namespace udm */
#endif /* _UDMSOLUTIONDATA_H_ */
