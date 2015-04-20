/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmLoadBalance.cpp
 * ロードバランス分割クラスのソースファイル
 */

#include "model/UdmModel.h"
#include "model/UdmZone.h"
#include "model/UdmSections.h"
#include "model/UdmCell.h"
#include "model/UdmSolid.h"
#include "model/UdmNode.h"
#include "model/UdmRankConnectivity.h"
#include "partition/UdmLoadBalance.h"
#include "utils/UdmSerialization.h"

namespace udm
{

/**
 * コンストラクタ
 * @param comm            MPIコミュニケータ
 */
UdmLoadBalance::UdmLoadBalance(const MPI_Comm& comm) : mpi_communicator(comm), zoltan(NULL), config(NULL)
{
    // 初期化を行う。
    this->initialize();
}

/**
 * デストラクタ
 */
UdmLoadBalance::~UdmLoadBalance()
{
    // 破棄処理を行う
    this->finalize();
}

/**
 * 初期化を行う.
 */
void UdmLoadBalance::initialize()
{
    this->createZoltan(this->mpi_communicator);
    this->config = new UdmSettingsConfig();
}

/**
 * 破棄処理を行う.
 */
void UdmLoadBalance::finalize()
{
    if (this->zoltan != NULL) delete this->zoltan;
    if (this->config != NULL) delete this->config;
}

/**
 * MPIコミュニケータを取得する.
 * @return        MPIコミュニケータ
 */
MPI_Comm UdmLoadBalance::getMpiComm() const
{
    return this->mpi_communicator;
}

/**
 * MPIコミュニケータを設定する.
 * @param comm        MPIコミュニケータ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmLoadBalance::setMpiComm(const MPI_Comm& comm)
{
    this->mpi_communicator = comm;
    return this->createZoltan(this->mpi_communicator);
}

/**
 * udmlib.tpからパラメータ情報を読み込む。
 * @param dfi_filename        udmlib.tpファイル名
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmLoadBalance::readDfiConfig(const char* dfi_filename)
{
    UdmError_t error = this->config->read(dfi_filename);
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    return UDM_OK;
}

/**
 * CGNS:Zoneを分割する.
 * @param zone        CGNS:Zone
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmLoadBalance::partitionZone(UdmZone *zone)
{
#ifndef WITHOUT_MPI
#ifdef _DEBUG_TRACE
    UDM_DEBUG_PRINTF("%s:%d [%s]",  __FILE__, __LINE__, __FUNCTION__);
#endif
#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START(__FUNCTION__);
#endif

    UdmError_t error = UDM_OK;

    int changes, numGidEntries, numLidEntries, numImport, numExport;
    ZOLTAN_ID_PTR importGlobalGids, importLocalGids, exportGlobalGids, exportLocalGids;
    int *importProcs, *importToPart, *exportProcs, *exportToPart;
    int zoltan_result;
    int mpi_flag;
    int n;

    // MPI初期化済みであるかチェックする.
    MPI_Initialized(&mpi_flag);
    if (!mpi_flag) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_INVALID_MPI, "Please execute MPI_Init beforehand.");
    }
    if (this->zoltan == NULL) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "zoltan is null.");
    }
    if (zone == NULL) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "zone is null.");
    }
    else if (zone->getParentModel() == NULL) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "model is null.");
    }
    else if (zone->getMpiProcessSize() < 1) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_INVALID_MPI, "Zone MPI Comm size is %d.", zone->getMpiProcessSize());
    }
    else if (this->getMpiProcessSize() < 1) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_INVALID_MPI, "Partition MPI Comm size is %d.", this->getMpiProcessSize());
    }

    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }

    // グラフデータを作成する.
    UdmScannerCells *scanner = new UdmScannerCells(zone);
    if (zone != NULL) {
        scanner->scannerGraph();
    }
    else {
        error = UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "zone is null.");
    }
    if (error == UDM_OK) {
        // Zoltanパラメータを設定する.
        if ((error = this->setZoltanParameters(scanner)) != UDM_OK) {
            error = UDM_ERRORNO_HANDLER(error);
        }
    }

    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        delete scanner;
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }

    // 分割ゾーン
    this->zone = zone;

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START("Zoltan::LB_Partition");
#endif

    // 分割実行
    zoltan_result = this->zoltan->LB_Partition(
                changes,        /* 1 if partitioning was changed, 0 otherwise */
                numGidEntries,  /* Number of integers used for a global ID */
                numLidEntries,  /* Number of integers used for a local ID */
                numImport,      /* Number of vertices to be sent to me */
                importGlobalGids,  /* Global IDs of vertices to be sent to me */
                importLocalGids,   /* Local IDs of vertices to be sent to me */
                importProcs,    /* Process rank for source of each incoming vertex */
                importToPart,   /* New partition for each incoming vertex */
                numExport,      /* Number of vertices I must send to other processes*/
                exportGlobalGids,  /* Global IDs of the vertices I must send */
                exportLocalGids,   /* Local IDs of the vertices I must send */
                exportProcs,    /* Process to which I send each of the vertices */
                exportToPart);  /* Partition to which each vertex will belong */
    if (zoltan_result != ZOLTAN_OK) {
        error = UDM_ERRORNO_HANDLER(UDM_ERROR_ZOLTAN_LB_PARTITION);
    }

    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        delete scanner;
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }
#ifdef _UDM_PROFILER
    UDM_STOPWATCH_STOP("Zoltan::LB_Partition");
#endif

    // Zoltan分割の変更がある(=1)の場合のみ実行する.
    if (changes == 1) {
        // エクスポート先のプロセス番号を設定する.
        this->exportProcess(zone, numGidEntries, numExport, exportGlobalGids, exportProcs);
    }

#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        UDM_DEBUG_PRINTF("UdmLoadBalance::partitionZone -> Migrate",  __FILE__, __LINE__, __FUNCTION__);
        std::string buf;
        zone->toString(buf);
        UDM_DEBUG_PRINTF(buf.c_str());
    }
