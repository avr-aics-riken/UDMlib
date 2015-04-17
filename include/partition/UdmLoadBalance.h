/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

#ifndef _UDMLOADBALANCE_H_
#define _UDMLOADBALANCE_H_

/**
 * @file UdmLoadBalance.h
 * ロードバランス分割クラスのヘッダーファイル
 */

#ifndef WITHOUT_MPI
#include <zoltan_cpp.h>
#endif
#include "UdmBase.h"
#include "config/UdmSettingsConfig.h"
#include "model/UdmZone.h"
#include "utils/UdmScannerCells.h"

/*
/// グローバルIDのデータサイズ:NUM_GID_ENTRIES
#define ZOLTAN_NUM_GID_ENTRIES                3
*/
/// ローカルIDのデータサイズ:NUM_LID_ENTRIES
#define ZOLTAN_NUM_LID_ENTRIES                1

namespace udm
{

/**
 * ロードバランス分割クラス
 */
class UdmLoadBalance: public UdmBase
{
private:
    MPI_Comm mpi_communicator;            ///< MPIコミュニケータ
#ifndef WITHOUT_MPI
    Zoltan *zoltan;                    ///< Zoltanオブジェクト
#else
    void *zoltan;                    ///< Zoltanオブジェクト:ダミー
#endif
    UdmSettingsConfig *config;        ///< udm.dfi設定
    UdmZone *zone;        ///< 分割ゾーン

public:
    UdmLoadBalance(const MPI_Comm &comm = MPI_COMM_WORLD);
    virtual ~UdmLoadBalance();

    MPI_Comm getMpiComm() const;
    UdmError_t setMpiComm(const MPI_Comm &comm);
    UdmError_t readDfiConfig(const char* dfi_filename);
    UdmError_t partitionZone(UdmZone *zone);
    UdmError_t getParameter(const std::string &name, std::string &value) const;
    UdmError_t setParameter(const std::string &name, const std::string &value);
    UdmError_t removeParameter(const std::string &name);
    UdmError_t setHyperGraphParameters(const std::string &approach);
    UdmError_t setGraphParameters(const std::string &approach);
    int getZoltanDebugLevel() const;
    UdmError_t setZoltanDebugLevel(int debug_level);

#ifndef WITHOUT_MPI
    // Zoltan呼び出し関数
    static int zoltan_get_num_obj(void *data, int *ierr);
    static void zoltan_get_obj_list(
                        void* data,
                        int num_gid_entries,
                        int num_lid_entries,
                        ZOLTAN_ID_PTR global_ids,
                        ZOLTAN_ID_PTR local_ids,
                        int wgt_dim,
                        float* obj_wgts,
                        int* ierr);
    static void zoltan_get_hg_size_cs(
                        void *data,
                        int *num_lists,
                        int *num_pins,
                        int *format,
                        int *ierr);
    static void zoltan_get_hg_cs(
                        void *data,
                        int num_gid_entries,
                        int num_vtx_edge,
                        int num_pins,
                        int format,
                        ZOLTAN_ID_PTR vtxedge_GID,
                        int *vtxedge_ptr,
                        ZOLTAN_ID_PTR pin_GID,
                        int *ierr);
    static void zoltan_get_hg_size_edge_wts(void *data, int *num_edges, int *ierr);
    static void zoltan_get_hg_edge_wts(
                        void *data,
                        int num_gid_entries,
                        int num_lid_entries,
                        int num_edges,
                        int edge_weight_dim,
                        ZOLTAN_ID_PTR edge_GID,
                        ZOLTAN_ID_PTR edge_LID,
                        float *edge_weight,
                        int *ierr);
    static void zoltan_get_num_edges_multi(
                        void *data,
                        int num_gid_entries,
                        int num_lid_entries,
                        int num_obj,
                        ZOLTAN_ID_PTR global_ids,
                        ZOLTAN_ID_PTR local_ids,
                        int *num_edges,
                        int *ierr);
    static void zoltan_get_edge_list_multi(
                        void *data,
                        int num_gid_entries,
                        int num_lid_entries,
                        int num_obj,
                        ZOLTAN_ID_PTR global_ids,
                        ZOLTAN_ID_PTR local_ids,
                        int *num_edges,
                        ZOLTAN_ID_PTR nbor_global_ids,
                        int *nbor_procs,
                        int wgt_dim,
                        float *ewgts,
                        int *ierr);
    static void zoltan_obj_size_multi (
                        void *data,
                        int num_gid_entries,
                        int num_lid_entries,
                        int num_ids,
                        ZOLTAN_ID_PTR global_ids,
                        ZOLTAN_ID_PTR local_ids,
                        int *sizes,
                        int *ierr);
    static void zoltan_pack_obj_multi(
                        void *data,
                        int num_gid_entries,
                        int num_lid_entries,
                        int num_ids,
                        ZOLTAN_ID_PTR global_ids,
                        ZOLTAN_ID_PTR local_ids,
                        int *dest,
                        int *sizes,
                        int *idx,
                        char *buf,
                        int *ierr);
    static void zoltan_unpack_obj_multi (
                        void *data,
                        int num_gid_entries,
                        int num_ids,
                        ZOLTAN_ID_PTR global_ids,
                        int *sizes,
                        int *idx,
                        char *buf,
                        int *ierr);
#endif

private:
    void initialize();
    void finalize();
    UdmError_t createZoltan(const MPI_Comm &comm);
    UdmError_t setZoltanParameters(UdmScannerCells *scanner);
    bool isHyperGraphPartition() const;
    bool isGraphPartition() const;
    static UdmCell* getEntityCell(const UdmZone *zone, UdmSize_t cell_id);
    int getMpiRankno() const;
    int getMpiProcessSize() const;
#ifndef WITHOUT_MPI
    UdmError_t exportProcess(
                    UdmZone *zone,
                    int numGidEntries,
                    int numExport,
                    ZOLTAN_ID_PTR exportGlobalGids,
                    int *exportProcs);
#endif        /* WITHOUT_MPI */
    UdmError_t getWriteDfiFilePath(std::string &filename) const;
    UdmError_t rebuildZone(UdmScannerCells *scanner);
};

} /* namespace udm */

#endif /* _UDMLOADBALANCE_H_ */
