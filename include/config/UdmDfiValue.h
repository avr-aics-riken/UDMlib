/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

#ifndef _UDMDFIVALUE_H_
#define _UDMDFIVALUE_H_

/**
 * @file UdmDfiValue.h
 * DFI設定値クラスのヘッダーファイル
 */

#include "config/UdmConfigBase.h"

namespace udm
{

/**
 * DFI設定値クラス
 */
template <class DATA_TYPE>
class UdmDfiValue : public UdmConfigBase
{
private:
    DATA_TYPE        value;        ///< DFI設定値
    bool            is_set;                ///< DFI設定の有無
    UdmDataType_t    value_type;            ///< DFI設定値タイプ

public:
    UdmDfiValue();
    UdmDfiValue(const UdmDfiValue<DATA_TYPE>& src_value);
    UdmDfiValue(const char *name, DATA_TYPE  value);
    virtual ~UdmDfiValue();
    DATA_TYPE getValue() const;
    bool isSetValue() const;
    UdmError_t setValue(DATA_TYPE value);
    UdmError_t toString(std::string &buf);

private:
    void initialize();
    UdmError_t toString(DATA_TYPE value, std::string &buf);

    /**
     * UdmDfiValueクラスでのパラメータの取得は不可とする.
     * @param [in]  label_path        DFIラベルパス
     * @param [out] value                設定値
     * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
     * @deprecated         UdmDfiValueクラスでは使用禁止
     */
    UdmError_t getDfiValue(const std::string &label_path, std::string &value) const
    {
        return UDM_ERROR;
    }

    /**
     * UdmDfiValueクラスでのパラメータの設定は不可とする.
     * @param [in]  label_path        DFIラベルパス
     * @param [in] value                設定値
     * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
     * @deprecated         UdmDfiValueクラスでは使用禁止
     */
    UdmError_t setDfiValue(const std::string &label_path, const std::string &value)
    {
        return UDM_ERROR;
    }


    /**
     * UdmDfiValueクラスでのDFIラベルパスのパラメータの設定数の取得は不可とする.
     * @param [in]  label_path        DFIラベルパス
     * @return        パラメータの設定値数
     * @deprecated         UdmDfiValueクラスでは使用禁止
     */
    int getNumDfiValue(const std::string &label_path) const
    {
        return UDM_ERROR;
    }

};

template <>
UdmError_t UdmDfiValue<double>::toString(double  value, std::string &buf);
template <>
UdmError_t UdmDfiValue<float>::toString(float  value, std::string &buf);


/**
 * コピーコンストラクタ
 * @param        src_value        コピー元データ
 */
template <class DATA_TYPE>
UdmDfiValue<DATA_TYPE>::UdmDfiValue()
{
    this->initialize();
}

/**
 * コピーコンストラクタ
 * @param        src_value        コピー元データ
 */
template <class DATA_TYPE>
UdmDfiValue<DATA_TYPE>::UdmDfiValue(const UdmDfiValue<DATA_TYPE>& src_value)
{
    this->initialize();
    this->setName(src_value.name);
    this->setValue(src_value.value);
}


/**
 * コンストラクタ:文字列設定.
 * 文字列による設定を行う。
 * @param   name        設定名称
 * @param   value        設定値文字列
 */
template <class DATA_TYPE>
UdmDfiValue<DATA_TYPE>::UdmDfiValue(const char *name, DATA_TYPE value)
        : UdmConfigBase(name)
{
    this->initialize();
    this->setName(name);
    this->setValue(value);
}

/**
 * デストラクタ
 */
template <class DATA_TYPE>
UdmDfiValue<DATA_TYPE>::~UdmDfiValue()
{
}


/**
 * DFI設定値を設定する:数値.
 * 数値の設定値の設定を行う。
 * @param value        設定値
 * @return        UDM_OK
 */
template <class DATA_TYPE>
DATA_TYPE UdmDfiValue<DATA_TYPE>::getValue() const
{
    return this->value;
}

/**
 * DFI設定値を設定する:数値.
 * 数値の設定値の設定を行う。
 * @param value        設定値
 * @return        UDM_OK
 */
template <class DATA_TYPE>
UdmError_t UdmDfiValue<DATA_TYPE>::setValue(DATA_TYPE value)
{
    this->value = value;
    this->is_set = true;
    return UDM_OK;
}

/**
 * 初期化を行う.
 */
template <class DATA_TYPE>
void UdmDfiValue<DATA_TYPE>::initialize()
{
    this->value_type = Udm_DataTypeUnknown;
    this->is_set = false;
}


/**
 * DFI設定値の文字列表記を取得する
 * @param [out] buf            文字列表記
 * @return        UDM_OK
 */
template <class DATA_TYPE>
UdmError_t UdmDfiValue<DATA_TYPE>::toString(std::string &buf)
{
    return this->toString(this->value, buf);
}


/**
 * DFI設定値の文字列表記を取得する:DATA_TYPE.
 * @param [in] value        設定値(DATA_TYPE)
 * @param [out] buf            文字列表記
 * @return        UDM_OK
 */
template <class DATA_TYPE>
UdmError_t UdmDfiValue<DATA_TYPE>::toString(DATA_TYPE value, std::string &buf)
{
    if (!this->is_set) {
        return UDM_ERROR;
    }
    std::ostringstream os;
    os << this->getName() << " = " << value;
    buf = os.str();

    return UDM_OK;
}


/**
 * DFI設定値の文字列表記を取得する
 * @param [out] buf            文字列表記
 * @return        UDM_OK
 */
template <class DATA_TYPE>
bool UdmDfiValue<DATA_TYPE>::isSetValue() const
{
    return this->is_set;
}


} /* namespace udm */

#endif /* _UDMDFIVALUE_H_ */