#endif

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START("Zoltan::Migrate");
#endif

    // 転送セルのオブジェクトサイズを取得する.
    size_t total_cells_size = scanner->scannerCellsSize(
                                                numGidEntries,
                                                numLidEntries,
                                                numExport,
                                                exportGlobalGids,
                                                exportLocalGids,
                                                exportProcs);

    // 転送回数を取得する。すべてのランク間のMAX値
    int num_divide = (int)ceil((double)total_cells_size/(double)UDM_MIGRATION_MAXSIZE);
    int num_migrate_count = 0;
    udm_mpi_allreduce(&num_divide, &num_migrate_count, 1, MPI_INT, MPI_MAX, this->getMpiComm());

    const std::vector<int> cell_sizes = scanner->getScanCellSizes();
    std::vector<int>::const_iterator cell_itr = cell_sizes.begin();
    int migrate_pos = 0;
    int num_sub_exports = 0;
    size_t migrate_size = 0;
    for (n=0; n<num_migrate_count; n++) {
        migrate_size = 0;
        num_sub_exports = 0;
        for (; cell_itr!=cell_sizes.end(); cell_itr++) {
            migrate_size += (*cell_itr);
            if (migrate_size > UDM_MIGRATION_MAXSIZE) {
                break;
            }
            num_sub_exports++;
        }

        // 転送位置を設定する
        scanner->setScanIterator(migrate_pos);

        // Zoltan転送を行う.
        zoltan_result = this->zoltan->Migrate(
                                -1,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                num_sub_exports,
                                exportGlobalGids + migrate_pos*numGidEntries,
                                exportLocalGids + migrate_pos*numLidEntries,
                                exportProcs + migrate_pos,
                                exportToPart + migrate_pos);

        migrate_pos += num_sub_exports;

        printf("[rank=%d] Zoltan::Migrate loop = %d/%d, size=%ld\n",
                this->getMpiRankno(), n+1, num_migrate_count, migrate_size);

#ifdef _DEBUG
        if (UDM_IS_DEBUG_LEVEL()) {
            UDM_DEBUG_PRINTF("[rank=%d] Zoltan::Migrate loop = %d/%d, size=%d\n",
                            this->getMpiRankno(), n+1, num_migrate_count, migrate_size);
        }
#endif
    }

    if (zoltan_result != ZOLTAN_OK) {
        error = UDM_ERRORNO_HANDLER(UDM_ERROR_ZOLTAN_MIGRATE);
    }

    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        delete scanner;
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }

#ifdef _UDM_PROFILER
    UDM_STOPWATCH_STOP("Zoltan::Migrate");
#endif

#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        std::stringstream stream;
        char buf[8096] = {0x00};
        int my_rank = zone->getMpiRankno();
        int pos;
        sprintf(buf, "[rank:%d] numImport=%d\n", my_rank, numImport);
        stream << buf;
        sprintf(buf, "[rank:%d] importGlobalGids : ", my_rank);
        stream << buf;
        pos = 0;
        for (n=0; n<numImport; n++) {
            UdmSize_t id;
            int rankno;
            pos += udm_get_entryid(importGlobalGids+pos, id, rankno, numGidEntries);
            sprintf(buf, "%d[%d] ", id, rankno);
            stream << buf;
        }
        sprintf(buf, "\n[rank:%d] importLocalGids : ", my_rank);
        stream << buf;
        for (n=0; n<numImport*numLidEntries; n++) {
            sprintf(buf, "%d ", importLocalGids[n]);
            stream << buf;
        }
        sprintf(buf, "\n[rank:%d] importProcs : ", my_rank);
        stream << buf;
        for (n=0; n<numImport; n++) {
            sprintf(buf, "%d ", importProcs[n]);
            stream << buf;
        }
        sprintf(buf, "\n[rank:%d] importToPart : ", my_rank);
        stream << buf;
        for (n=0; n<numImport; n++) {
            sprintf(buf, "%d ", importToPart[n]);
            stream << buf;
        }
        sprintf(buf, "\n\n[rank:%d] numExport=%d\n", my_rank, numExport);
        stream << buf;
        sprintf(buf, "[rank:%d] exportGlobalGids : ", my_rank);
        stream << buf;
        pos = 0;
        for (n=0; n<numExport; n++) {
            UdmSize_t id;
            int rankno;
            pos += udm_get_entryid(exportGlobalGids+pos, id, rankno, numGidEntries);
                sprintf(buf, "%d[%d] ", id, rankno);
                stream << buf;
        }
        sprintf(buf, "\n[rank:%d] exportLocalGids : ", my_rank);
        stream << buf;
        for (n=0; n<numExport*numLidEntries; n++) {
            sprintf(buf, "%d ", exportLocalGids[n]);
            stream << buf;
        }
        sprintf(buf, "\n[rank:%d] exportProcs : ", my_rank);
        stream << buf;
        for (n=0; n<numExport; n++) {
            sprintf(buf, "%d ", exportProcs[n]);
            stream << buf;
        }
        sprintf(buf, "\n[rank:%d] exportToPart : ", my_rank);
        stream << buf;
        for (n=0; n<numExport; n++) {
            sprintf(buf, "%d ", exportToPart[n]);
            stream << buf;
        }
        stream << buf << std::endl;
        UDM_DEBUG_PRINTF(stream.str().c_str());
    }

#endif

    // 領域開放
    this->zoltan->LB_Free_Part(
                &importGlobalGids,
                &importLocalGids,
                &importProcs,
                &importToPart);
    this->zoltan->LB_Free_Part(
                &exportGlobalGids,
                &exportLocalGids,
                &exportProcs,
                &exportToPart);

    // Zoltan分割の変更がある(=1)の場合のみ実行する.
    if (changes == 1) {
        // 分割実行によりインポート、エクスポートした要素（セル）、節点（ノード）の削除、追加を行う。
        error = this->rebuildZone(scanner);
        if (error != UDM_OK) {
            delete scanner;
            return UDM_ERRORNO_HANDLER(error);
        }
    }
    delete scanner;

    // udmlib.tpを出力する.
    if (this->config != NULL) {
        std::string filename;
        this->getWriteDfiFilePath(filename);
        if (filename.empty()) {
            return UDM_ERRORNO_HANDLER(UDM_ERROR_WRITE_DFI);
        }
        if (this->config->write(filename.c_str()) != UDM_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_WRITE_DFI, "filename=%s.", filename.c_str());
        }
    }

#ifdef _UDM_PROFILER
    UDM_STOPWATCH_STOP(__FUNCTION__);
#endif

    // Zoltan分割変更なし警告
    if (changes != 1) {
        return UDM_WARNINGNO_HANDLER(UDM_WARNING_ZOLTAN_NOCHANGE);
    }
#endif        /* WITHOUT_MPI */

    return UDM_OK;
}

