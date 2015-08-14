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
 * @file udmlib.c
 * UDMlibライブラリのC言語インターフェイスのソースファイル
 */


#include "udmlib.h"
#include "model/UdmModel.h"

namespace udm {

/**
 * UdmModelモデルクラスを生成する.
 * @return        生成UdmModelクラスポインタ
 */
UdmHanler_t udm_create_model()
{
    return static_cast<void*> (new UdmModel());
}

/**
 * UdmModelクラスオブジェクトを破棄する.
 * @param udm_handler           UdmModelクラスポインタ
 */
void udm_delete_model(UdmHanler_t udm_handler)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return;
    delete model;
}


/**
 * DFIファイルの設定情報に従って、CGNSファイルを読み込む.
 * @param udm_handler           UdmModelクラスポインタ
 * @param dfi_filename        index.dfiファイル名 (-1 = 最初の時系列データ)
 * @param timeslice_step    読込ステップ番号
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_load_model(
                        UdmHanler_t udm_handler,
                        const char* dfi_filename,
                        int timeslice_step)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    return model->loadModel(dfi_filename, timeslice_step);
}


/**
 * CGNSファイルを読みこむ.
 * @param udm_handler           UdmModelクラスポインタ
 * @param cgns_filename        CGNSファイル名
 * @param timeslice_step       CGNS読込ステップ番号 (-1 = 最初の時系列データ)
 * @return        エラー番号 : UDM_OK | UDM_ERROR, etc.
 */
UdmError_t udm_read_cgns(   UdmHanler_t udm_handler,
                            const char* cgns_filename,
                            int timeslice_step)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    return model->readCgns(cgns_filename, timeslice_step);
}

/**
 * DFI設定に従って、CGNSファイル, index.dfiを出力する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param timeslice_step        時系列ステップ数
 * @param timeslice_time        時系列ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_write_model( UdmHanler_t udm_handler,
                            int timeslice_step,
                            float timeslice_time)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    return model->writeModel(timeslice_step, timeslice_time);

}

/**
 * DFI設定に従って、CGNSファイル, index.dfiを出力する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param timeslice_step        時系列ステップ数
 * @param timeslice_time        時系列ステップ時間
 * @param average_step            平均ステップ数
 * @param average_time            平均ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_write_model_average(
                            UdmHanler_t udm_handler,
                            int timeslice_step,
                            float timeslice_time,
                            int average_step,
                            float average_time)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    return model->writeModel(timeslice_step, timeslice_time, average_step, average_time);
}

/**
 * ベースファイル名を設定する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param prefix        ベースファイル名
 */
void udm_config_setfileprefix(UdmHanler_t udm_handler, const char* prefix)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return;
    if (model->getDfiConfig() == NULL) return;
    model->getDfiConfig()->setFileInfoPrefix(prefix);
    return;
}

/**
 * 出力ディレクトリを設定する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param path        出力ディレクトリ
 */
void udm_config_setoutputpath(UdmHanler_t udm_handler, const char* path)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return;
    if (model->getDfiConfig() == NULL) return;
    model->getDfiConfig()->setOutputPath(path);
    return;
}

/**
 * ゾーンを生成して、ゾーンをモデルに追加する.
 * 既定の命名規約のゾーン名称にてゾーンを作成する.
 * @param udm_handler           UdmModelクラスポインタ
 * @return        ゾーンID（１～）: 0の場合は生成エラー
 */
int udm_create_zone(UdmHanler_t udm_handler)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return 0;
    UdmZone* zone = model->createZone();
    if (zone == NULL) return 0;
    return zone->getId();
}


/**
 * 節点（ノード）グリッド座標を追加する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param zone_id     ゾーンID
 * @param x        グリッド座標X
 * @param y        グリッド座標Y
 * @param z        グリッド座標Z
 * @return        節点（ノード）ID（１～）: 0の場合は追加エラー
 */
UdmSize_t udm_insert_gridcoordinates(UdmHanler_t udm_handler, int zone_id, UdmReal_t x, UdmReal_t y, UdmReal_t z)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return 0;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return 0;
    UdmGridCoordinates* grid = zone->getGridCoordinates();
    if (grid == NULL) return 0;
    return grid->insertGridCoordinates(x, y, z);
}


/**
 * セクション（要素構成）を作成する.
 * セクション（要素構成）を作成して、セクション（要素構成）リストに追加する.
 * 既定の命名規約に従いセクション（要素構成）名称を作成する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param zone_id     ゾーンID
 * @param element_type            要素タイプ
 * @return        生成セクション（要素構成）ID（１～）: 0の場合は生成エラー
 */
int udm_create_section(
                    UdmHanler_t udm_handler,
                    int zone_id,
                    UdmElementType_t element_type)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return 0;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return 0;
    UdmSections *sections = zone->getSections();
    if (sections == NULL) return 0;
    UdmElements *elements = sections->createSection(element_type);
    if (elements == NULL) return 0;
    return elements->getId();
}

/**
 * 要素（セル）をセクション（要素構成）に追加する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param zone_id     ゾーンID
 * @param elem_type        要素タイプ
 * @param node_ids        要素接続情報（ノードリスト）
 * @return        要素（セル）ID（１～）: 0の場合は挿入エラー
 */
UdmSize_t udm_insert_cellconnectivity(
                    UdmHanler_t udm_handler,
                    int zone_id,
                    UdmElementType_t elem_type,
                    UdmSize_t* node_ids)
{
    int element_id = 1;      // セクション（要素構成）ID=1固定
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return 0;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return 0;
    UdmSections *sections = zone->getSections();
    if (sections == NULL) return 0;
    UdmElements *elements = sections->getSection(element_id);
    if (elements == NULL) return 0;
    return elements->insertCellConnectivity(elem_type, node_ids);
}


/**
 * 内部境界情報を節点（ノード）に追加する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param zone_id     ゾーンID
 * @param node_id        節点（ノード）ID（１～）
 * @param rankno        接続先MPIランク番号（０～）
 * @param localid        接続先節点（ノード）ID（１～）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_insert_rankconnectivity(
                    UdmHanler_t udm_handler,
                    int zone_id,
                    UdmSize_t node_id,
                    int rankno,
                    UdmSize_t localid)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmGridCoordinates *grid = zone->getGridCoordinates();
    if (grid == NULL) return UDM_ERROR_NULL_VARIABLE;
    return grid->insertRankConnectivity(node_id, rankno, localid);
}


/**
 * モデルの再構築を行う.
 * @param udm_handler           UdmModelクラスポインタ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_rebuild_model(UdmHanler_t udm_handler)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    return model->rebuildModel();
}


/**
 * UdmRankConnectivityの接続面をCGNSファイル出力する.
 * UdmRankConnectivityの検証用関数.
 * @param udm_handler           UdmModelクラスポインタ
 * @param cgns_filename        出力CGNSファイル
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_write_rankconnectivity(
                        UdmHanler_t udm_handler,
                        const char* cgns_filename)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    return model->writeRankConnectivity(cgns_filename);
}

/**
 * 仮想セルをCGNSファイル出力する.
 * 仮想セルの検証用関数.
 * @param udm_handler           UdmModelクラスポインタ
 * @param cgns_filename        出力CGNSファイル
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_write_virtualcells(
                        UdmHanler_t udm_handler,
                        const char* cgns_filename)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    return model->writeVirtualCells(cgns_filename);
}

/**
 * ゾーン数を取得する.
 * @param udm_handler           UdmModelクラスポインタ
 * @return        ゾーン数 : -1の場合は、エラー
 */
