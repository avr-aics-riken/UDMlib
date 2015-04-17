/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

#include "config/UdmDfiValue.h"

/**
 * @file UdmDfiValue.cpp
 * DFI設定値クラスのソースファイル
 */

namespace udm
{


/**
 * DFI設定値の文字列表記を取得する:double.
 * @param [in] value        設定値(double)
 * @param [out] buf            文字列表記
 * @return        UDM_OK
 */
template <>
UdmError_t UdmDfiValue<double>::toString(double value, std::string &buf)
{
    if (!this->is_set) {
        return UDM_ERROR;
    }
    char value_buf[std::numeric_limits<double>::max_exponent10
             + 6   // fixed precision (printf's default)
             + 3]; // '-' + '.' + NULL
    std::sprintf(value_buf, "%e", value);

    std::ostringstream os;
    os << this->getName() << " = " << value_buf;
    buf = os.str();
    return UDM_OK;
}

/**
 * DFI設定値の文字列表記を取得する:float.
 * @param [in] value        設定値(float)
 * @param [out] buf            文字列表記
 * @return        UDM_OK
 */
template <>
UdmError_t UdmDfiValue<float>::toString(float value, std::string &buf)
{
    if (!this->is_set) {
        return UDM_ERROR;
    }
    char value_buf[std::numeric_limits<float>::max_exponent10
            + 6   // fixed precision (printf's default)
            + 3]; // '-' + '.' + NULL
    std::sprintf(value_buf, "%e", value);

    std::ostringstream os;
    os << this->getName() << " = " << value_buf;
    buf = os.str();
    return UDM_OK;
}

} /* namespace udm */