/**
 * Zoltan分割パラメータを取得する.
 * @param [in] name        パラメータ名
 * @param [out] value        設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmLoadBalance::getParameter(const std::string& name, std::string& value) const
{
    if (this->config == NULL) return UDM_ERROR;
    return this->config->getParameter(name, value);
}

/**
 * Zoltan分割パラメータを設定する.
 * @param [in] name        パラメータ名
 * @param [in] value        設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmLoadBalance::setParameter(const std::string& name, const std::string& value)
{
    if (this->config == NULL) return UDM_ERROR;
    return this->config->setParameter(name, value);
}

/**
 * Zoltanパラメータを削除、又はデフォルト値とする.
 * @param [in] name        パラメータ名
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmLoadBalance::removeParameter(const std::string& name)
{
    if (this->config == NULL) return UDM_ERROR;
    return this->config->removeParameter(name);
}

/**
 * Hypergraph partitioningのパラメータの設定を行う.
 * @param approach        LB_APPROACHパラメータ値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmLoadBalance::setHyperGraphParameters(const std::string& approach)
{
    if (this->config == NULL) return UDM_ERROR;
    this->config->setParameter("LB_METHOD", "HYPERGRAPH");
    this->config->setParameter("GRAPH_PACKAGE", "PHG");
    this->config->setParameter("LB_APPROACH", approach);

    return UDM_OK;
}

/**
 * Graph Partitioningのパラメータの設定を行う.
 * @param approach        LB_APPROACHパラメータ値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmLoadBalance::setGraphParameters(const std::string& approach)
{
    if (this->config == NULL) return UDM_ERROR;
    this->config->setParameter("LB_METHOD", "GRAPH");
    this->config->setParameter("GRAPH_PACKAGE", "PHG");
    this->config->setParameter("LB_APPROACH", approach);

    return UDM_OK;
}



/**
 * Zoltanオブジェクトを生成する.
 * @param comm        MPIコミュニケータ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmLoadBalance::createZoltan(const MPI_Comm& comm)
{
#ifndef WITHOUT_MPI
    if (this->zoltan != NULL) delete zoltan;
    if (comm != MPI_COMM_NULL) {
        this->zoltan = new Zoltan(comm);
    }
    if (this->zoltan == NULL) return UDM_ERROR;
#endif        /* WITHOUT_MPI */

    return UDM_OK;
}

/**
 * HyperGraph分割であるかチェックする.
 * @return        true=HyperGraph分割
 */
bool UdmLoadBalance::isHyperGraphPartition() const
{
    std::string name;
    std::string value;

    // LB_METHOD
    name = "LB_METHOD";
    if (this->config->getZoltanParameter(name, value) != UDM_OK) {
        return false;
    }
    return (strcmp(value.c_str(), "HYPERGRAPH") == 0);
}


/**
 * Graph分割であるかチェックする.
 * @return        true=HyperGraph分割
 */
bool UdmLoadBalance::isGraphPartition() const
{
    std::string name;
    std::string value;

    // LB_METHOD
    name = "LB_METHOD";
    if (this->config->getZoltanParameter(name, value) != UDM_OK) {
        return false;
    }
    return (strcmp(value.c_str(), "GRAPH") == 0);
}

/**
 * Zoltanにパラメータを設定する.
 * @param scanner        要素（セル）グラフデータ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmLoadBalance::setZoltanParameters(UdmScannerCells *scanner)
{
#ifndef WITHOUT_MPI

    if (this->zoltan == NULL) return UDM_ERROR_NULL_VARIABLE;
    if (this->config == NULL) return UDM_ERROR_NULL_VARIABLE;
    if (scanner->getScanZone() == NULL) return UDM_ERROR_NULL_VARIABLE;

    std::string name;
    std::string value;
    int params_size, n;
    char param_value[32];
    bool set_weight = scanner->getScanZone()->isSetPartitionWeight();

    // LB_METHODが設定済みか
    if (!this->isHyperGraphPartition() && !this->isGraphPartition()) {
        // LB_METHOD未設定に付き、デフォルト設定を設定する.
        this->setHyperGraphParameters(ZOLTAN_DEFAULT_LB_APPROACH);
    }

    // パラメータを設定する.
    params_size = this->config->getNumZoltanParameters();
    for (n=1; n<=params_size; n++) {
        value = "";
        this->config->getZoltanParameterById(n, name, value);
        if (!name.empty() && !value.empty()) {
            this->zoltan->Set_Param(name, value);
        }
    }
    // this->zoltan->Set_Param("PHG_CUT_OBJECTIVE", "HYPEREDGES");
    // this->zoltan->Set_Param("PHG_CUT_OBJECTIVE", "CONNECTIVITY");

    // グローバルID、ローカルIDデータサイズ
    sprintf(param_value, "%d", this->config->getDebugLevel());
    this->zoltan->Set_Param("DEBUG_LEVEL", param_value);
    int gid_size = sizeof(UdmSize_t)/sizeof(int) + 1;
    sprintf(param_value, "%d", gid_size);
    this->zoltan->Set_Param("NUM_GID_ENTRIES", param_value);    /* global IDs are integers */
    sprintf(param_value, "%d", ZOLTAN_NUM_LID_ENTRIES);
    this->zoltan->Set_Param("NUM_LID_ENTRIES", param_value);    /* local IDs are integers */
    this->zoltan->Set_Param("RETURN_LISTS", "ALL");     /* export AND import lists */

    // 要素（セル）に重みが設定されていれば、グラフ頂点重みデータ数を設定する.
    if (set_weight) {
        this->zoltan->Set_Param("OBJ_WEIGHT_DIM", "1");     /* use Zoltan default vertex weights */
    }
    else {
        this->zoltan->Set_Param("OBJ_WEIGHT_DIM", "0");     /* use Zoltan default vertex weights */
    }
    this->zoltan->Set_Param("EDGE_WEIGHT_DIM", "0");    /* use Zoltan default hyperedge weights */
    this->zoltan->Set_Param("AUTO_MIGRATE", "0");

    // Zoltan呼出関数設定
    this->zoltan->Set_Num_Obj_Fn(UdmLoadBalance::zoltan_get_num_obj, scanner);
    this->zoltan->Set_Obj_List_Fn(UdmLoadBalance::zoltan_get_obj_list, scanner);
    if (this->isHyperGraphPartition()) {
        this->zoltan->Set_HG_Size_CS_Fn(UdmLoadBalance::zoltan_get_hg_size_cs, scanner);
        this->zoltan->Set_HG_CS_Fn(UdmLoadBalance::zoltan_get_hg_cs, scanner);
        // this->zoltan->Set_HG_Size_Edge_Wts_Fn(UdmLoadBalance::zoltan_get_hg_size_edge_wts, scanner);
        // this->zoltan->Set_HG_Edge_Wts_Fn(UdmLoadBalance::zoltan_get_hg_edge_wts, scanner);
    }
    else if (this->isGraphPartition()) {
        this->zoltan->Set_Num_Edges_Multi_Fn(UdmLoadBalance::zoltan_get_num_edges_multi, scanner);
        this->zoltan->Set_Edge_List_Multi_Fn(UdmLoadBalance::zoltan_get_edge_list_multi, scanner);
    }

    this->zoltan->Set_Obj_Size_Multi_Fn(UdmLoadBalance::zoltan_obj_size_multi, scanner);
    this->zoltan->Set_Pack_Obj_Multi_Fn(UdmLoadBalance::zoltan_pack_obj_multi, scanner);
    this->zoltan->Set_Unpack_Obj_Multi_Fn(UdmLoadBalance::zoltan_unpack_obj_multi, scanner);