int udm_getnum_zones(UdmHanler_t udm_handler)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return -1;
    return model->getNumZones();
}

/**
 * セクション（要素構成）数を取得する.
 * @param udm_handler           UdmModelクラスポインタ
 * @return        セクション（要素構成）数 : -1の場合は、エラー
 */
int udm_getnum_sections(UdmHanler_t udm_handler, int zone_id)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return -1;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return -1;
    UdmSections *sections = zone->getSections();
    if (sections == NULL) return -1;
    return sections->getNumSections();
}

/**
 * ユーザ定義データ情報を取得する.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] zone_id        ゾーンID（１～）
 * @param [in]  array_name        ユーザ定義データ名
 * @param [out] data_type            データ型
 * @param [out] dimension            データ次元数
 * @param [out] dim_sizes            データ次元毎のデータ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_user_getinfo(
                UdmHanler_t udm_handler,
                int zone_id,
                const char* user_name,
                UdmDataType_t* data_type,
                int* dimension,
                UdmSize_t* dim_sizes)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmUserDefinedDatas* user_defined = zone->getUserDefinedDatas();
    if (user_defined == NULL) return UDM_ERROR_NULL_VARIABLE;

    return user_defined->getUserDataInfo(user_name, *data_type, *dimension, dim_sizes);
}


/**
 * ユーザ定義データを取得する.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] zone_id        ゾーンID（１～）
 * @param [in] user_name        ユーザ定義データ名
 * @param [in] data_type            取得データ型
 * @param [out] data                ユーザ定義データ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_user_getdata(
                UdmHanler_t udm_handler,
                int zone_id,
                const char* user_name,
                UdmDataType_t data_type,
                void* data)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmUserDefinedDatas* user_defined = zone->getUserDefinedDatas();
    if (user_defined == NULL) return UDM_ERROR_NULL_VARIABLE;

    return user_defined->getUserDataArray(user_name, data_type, data);
}

/**
 * ユーザ定義データを設定する.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] zone_id        ゾーンID（１～）
 * @param [in] user_name        ユーザ定義データ名
 * @param [in] data_type            データ型
 * @param [in] dimension            データ次元数
 * @param [in] dim_sizes            データ次元毎のデータ数
 * @param [in] data                ユーザ定義データ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_user_setdata(
                UdmHanler_t udm_handler,
                int zone_id,
                const char* user_name,
                UdmDataType_t data_type,
                int dimension,
                UdmSize_t* dim_sizes,
                void* data)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmUserDefinedDatas* user_defined = zone->getUserDefinedDatas();
    if (user_defined == NULL) return UDM_ERROR_NULL_VARIABLE;

    return user_defined->setUserData(user_name, data_type, dimension, dim_sizes, data);
}

/**
 * ユーザ定義データを削除する.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] zone_id        ゾーンID（１～）
 * @param [in] user_name        ユーザ定義データ名
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_user_remove(
                UdmHanler_t udm_handler,
                int zone_id,
                const char* user_name)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmUserDefinedDatas* user_defined = zone->getUserDefinedDatas();
    if (user_defined == NULL) return UDM_ERROR_NULL_VARIABLE;

    return user_defined->removeUserData(user_name);
}

/**
 * 入力CGNSファイルパスを取得する.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [out]  file_path            入力CGNSファイルパス
 * @param [in] rank_no            MPIランク番号
 * @return        入力CGNSファイルポインタ ：エラーの場合はNULL
 */
const char* udm_config_getcgnsinputfile(
                UdmHanler_t udm_handler,
                char* file_path,
                int rank_no)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return NULL;
    if (model->getDfiConfig() == NULL) return NULL;
    std::string path;
    if (model->getDfiConfig()->getCgnsInputFilePath(path, rank_no) != UDM_OK) {
        return NULL;
    }
    strcpy(file_path, path.c_str());
    return file_path;
}

/**
 * リンクCGNSファイルパスを取得する.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [out]  file_path            リンクCGNSファイルパス
 * @param [in] rank_no            MPIランク番号
 * @return        リンクCGNSファイルポインタ ：エラーの場合はNULL
 */
const char* udm_config_getcgnslinkfile(
                UdmHanler_t udm_handler,
                char* file_path,
                int rank_no)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return NULL;
    if (model->getDfiConfig() == NULL) return NULL;
    std::string path;
    if (model->getDfiConfig()->getCgnsLinkFilePath(path, rank_no) != UDM_OK) {
        return NULL;
    }
    strcpy(file_path, path.c_str());
    return file_path;
}


/**
 * ゾーンの分割を行う。
 * @param udm_handler           UdmModelクラスポインタ
 * @param zone_id        ゾーンID（１～）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_partition_zone(
                UdmHanler_t udm_handler,
                int zone_id)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    return model->partitionZone(zone_id);
}


/**
 * Hypergraph partitioningのパラメータの設定を行う.
 * @param udm_handler           UdmModelクラスポインタ
 * @param approach        LB_APPROACHパラメータ値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t udm_partition_sethypergraph(
                UdmHanler_t udm_handler,
                const char* approach)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmLoadBalance *partition = model->getLoadBalance();
    if (partition == NULL) return UDM_ERROR_NULL_VARIABLE;
    return partition->setHyperGraphParameters(approach);
}

/**
 * Graph Partitioningのパラメータの設定を行う.
 * @param udm_handler           UdmModelクラスポインタ
 * @param approach        LB_APPROACHパラメータ値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t udm_partition_setgraph(
                UdmHanler_t udm_handler,
                const char* approach)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmLoadBalance *partition = model->getLoadBalance();
    if (partition == NULL) return UDM_ERROR_NULL_VARIABLE;
    return partition->setGraphParameters(approach);
}

/**
 * Zoltan分割パラメータを取得する.
 * @param [in]  udm_handler           UdmModelクラスポインタ
 * @param [in] name        パラメータ名
 * @param [out] value        設定値
 * @return        取得設定値ポインタ : NULLの場合は取得エラー
 */
const char* udm_partition_getparameter(
                UdmHanler_t udm_handler,
                const char* name,
                char* value)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return NULL;
    UdmLoadBalance *partition = model->getLoadBalance();
    if (partition == NULL) return NULL;
    std::string param_value;
    UdmError_t error = UDM_OK;
    if ( (error = partition->getParameter(name, param_value)) != UDM_OK) {
        return NULL;
    }
    strcpy(value, param_value.c_str());
    return value;
}

