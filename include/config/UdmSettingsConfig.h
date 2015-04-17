/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

#ifndef _UDMPARTITIONCONFIG_H_
#define _UDMPARTITIONCONFIG_H_

/**
 * @file UdmSettingsConfig.h
 * udmlib.tpファイルの入出力クラスのヘッダーファイル
 */

#include "config/UdmConfigBase.h"

/// LB_APPROACHのデフォルト設定値
//#define ZOLTAN_DEFAULT_LB_APPROACH            "REPARTITION"
#define ZOLTAN_DEFAULT_LB_APPROACH            "PARTITION"
/// Zoltanデフォルトデバッグレベル
#define ZOLTAN_DEFAULT_DEBUG_LEVEL            1

namespace udm {

/**
 * udmlib.tpファイルの入出力クラス
 */
class UdmSettingsConfig: public UdmConfigBase  {
private:
    std::string     filename_udm_dfi;        ///< 入力udm.dfiファイル名
    int debug_level;                        ///< デバッグレベル
    UdmEnable_t     mxm_partition;            ///< MxM_PARTITION:分割数が同じ場合の分割実行
    std::map<std::string, std::string, map_case_compare>    zoltan_parameters;        ///< Zontalパラメータ

public:
    UdmSettingsConfig();
    UdmSettingsConfig(TextParser *parser);
    virtual ~UdmSettingsConfig();
    UdmError_t read(const char* filename);
    UdmError_t write(const char* filename);
    UdmError_t getUdmDfiPath(std::string &filepath) const;
    int getDebugLevel() const;
    UdmError_t setDebugLevel(int debuf_level);
    bool isMxMPartition() const;
    UdmError_t setMxMPartition(bool mxm_partition);
    int getNumZoltanParameters() const;
    UdmError_t getZoltanParameterById(int param_id, std::string &name, std::string &value) const;
    UdmError_t getZoltanParameter(const std::string &name, std::string &value) const;
    UdmError_t setZoltanParameter(const std::string &name, const std::string &value);
    UdmError_t getDfiValue(const std::string &label_path, std::string &value) const;
    UdmError_t setDfiValue(const std::string &label_path, const std::string &value);
    int getNumDfiValue(const std::string &label_path) const;
    UdmError_t removeZoltanParameter(const std::string &param_name);
    UdmError_t clearZoltanParameters();
    UdmError_t getParameter(const std::string &name, std::string &value) const;
    UdmError_t setParameter(const std::string &name, const std::string &value);
    UdmError_t removeParameter(const std::string &name);

private:
    void initialize();
};

} /* namespace udm */

#endif /* _UDMPARTITIONCONFIG_H_ */
