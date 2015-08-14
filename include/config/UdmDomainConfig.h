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

#ifndef _UDMDOMAINCONFIG_H_
#define _UDMDOMAINCONFIG_H_

/**
 * @file UdmDomainConfig.h
 * proc.dfiファイルのDomainデータクラスのヘッダーファイル
 */
#include "config/UdmConfigBase.h"

namespace udm {

/**
 * proc.dfiファイルのDomainデータクラス
 */
class UdmDomainConfig: public UdmConfigBase {
private:
    int        cell_dimension;            ///< 非構造格子モデルの次元数（=1D, 2D, 3D）
    UdmSize_t     vertex_size;            ///< 全体のノード数
    UdmSize_t    cell_size;                ///< 全体のセル（要素）数

public:
    UdmDomainConfig();
    UdmDomainConfig(TextParser *parser);
    virtual ~UdmDomainConfig();
    UdmError_t read();
    UdmError_t write(FILE *fp, unsigned int indent);
    UdmSize_t getCellDimension() const;
    void setCellDimension(int cell_dimension);
    UdmSize_t getVertexSize() const;
    void setVertexSize(UdmSize_t vertex_size);
    UdmSize_t getCellSize() const;
    void setCellSize(UdmSize_t cell_size);
    UdmError_t getDfiValue(const std::string &label_path, std::string &value) const;
    UdmError_t setDfiValue(const std::string &label_path, const std::string &value);
    int getNumDfiValue(const std::string &label_path) const;

private:
    void initialize();

};

} /* namespace udm */

#endif /* _UDMDOMAINCONFIG_H_ */