/**
 * Zoltan分割パラメータを設定する.
 * @param [in]  udm_handler           UdmModelクラスポインタ
 * @param [in] name        パラメータ名
 * @param [in] value        設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t udm_partition_setparameter(
                UdmHanler_t udm_handler,
                const char* name,
                const char* value)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmLoadBalance *partition = model->getLoadBalance();
    if (partition == NULL) return UDM_ERROR_NULL_VARIABLE;
    return partition->setParameter(name, value);
}

/**
 * Zoltan分割パラメータを削除、又はデフォルト値とする.
 * @param [in]  udm_handler           UdmModelクラスポインタ
 * @param [in] name        パラメータ名
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t udm_partition_removeparameter(
                UdmHanler_t udm_handler,
                const char* name)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmLoadBalance *partition = model->getLoadBalance();
    if (partition == NULL) return UDM_ERROR_NULL_VARIABLE;
    return partition->removeParameter(name);
}

/**
 * Zoltanのデバッグレベルを設定する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param debug_level        Zoltanのデバッグレベル
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t udm_partition_setdebuglevel(UdmHanler_t udm_handler, int debug_level)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmLoadBalance *partition = model->getLoadBalance();
    if (partition == NULL) return UDM_ERROR_NULL_VARIABLE;
    return partition->setZoltanDebugLevel(debug_level);

}


/**
 * ベースファイル名を取得する.
 * @param [in]  udm_handler           UdmModelクラスポインタ
 * @param [out] prefix        ベースファイル名
 * @return        取得ベースファイル名ポインタ : NULLの場合は取得エラー
 */
const char* udm_config_getfileprefix(UdmHanler_t udm_handler, char* prefix)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return NULL;
    if (model->getDfiConfig() == NULL) return  NULL;
    UdmFileInfoConfig* fileconfig = model->getDfiConfig()->getFileinfoConfig();
    if (fileconfig == NULL) return  NULL;
    std::string value;
    fileconfig->getPrefix(value);
    strcpy(prefix, value.c_str());
    return prefix;
}


/**
 * 出力ディレクトリを取得する.
 * @param [in]  udm_handler           UdmModelクラスポインタ
 * @param [out] path        出力ディレクトリ
 * @return        出力ディレクトリポインタ : NULLの場合は取得エラー
 */
const char* udm_config_getoutputpath(UdmHanler_t udm_handler, char* path)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return NULL;
    if (model->getDfiConfig() == NULL) return  NULL;
    const std::string value = model->getDfiConfig()->getOutputPath();
    strcpy(path, value.c_str());
    return path;
}


/**
 * DFIの出力ディレクトリを取得する.
 * 読込index.dfiのパス + 出力ディレクトリとする.
 * @param [in]  udm_handler           UdmModelクラスポインタ
 * @param [out] path               出力ディレクトリ
 * @return
 */
const char* udm_config_getoutputdirectory(
                UdmHanler_t udm_handler,
                char* path)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return NULL;
    if (model->getDfiConfig() == NULL) return  NULL;
    std::string value;
    if (model->getDfiConfig()->getDfiOutputDirectory(value) != UDM_OK) {
        return NULL;
    }
    strcpy(path, value.c_str());
    return path;
}

/**
 * 内部CGNSデータの検証を行う.
 * @param   udm_handler           UdmModelクラスポインタ
 * @return        true=検証OK
 */
bool udm_validate_cgns(UdmHanler_t udm_handler)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return false;
    return model->validateCgns();
}

/**
 * 時間測定を開始する.
 * @param label        時間測定ラベル
 */
void udm_stopwatch_start(const char* label)
{
    UdmStopWatch::getInstance()->start(label);
}

/**
 * 時間測定を停止する.
 * @param label        時間測定ラベル
 */
void udm_stopwatch_stop(const char* label)
{
    UdmStopWatch::getInstance()->stop(label);
}

/**
 * 測定時間を文字列出力する.
 */
void udm_stopwatch_print()
{
    UdmStopWatch::getInstance()->print();
}

/**
 * 追加情報を設定する
 * @param label        時間測定ラベル
 * @param information        追加情報
 */
void udm_stopwatch_info(const char* label, const char* info)
{
    UdmStopWatch::getInstance()->addInformation(label,info);
}

/**
 * デバッグレベルを設定する.
 * デバッグレベル : デフォルト 0x0002
 * 0x0000                エラーメッセージ出力なし：エラーメッセージはUdmErrorHandler::messageにセットするのみ。
 * 0x0001                エラーメッセージを常時出力する.
 * 0x0002                エラー、警告メッセージを常時出力する.
 * 0x0003                エラー、警告、情報メッセージを常時出力する.
 * 0x0004                エラー、警告、情報、デバッグメッセージを常時出力する.
 * @param debug_level        デバッグレベル
 */
void udm_debug_setlevel(unsigned int level)
{
    UdmErrorHandler *handler = UdmErrorHandler::getInstance();
    handler->setDebugLevel(level);
}


/**
 * デバッグレベルを取得する.
 * デバッグレベル : デフォルト 0x0002
 * 0x0000                エラーメッセージ出力なし：エラーメッセージはUdmErrorHandler::messageにセットするのみ。
 * 0x0001                エラーメッセージを常時出力する.
 * 0x0002                エラー、警告メッセージを常時出力する.
 * 0x0003                エラー、警告、情報メッセージを常時出力する.
 * 0x0004                エラー、警告、情報、デバッグメッセージを常時出力する.
 * @return        デバッグレベル
 */
unsigned int udm_debug_getlevel()
{
    UdmErrorHandler *handler = UdmErrorHandler::getInstance();
    return handler->getDebugLevel();
}


/**
 * 出力先を設定する.
 * 出力先 : デフォルト 0x01
 * 0x00                    標準出力、ファイル出力なし
 * 0x01                    標準出力のみに出力
 * 0x02                    ファイルのみに出力
 * 0x03                    標準出力、ファイルに出力
 * @param output_target            出力先
 */
void udm_debug_setoutput(unsigned int output)
{
    UdmErrorHandler *handler = UdmErrorHandler::getInstance();
    handler->setOutputTarget(output);
}

/**
 * 出力先（デフォルト 0x01）を取得する.
 * 出力先 : デフォルト 0x01
 * 0x00                    標準出力、ファイル出力なし
 * 0x01                    標準出力のみに出力
 * 0x02                    ファイルのみに出力
 * 0x03                    標準出力、ファイルに出力
 * @return        出力先
 */
unsigned int udm_debug_getoutput()
{
    UdmErrorHandler *handler = UdmErrorHandler::getInstance();
    return handler->getOutputTarget();
}

