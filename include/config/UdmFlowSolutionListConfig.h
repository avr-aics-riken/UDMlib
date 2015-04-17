/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

#ifndef _UDMFLOWSOLUTIONLISTCONFIG_H_
#define _UDMFLOWSOLUTIONLISTCONFIG_H_


/**
 * @file UdmFlowSolutionListConfig.h
 * index.dfiファイルのFlowSolutionデータクラスのヘッダーファイル
 */
#include "config/UdmConfigBase.h"
#include "config/UdmDfiValue.h"
#include "config/UdmSolutionFieldConfig.h"

namespace udm {

/**
 * index.dfiファイルのFlowSolutionListデータクラス
 */
class UdmFlowSolutionListConfig: public UdmConfigBase {
private:
    std::map<std::string, UdmSolutionFieldConfig*, map_case_compare> solution_list;

public:
    UdmFlowSolutionListConfig();
    UdmFlowSolutionListConfig(TextParser *parser);
    virtual ~UdmFlowSolutionListConfig();
    UdmError_t read();
    UdmError_t write(FILE *fp, unsigned int indent);
    const UdmSolutionFieldConfig* getSolutionFieldConfig(const std::string &solution_name) const;
    UdmSolutionFieldConfig* getSolutionFieldConfig(const std::string &solution_name);
    UdmError_t setSolutionFieldConfig(const UdmSolutionFieldConfig* solution);
    UdmSize_t getNumSolutionConfig() const;
    UdmSize_t getSolutionId(const std::string &solution_name)  const;
    UdmError_t getSolutionName(UdmSize_t solution_id, std::string &solution_name) const;
    UdmSize_t getSolutionNameList(std::vector<std::string> &solution_names) const;
    UdmGridLocation_t getGridLocation(const std::string &solution_name) const;
    UdmDataType_t getDataType(const std::string &solution_name) const;
    UdmVectorType_t getVectorType(const std::string &solution_name) const;
    int getNvectorSize(const std::string &solution_name) const;
    bool isConstantFlag(const std::string &solution_name) const;
    UdmError_t getSolutionFieldInfo(
                    const std::string     &solution_name,
                    UdmGridLocation_t    &grid_location,
                    UdmDataType_t        &data_type,
                    UdmVectorType_t        &vector_type,
                    int                    &nvector_size,
                    bool            &constant_flag) const;
    UdmError_t setSolutionFieldInfo(
                    const std::string     &solution_name,
                    UdmGridLocation_t    grid_location,
                    UdmDataType_t        data_type,
                    UdmVectorType_t        vector_type,
                    int                    nvector_size,
                    bool                   constant_flag = false);
    UdmError_t setSolutionFieldInfo(
                    const std::string     &solution_name,
                    UdmGridLocation_t    grid_location,
                    UdmDataType_t        data_type);
    bool existsSolutionConfig(const std::string &solution_name) const;
    UdmError_t removeSolutionConfig(const std::string &solution_name);
    UdmError_t clear();
    UdmError_t rebuildSolutionList();
    UdmError_t getDfiValue(const std::string &label_path, std::string &value) const;
    UdmError_t setDfiValue(const std::string &label_path, const std::string &value);
    int getNumDfiValue(const std::string &label_path) const;
    UdmSolutionFieldConfig* findSolutionFieldByCgnsName(const std::string& cgns_field_name, int &vector_id) const;
    int findSolutionFieldConfigs(std::vector<const UdmSolutionFieldConfig*> &list, UdmGridLocation_t location, bool constant_flag) const;

private:
    void initialize();
};

} /* namespace udm */

#endif /* _UDMFLOWSOLUTIONLISTCONFIG_H_ */
