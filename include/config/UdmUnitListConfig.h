/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

#ifndef _UDMUNITLISTCONFIG_H_
#define _UDMUNITLISTCONFIG_H_

/**
 * @file UdmUnitListConfig.h
 * index.dfiファイルのUnitListデータクラスのヘッダーファイル
 */

#include "config/UdmConfigBase.h"
#include "config/UdmDfiValue.h"

namespace udm {

/**
 * index.dfiファイルのUnitList/[物理名称]要素データクラス
 */
class UdmUnitConfig: public UdmConfigBase {
private:
    std::string        unit_name;        ///< 単位系名称
    std::string        unit;            ///< 単位
    UdmDfiValue<float>     *reference;        ///< 基準値（実数）
    UdmDfiValue<float>     *difference;        ///< 差分値（実数）

public:
    UdmUnitConfig();
    UdmUnitConfig(TextParser *parser);
    UdmUnitConfig(const UdmUnitConfig &src_unit_config);
    UdmUnitConfig(const std::string &unit_name, const std::string &unit, float reference);
    UdmUnitConfig(const std::string &unit_name, const std::string &unit, float reference, float difference);
    virtual ~UdmUnitConfig();
    UdmError_t read(const std::string& label);
    UdmError_t write(FILE *fp, unsigned int indent);
    const std::string& getUnitName() const;
    void setUnitName(const std::string& unitName);
    const std::string& getUnit() const;
    void setUnit(const std::string& unit);
    float getReference() const;
    bool isSetReference() const;
    float getDifference() const;
    bool isSetDifference() const;
    void setReference(float value);
    void setDifference(float value);
    void unsetReference();
    void unsetDifference();
    UdmError_t getDfiValue(const std::string &label_path, std::string &value) const;
    UdmError_t setDfiValue(const std::string &label_path, const std::string &value);
    int getNumDfiValue(const std::string &label_path) const;

private:
    void initialize();
};

/**
 * index.dfiファイルのUnitListデータクラス
 */
class UdmUnitListConfig: public UdmConfigBase {
private:
    std::map<std::string, UdmUnitConfig*, map_case_compare> unit_list;

public:
    UdmUnitListConfig();
    UdmUnitListConfig(TextParser *parser);
    virtual ~UdmUnitListConfig();
    UdmError_t read();
    UdmError_t write(FILE *fp, unsigned int indent);
    const UdmUnitConfig* getUnitConfig(const std::string &unit_name) const;
    UdmUnitConfig* getUnitConfig(const std::string &unit_name);
    UdmError_t setUnitConfig(const UdmUnitConfig* unit);
    unsigned int getNumUnitConfig() const;
    unsigned int getUnitNameList(std::vector<std::string> &unit_names) const;
    UdmError_t getUnit(const std::string &unit_name, std::string& unit) const;
    UdmError_t getReference(const std::string &unit_name, float &reference) const;
    UdmError_t getDifference(const std::string &unit_name, float &difference) const;
    UdmError_t setUnitConfig(const std::string &unit_name, const std::string &unit, float reference);
    UdmError_t setUnitConfig(const std::string &unit_name, const std::string &unit, float reference, float diffrence);
    bool existsUnitConfig(const std::string &unit_name) const;
    UdmError_t removeUnitConfig(const std::string &unit_name);
    UdmError_t clear();
    UdmError_t getDfiValue(const std::string &label_path, std::string &value) const;
    UdmError_t setDfiValue(const std::string &label_path, const std::string &value);
    int getNumDfiValue(const std::string &label_path) const;

private:
    void initialize();
};

} /* namespace udm */

#endif /* _UDMUNITLISTCONFIG_H_ */