/**
 * 物理量情報を取得する.
 * 物理量変数名称が存在していない場合は、UDM_ERRORを返す.
 * @param [in]  udm_handler           UdmModelクラスポインタ
 * @param [in]  solution_name        物理量変数名称
 * @param [out] grid_location        物理量の定義位置
 * @param [out] data_type            データ型
 * @param [out] vector_type        ベクトル型
 * @param [out] nvector_size        N成分数
 * @param [out] constant_flag        物理データ固定値フラグ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_config_getsolution(
                UdmHanler_t udm_handler,
                const char* solution_name,
                UdmGridLocation_t* grid_location,
                UdmDataType_t* data_type,
                UdmVectorType_t* vector_type,
                int* nvector_size,
                bool* constant_flag)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmDfiConfig* config = model->getDfiConfig();
    if (config == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmFlowSolutionListConfig* solutions = config->getFlowSolutionListConfig();
    if (solutions == NULL) return UDM_ERROR_NULL_VARIABLE;
    return solutions->getSolutionFieldInfo(
                            solution_name,
                            *grid_location,
                            *data_type,
                            *vector_type,
                            *nvector_size,
                            *constant_flag);
}

/**
 * 物理量情報を設定する.
 * 物理量変数名称が存在している場合は、上書きする.
 * 存在していない場合は、追加する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param solution_name        物理量変数名称
 * @param grid_location        物理量の定義位置
 * @param data_type            データ型
 * @param vector_type        ベクトル型
 * @param nvector_size        N成分数
 * @param constant_flag        物理データ固定値フラグ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_config_setsolution(
                UdmHanler_t udm_handler,
                const char* solution_name,
                UdmGridLocation_t grid_location,
                UdmDataType_t data_type,
                UdmVectorType_t vector_type,
                int nvector_size,
                bool constant_flag)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmDfiConfig* config = model->getDfiConfig();
    if (config == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmFlowSolutionListConfig* solutions = config->getFlowSolutionListConfig();
    if (solutions == NULL) return UDM_ERROR_NULL_VARIABLE;
    return solutions->setSolutionFieldInfo(
                            solution_name,
                            grid_location,
                            data_type,
                            vector_type,
                            nvector_size,
                            constant_flag);
}


/**
 * 物理量情報を設定する:基本情報.
 * 物理量変数名称が存在している場合は、上書きする.
 * 存在していない場合は、追加する.
 * ベクトル型=Udm_Scalar, 初期設定値=0.0, 固定値フラグ=falseにて設定する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param solution_name        物理量変数名称
 * @param grid_location        物理量の定義位置
 * @param data_type            データ型
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_config_setscalarsolution(
                UdmHanler_t udm_handler,
                const char *solution_name,
                UdmGridLocation_t grid_location,
                UdmDataType_t data_type)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmDfiConfig* config = model->getDfiConfig();
    if (config == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmFlowSolutionListConfig* solutions = config->getFlowSolutionListConfig();
    if (solutions == NULL) return UDM_ERROR_NULL_VARIABLE;
    return solutions->setSolutionFieldInfo(
                            solution_name,
                            grid_location,
                            data_type);
}


/**
 * 物理量変数名称の物理量情報が存在するかチェックする.
 * @param udm_handler           UdmModelクラスポインタ
 * @param solution_name        物理量変数名称
 * @return        true=物理量情報が存在する
 */
bool udm_config_existssolution(
                UdmHanler_t udm_handler,
                const char *solution_name)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return false;
    UdmDfiConfig* config = model->getDfiConfig();
    if (config == NULL) return false;
    UdmFlowSolutionListConfig* solutions = config->getFlowSolutionListConfig();
    if (solutions == NULL) return false;
    return solutions->existsSolutionConfig(solution_name);
}


/**
 * グリッド構成ノード数を取得する.
 * @param  udm_handler           UdmModelクラスポインタ
 * @param  zone_id        ゾーンID（１～）
 * @return        グリッド構成ノード数
 */
UdmSize_t udm_getnum_nodes(UdmHanler_t udm_handler, int zone_id)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return 0;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return 0;
    UdmGridCoordinates* grid = zone->getGridCoordinates();
    if (grid == NULL) return 0;
    return grid->getNumNodes();
}

/**
 * 節点（ノード）座標を取得する.
 * @param [in]  udm_handler           UdmModelクラスポインタ
 * @param [in]  zone_id        ゾーンID（１～）
 * @param node_id        節点（ノード）ID（１～）
 * @param [out] x        X座標
 * @param [out] y        Y座標
 * @param [out] z        Z座標
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_get_gridcoordinates(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t node_id,
                UdmReal_t* x,
                UdmReal_t* y,
                UdmReal_t* z)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmGridCoordinates* grid = zone->getGridCoordinates();
    if (grid == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmNode* node = grid->getNodeByLocalId(node_id);
    if (node == NULL) return UDM_ERROR_NULL_VARIABLE;
    return node->getCoords(*x, *y, *z);
}

/**
 * 節点（ノード）座標を設定する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param zone_id        ゾーンID（１～）
 * @param node_id        節点（ノード）ID（１～）
 * @param x        X座標
 * @param y        Y座標
 * @param z        Z座標
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_set_gridcoordinates(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t node_id,
                UdmReal_t x,
                UdmReal_t y,
                UdmReal_t z)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmGridCoordinates* grid = zone->getGridCoordinates();
    if (grid == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmNode* node = grid->getNodeByLocalId(node_id);
    if (node == NULL) return UDM_ERROR_NULL_VARIABLE;
    return node->setCoords(x, y, z);
}


/**
 * 節点（ノード）に物理量データ値リストを設定する:integer(ベクトル).
 * @param udm_handler           UdmModelクラスポインタ
 * @param zone_id        ゾーンID（１～）
 * @param node_id        節点（ノード）ID（１～）
 * @param solution_name        物理量データ名称
 * @param values        物理量データ値リスト:integer
 * @param size            物理量データ数 (default = 3)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_set_nodesolutions_integer(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t node_id,
                const char* solution_name,
                const UdmInteger_t *values,
                int size)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmGridCoordinates* grid = zone->getGridCoordinates();
    if (grid == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmNode* node = grid->getNodeByLocalId(node_id);
    if (node == NULL) return UDM_ERROR_NULL_VARIABLE;
    return node->setSolutionVector(solution_name, values, size);
}


/**
 * 節点（ノード）に物理量データ値を設定する:real(ベクトル).
 * @param udm_handler           UdmModelクラスポインタ
 * @param zone_id        ゾーンID（１～）
 * @param node_id        節点（ノード）ID（１～）
 * @param solution_name        物理量データ名称
 * @param values        物理量データ値リスト:real
 * @param size            物理量データ数 (default = 3)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_set_nodesolutions_real(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t node_id,
                const char* solution_name,
                const UdmReal_t *values,
                int size)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmGridCoordinates* grid = zone->getGridCoordinates();
    if (grid == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmNode* node = grid->getNodeByLocalId(node_id);
    if (node == NULL) return UDM_ERROR_NULL_VARIABLE;
    return node->setSolutionVector(solution_name, values, size);
}


/**
 * 要素（セル）に物理量データ値リストを設定する:integer(ベクトル).
 * @param udm_handler           UdmModelクラスポインタ
 * @param zone_id        ゾーンID（１～）
 * @param cell_id        要素（セル）ID（１～）
 * @param solution_name        物理量データ名称
 * @param values        物理量データ値リスト:integer
 * @param size            物理量データ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_set_cellsolutions_integer(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t cell_id,
                const char* solution_name,
                const UdmInteger_t *values,
                int size)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmSections *sections  = zone->getSections();
    if (sections == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmCell* cell = sections->getCellByLocalId(cell_id);
    if (cell == NULL) return UDM_ERROR_NULL_VARIABLE;
    return cell->setSolutionVector(solution_name, values, size);
}


/**
 * 要素（セル）に物理量データ値を設定する:real(ベクトル).
 * @param udm_handler           UdmModelクラスポインタ
 * @param zone_id        ゾーンID（１～）
 * @param cell_id        要素（セル）ID（１～）
 * @param solution_name        物理量データ名称
 * @param values        物理量データ値:real
 * @param size            物理量データ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_set_cellsolutions_real(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t cell_id,
                const char* solution_name,
                const UdmReal_t *values,
                int size)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmSections *sections  = zone->getSections();
    if (sections == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmCell* cell = sections->getCellByLocalId(cell_id);
    if (cell == NULL) return UDM_ERROR_NULL_VARIABLE;
    return cell->setSolutionVector(solution_name, values, size);
}

/**
 * セクション（要素構成）の要素（セル）数を取得する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param zone_id        ゾーンID（１～）
 * @return        要素（セル）数
 */