#endif        /* WITHOUT_MPI */

    return UDM_OK;
}

#ifndef WITHOUT_MPI
/**
 * Zoltan呼出関数:グラフ頂点の数を設定する.
 * @param [in] data        UdmScannerCells
 * @param [out] ierr        エラー番号
 * @return        グラフ頂点の数
 */
int UdmLoadBalance::zoltan_get_num_obj(void* data, int* ierr)
{
    UdmScannerCells* scanner = static_cast<UdmScannerCells*>(data);
    if (scanner == NULL) {
        *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "UdmScannerCells is null.");
        return 0;
    }
    // 要素（セル）数を取得する.
    UdmSize_t num_globalids = scanner->getNumObjects();

    return (int)num_globalids;
}

/**
 * Zoltan呼出関数:グラフ頂点のグローバルID、ローカルID、頂点重みを設定する.
 * @param [in]  data        UdmScannerCells
 * @param [in]  num_gid_entries        グローバルIDのデータサイズ
 * @param [in]  num_lid_entries        ローカルIDのデータサイズ
 * @param [out] global_ids    グローバルIDのリスト(zoltan_get_num_obj*num_gid_entries)
 * @param [out] local_ids        ローカルIDのリスト(zoltan_get_num_obj*num_lid_entries)
 * @param [in]  wgt_dim        頂点重みのデータサイズ
 * @param [out] obj_wgts    頂点重みのリスト(zoltan_get_num_obj*wgt_dim)
 * @param [out] ierr        エラー番号
 */
void UdmLoadBalance::zoltan_get_obj_list(
                    void* data,
                    int num_gid_entries,
                    int num_lid_entries,
                    ZOLTAN_ID_PTR global_ids,
                    ZOLTAN_ID_PTR local_ids,
                    int wgt_dim,
                    float* obj_wgts,
                    int* ierr)
{
    UdmScannerCells* scanner = static_cast<UdmScannerCells*>(data);
    if (scanner == NULL) {
        *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "UdmScannerCells is null.");
        return;
    }
    // グローバルIDのデータサイズ,ローカルIDのデータサイズを設定する.
    scanner->setEntriesIdSize(num_gid_entries, num_lid_entries);

    // 要素（セル）情報を取得する.
    scanner->getScanObjects(global_ids, local_ids);

    if (obj_wgts != NULL) {
        // 要素（セル）重みを取得する.
        scanner->getObjectWeights(obj_wgts);
    }

    return;
}

/**
 * Zoltan呼出関数:Hypergraphの頂点間の接続サイズを設定する.
 * @param [in]  data            UdmScannerCells
 * @param [out] num_lists        hyperedgeの頂点の数
 * @param [out] num_pins        hyperedgeの接続数
 * @param [out] format            グラフデータのフォーマット（ZOLTAN_COMPRESSED_EDGE and ZOLTAN_COMPRESSED_VERTEX）
 * @param [out] ierr            エラー番号
 */
void UdmLoadBalance::zoltan_get_hg_size_cs(
                    void* data,
                    int* num_lists,
                    int* num_pins,
                    int* format,
                    int* ierr)
{
    UdmScannerCells* scanner = static_cast<UdmScannerCells*>(data);
    if (scanner == NULL) {
        *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "UdmScannerCells is null.");
        return;
    }

    // 要素（セル）接続数を取得する.
    UdmSize_t num_globalids, num_neighbors;
    scanner->getNumHyperGraphDatas(num_globalids, num_neighbors);
    *num_lists = num_globalids;
    *num_pins = num_neighbors;
    *format = ZOLTAN_COMPRESSED_EDGE;

    return;
}

/**
 * Zoltan呼出関数:Hypergraphの頂点間の接続情報を設定する.
 * @param [in]  data            UdmScannerCells
 * @param [in]  num_gid_entries        グローバルIDのデータサイズ
 * @param [in]  num_vtx_edge        hyperedgeの頂点の数
 * @param [in]  num_pins            hyperedgeの接続数
 * @param [in]  format                グラフデータのフォーマット（ZOLTAN_COMPRESSED_EDGE and ZOLTAN_COMPRESSED_VERTEX）
 * @param [out] vtxedge_GID            hyperedgeのグローバルIDリスト(num_gid_entries*num_vtx_edge)
 * @param [out] vtxedge_ptr            pin_GIDの位置ポインタ（vtxedge_GIDと同一サイズ)
 * @param [out] pin_GID                グラフデータ（format書式に従う）
 * @param [out] ierr        エラー番号
 */
void UdmLoadBalance::zoltan_get_hg_cs(
                        void* data,
                        int num_gid_entries,
                        int num_vtx_edge,
                        int num_pins,
                        int format,
                        ZOLTAN_ID_PTR vtxedge_GID,
                        int* vtxedge_ptr,
                        ZOLTAN_ID_PTR pin_GID,
                        int* ierr)
{
    UdmScannerCells* scanner = static_cast<UdmScannerCells*>(data);
    if (scanner == NULL) {
        *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "UdmScannerCells is null.");
        return;
    }

    // グローバルIDのデータサイズ,ローカルIDのデータサイズを設定する.
    scanner->setEntriesIdSize(num_gid_entries);

    // 要素（セル）接続情報を取得する.
    scanner->getScanHyperGraphDatas(vtxedge_GID, vtxedge_ptr, pin_GID);

