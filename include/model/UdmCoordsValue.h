/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

#ifndef _UDMCOORDSVALUE_H_
#define _UDMCOORDSVALUE_H_

/**
 * @file UdmCoordsValue.h
 * 座標値クラスのヘッダーファイル
 */

#include "model/UdmGeneral.h"
#include "utils/UdmSerialization.h"

namespace udm
{

/**
 * 座標値クラスの抽象クラス
 * 座標値クラスのラップクラス、座標値クラスのオブジェクト生成を行う。
 */
class UdmCoordsValue:  public UdmISerializable
{
public:
    static UdmCoordsValue* factoryCoordsValue(UdmDataType_t datatype);
    template<class VALUE_TYPE> void getCoords(VALUE_TYPE &x, VALUE_TYPE &y, VALUE_TYPE &z) const;
    template<class VALUE_TYPE> VALUE_TYPE getCoordinateX() const;
    template<class VALUE_TYPE> VALUE_TYPE getCoordinateY() const;
    template<class VALUE_TYPE> VALUE_TYPE getCoordinateZ() const;
    template<class VALUE_TYPE> void setCoords(const VALUE_TYPE coords[3]);
    template<class VALUE_TYPE> void setCoords(VALUE_TYPE x, VALUE_TYPE y, VALUE_TYPE z);
    int compareCoords(const UdmCoordsValue &coords2 ) const;

    // 関係演算子
    bool operator==(const UdmCoordsValue& right) { return (this->compareCoords(right) == 0); };
    bool operator!=(const UdmCoordsValue& right) { return (this->compareCoords(right) != 0); };
    bool operator>=(const UdmCoordsValue& right) { return (this->compareCoords(right) >= 0); };
    bool operator<=(const UdmCoordsValue& right) { return (this->compareCoords(right) <= 0); };
    bool operator>(const UdmCoordsValue& right) { return (this->compareCoords(right) > 0); };
    bool operator<(const UdmCoordsValue& right) { return (this->compareCoords(right) < 0); };
    virtual void toString(std::string &buf) const = 0;
    virtual void setDataType(UdmDataType_t datatype) = 0;
    virtual UdmDataType_t getDataType() const = 0;
    virtual size_t getMemSize() const = 0;

    // シリアライズ
    UdmSerializeArchive& serialize(UdmSerializeArchive &archive) const;
    UdmSerializeArchive& deserialize(UdmSerializeArchive &archive);

};


/**
 * 座標データクラス
 */
template <class DATA_TYPE>
class UdmCoordsValueImpl: public UdmCoordsValue
{
private:
    /**
     * 座標データ
     */
    DATA_TYPE coords[3];

    /**
     * 座標データ型を定義します.
     */
    UdmDataType_t datatype;

public:
    UdmCoordsValueImpl();
    UdmCoordsValueImpl(DATA_TYPE x, DATA_TYPE y, DATA_TYPE z);
    UdmCoordsValueImpl(DATA_TYPE coords[3]);
    UdmCoordsValueImpl(const UdmCoordsValueImpl &coords_value);
    virtual ~UdmCoordsValueImpl();
    const DATA_TYPE* getCoords() const;
    DATA_TYPE* getCoords();
    DATA_TYPE getCoordinateX() const;
    DATA_TYPE getCoordinateY() const;
    DATA_TYPE getCoordinateZ() const;
    void setCoords(const DATA_TYPE coords[3]);
    void setCoords(DATA_TYPE x, DATA_TYPE y, DATA_TYPE z);
    int compareCoords(const UdmCoordsValueImpl &coords2 ) const;