UdmSize_t udm_getnum_cells(UdmHanler_t udm_handler, int zone_id)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmSections *sections  = zone->getSections();
    if (sections == NULL) return UDM_ERROR_NULL_VARIABLE;
    return sections->getNumEntityCells();
}

/**
 * 節点（ノード）の物理量データ値を取得する:integer.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] zone_id        ゾーンID（１～）
 * @param [in] node_id        節点（ノード）ID（１～）
 * @param [in]  solution_name        物理量データ名称
 * @param [out] values        取得物理データ値
 * @param [out] size        取得物理データ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_get_nodesolutions_integer(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t node_id,
                const char* solution_name,
                UdmInteger_t* values,
                int* size)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmGridCoordinates* grid = zone->getGridCoordinates();
    if (grid == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmNode* node = grid->getNodeByLocalId(node_id);
    if (node == NULL) return UDM_ERROR_NULL_VARIABLE;
    if (size != NULL) {
        *size = node->getNumSolutionValue(solution_name);
    }
    if (values != NULL) {
        *size = node->getSolutionVector(solution_name, values);
    }

    return UDM_OK;
}


/**
 * 節点（ノード）の物理量データ値を取得する:real.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] zone_id        ゾーンID（１～）
 * @param [in] node_id        節点（ノード）ID（１～）
 * @param [in]  solution_name        物理量データ名称
 * @param [out] values        取得物理データ値
 * @param [out] size        取得物理データ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_get_nodesolutions_real(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t node_id,
                const char* solution_name,
                UdmReal_t* values,
                int* size)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmGridCoordinates* grid = zone->getGridCoordinates();
    if (grid == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmNode* node = grid->getNodeByLocalId(node_id);
    if (node == NULL) return UDM_ERROR_NULL_VARIABLE;
    if (size != NULL) {
        *size = node->getNumSolutionValue(solution_name);
    }
    if (values != NULL) {
        *size = node->getSolutionVector(solution_name, values);
    }

    return UDM_OK;
}

/**
 * 要素（セル）の物理量データ値を取得する:integer.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] zone_id        ゾーンID（１～）
 * @param [in] cell_id        要素（セル）ID（１～）
 * @param [in]  solution_name        物理量データ名称
 * @param [out] values        取得物理データ値
 * @param [out] size        取得物理データ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_get_cellsolutions_integer(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t cell_id,
                const char* solution_name,
                UdmInteger_t* values,
                int* size)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmSections *sections  = zone->getSections();
    if (sections == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmCell* cell = sections->getCellByLocalId(cell_id);
    if (cell == NULL) return UDM_ERROR_NULL_VARIABLE;
    if (size != NULL) {
        *size = cell->getNumSolutionValue(solution_name);
    }
    if (values != NULL) {
        *size = cell->getSolutionVector(solution_name, values);
    }

    return UDM_OK;
}

/**
 * 要素（セル）の物理量データ値を取得する:real.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] zone_id        ゾーンID（１～）
 * @param [in] cell_id        要素（セル）ID（１～）
 * @param [in]  solution_name        物理量データ名称
 * @param [out] values        取得物理データ値
 * @param [out] size        取得物理データ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_get_cellsolutions_real(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t cell_id,
                const char* solution_name,
                UdmReal_t* values,
                int* size)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmSections *sections  = zone->getSections();
    if (sections == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmCell* cell = sections->getCellByLocalId(cell_id);
    if (cell == NULL) return UDM_ERROR_NULL_VARIABLE;
    if (size != NULL) {
        *size = cell->getNumSolutionValue(solution_name);
    }
    if (values != NULL) {
        *size = cell->getSolutionVector(solution_name, values);
    }

    return UDM_OK;
}

/**
 * 要素（セル）の接続情報を取得する.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] zone_id        ゾーンID（１～）
 * @param [in] cell_id        要素（セル）ID（１～）
 * @param [out] elem_type        要素（セル）形状タイプ
 * @param [out] node_ids        接続節点（ノード）IDリスト
 * @param [out] num_nodes        接続節点（ノード）数
 * @return        接続節点（ノード）数
 */
int udm_get_cellconnectivity(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t cell_id,
                UdmElementType_t* elem_type,
                UdmSize_t* node_ids,
                int* num_nodes)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return 0;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return 0;
    UdmSections *sections  = zone->getSections();
    if (sections == NULL) return 0;
    UdmCell* cell = sections->getCellByLocalId(cell_id);
    if (cell == NULL) return 0;
    if (elem_type != NULL) {
        *elem_type = cell->getElementType();
    }
    if (node_ids != NULL) {
        int n;
        for (n=1; n<=cell->getNumNodes(); n++) {
            UdmNode *node =  cell->getNode(n);
            node_ids[n-1] = node->getLocalId();
        }
    }

    if (num_nodes != NULL) {
        *num_nodes = cell->getNumNodes();
    }
    return cell->getNumNodes();
}

/**
 * 要素（セル）に分割重みを設定する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param zone_id        ゾーンID（１～）
 * @param cell_id        要素（セル）ID（１～）
 * @param weight        分割重み
 */
void udm_set_partitionweight(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t cell_id,
                const float weight)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return;
    UdmSections *sections  = zone->getSections();
    if (sections == NULL) return;
    UdmCell* cell = sections->getCellByLocalId(cell_id);
    if (cell == NULL) return;
    cell->setPartitionWeight(weight);

    return;
}

/**
 * 分割重みを取得する.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] zone_id        ゾーンID（１～）
 * @param [in] cell_id        要素（セル）ID（１～）
 * @param [out]  weight      分割重み
 */
void udm_get_partitionweight(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t cell_id,
                float* weight)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return;
    UdmSections *sections  = zone->getSections();
    if (sections == NULL) return;
    UdmCell* cell = sections->getCellByLocalId(cell_id);
    if (cell == NULL) return;
    *weight = cell->getPartitionWeight();

    return;
}

/**
 * 分割重みをクリアする.
 * 分割重みフラグをクリアする.
 * 要素（セル）に設定している重み値をクリアする.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] zone_id        ゾーンID（１～）
 */
void udm_clear_partitionweight(
                UdmHanler_t udm_handler,
                int zone_id)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return;
    zone->clearPartitionWeight();

    return;
}