#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        std::stringstream stream;
        char buf[8096] = {0x00};
    UdmZone* zone = scanner->getScanZone();
    int my_rank = zone->getMpiRankno();
    int n, m, i, pos = 0, nbor_pos = 0;
        sprintf(buf, "[rank:%d] num_vtx_edge = %d \n", my_rank, num_vtx_edge);
        stream << buf;
        sprintf(buf, "[rank:%d] vtxedge_GID num_gid_entries=%d : ", my_rank, num_gid_entries);
        stream << buf;
    pos = 0;
    for (n=0; n<num_vtx_edge; n++) {
        UdmSize_t id;
        int rankno;
        pos += udm_get_entryid(vtxedge_GID+pos, id, rankno, num_gid_entries);
            sprintf(buf, "%d[%d] ", id, rankno);
            stream << buf;
    }
        stream << std::endl;

        sprintf(buf, "[rank:%d] vtxedge_ptr : ", my_rank);
        stream << buf;
    for (n=0; n<num_vtx_edge; n++) {
            sprintf(buf, "%d ", vtxedge_ptr[n]);
            stream << buf;
    }
        stream << std::endl;

    pos = 0;
        sprintf(buf, "[rank:%d] num_pins = %d \n", my_rank, num_pins);
        stream << buf;
        sprintf(buf, "[rank:%d] pin_GID num_gid_entries=%d: ", my_rank, num_gid_entries);
        stream << buf;
    for (n=0; n<num_pins; n++) {
        UdmSize_t id;
        int rankno;
        pos += udm_get_entryid(pin_GID+pos, id, rankno, num_gid_entries);
            sprintf(buf, "%d[%d] ", id, rankno);
            stream << buf;
    }
        stream << std::endl;
        UDM_DEBUG_PRINTF(stream.str().c_str());
    }

#endif
    return;
}

/**
 * Zoltan呼出関数:Hypergraphの頂点間の重みの数を設定する.
 * @param [in]  data        UdmScannerCells
 * @param [out] num_edges    頂点間の数 (=num_pins:    hyperedgeの接続数)
 * @param [out] ierr        エラー番号
 */
void UdmLoadBalance::zoltan_get_hg_size_edge_wts(void* data, int* num_edges, int* ierr)
{
    UdmScannerCells* scanner = static_cast<UdmScannerCells*>(data);
    if (scanner == NULL) {
        *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "UdmScannerCells is null.");
        return;
    }
    return;
}

/**
 * Zoltan呼出関数:Hypergraphの頂点間の重みを設定する.
 * @param [in]  data        UdmScannerCells
 * @param [in] num_gid_entries    グローバルIDのデータサイズ
 * @param [in] num_lid_entries    ローカルIDのデータサイズ
 * @param [in] num_edges        頂点間の数 (=num_pins:    hyperedgeの接続数)
 * @param [in] edge_weight_dim    重みのデータサイズ
 * @param [out] edge_GID        頂点の接続間のグローバルID
 * @param [out] edge_LID        頂点の接続間のローカルID
 * @param [out] edge_weight        頂点の接続間の重み
 * @param [out] ierr        エラー番号
 */
void UdmLoadBalance::zoltan_get_hg_edge_wts(
                        void* data,
                        int num_gid_entries,
                        int num_lid_entries,
                        int num_edges,
                        int edge_weight_dim,
                        ZOLTAN_ID_PTR edge_GID,
                        ZOLTAN_ID_PTR edge_LID,
                        float* edge_weight,
                        int* ierr)
{
    UdmScannerCells* scanner = static_cast<UdmScannerCells*>(data);
    if (scanner == NULL) {
        *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "UdmScannerCells is null.");
        return;
    }

    return;
}

/**
 * Zoltan呼出関数:Graphの頂点間の接続サイズを設定する.
 * @param [in]  data        UdmScannerCells
 * @param [in] num_gid_entries    グローバルIDのデータサイズ
 * @param [in] num_lid_entries    ローカルIDのデータサイズ
 * @param [in] num_obj            頂点数
 * @param [in]  global_ids        グローバルIDリスト
 * @param [in]  local_ids        ローカルIDリスト
 * @param [out] numEdges        頂点のグラフエッジ数リスト
 * @param [out] ierr        エラー番号
 */
void UdmLoadBalance::zoltan_get_num_edges_multi(
                        void* data,
                        int num_gid_entries,
                        int num_lid_entries,
                        int num_obj,
                        ZOLTAN_ID_PTR global_ids,
                        ZOLTAN_ID_PTR local_ids,
                        int* num_edges,
                        int* ierr)
{
    UdmScannerCells* scanner = static_cast<UdmScannerCells*>(data);
    if (scanner == NULL) {
        *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "UdmScannerCells is null.");
        return;
    }

    // 要素（セル）接続数を取得する.
    UdmSize_t num_globalids;
    scanner->getNumGraphDatas(num_globalids, num_edges);

    return;
}

/**
 * Zoltan呼出関数:Graphの頂点間の接続情報を設定する.
 * @param [in]  data        UdmScannerCells
 * @param [in] num_gid_entries    グローバルIDのデータサイズ
 * @param [in] num_lid_entries    ローカルIDのデータサイズ
 * @param [in] num_obj            頂点数
 * @param [in]  global_ids        グローバルIDリスト
 * @param [in]  local_ids        ローカルIDリスト
 * @param [in] numEdges        頂点のグラフエッジ数リスト
 * @param [out] nbor_global_ids            エッジの構成頂点のグローバルIDリスト
 * @param [out] nbor_procs        エッジの構成頂点のプロセス番号リスト
 * @param [in]  wgt_dim            頂点重みの数
 * @param [out] ewgts            頂点重みリスト
 * @param [out] ierr            エラー番号
 */
void UdmLoadBalance::zoltan_get_edge_list_multi(
                            void* data,
                            int num_gid_entries,
                            int num_lid_entries,
                            int num_obj,
                            ZOLTAN_ID_PTR global_ids,
                            ZOLTAN_ID_PTR local_ids,
                            int* num_edges,
                            ZOLTAN_ID_PTR nbor_global_ids,
                            int* nbor_procs,
                            int wgt_dim,
                            float* ewgts,
                            int* ierr)
{
    UdmScannerCells* scanner = static_cast<UdmScannerCells*>(data);
    if (scanner == NULL) {
        *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "UdmScannerCells is null.");
        return;
    }

    // 要素（セル）接続情報を取得する.
    scanner->getScanGraphDatas(nbor_global_ids, nbor_procs);

#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        std::stringstream stream;
        char buf[8096] = {0x00};
        int n, m, i = 0, pos = 0, nbor_pos = 0;
        UdmSize_t count = 0;
        sprintf(buf, "num_obj = %d \n", num_obj);
        stream << buf;
        for (n=0; n<num_obj; n++) {
            //stream.str(""); // バッファをクリアする。
            //stream.clear(std::stringstream::goodbit);

            UdmSize_t id;
            int rankno;
            pos += udm_get_entryid(global_ids+pos, id, rankno, num_gid_entries);
                sprintf(buf, "%ld : num=%d ", id, num_edges[n]);
                stream << buf;

            for (m=0; m<num_edges[n]; m++) {
                UdmSize_t nbor_id;
                int nbor_rankno;
                nbor_pos += udm_get_entryid(nbor_global_ids+nbor_pos, nbor_id, nbor_rankno, num_gid_entries);
                    sprintf(buf, "%ld[%d] ", nbor_id, nbor_procs[i]);
                    stream << buf;
                i++;
                count++;
            }
            stream << std::endl;
        }
        stream << "Number of vertex neighbors = " << count << std::endl;
        UDM_DEBUG_PRINTF(stream.str().c_str());
    }
