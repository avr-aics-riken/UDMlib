/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

#ifndef _UDMSOLUTIONFIELDCONFIG_H_
#define _UDMSOLUTIONFIELDCONFIG_H_


/**
 * @file UdmSolutionFieldConfig.h
 * [物理量名称]データクラスのヘッダーファイル
 */
#include "config/UdmConfigBase.h"
#include "config/UdmDfiValue.h"

namespace udm {

/**
 * [物理量名称]データクラス
 */
class UdmSolutionFieldConfig: public UdmConfigBase {
private:
    UdmSize_t              solution_id;            ///< 物理量ID （１～）
    std::string            solution_name;        ///< 物理量名称
    UdmGridLocation_t    grid_location;        ///< 物理量適用位置
    UdmDataType_t        data_type;            ///< データ型
    UdmVectorType_t        vector_type;        ///< ベクトル型
    int             nvector_size;                ///< データ成分数
    bool                constant_flag;            ///< 固定値フラグ

public:
    UdmSolutionFieldConfig();
    UdmSolutionFieldConfig(TextParser *parser);
    UdmSolutionFieldConfig(const UdmSolutionFieldConfig &src_solution_config);
    UdmSolutionFieldConfig(
                        const std::string     &solution_name,
                        UdmGridLocation_t     grid_location,
                        UdmDataType_t         data_type,
                        UdmVectorType_t       vector_type,
                        int                   nvector_size,
                        bool                  constant_flag);
    UdmSolutionFieldConfig(
                        const std::string     &solution_name,
                        UdmGridLocation_t     grid_location,
                        UdmDataType_t         data_type);
    virtual ~UdmSolutionFieldConfig();
    UdmError_t read(const std::string& solution_label);
    UdmError_t write(FILE *fp, unsigned int indent);
    const std::string& getSolutionName() const;
    void setSolutionName(const std::string& solution_name);
    UdmGridLocation_t getGridLocation() const;
    void setGridLocation(UdmGridLocation_t grid_location);
    UdmDataType_t getDataType() const;
    void setDataType(UdmDataType_t data_type);
    UdmVectorType_t getVectorType() const;
    void setVectorType(UdmVectorType_t vector_type);
    int getNvectorSize() const;
    void setNvectorSize(int nvector_size);
    bool isConstantFlag() const;
    void setConstantFlag(bool  constant_flag);
    UdmError_t getDfiValue(const std::string &label_path, std::string &value) const;
    UdmError_t setDfiValue(const std::string &label_path, const std::string &value);
    int getNumDfiValue(const std::string &label_path) const;
    UdmSize_t getSolutionId() const;
    void setSolutionId(UdmSize_t solutionId);
    int getVectorSolutionNames(std::vector<std::string> &vector_names) const;
    UdmError_t generateCgnsFlowSolutionName(std::string &cgns_name, int timeslice_step) const;

private:
    void initialize();
};

} /* namespace udm */

#endif /* _UDMSOLUTIONFIELDCONFIG_H_ */