/**
 * 単位系が存在するかチェックする.
 * @param  udm_handler           UdmModelクラスポインタ
 * @param unit_name        単位系名称
 * @return        true=単位系が存在する
 */
bool udm_config_existsunit(UdmHanler_t udm_handler, const char* unit_name)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return false;
    UdmDfiConfig *config = model->getDfiConfig();
    if (config == NULL) return false;
    UdmUnitListConfig *units_config = config->getUnitListConfig();
    if (units_config == NULL) return false;
    return units_config->existsUnitConfig(unit_name);
}

/**
 * 単位系を設定する（差分値を除く）.
 * 差分値を除き、単位系を設定する.
 * 同名の単位系が存在していた場合、上書きする.
 * 同名の単位系が存在しない場合、追加する.
 * @param  udm_handler           UdmModelクラスポインタ
 * @param unit_name        単位系名称
 * @param unit            単位
 * @param reference        基準値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t udm_config_setunit(
                UdmHanler_t udm_handler,
                const char* unit_name,
                const char* unit,
                float reference)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmDfiConfig *config = model->getDfiConfig();
    if (config == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmUnitListConfig *units_config = config->getUnitListConfig();
    if (units_config == NULL) return UDM_ERROR_NULL_VARIABLE;
    return units_config->setUnitConfig(unit_name, unit, reference);
}

/**
 * 単位系を設定する（差分値を含む）.
 * 差分値を含めて、単位系を設定する.
 * 同名の単位系が存在していた場合、上書きする.
 * 同名の単位系が存在しない場合、追加する.
 * @param  udm_handler           UdmModelクラスポインタ
 * @param unit_name        単位系名称
 * @param unit            単位
 * @param reference        基準値
 * @param difference    差分値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t udm_config_setunitwithdiff(
                UdmHanler_t udm_handler,
                const char* unit_name,
                const char* unit,
                float reference,
                float difference)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmDfiConfig *config = model->getDfiConfig();
    if (config == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmUnitListConfig *units_config = config->getUnitListConfig();
    if (units_config == NULL) return UDM_ERROR_NULL_VARIABLE;
    return units_config->setUnitConfig(unit_name, unit, reference, difference);
}

/**
 * 単位系情報を取得する.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] unit_name        単位系名称
 * @param [out] unit            単位
 * @param [out] reference        基準値
 * @param [out] difference    差分値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t udm_config_getunit(
                UdmHanler_t udm_handler,
                const char* unit_name,
                char* unit,
                float* reference,
                float* difference)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmDfiConfig *config = model->getDfiConfig();
    if (config == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmUnitListConfig *units_config = config->getUnitListConfig();
    if (units_config == NULL) return UDM_ERROR_NULL_VARIABLE;
    if (!units_config->existsUnitConfig(unit_name)) return UDM_ERROR_INVALID_UNITLIST_UNITNAME;

    std::string unit_value;
    if (units_config->getUnit(unit_name, unit_value) == UDM_OK) {
        strcpy(unit, unit_value.c_str());
    }
    units_config->getReference(unit_name, *reference);
    units_config->getDifference(unit_name, *difference);

    return UDM_OK;
}

/**
 * 単位系を削除する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param unit_name        単位系名称
 */
void udm_config_removeunit(
                UdmHanler_t udm_handler,
                const char* unit_name)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return;
    UdmDfiConfig *config = model->getDfiConfig();
    if (config == NULL) return;
    UdmUnitListConfig *units_config = config->getUnitListConfig();
    if (units_config == NULL) return;
    units_config->removeUnitConfig(unit_name);

    return;
}

/**
 * CGNSファイル構成タイプを設定する.
 * 同一設定が不可なタイプが存在する場合は、置換を行う.
 * 同一設定が不可なタイプ.
 *         CGNS:GridCoordinates出力方法            : [IncludeGrid | ExcludeGrid]
 *         CGNS:FlowSolutionのステップ出力方法    : [AppendStep | EachStep]
 *         CGNS:GridCoordinatesの時系列出力方法    : [GridConstant | GridTimeSlice]
 * @param udm_handler           UdmModelクラスポインタ
 * @param type        CGNSファイル構成タイプ
 */
void udm_config_setfilecomposition(
                UdmHanler_t udm_handler,
                UdmFileCompositionType_t type)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return;
    UdmDfiConfig *config = model->getDfiConfig();
    if (config == NULL) return;
    // CGNS構成
    UdmFileInfoConfig* fileinfo = config->getFileinfoConfig();
    if (fileinfo == NULL) return;
    fileinfo->setFileCompositionType(type);

    return;
}

/**
 * CGNSファイル構成タイプをが設定されているかチェックする
 * @param udm_handler           UdmModelクラスポインタ
 * @param type        CGNSファイル構成タイプ
 * @return true=設定済み
 */
bool udm_config_existsfilecomposition(
                UdmHanler_t udm_handler,
                UdmFileCompositionType_t type)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return false;
    UdmDfiConfig *config = model->getDfiConfig();
    if (config == NULL) return false;
    // CGNS構成
    UdmFileInfoConfig* fileinfo = config->getFileinfoConfig();
    if (fileinfo == NULL) return false;
    return fileinfo->existsFileCompositionType(type);
}

/**
 * フィールドデータディレクトリを設定する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param directory_path        フィールドデータディレクトリ
 */
void udm_config_setfielddirectory(
                UdmHanler_t udm_handler,
                const char* directory)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return;
    UdmDfiConfig *config = model->getDfiConfig();
    if (config == NULL) return;
    // CGNS構成
    UdmFileInfoConfig* fileinfo = config->getFileinfoConfig();
    if (fileinfo == NULL) return;
    fileinfo->setDirectoryPath(directory);

    return;
}
/**
 * フィールドデータディレクトリを取得する.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [out] directorypath        フィールドデータディレクトリ
 * @return        取得フィールドデータディレクトリポインタ
 */
const char* udm_config_getfielddirectory(
                UdmHanler_t udm_handler,
                char* directory)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return NULL;
    UdmDfiConfig *config = model->getDfiConfig();
    if (config == NULL) return  NULL;
    // CGNS構成
    UdmFileInfoConfig* fileinfo = config->getFileinfoConfig();
    if (fileinfo == NULL) return NULL;
    std::string value;
    fileinfo->getDirectoryPath(value);
    strcpy(directory, value.c_str());
    return directory;
}
/**
 * 時刻ディレクトリ作成オプションを設定する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param timeslice_directory        時刻ディレクトリ作成オプション
 */
void udm_config_settimeslicedirectory(
                UdmHanler_t udm_handler,
                bool timeslice_directory)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return;
    UdmDfiConfig *config = model->getDfiConfig();
    if (config == NULL) return;
    // CGNS構成
    UdmFileInfoConfig* fileinfo = config->getFileinfoConfig();
    if (fileinfo == NULL) return;
    fileinfo->setTimeSliceDirectory(timeslice_directory);

    return;
}

/**
 * 時刻ディレクトリ作成オプションを取得する.
 * @param udm_handler           UdmModelクラスポインタ
 * @return        時刻ディレクトリ作成オプション
 */