#endif

    return;
}


/**
 * Zoltan:Migration呼出関数:データ転送を行うデータサイズを設定する.
 * @param [in]  data        UdmScannerCells
 * @param [in] num_gid_entries    グローバルIDのデータサイズ
 * @param [in] num_lid_entries    ローカルIDのデータサイズ
 * @param [in] num_ids            データ転送頂点数
 * @param [in] global_ids        グローバルIDリスト
 * @param [in] local_ids        ローカルIDリスト
 * @param [out] sizes            データサイズ
 * @param [out] ierr            エラー番号
 */
void UdmLoadBalance::zoltan_obj_size_multi(
                            void* data,
                            int num_gid_entries,
                            int num_lid_entries,
                            int num_ids,
                            ZOLTAN_ID_PTR global_ids,
                            ZOLTAN_ID_PTR local_ids,
                            int* sizes,
                            int* ierr)
{
    UdmScannerCells* scanner = static_cast<UdmScannerCells*>(data);
    if (scanner == NULL) {
        *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "UdmScannerCells is null.");
        return;
    }

#ifdef _DEBUG
    size_t proc1_size = 0;
    int pos = 0;
    UdmSize_t cell_id;
    int rankno;
#endif

    int i;
    size_t total_size = 0;
    // 転送位置
    int migrate_pos = scanner->getScanIterator();
    // 取得済みのシリアライズセルサイズ
    const std::vector<int> cell_sizes = scanner->getScanCellSizes();
    if (cell_sizes.size() > 0) {
        for (i = 0; i < num_ids; i++) {

#ifdef _DEBUG
            // global_idsから要素（セル）IDの取得
            pos += udm_get_entryid(global_ids+pos, cell_id, rankno, num_gid_entries);
#endif

            if (cell_sizes.size() <= (size_t)migrate_pos+i) {
                *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_ZOLTAN_MIGRATE,
                                "get position[%d] over than cell_sizes.size()[%d].",
                                migrate_pos+i, cell_sizes.size());
                return;
            }
            sizes[i] = cell_sizes[migrate_pos+i];
            total_size += sizes[i];
        }
    }
    else {
        /**** ここを通ることはない  ****/
        // 要素（セル）のシリアライズサイズを取得する.
        const UdmZone *zone = scanner->getScanZone();
        if (zone == NULL) {
            *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "UdmZone is null.");
            return;
        }

        UdmSerializeBuffer streamBuffer;        // バッファ(char*)をNULLとする
        UdmSerializeArchive archive(&streamBuffer);
        UdmSize_t cell_id;
        int rankno;
        int pos = 0;
        for (i = 0; i < num_ids; i++) {
            // global_idsから要素（セル）IDの取得
            pos += udm_get_entryid(global_ids+pos, cell_id, rankno, num_gid_entries);
            const UdmCell *cell = UdmLoadBalance::getEntityCell(zone, cell_id);
            if (cell == NULL) {
                *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "not found cell(cell_id=%d,rankno=%d)", cell_id, rankno);
                return;
            }
            archive << *cell;
#if 0
            // 終了マーク
            archive << "END";
#endif
            size_t buf_size = archive.getOverflowSize();
            sizes[i] = buf_size;
            total_size += buf_size;
            // バッファークリア
            archive.clear();
        }
    }

    return;
}

/**
 * Zoltan:Migration呼出関数:転送データを送信する.
 * @param [in]  data        UdmScannerCells
 * @param [in] num_gid_entries    グローバルIDのデータサイズ
 * @param [in] num_lid_entries    ローカルIDのデータサイズ
 * @param [in] num_ids            データ転送頂点数
 * @param [in] global_ids        グローバルIDリスト
 * @param [in] local_ids        ローカルIDリスト
 * @param [in]  dests            転送先プロセス番号
 * @param [in] sizes            転送データサイズ
 * @param [in]  idx                転送バッファ(=buf)のデータ先頭位置リスト
 * @param [out] buf                転送バッファ
 * @param [out] ierr            エラー番号
 */
void UdmLoadBalance::zoltan_pack_obj_multi(
                            void* data,
                            int num_gid_entries,
                            int num_lid_entries,
                            int num_ids,
                            ZOLTAN_ID_PTR global_ids,
                            ZOLTAN_ID_PTR local_ids,
                            int* dests,
                            int* sizes,
                            int* idx,
                            char* buf,
                            int* ierr)
{
#ifdef _UDM_PROFILER
    UDM_STOPWATCH_START(__FUNCTION__);
#endif

    UdmScannerCells* scanner = static_cast<UdmScannerCells*>(data);
    if (scanner == NULL) {
        *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "UdmScannerCells is null.");
        return;
    }

    // 要素（セル）のシリアライズデータを取得する.
    const UdmZone *zone = scanner->getScanZone();
    if (zone == NULL) {
        *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "UdmZone is null.");
        return;
    }
#ifdef _DEBUG
    size_t proc1_size = 0;
#endif

    UdmSerializeBuffer streamBuffer;
    UdmSerializeArchive archive(&streamBuffer);
    UdmSize_t cell_id;
    int rankno;
    int i;
    int pos = 0;
    int total_size = 0;
    for (i = 0; i < num_ids; i++) {
        // global_idsから要素（セル）IDの取得
        pos += udm_get_entryid(global_ids+pos, cell_id, rankno, num_gid_entries);
        UdmCell *cell = UdmLoadBalance::getEntityCell(zone, cell_id);
        if (cell == NULL) {
            *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "not found cell(cell_id=%d,rankno=%d)", cell_id, rankno);
            return;
        }

        // charバッファーをセットする.
        archive.initialize(buf + idx[i], sizes[i]);
        // シリアライズ
        archive << *cell;
#if 0
        // 終了マーク
        archive << "END";
#endif
        total_size += sizes[i];

//        if (!archive.validateFinish()) {
//            *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_SERIALIZE, "abend serialize [num_ids=%d]", i);
//            return;
//        }

        // エクスポート要素（セル）の追加
        scanner->insertExportCell(cell);

        // バッファークリア
        archive.clear();

