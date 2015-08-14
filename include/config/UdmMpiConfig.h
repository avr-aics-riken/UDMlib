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

#ifndef _UDMMPICONFIG_H_
#define _UDMMPICONFIG_H_

/**
 * @file UdmMpiConfig.h
 * proc.dfiファイルのMPIデータクラスのヘッダーファイル
 */
#include "config/UdmConfigBase.h"

namespace udm {

/**
 * proc.dfiファイルのMPIデータクラス
 */
class UdmMpiConfig: public UdmConfigBase {
private:
    int        number_rank;            ///< 並列実行数
    int        number_group;            ///< 並列実行グループ数

public:
    UdmMpiConfig();
    UdmMpiConfig(TextParser *parser);
    virtual ~UdmMpiConfig();
    UdmError_t read();
    UdmError_t write(FILE *fp, unsigned int indent);
    int getNumberGroup() const;
    void setNumberGroup(int number_rank);
    int getNumberRank() const;
    void setNumberRank(int number_group);
    UdmError_t getDfiValue(const std::string &label_path, std::string &value) const;
    UdmError_t setDfiValue(const std::string &label_path, const std::string &value);
    int getNumDfiValue(const std::string &label_path) const;

private:
    void initialize();
};

} /* namespace udm */

#endif /* _UDMMPICONFIG_H_ */