bool udm_config_istimeslicedirectory(
                UdmHanler_t udm_handler)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return false;
    UdmDfiConfig *config = model->getDfiConfig();
    if (config == NULL) return  false;
    // CGNS構成
    UdmFileInfoConfig* fileinfo = config->getFileinfoConfig();
    if (fileinfo == NULL) return false;
    return fileinfo->isTimeSliceDirectory();
}

/**
 * 物理量変数名称の物理量情報を削除する.
 * @param udm_handler           UdmModelクラスポインタ
 * @param solution_name        物理量変数名称
 */
void udm_config_removesolution(
                UdmHanler_t udm_handler,
                const char* solution_name)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return;
    UdmDfiConfig* config = model->getDfiConfig();
    if (config == NULL) return;
    UdmFlowSolutionListConfig* solutions = config->getFlowSolutionListConfig();
    if (solutions == NULL) return;
    solutions->removeSolutionConfig(solution_name);

    return;
}

/**
 * 仮想セルの転送を行う.
 * @param udm_handler           UdmModelクラスポインタ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_transfer_virtualcells(UdmHanler_t udm_handler)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    return model->transferVirtualCells();
}

/**
 * 構成ノード（節点）数を取得する.
 * @param  udm_handler           UdmModelクラスポインタ
 * @param  zone_id        ゾーンID（１～）
 * @param  cell_id        要素（セル）ID（１～）
 * @return        構成ノード（節点）数
 */
int udm_getnum_cellconnectivity(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t cell_id)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return 0;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return 0;
    UdmSections *sections  = zone->getSections();
    if (sections == NULL) return 0;
    UdmCell* cell = sections->getCellByLocalId(cell_id);
    if (cell == NULL) return 0;
    return cell->getNumNodes();
}

/**
 * 節点（ノード）が接続している要素（セル）数を取得する.
 * @param  udm_handler           UdmModelクラスポインタ
 * @param  zone_id        ゾーンID（１～）
 * @param node_id        節点（ノード）ID（１～）
 * @return            接続要素（セル）数
 */
int udm_getnum_nodeconnectivity(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t node_id)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return 0;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return 0;
    UdmGridCoordinates* grid = zone->getGridCoordinates();
    if (grid == NULL) return 0;
    UdmNode *node = grid->getNodeByLocalId(node_id);
    if (node == NULL) return 0;

    return node->getNumConnectivityCells();
}


/**
 * 節点（ノード）が接続している要素（セル）を取得する.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] zone_id        ゾーンID（１～）
 * @param [in] node_id        節点（ノード）ID（１～）
 * @param [out] cell_ids        接続要素（セル）IDリスト
 * @param [out] cell_types      接続要素（セル）タイプリスト
 * @param [out] num_cells        接続要素（セル）数
 * @return        接続要素（セル）数
 */
int udm_get_nodeconnectivity(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t node_id,
                UdmSize_t* cell_ids,
                UdmRealityType_t *cell_types,
                int* num_cells)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return 0;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return 0;
    UdmGridCoordinates* grid = zone->getGridCoordinates();
    if (grid == NULL) return 0;
    UdmNode *node = grid->getNodeByLocalId(node_id);
    if (node == NULL) return 0;
    UdmSize_t size = node->getNumConnectivityCells();
    if (cell_ids != NULL) {
        UdmSize_t n;
        for (n=1; n<=size; n++) {
            UdmCell *cell = node->getConnectivityCell(n);
            cell_ids[n-1] = cell->getLocalId();
            cell_types[n-1] = cell->getRealityType();
        }
    }
    if (num_cells != NULL) {
        *num_cells = size;
    }
    return size;
}

/**
 * 要素（セル）の隣接要素（セル）数を取得する.
 * @param  udm_handler           UdmModelクラスポインタ
 * @param  zone_id        ゾーンID（１～）
 * @param  cell_id        要素（セル）ID（１～）
 * @return        隣接要素（セル）数
 */
int udm_getnum_neighborcells(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t cell_id)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return 0;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return 0;
    UdmSections *sections  = zone->getSections();
    if (sections == NULL) return 0;
    UdmCell* cell = sections->getCellByLocalId(cell_id);
    return cell->getNumNeighborCells();
}


/**
 * 要素（セル）の隣接要素（セル）を取得する.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] zone_id        ゾーンID（１～）
 * @param [in] cell_id        要素（セル）ID（１～）
 * @param [out] neighbor_cellids        隣接要素（セル）IDリスト
 * @param [out] neighbor_types        隣接要素（セル）タイプリスト
 * @param [out] num_neighbors        隣接要素（セル）数
 * @return        隣接要素（セル）数
 */
int udm_get_neighborcells(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t cell_id,
                UdmSize_t* neighbor_cellids,
                UdmRealityType_t *neighbor_types,
                int* num_neighbors)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return 0;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return 0;
    UdmSections *sections  = zone->getSections();
    if (sections == NULL) return 0;
    UdmCell* cell = sections->getCellByLocalId(cell_id);
    int size = cell->getNumNeighborCells();
    if (neighbor_cellids != NULL) {
        int n;
        for (n=1; n<=size; n++) {
            UdmCell *neighbor_cell = cell->getNeighborCell(n);
            neighbor_cellids[n-1] = neighbor_cell->getLocalId();
            neighbor_types[n-1] = neighbor_cell->getRealityType();
        }
    }
    if (num_neighbors != NULL) {
        *num_neighbors = size;
    }

    return size;
}


/**
 * 節点（ノード）の隣接節点（ノード）数を取得する.
 * @param  udm_handler           UdmModelクラスポインタ
 * @param  zone_id        ゾーンID（１～）
 * @param node_id        節点（ノード）ID（１～）
 * @return            隣接節点（ノード）数
 */
int udm_getnum_neighbornodes(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t node_id)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return 0;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return 0;
    UdmGridCoordinates* grid = zone->getGridCoordinates();
    if (grid == NULL) return 0;
    UdmNode *node = grid->getNodeByLocalId(node_id);
    if (node == NULL) return 0;

    return node->getNumNeighborNodes();
}


/**
 * 節点（ノード）の隣接節点（ノード）を取得する.
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] zone_id        ゾーンID（１～）
 * @param [in] node_id        節点（ノード）ID（１～）
 * @param [out] neighbor_nodeids        隣接節点（ノード）IDリスト
 * @param [out] neighbor_types        隣接節点（ノード）タイプリスト
 * @param [out] num_neighbors            隣接節点（ノード）数
 * @return            隣接節点（ノード）数
 */
int udm_get_neighbornodes(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t node_id,
                UdmSize_t* neighbor_nodeids,
                UdmRealityType_t *neighbor_types,
                int* num_neighbors)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return 0;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return 0;
    UdmGridCoordinates* grid = zone->getGridCoordinates();
    if (grid == NULL) return 0;
    UdmNode *node = grid->getNodeByLocalId(node_id);
    if (node == NULL) return 0;
    int size = node->getNumNeighborNodes();
    if (neighbor_nodeids != NULL) {
        int n;
        for (n=1; n<=size; n++) {
            UdmNode *neighbor_node = node->getNeighborNode(n);
            neighbor_nodeids[n-1] = neighbor_node->getLocalId();
            neighbor_types[n-1] = neighbor_node->getRealityType();
        }
    }
    if (num_neighbors != NULL) {
        *num_neighbors = size;
    }

    return size;

}

