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
 * @file UdmBase.cpp
 * UDMlibライブラリのすべてのクラスの基底クラスのソースファイル
 */

#include "UdmBase.h"

namespace udm {

/**
 * コンストラクタ
 */
UdmBase::UdmBase() {

}

/**
 * デストラクタ
 */
UdmBase::~UdmBase() {

}

/**
 * 文字列を分解する
 * @param [in] str            分解対象文字列
 * @param [in] delim        分解文字
 * @param [out] results        分解文字列リスト
 * @return            分解文字列リスト数
 */
int UdmBase::split(const std::string &str, char delim, std::vector<std::string>& results) const
{
    std::string value;
    size_t current = 0, found;
    while ((found = str.find_first_of(delim, current)) != std::string::npos) {
        value = std::string(str, current, found - current);
        if (!value.empty()) {
            results.push_back(value);
        }
        current = found + 1;
    }
    value = std::string(str, current, str.size() - current);
    if (!value.empty()) {
        results.push_back(value);
    }
    return results.size();
}


/**
 * 文字列から前後の空白を削除する.
 * @param [inout] buf        空白削除文字列
 */
void UdmBase::trim(char *buf)
{
    char *start, *end;
    char *p = buf;

    while(isspace(*p)) p++;
    start = p;
    end = NULL;
    while(*p != '\0') {
        if(!isspace(*p)) end = p;
        p++;
    }

    if (end != NULL) {
        memmove(buf, start, end - start + 1);
        buf[end - start + 1] = '\0';
    }
    else
        buf[0] = '\0';
}

/**
 * 文字列１と文字列２を大文字・小文字を関係なく比較する.
 * @param str1        文字列１
 * @param str2        文字列２
 * @return        true=一致
 */
bool UdmBase::compareCaseInsensitive(const std::string& str1, const std::string& str2) {
    if (str1.size() != str2.size()) {
        return false;
    }
    for (std::string::const_iterator c1 = str1.begin(), c2 = str2.begin(); c1 != str1.end(); ++c1, ++c2) {
        if (tolower(*c1) != tolower(*c2)) {
            return false;
        }
    }
    return true;
};


} /* namespace udm */