    // 関係演算子
    bool operator==(const UdmCoordsValueImpl& right) { return (this->compareCoords(right) == 0); };
    bool operator!=(const UdmCoordsValueImpl& right) { return (this->compareCoords(right) != 0); };
    bool operator>=(const UdmCoordsValueImpl& right) { return (this->compareCoords(right) >= 0); };
    bool operator<=(const UdmCoordsValueImpl& right) { return (this->compareCoords(right) <= 0); };
    bool operator>(const UdmCoordsValueImpl& right) { return (this->compareCoords(right) > 0); };
    bool operator<(const UdmCoordsValueImpl& right) { return (this->compareCoords(right) < 0); };
    void toString(std::string &buf) const;
    size_t getMemSize() const;

private:
    /**
     * CGNSノードのデータ型を設定する.
     * @param dataType        データ型
     */
    void setDataType(UdmDataType_t datatype)
    {
        // UdmGeneral::setDataType(datatype);
        this->datatype = datatype;
        return;
    }
    /**
     * CGNSノードのデータ型を設定する.
     * @param dataType        データ型
     */
    UdmDataType_t getDataType() const
    {
        return this->datatype;
    }
    void setDataType();
    void initialize(void);
};

template <> void UdmCoordsValueImpl<double>::setDataType();
template <> void UdmCoordsValueImpl<float>::setDataType();
template <> void UdmCoordsValueImpl<int>::setDataType();
template <> void UdmCoordsValueImpl<long long>::setDataType();


/**
 * XYZ座標を取得する.
 * @param [out] x        X座標
 * @param [out] y        Y座標
 * @param [out] z        Z座標
 */
template<class VALUE_TYPE>
void UdmCoordsValue::getCoords(VALUE_TYPE &x, VALUE_TYPE &y, VALUE_TYPE &z) const
{
    UdmDataType_t datatype = this->getDataType();
    if (datatype == Udm_Integer) {
        const UdmCoordsValueImpl<int>* coords = dynamic_cast<const UdmCoordsValueImpl<int>*>(this);
        x = (VALUE_TYPE)coords->getCoordinateX();
        y = (VALUE_TYPE)coords->getCoordinateY();
        z = (VALUE_TYPE)coords->getCoordinateZ();
    }
    else if (datatype == Udm_LongInteger) {
        const UdmCoordsValueImpl<long long>* coords = dynamic_cast<const UdmCoordsValueImpl<long long>*>(this);
        x = (VALUE_TYPE)coords->getCoordinateX();
        y = (VALUE_TYPE)coords->getCoordinateY();
        z = (VALUE_TYPE)coords->getCoordinateZ();
    }
    else if (datatype == Udm_RealSingle) {
        const UdmCoordsValueImpl<float>* coords = dynamic_cast<const UdmCoordsValueImpl<float>*>(this);
        x = (VALUE_TYPE)coords->getCoordinateX();
        y = (VALUE_TYPE)coords->getCoordinateY();
        z = (VALUE_TYPE)coords->getCoordinateZ();
    }
    else if (datatype == Udm_RealDouble) {
        const UdmCoordsValueImpl<double>* coords = dynamic_cast<const UdmCoordsValueImpl<double>*>(this);
        x = (VALUE_TYPE)coords->getCoordinateX();
        y = (VALUE_TYPE)coords->getCoordinateY();
        z = (VALUE_TYPE)coords->getCoordinateZ();
    }
    return;
}

/**
 * X座標を取得する.
 * @return        X座標
 */
template<class VALUE_TYPE>
VALUE_TYPE UdmCoordsValue::getCoordinateX() const
{
    VALUE_TYPE x, y, z;
    this->getCoords(x, y, z);
    return x;
}

/**
 * Y座標を取得する.
 * @return        Y座標
 */
template<class VALUE_TYPE>
VALUE_TYPE UdmCoordsValue::getCoordinateY() const
{
    VALUE_TYPE x, y, z;
    this->getCoords(x, y, z);
    return y;
}

/**
 * Z座標を取得する.
 * @return        Z座標
 */
template<class VALUE_TYPE>
VALUE_TYPE UdmCoordsValue::getCoordinateZ() const
{
    VALUE_TYPE x, y, z;
    this->getCoords(x, y, z);
    return z;
}

/**
 * XYZ座標を設定する.
 * @param coords        XYZ座標
 */
template<class VALUE_TYPE>
void UdmCoordsValue::setCoords(const VALUE_TYPE coords[3])
{
    this->setCoords(coords[0], coords[1], coords[2]);
}

/**
 * XYZ座標を設定する.
 * @param x        X座標
 * @param y        Y座標
 * @param z        Z座標
 */
template<class VALUE_TYPE>
void UdmCoordsValue::setCoords(VALUE_TYPE x, VALUE_TYPE y, VALUE_TYPE z)
{
    UdmDataType_t datatype = this->getDataType();
    if (datatype == Udm_Integer) {
        UdmCoordsValueImpl<int>* coords = dynamic_cast<UdmCoordsValueImpl<int>*>(this);
        coords->setCoords(x, y, z);
    }
    else if (datatype == Udm_LongInteger) {
        UdmCoordsValueImpl<long long>* coords = dynamic_cast<UdmCoordsValueImpl<long long>*>(this);
        coords->setCoords(x, y, z);
    }
    else if (datatype == Udm_RealSingle) {
        UdmCoordsValueImpl<float>* coords = dynamic_cast<UdmCoordsValueImpl<float>*>(this);
        coords->setCoords(x, y, z);
    }
    else if (datatype == Udm_RealDouble) {
        UdmCoordsValueImpl<double>* coords = dynamic_cast<UdmCoordsValueImpl<double>*>(this);
        coords->setCoords(x, y, z);
    }
    return;
}

/**
 * コンストラクタ
 */
template <class DATA_TYPE>
UdmCoordsValueImpl<DATA_TYPE>::UdmCoordsValueImpl()
{
    this->initialize();

    this->coords[0] = 0;
    this->coords[1] = 0;
    this->coords[2] = 0;
}


/**
 * コンストラクタ : XYZ座標
 * @param x        X座標
 * @param y        Y座標
 * @param z        Z座標
 */
template <class DATA_TYPE>
UdmCoordsValueImpl<DATA_TYPE>::UdmCoordsValueImpl(DATA_TYPE x, DATA_TYPE y, DATA_TYPE z)
{
    this->initialize();
    this->setCoords(x, y, z);
}

/**
 * コンストラクタ : 3D座標
 * @param coords    3D座標
 */
template <class DATA_TYPE>
UdmCoordsValueImpl<DATA_TYPE>::UdmCoordsValueImpl(DATA_TYPE coords[3])
{
    this->initialize();
    this->setCoords(coords);
}


/**
 * コピーコンストラクタ
 * @param coords    3D座標
 */
template <class DATA_TYPE>
UdmCoordsValueImpl<DATA_TYPE>::UdmCoordsValueImpl(const UdmCoordsValueImpl &coords_value)
{
    this->initialize();
    this->setCoords(coords_value.coords);
}

/**
 * デストラクタ
 */
template <class DATA_TYPE>
UdmCoordsValueImpl<DATA_TYPE>::~UdmCoordsValueImpl()
{

}

/**
 * 初期化を行う.
 */
template <class DATA_TYPE>
void UdmCoordsValueImpl<DATA_TYPE>::initialize()
{
    this->setDataType();
    memset(this->coords, 0x00, sizeof(this->coords));
}

/**
 * 座標データを取得する : const.
 * @return        座標データ[length=3]
 */
template <class DATA_TYPE>
inline const DATA_TYPE* UdmCoordsValueImpl<DATA_TYPE>::getCoords() const
{
    return this->coords;
}

/**
 * 座標データを取得する.
 * @return        座標データ[length=3]
 */
template <class DATA_TYPE>
inline DATA_TYPE* UdmCoordsValueImpl<DATA_TYPE>::getCoords()
{
    return this->coords;
}

/**
 * X座標を取得する.
 * @return        X座標
 */
template <class DATA_TYPE>
inline DATA_TYPE UdmCoordsValueImpl<DATA_TYPE>::getCoordinateX() const
{
    return this->coords[0];
}

/**
 * Y座標を取得する.
 * @return        Y座標
 */
template <class DATA_TYPE>
inline DATA_TYPE UdmCoordsValueImpl<DATA_TYPE>::getCoordinateY() const
{
    return this->coords[1];

}

/**
 * Z座標を取得する.
 * @return        Z座標
 */
template <class DATA_TYPE>
inline DATA_TYPE UdmCoordsValueImpl<DATA_TYPE>::getCoordinateZ() const
{
    return this->coords[2];

}

/**
 * 座標データを設定する.
 * @param coords       座標データ[length=3]
 */
template <class DATA_TYPE>
inline void UdmCoordsValueImpl<DATA_TYPE>::setCoords(const DATA_TYPE coords[3])
{
    this->setCoords(coords[0], coords[1], coords[2]);
}


/**
 * 座標データを設定する:XYZ座標.
 * @param x        X座標
 * @param y        Y座標
 * @param z        Z座標
 */
template <class DATA_TYPE>
inline void UdmCoordsValueImpl<DATA_TYPE>::setCoords(DATA_TYPE x, DATA_TYPE y, DATA_TYPE z)
{
    this->coords[0] = x;
    this->coords[1] = y;
    this->coords[2] = z;
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
template <class DATA_TYPE>
inline int UdmCoordsValueImpl<DATA_TYPE>::compareCoords(const UdmCoordsValueImpl &coords2 ) const
{
    if (this->coords[2] < coords2.coords[2]) return -1;
    else if (this->coords[2] > coords2.coords[2]) return 1;
    if (this->coords[1] < coords2.coords[1]) return -1;
    else if (this->coords[1] > coords2.coords[1]) return 1;
    if (this->coords[0] < coords2.coords[0]) return -1;
    else if (this->coords[0] > coords2.coords[0]) return 1;

    return 0;
}


/**
 * 座標値を文字列として出力する.
 * @param buf        出力文字列
 */
template <class DATA_TYPE>
inline void UdmCoordsValueImpl<DATA_TYPE>::toString(std::string &buf) const
{
    buf.clear();

    char tmp[128];
    sprintf(tmp, "x = %e \n", (float)this->coords[0]);
    buf += tmp;
    sprintf(tmp, "y = %e \n", (float)this->coords[1]);
    buf += tmp;
    sprintf(tmp, "z = %e \n", (float)this->coords[2]);
    buf += tmp;

    return;
}


/**
 * メモリサイズを取得する.
 * @return        メモリサイズ
 */
template <class DATA_TYPE>
inline size_t UdmCoordsValueImpl<DATA_TYPE>::getMemSize() const
{
    return sizeof(*this);
}


} /* namespace udm */

#endif /* _UDMCOORDSVALUE_H_ */


