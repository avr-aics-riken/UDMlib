/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

#ifndef _UDMFILEPATHCONFIG_H_
#define _UDMFILEPATHCONFIG_H_

/**
 * @file UdmFilePathConfig.h
 * index.dfiファイルのFilePathデータクラスのヘッダーファイル
 */
#include "config/UdmConfigBase.h"

namespace udm {

/**
 * index.dfiファイルのFilePathデータクラス
 */
class UdmFilePathConfig: public UdmConfigBase {
private:
    std::string        process;        ///< proc.dfiファイル名
    void initialize();
public:
    UdmFilePathConfig();
    UdmFilePathConfig(TextParser *parser);
    virtual ~UdmFilePathConfig();
    UdmError_t read();
    UdmError_t write(FILE *fp, unsigned int indent);
    UdmError_t  getProcess(std::string& process) const;
    void setProcess(const std::string& process);
    UdmError_t getDfiValue(const std::string &label_path, std::string &value) const;
    UdmError_t setDfiValue(const std::string &label_path, const std::string &value);
    int getNumDfiValue(const std::string &label_path) const;

};

} /* namespace udm */

#endif /* _UDMFILEPATHCONFIG_H_ */