/**
 * CGNS:GridCoordinatesが固定タイプ設定であるかチェックする.
 * @param  udm_handler           UdmModelクラスポインタ
 * @return        true=固定タイプ(初回のみ出力する)
 */
bool udm_config_isfilegridconstant(UdmHanler_t udm_handler)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return false;
    UdmDfiConfig *config = model->getDfiConfig();
    if (config == NULL) return false;
    // CGNS構成
    UdmFileInfoConfig* fileinfo = config->getFileinfoConfig();
    if (fileinfo == NULL) return false;
    return fileinfo->isFileGridConstant();
}


/**
 * 節点（ノード）の物理量データ値を設定する:integer(スカラデータ).
 * @param udm_handler           UdmModelクラスポインタ
 * @param zone_id        ゾーンID（１～）
 * @param node_id        節点（ノード）ID（１～）
 * @param solution_name        物理量データ名称
 * @param value        物理データ値:integer(スカラデータ)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_set_nodesolution_integer(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t node_id,
                const char* solution_name,
                UdmInteger_t value)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmGridCoordinates* grid = zone->getGridCoordinates();
    if (grid == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmNode* node = grid->getNodeByLocalId(node_id);
    if (node == NULL) return UDM_ERROR_NULL_VARIABLE;
    return node->setSolutionScalar(solution_name, value);
}

/**
 * 節点（ノード）の物理量データ値を設定する:real(スカラデータ).
 * @param udm_handler           UdmModelクラスポインタ
 * @param zone_id        ゾーンID（１～）
 * @param node_id        節点（ノード）ID（１～）
 * @param solution_name        物理量データ名称
 * @param value        物理データ値:real(スカラデータ)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_set_nodesolution_real(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t node_id,
                const char* solution_name,
                UdmReal_t value)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmGridCoordinates* grid = zone->getGridCoordinates();
    if (grid == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmNode* node = grid->getNodeByLocalId(node_id);
    if (node == NULL) return UDM_ERROR_NULL_VARIABLE;
    return node->setSolutionScalar(solution_name, value);
}

/**
 * 節点（ノード）の物理量データ値を取得する:integer(スカラデータ).
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] zone_id        ゾーンID（１～）
 * @param [in] node_id        節点（ノード）ID（１～）
 * @param [in]  solution_name        物理量データ名称
 * @param [out] value        取得物理データ値:integer(スカラデータ)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_get_nodesolution_integer(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t node_id,
                const char* solution_name,
                UdmInteger_t* value)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmGridCoordinates* grid = zone->getGridCoordinates();
    if (grid == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmNode* node = grid->getNodeByLocalId(node_id);
    if (node == NULL) return UDM_ERROR_NULL_VARIABLE;
    return node->getSolutionScalar(solution_name, *value);
}

/**
 * 節点（ノード）の物理量データ値を取得する:real(スカラデータ).
 * @param [in] udm_handler           UdmModelクラスポインタ
 * @param [in] zone_id        ゾーンID（１～）
 * @param [in] node_id        節点（ノード）ID（１～）
 * @param [in]  solution_name        物理量データ名称
 * @param [out] value        取得物理データ値:real(スカラデータ)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_get_nodesolution_real(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t node_id,
                const char* solution_name,
                UdmReal_t* value)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmGridCoordinates* grid = zone->getGridCoordinates();
    if (grid == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmNode* node = grid->getNodeByLocalId(node_id);
    if (node == NULL) return UDM_ERROR_NULL_VARIABLE;
    return node->getSolutionScalar(solution_name, *value);
}


/**
 * 要素（セル）に物理量データ値を設定する:integer(スカラ).
 * @param udm_handler           UdmModelクラスポインタ
 * @param zone_id        ゾーンID（１～）
 * @param cell_id        要素（セル）ID（１～）
 * @param solution_name        物理量データ名称
 * @param value        物理データ値:integer(スカラ)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_set_cellsolution_integer(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t cell_id,
                const char* solution_name,
                UdmInteger_t value)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmSections *sections  = zone->getSections();
    if (sections == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmCell* cell = sections->getCellByLocalId(cell_id);
    if (cell == NULL) return UDM_ERROR_NULL_VARIABLE;
    return cell->setSolutionScalar(solution_name, value);
}


/**
 * 要素（セル）に物理量データ値を設定する:real(スカラ).
 * @param udm_handler           UdmModelクラスポインタ
 * @param zone_id        ゾーンID（１～）
 * @param cell_id        要素（セル）ID（１～）
 * @param solution_name        物理量データ名称
 * @param value        物理データ値:real(スカラ)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_set_cellsolution_real(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t cell_id,
                const char* solution_name,
                UdmReal_t value)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmSections *sections  = zone->getSections();
    if (sections == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmCell* cell = sections->getCellByLocalId(cell_id);
    if (cell == NULL) return UDM_ERROR_NULL_VARIABLE;
    return cell->setSolutionScalar(solution_name, value);
}
/**
 * 要素（セル）の物理量データ値を取得する:integer(スカラ).
 * @param [in]  udm_handler           UdmModelクラスポインタ
 * @param [in]  zone_id        ゾーンID（１～）
 * @param [in]  cell_id        要素（セル）ID（１～）
 * @param [in]  solution_name        物理量データ名称
 * @param [out] value        取得物理データ値:integer(スカラ)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_get_cellsolution_integer(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t cell_id,
                const char* solution_name,
                UdmInteger_t* value)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmSections *sections  = zone->getSections();
    if (sections == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmCell* cell = sections->getCellByLocalId(cell_id);
    if (cell == NULL) return UDM_ERROR_NULL_VARIABLE;
    return cell->getSolutionScalar(solution_name, *value);
}

/**
 * 要素（セル）の物理量データ値を取得する:real(スカラ).
 * @param [in]  udm_handler           UdmModelクラスポインタ
 * @param [in]  zone_id        ゾーンID（１～）
 * @param [in]  cell_id        要素（セル）ID（１～）
 * @param [in]  solution_name        物理量データ名称
 * @param [out] value        取得物理データ値:real(スカラ)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t udm_get_cellsolution_real(
                UdmHanler_t udm_handler,
                int zone_id,
                UdmSize_t cell_id,
                const char* solution_name,
                UdmReal_t* value)
{
    UdmModel* model = static_cast<UdmModel*> (udm_handler);
    if (model == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmZone* zone = model->getZone(zone_id);
    if (zone == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmSections *sections  = zone->getSections();
    if (sections == NULL) return UDM_ERROR_NULL_VARIABLE;
    UdmCell* cell = sections->getCellByLocalId(cell_id);
    if (cell == NULL) return UDM_ERROR_NULL_VARIABLE;
    return cell->getSolutionScalar(solution_name, *value);
}

} /* namespace udm */