#ifdef _DEBUG
        if (dests[i] == 1) {
            proc1_size += sizes[i];
        }
#endif
    }
#ifdef _DEBUG
    printf("[rank=%d] zoltan_pack_obj_multi: proc1 size = %ld\n",
            zone->getMpiRankno(), proc1_size);
#endif

#ifdef _UDM_PROFILER
    UDM_STOPWATCH_STOP(__FUNCTION__);
    char info[128] = {0x00};
    sprintf(info, "zoltan_pack_obj_multi:count=%d,total_size=%d", num_ids, total_size);
    UDM_STOPWATCH_INFORMATION(__FUNCTION__, info);
#endif
    return;
}

/**
 * Zoltan:Migration呼出関数:転送データを受信する.
 * @param [in]  data        UdmScannerCells
 * @param [in] num_gid_entries    グローバルIDのデータサイズ
 * @param [in] num_ids            データ転送頂点数
 * @param [in] global_ids        グローバルIDリスト
 * @param [in] sizes            転送データサイズ
 * @param [in] idx                転送バッファ(=buf)のデータ先頭位置リスト
 * @param [in] buf                転送バッファ
 * @param [out] ierr            エラー番号
 */
void UdmLoadBalance::zoltan_unpack_obj_multi(
                            void* data,
                            int num_gid_entries,
                            int num_ids,
                            ZOLTAN_ID_PTR global_ids,
                            int* sizes,
                            int* idx,
                            char* buf,
                            int* ierr)
{
#ifdef _UDM_PROFILER
    UDM_STOPWATCH_START(__FUNCTION__);
#endif

    UdmScannerCells* scanner = static_cast<UdmScannerCells*>(data);
    if (scanner == NULL) {
        *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "UdmScannerCells is null.");
        return;
    }

    // 要素（セル）のシリアライズデータを取得する.
    const UdmZone *zone = scanner->getScanZone();
    if (zone == NULL) {
        *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "UdmZone is null.");
        return;
    }

    int myrank = zone->getMpiRankno();
    UdmSize_t cell_id;
    int rankno;
    int i;
    int pos = 0;
    size_t total_size = 0;
    for (i = 0; i < num_ids; i++) {
        if (sizes[i] == 0) continue;

        // global_idsから要素（セル）IDの取得
        pos += udm_get_entryid(global_ids+pos, cell_id, rankno, num_gid_entries);
        const UdmCell *cell = NULL;
        if (myrank == rankno) {
            cell = UdmLoadBalance::getEntityCell(zone, cell_id);
            if (cell == NULL) {
                *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "not found cell(cell_id=%d,rankno=%d)", cell_id, rankno);
                return;
            }
        }

        // デシリアライズを行う.
        char* unpack_buf = buf + idx[i];
        int unpack_size = sizes[i];
        total_size += sizes[i];

        UdmSerializeBuffer streamBuffer(unpack_buf, unpack_size);
        UdmSerializeArchive archive(&streamBuffer);

#if 0         // tellg, seekg削除
        // 要素（セル）:現在位置
        // std::iostream::pos_type cur = archive.tellg();
        size_t cur = archive.tellg();

        UdmSize_t id;
        UdmDataType_t data_type;
        UdmElementType_t element_type;

        // UdmEntity基本情報
        UdmCell::deserializeEntityBase(archive, id, element_type);

        // 要素（セル）の生成
        UdmCell *deser_cell = UdmCell::factoryCell(element_type);

        // 要素（セル）:現在位置まで戻す。
        archive.seekg(cur);
#endif

        // デシリアライズを行う。UdmSolid固定とする
        UdmCell *deser_cell = new UdmSolid();
        archive >> *deser_cell;

#if 0
        // 終了マークチェック
        char strend[8] = {0x00};
        archive.readString(strend, 3);
        if (strcmp(strend, "END") != 0) {
            *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_DESERIALIZE, "abend deserialize to check end mark [num_ids=%d]", i);
            return;
        }
#endif
        if (!archive.validateFinish()) {
            *ierr = (int)UDM_ERROR_HANDLER(UDM_ERROR_DESERIALIZE, "abend deserialize [num_ids=%d]", i);
            return;
        }

        // インポート要素（セル）の追加
        scanner->insertImportCell(deser_cell);
    }

#ifdef _UDM_PROFILER
    UDM_STOPWATCH_STOP(__FUNCTION__);
    char info[128] = {0x00};
    sprintf(info, "zoltan_unpack_obj_multi:count=%d,total_size=%d", num_ids, total_size);
    UDM_STOPWATCH_INFORMATION(__FUNCTION__, info);
#endif
    return;
}
#endif        /* WITHOUT_MPI */

/**
 * ゾーンから要素（セル）IDの要素を取得する.
 * @param zone            取得ゾーン
 * @param cell_id        要素（セル）ID
 * @return            要素
 */
UdmCell* UdmLoadBalance::getEntityCell(const UdmZone *zone, UdmSize_t cell_id)
{
    // 要素（セル）を取得する.
    if (zone == NULL) return NULL;
    const UdmSections *sections = zone->getSections();
    if (sections == NULL) return NULL;
    UdmCell *cell = sections->getEntityCell(cell_id);
    return cell;
}

/**
 * MPIプロセス数を取得する.
 * @return        MPIプロセス数
 */
int UdmLoadBalance::getMpiProcessSize() const
{
    if (this->mpi_communicator == MPI_COMM_NULL) {
        return -1;
    }
    int num_procs = -1;
    udm_mpi_comm_size(this->mpi_communicator, &num_procs);

    return num_procs;
}

/**
 * MPIランク番号を取得する.
 * @return        MPIランク番号
 */
int UdmLoadBalance::getMpiRankno() const
{
    if (this->mpi_communicator == MPI_COMM_NULL) {
        return -1;
    }
    int rankno = -1;
    udm_mpi_comm_rank(this->mpi_communicator, &rankno);

    return rankno;
}

