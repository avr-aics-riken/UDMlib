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
#ifndef _UDMDFICONFIG_H_
#define _UDMDFICONFIG_H_

/**
 * @file UdmDfiConfig.h
 * index.dfi, proc.dfiの入出力クラスのヘッダーファイル
 */

#include "config/UdmConfigBase.h"
#include "config/UdmFileInfoConfig.h"
#include "config/UdmFilePathConfig.h"
#include "config/UdmTimeSliceConfig.h"
#include "config/UdmUnitListConfig.h"
#include "config/UdmFlowSolutionListConfig.h"
#include "config/UdmDomainConfig.h"
#include "config/UdmMpiConfig.h"
#include "config/UdmProcessConfig.h"

namespace udm {

/**
 * index.dfi, proc.dfiの入出力クラス
 */
class UdmDfiConfig: public UdmConfigBase {
private:
    std::string                filename_input_dfi;        ///< index.dfiファイル名
    std::string              output_path;                ///< 出力パス
    UdmFileInfoConfig        *fileinfo;        ///< index.dfi/FileInfoデータ
    UdmFilePathConfig        *filepath;        ///< index.dfi/FilePathデータ
    UdmTimeSliceConfig        *timeslice;        ///< index.dfi/TimeSliceデータ
    UdmUnitListConfig        *unitlist;        ///< index.dfi/UnitListデータ
    UdmFlowSolutionListConfig    *flowsolutionlist;    ///< index.dfi/FlowSolutionデータ
    UdmDomainConfig            *domain;        ///< proc.dfi/Domainデータ
    UdmMpiConfig            *mpi;            ///< proc.dfi/MPIデータ
    UdmProcessConfig            *process;            ///< proc.dfi/Processデータ
    std::map<int, int>        io_rank_table;        ///< 入出力ランク番号テーブル={CGNS出力ランク番号, CGNS読込ランク番号}

public:
    UdmDfiConfig();
    virtual ~UdmDfiConfig();
    UdmError_t readDfi(const char* filename);
    UdmError_t writeDfi(const char* filename);
    UdmFileInfoConfig* getFileinfoConfig() const;
    UdmFilePathConfig* getFilepathConfig() const;
    UdmFlowSolutionListConfig* getFlowSolutionListConfig() const;
    UdmTimeSliceConfig* getTimesliceConfig() const;
    UdmUnitListConfig* getUnitListConfig() const;
    UdmDomainConfig* getDomainConfig() const;
    UdmMpiConfig* getMpiConfig() const;
    UdmProcessConfig* getProcessConfig() const;
    UdmError_t getIndexDfiPath(std::string &filepath) const;
    UdmError_t getIndexDfiFolder(std::string &filepath) const;
    UdmError_t getDfiValue(const std::string &label_path, std::string &value) const;
    UdmError_t setDfiValue(const std::string &label_path, const std::string &value);
    int getNumDfiValue(const std::string &label_path) const;
    UdmError_t setUnitList(const std::string &unit_name, const std::string &unit, double reference);
    UdmError_t setUnitList(const std::string &unit_name, const std::string &unit, double reference, double difference);
    unsigned int getUnitNameList(std::vector<std::string> &unit_names) const;
    unsigned int getSolutionNameList(std::vector<std::string> &solution_names) const;
    UdmError_t setSolutionConfig(
                    const std::string     &solution_name,
                    UdmGridLocation_t    grid_location,
                    UdmDataType_t        data_type,
                    UdmVectorType_t        vector_type,
                    int                        nvector_size,
                    bool            constant_flag);
    // TimeSlice
    UdmError_t insertTimeSlice(UdmSize_t step, double time);
    UdmError_t insertTimeSlice(UdmSize_t step, double time, UdmSize_t average_step, double average_time);
    UdmSize_t getNumCurrentTimeSlices() const;

    UdmError_t setProcessRank(int rankid, UdmSize_t vertex_size, UdmSize_t cell_size);
    UdmError_t getCgnsInputFilePath(std::string& file_path, int rank_no);
    UdmError_t getCgnsLinkFilePath(std::string &file_path, int rank_no);
    UdmError_t getCgnsTimeSliceFilePath(std::string &file_path, int rank_no, int timeslice_step, bool mpiexec = true);
    UdmError_t getCgnsGridFilePath(std::string &file_path, int rank_no, int timeslice_step);
    UdmError_t getCgnsSolutionFilePath(std::string &file_path, int rank_no, int timeslice_step);
    UdmError_t setDomainConfig(int dimmention, UdmSize_t vertex_size, UdmSize_t cell_size);
    UdmError_t setMpiConfig(int num_process, int num_group);
    UdmError_t setProcessConfig(int num_process, const UdmSize_t *vertex_sizes, const UdmSize_t *cell_sizes);
    const std::string &getOutputPath() const;
    void setOutputPath(const std::string path);
    UdmError_t getDfiOutputDirectory(std::string &path) const;
    UdmError_t getDfiOutputDirectory(char *path) const;
    UdmError_t getCgnsOutputDirectory(std::string &path) const;
    UdmError_t getCgnsTimeSliceDirectory(std::string &path, UdmSize_t time_step) const;
    UdmError_t getWriteIndexDfiPath(std::string &dfi_path) const;
    UdmError_t getWriteProcDfiPath(std::string &filepath) const;
    UdmError_t getReadProcDfiPath(std::string &filepath) const;
    void clearProcessRank();

    // FileInfo
    void setFileInfoPrefix(const std::string &prefix);

    // 入出力ランク番号テーブル
    int makeIoRankTable(int total_ranks, int myrank, std::vector<int>& input_rank_ids);
    int getInputRankno(int output_rankno) const;
    bool existsIoRankTable() const;
    void clearIoRankTable();

private:
    void initialize();
    UdmError_t readProcDfi(const char* filename);
    UdmError_t writeProcDfi(const char* filename);
    UdmError_t connectOutputDirectory(const std::string file_name, std::string &path) const;

};

} /* namespace udm */

#endif /* _UDMDFICONFIG_H_ */
