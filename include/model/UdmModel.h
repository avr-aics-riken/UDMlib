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

#ifndef _UDMMODEL_H_
#define _UDMMODEL_H_

/**
 * @file UdmModel.h
 * ベースモデルクラスのヘッダーファイル
 */

#include "config/UdmDfiConfig.h"
#include "model/UdmGeneral.h"
#include "model/UdmIterativeData.h"
#include "model/UdmZone.h"
#include "partition/UdmLoadBalance.h"
#include "utils/UdmSerialization.h"

namespace udm
{

/**
 * ベースモデルクラス.
 * CGNS:Baseに相当クラスです.
 */
class UdmModel: public UdmGeneral, public UdmISerializable
{
private:

    int cell_dimension ;                ///< 要素（セル）次元数
    int physical_dimension ;            ///< ノードの次元数
    UdmSimulationType_t simulation_type;    ///< シミュレーションタイプ

    /**
     * ゾーンクラス
     */
    std::vector<UdmZone*> zones;

    /**
     * 時系列データ
     */
    UdmIterativeData  *iterator;

    /**
     * DFI設定
     */
    UdmDfiConfig *config;

    // MPI
    MPI_Comm mpi_communicator;        ///< MPIコミュニケータ
    int mpi_rankno;                    ///< MPIランク番号
    int mpi_num_process;            ///< MPIプロセス数

    // CGNS:UdmInfo
    std::string udminfo_version;        ///< CGNS:UdmInfo:CGNS出力時のUDMlibのバージョン
    int udminfo_process_size;            ///< CGNS:UdmInfo:CGNS出力時のプロセスの総数
    int udminfo_rankno;                    ///< CGNS:UdmInfo:CGNS出力時のランク番号

    // partition
    UdmLoadBalance *partition;            ///< ロードバランスクラス

public:
    UdmModel();
    UdmModel(const std::string &base_name, UdmSize_t base_id);
    UdmModel(const char* dfi_filename);
    UdmModel(const MPI_Comm& comm);
    virtual ~UdmModel();

    // UdmZone
    int getNumZones() const;
    UdmZone* getZone(const std::string &zone_name) const;
    UdmZone* getZone(int zone_id = 1) const;
    UdmZone* createZone();
    UdmZone* createZone(const std::string &zone_name);
    int insertZone(UdmZone* zone);
    UdmError_t removeZone(const std::string &zone_name);
    UdmError_t removeZone(int zone_id);
    bool existsZone(const std::string &zone_name) const;
    int getZoneId(const std::string &zone_name) const;
    UdmError_t clearZone();

    // UdmIterativeData
    bool isSetIterativeData() const;
    UdmError_t getIterativeData(UdmSize_t &step, double &time) const;
    UdmError_t getIterativeData(UdmSize_t &step, double &time, UdmSize_t &average_step, double &average_time);
    UdmError_t setIterativeData(UdmSize_t step, double time);
    UdmError_t setIterativeData(UdmSize_t step, double time, UdmSize_t average_step, double average_time);
    UdmError_t clearIterativeData();

    // CGNS
    UdmError_t loadModel(const char* dfi_filename, int timeslice_step = -1);
    UdmError_t readCgns(const char* cgns_filename, int timeslice_step = -1, const char* element_path = NULL);
    UdmError_t writeTimeSlice(int timeslice_step, float timeslice_time = NAN);
    UdmError_t writeCgnsTimeSliceFile(
                            const char* cgns_filename,
                            int timeslice_step,
                            float timeslice_time,
                            bool grid_timeslice,
                            bool write_constsolutions);
    UdmError_t writeCgnsGridCoordinatesFile(const char* cgns_filename, int timeslice_step, float timeslice_time, bool grid_timeslice);
    UdmError_t writeCgnsFlowSolutionFile(const char* cgns_filename, int timeslice_step, float timeslice_time, bool write_constsolutions);
    UdmError_t writeCgnsLinkFile(
                            const char* cgns_filename,
                            const std::vector<std::string> &linked_files,
                            int timeslice_step,
                            float timeslice_time,
                            bool grid_timeslice);
    UdmError_t writeModel(int timeslice_step, float timeslice_time);
    UdmError_t writeModel(int timeslice_step, float timeslice_time, int average_step, float average_time);
    UdmError_t initializeCgnsFile(const char *filename);
    int readCgnsBaseIterativeDatas(
                    int index_file,
                    int index_base,
                    std::vector<int> &timeslice_steps,
                    std::vector<float> &timeslice_times);
    UdmError_t writeRankConnectivity(const char* cgns_filename);
    UdmError_t writeVirtualCells(const char* cgns_filename);
    bool validateCgns() const;
    UdmError_t rebuildModel();
    UdmError_t writeCgnsModel(int timeslice_step, float timeslice_time);
    UdmError_t initializeTimeSlice();

    int getCellDimension() const;
    void setCellDimension(int cell_dimension);
    int getPhysicalDimension() const;
    void setPhysicalDimension(int physical_dimension);
    UdmSimulationType_t getSimulationType() const;
    void setSimulationType(UdmSimulationType_t simulation_type);
    UdmSimulationType_t setSimulationType(SimulationType_t simulation_type);

    // DFI
    UdmError_t readDfi(const char* dfi_filename);
    UdmError_t writeDfi();
    UdmDfiConfig *getDfiConfig();
    const UdmDfiConfig *getDfiConfig() const;
    const UdmFileInfoConfig* getConfigFileinfo() const;
    void setOutputPath(const char* path);

    // MPI
    MPI_Comm getMpiComm() const;
    UdmError_t setMpiComm(const MPI_Comm &comm);
    int getMpiRankno() const;
    void setMpiRankno(int rankno);
    int getMpiProcessSize() const;
    void setMpiProcessSize(int proc_size);
    UdmError_t broadcastModel();
    UdmError_t transferVirtualCells(std::vector<UdmNode*> &virtual_nodes);
    UdmError_t transferVirtualCells();

    // シリアライズ
    UdmSerializeArchive& serialize(UdmSerializeArchive &archive) const;
    UdmSerializeArchive& deserialize(UdmSerializeArchive &archive);

    // for debug
    void toString(std::string &buf) const;

    // CGNS:UdmInfo
    int getUdminfoProcessSize() const;
    void setUdminfoProcessSize(int process_size);
    int getUdminfoRankno() const;
    void setUdminfoRankno(int rankno);
    const std::string& getUdminfoVersion() const;
    void setUdminfoVersion(const std::string& version);
    int getCgnsRankno() const;

    // partition
    UdmError_t partitionZone(int zone_id = 1);
    UdmLoadBalance *getLoadBalance();

private:
    void initialize();

    // CGNS
    UdmError_t writeCgnsBase(const char* cgns_filename, int &index_file, int &index_base);
    UdmError_t writeCgnsIterativeDatas(
                    int index_file,
                    int index_base,
                    int timeslice_step,
                    float timeslice_time,
                    bool write_gridflag);
    int getNumCgnsIterativeDatas() const;
    bool equalsModel(const UdmModel &model) const;
    UdmError_t createModel(UdmSize_t index_base,
                    std::string name_base,
                    int cell_dim,
                    int phys_dim,
                    UdmSimulationType_t simulation_type);
    UdmError_t readCgnsUdmInfo(int index_file, int index_base);
    UdmError_t writeCgnsUdmInfo(int index_file, int index_base);
    void closeWriteCgnsFile(int index_file);
    UdmError_t createComponentCells();

};

} /* namespace udm */

#endif /* _UDMMODEL_H_ */
