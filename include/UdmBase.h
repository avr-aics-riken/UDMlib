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

#ifndef _UDMBASE_H_
#define _UDMBASE_H_

/**
 * @file UdmBase.h
 * UDMlibライブラリのすべてのクラスの基底クラスのヘッダーファイル
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <cstdio>
#include <limits>
#include <sstream>
#include <math.h>
#include <limits>
#include "UdmErrorHandler.h"
#include "utils/UdmStopWatch.h"
#include "udmlib.h"
#include "udm_memutils.h"
#include "udm_define.h"

/**
 * @file UdmGeneral.h
 * モデル構成の基底クラスのヘッダーファイル
 */

/**
 * UDMlibライブラリのネームスペース
 */
namespace udm {

/**
 * 大文字・小文字を無視して比較する.
 */
struct insensitive_compare {
    std::string _compare;
    insensitive_compare( const std::string  &src ) : _compare(src) {}
    bool operator() ( const std::string  &dest ) const {
        if (_compare.size() != dest.size()) {
            return false;
        }
        for (std::string::const_iterator c1 = _compare.begin(), c2 = dest.begin(); c1 != _compare.end(); ++c1, ++c2) {
            if (tolower(*c1) != tolower(*c2)) {
                return false;
            }
        }
        return true;
    }
};


struct map_case_compare {
    bool operator() (const std::string& lhs, const std::string& rhs) const {
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
    }
};

/**
 * UDMlibライブラリのすべてのクラスの基底クラス
 */
class UdmBase
{
public:
    UdmBase();
    virtual ~UdmBase();

protected:
    int split(const std::string &str, char delim, std::vector<std::string>& results) const;
    void trim(char *buf);
    bool compareCaseInsensitive(const std::string& str1, const std::string& str2);

};

} /* namespace udm */

#endif /* _UDMBASE_H_ */
