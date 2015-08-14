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
 * @file UdmFlowSolutions.h
 * 物理量管理クラスのヘッダーファイル
 */

#ifndef _UDMFLOWSOLUTIONS_H_
#define _UDMFLOWSOLUTIONS_H_

#include "model/UdmGeneral.h"
#include "config/UdmFlowSolutionListConfig.h"

namespace udm
{
class UdmZone;


/**
 * 物理量管理クラス
 */
class UdmFlowSolutions: public UdmGeneral
{
    /**
     * 親ゾーン
     */
    UdmZone *parent_zone;

    /**
     * 出力結果CGNS:FlowSolution名リスト
     */
    std::vector<std::string> cgns_writesolutions;

public:
    UdmFlowSolutions();
    UdmFlowSolutions(UdmZone *parent_zone);
    virtual ~UdmFlowSolutions();
    UdmFlowSolutionListConfig* getFlowSolutionsConfig() const;

    UdmSize_t getNumSolutionFields() const;
    UdmSolutionFieldConfig* getSolutionField(UdmSize_t field_id) const;
    UdmSolutionFieldConfig* getSolutionField(const std::string &field_name) const;
    UdmSize_t createSolutionField(
                    const std::string &field_name,
                    UdmDataType_t data_type,
                    UdmGridLocation_t location);
    UdmSize_t createSolutionField(
                    const std::string     &field_name,
                    UdmDataType_t         data_type,
                    UdmGridLocation_t     grid_location,
                    UdmVectorType_t       vector_type,
                    int                   nvector_size,
                    bool                  constant_flag);
    bool existsFieldName(const std::string &field_name) const;
    UdmError_t removeSolutionField(UdmSize_t field_id);
    UdmError_t removeSolutionField(const std::string &field_name);
    UdmError_t clearSolutionFields();

    // UdmSolutionFieldConfig項目
    UdmError_t getFieldName(UdmSize_t field_id, std::string& field_name) const;
    UdmDataType_t getFieldDataType(UdmSize_t field_id) const;
    UdmGridLocation_t getFieldGridLocation(UdmSize_t field_id) const;
    UdmVectorType_t getFieldVectorType(UdmSize_t field_id) const;
    int getFieldNvectorSize(UdmSize_t field_id) const;
    bool isFieldConstant(UdmSize_t field_id) const;
    UdmSize_t getFieldId(const std::string &field_name) const;
    UdmDataType_t getFieldDataType(const std::string &field_name) const;
    UdmGridLocation_t getFieldGridLocation(const std::string &field_name) const;
    UdmVectorType_t getFieldVectorType(const std::string &field_name) const;
    int getFieldNvectorSize(const std::string &field_name) const;
    bool isFieldConstant(const std::string &field_name) const;

    // CGNSアクセス
    UdmError_t readCgns(int index_file, int index_base, int index_zone, int timeslice_step = -1);
    UdmError_t getCgnsFlowSolutionName(UdmSize_t field_id, std::string &cgns_name) const;
    UdmError_t getCgnsFlowSolutionName(const std::string& field_name, std::string &cgns_name) const;
    UdmSolutionFieldConfig* findSolutionFieldByCgnsName(const std::string& cgns_field_name, int &vector_id) const;
    int getCgnsSolutionFieldNames(UdmSize_t field_id, std::vector<std::string> &cgns_names) const;
    int getCgnsSolutionFieldNames(const std::string& field_name, std::vector<std::string> &cgns_names) const;
    UdmError_t writeCgns(int index_file, int index_base, int index_zone, int timeslice_step, bool write_constsolutions);
    UdmError_t writeCgnsLinkFile(int index_file, int index_base, int index_zone, const std::string &link_output_path, const std::vector<std::string> &linked_files, int timeslice_step);
    UdmError_t initializeWriteCgns();
    const std::vector<std::string>& getCgnsWriteSolutions() const;
    int addCgnsWriteSolutions(const std::string solution_name);
    void clearCgnsWriteSolutions();
    int getNumCgnsWriteSolutions() const;

    // UdmZone
    UdmZone* getParentZone();
    const UdmZone* getParentZone() const;
    UdmError_t setParentZone(UdmZone *zone);

    static bool existsCgnsConstantSolutions(
                            const std::string& filename,
                            int index_base = 1,
                            int index_zone = 1);
private:
    void initialize();
    UdmError_t rebuildSolutionFields();
    UdmError_t getCgnsFlowSolutionIds(int index_dfi_field, int &index_solution, std::vector<int> &index_fields, int index_file, int index_base, int index_zone, int timeslice_step = -1);
    UdmError_t writeCgnsSolutionField(const UdmSolutionFieldConfig *config, int index_file, int index_base, int index_zone, int timeslice_step);
    int findCgnsFlowSolution(int index_file, int index_base, int index_zone, const UdmSolutionFieldConfig *config, int timeslice_step);
    int findCgnsSolutionFieldName(
                            int index_file,
                            int index_base,
                            int index_zone,
                            UdmGridLocation_t grid_location,
                            const std::vector<std::string> &cgns_fieldnames,
                            std::vector<std::string> &solution_names) const;

};

} /* namespace udm */

#endif /* _UDMFLOWSOLUTIONS_H_ */