#ifndef WITHOUT_MPI
/**
 * エクスポート要素、節点に対してエクスポートプロセス情報を設定する.
 * @param zone                分割ゾーン
 * @param numGidEntries    グローバルIDのデータサイズ
 * @param numExport            エクスポート頂点数
 * @param exportGlobalGids    エクスポートグローバルIDリスト
 * @param exportProcs        エクスポート先プロセス番号
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmLoadBalance::exportProcess(
                        UdmZone* zone,
                        int numGidEntries,
                        int numExport,
                        ZOLTAN_ID_PTR exportGlobalGids,
                        int *exportProcs)
{

    UdmSize_t cell_id;
    int rankno;
    int i;
    int pos = 0;
    for (i = 0; i < numExport; i++) {
        // global_idsから要素（セル）IDの取得
        pos += udm_get_entryid(exportGlobalGids+pos, cell_id, rankno, numGidEntries);
        UdmCell *cell = UdmLoadBalance::getEntityCell(zone, cell_id);
        if (cell == NULL) {
            return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "not found cell(cell_id=%d,rankno=%d)", cell_id, rankno);
        }

        // 転送先プロセス番号を仮設定する.
        // 仮設定はUdmRankConnectivity::eraseRankConnectivity()で削除する.
        cell->setExportRankno(exportProcs[i]);
    }

    // 内部境界情報を送受信する.
    UdmRankConnectivity *inner = zone->getRankConnectivity();
    UdmError_t error = inner->exportProcess();
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    return UDM_OK;
}
#endif        /* WITHOUT_MPI */

/**
 * 出力DFIファイルパスを取得する.
 * @param  [out] filename            出力DFIファイル名
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmLoadBalance::getWriteDfiFilePath(std::string &filename) const
{
    filename = UDM_DFI_FILENAME_UDMLIB;
    std::string dfi_name;
    if (this->config == NULL) return UDM_ERROR;
    if (this->zone == NULL) return UDM_ERROR;
    this->config->getUdmDfiPath(dfi_name);
    if (!dfi_name.empty()) {
        // ファイル名のみ取り出す.
        char name[256] = {0x00};
        udm_get_filename(dfi_name.c_str(), name);
        filename = name;
    }

    UdmDfiConfig *index_config = this->zone->getParentModel()->getDfiConfig();
    if (index_config == NULL) return UDM_ERROR;
    std::string fieldpath;
    index_config->getDfiOutputDirectory(fieldpath);
    if (fieldpath.empty()) {
        filename = UDM_DFI_FILENAME_UDMLIB;
        return UDM_OK;
    }

    char path[512] = {0x00};
    udm_connect_path(fieldpath.c_str(), filename.c_str(), path);
    filename = path;

    return UDM_OK;
}

/**
 * Zoltanのデバッグレベルを取得する
 * @return        Zoltanのデバッグレベル
 */
int UdmLoadBalance::getZoltanDebugLevel() const
{
    if (this->config == NULL) return 0;
    return this->config->getDebugLevel();
}

/**
 * Zoltanのデバッグレベルを設定する.
 * @param debug_level        Zoltanのデバッグレベル
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmLoadBalance::setZoltanDebugLevel(int debug_level)
{
    if (this->config == NULL) return UDM_ERROR;
    return this->config->setDebugLevel(debug_level);
}

/**
 * ゾーンの要素（セル）、節点（ノード）を再構築する.
 * 分割実行によりインポート、エクスポートした要素（セル）、節点（ノード）の削除、追加を行う。
 * 仮想要素（セル）、節点（ノード）を削除し、分割結果から再作成する。
 * @param scanner        分割スキャナー
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmLoadBalance::rebuildZone(UdmScannerCells *scanner)
{
#ifdef _DEBUG_TRACE
    UDM_DEBUG_PRINTF("%s:%d [%s]",  __FILE__, __LINE__, __FUNCTION__);
#endif
#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START("UdmLoadBalance::rebuildZone");
#endif

    UdmError_t error = UDM_OK;
    if (scanner == NULL) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "UdmScannerCells is null.");
    }

    UdmZone *zone = NULL;
    UdmRankConnectivity *inner = NULL;
    if (scanner != NULL) zone = scanner->getScanZone();
    if (zone == NULL) {
        error =  UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "UdmZone is null.");
    }
    else if ( (inner = zone->getRankConnectivity()) == NULL) {
        error =  UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "UdmRankConnectivity is null.");
    }

    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }

    // 内部境界を削除する.
    // inner->clear();

    // 仮想要素（セル）、節点（ノード）を削除する.
    error = zone->clearVertialCells();
    if (error != UDM_OK) {
        error = UDM_ERRORNO_HANDLER(error);
    }

#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        UDM_DEBUG_PRINTF("UdmLoadBalance::rebuildZone -> exportCells",  __FILE__, __LINE__, __FUNCTION__);
        std::string buf;
        zone->toString(buf);
        UDM_DEBUG_PRINTF(buf.c_str());
    }
#endif

    // エクスポートした要素（セル）を削除する.
    std::vector<UdmEntity*> export_nodes;
    error = zone->exportCells(scanner->getExportCells(), export_nodes);
    if (error != UDM_OK) {
        error = UDM_ERRORNO_HANDLER(error);
    }

#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        UDM_DEBUG_PRINTF("UdmLoadBalance::rebuildZone -> importCells",  __FILE__, __LINE__, __FUNCTION__);
        std::string buf;
        zone->toString(buf);
        UDM_DEBUG_PRINTF(buf.c_str());
    }
#endif

    // インポートした要素（セル）を追加する.
    std::vector<UdmNode*> import_nodes;
    error = zone->importCells(scanner->getImportCells(), import_nodes);
    if (error != UDM_OK) {
        error =  UDM_ERRORNO_HANDLER(error);
    }

    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }

    // 内部境界節点（ノード）の送受信を行う.
    error = inner->migrationBoundary();
    if (error != UDM_OK) {
        error =  UDM_ERRORNO_HANDLER(error);
    }

    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }

    // 以前のランクID、IDをクリアする.
    zone->clearPreviousInfos();

    // 要素構成を再作成する.
    error = zone->rebuildZone(import_nodes);
    if (error != UDM_OK) {
        error =  UDM_ERRORNO_HANDLER(error);
    }

    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }

#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        UDM_DEBUG_PRINTF("UdmLoadBalance::rebuildZone -> transferVirtualCells",  __FILE__, __LINE__, __FUNCTION__);
        std::string buf;
        zone->toString(buf);
        UDM_DEBUG_PRINTF(buf.c_str());
    }
#endif

    // 仮想要素（セル）を転送する.
    std::vector<UdmNode*> virtual_nodes;
    error = inner->transferVirtualCells(virtual_nodes);
    if (error != UDM_OK) {
        error =  UDM_ERRORNO_HANDLER(error);
    }

    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }

#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        UDM_DEBUG_PRINTF("UdmLoadBalance::rebuildZone (finish)",  __FILE__, __LINE__, __FUNCTION__);
        std::string buf;
        zone->toString(buf);
        UDM_DEBUG_PRINTF(buf.c_str());
    }
#endif

#ifdef _UDM_PROFILER
    UDM_STOPWATCH_STOP("UdmLoadBalance::rebuildZone");
#endif

    return UDM_OK;
}

} /* namespace udm */
