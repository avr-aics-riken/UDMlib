/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file udmlib.h
 * UDMlibインターフェイスヘッダーファイル
 */

#ifndef _UDMLIB_H_
#define _UDMLIB_H_

#include "udm_mpiutils.h"

#include "udm_errorno.h"
#include "udm_pathutils.h"
#include "udm_version.h"
#include "udm_define.h"
#include "udm_strutils.h"

/**
 * UDMlibライブラリのネームスペース
 */
#ifdef __cplusplus
namespace udm {
#endif


/**
 * ON/OFFタイプ
 */
typedef enum { Udm_OnOffUnknown = 0,
                Udm_off = 1,
                Udm_on = 2,
                Udm_OnOffDeault = 1
} UdmOnOff_t;

/**
 * ENABLEタイプ
 */
typedef enum { Udm_EnableUnknown, udm_disable, udm_enable } UdmEnable_t;

/**
 * データ型列挙型
 */
typedef enum {    Udm_DataTypeUnknown,
                Udm_Integer,
                Udm_LongInteger,
                Udm_RealSingle,
                Udm_RealDouble,
                Udm_Real,
                Udm_String,
                Udm_Boolean,
                Udm_Numeric
} UdmDataType_t;

/**
 * CGNSファイル構成タイプ
 */
typedef enum {    Udm_FileCompositionTypeUnknown,
                Udm_IncludeGrid, Udm_ExcludeGrid,
                Udm_AppendStep, Udm_EachStep,
                Udm_GridConstant, Udm_GridTimeSlice
} UdmFileCompositionType_t;

/**
 * ゾーンタイプ.
 * デフォルト=Udm_Unstructured
 */
typedef enum {
  Udm_ZoneTypeUnknown, //!< Udm_ZoneTypeUnknown
  Udm_Structured,      //!< Udm_Structured
  Udm_Unstructured     //!< Udm_Unstructured
} UdmZoneType_t;

/**
 * CGNS物理量適用位置
 */
typedef enum {    Udm_GridLocationUnknown,
                Udm_Vertex,            ///< ノード（頂点）
                Udm_CellCenter        ///< 要素（セル）センター
} UdmGridLocation_t;


/**
 * ベクトルデータタイプ
 */
typedef enum {  Udm_VectorTypeUnknown = 0,     ///< 不明
                Udm_Scalar = 1,                ///< スカラデータ型
                Udm_Vector = 3,                ///< ベクトルデータ型
                Udm_Nvector = 9                ///< N成分データ型
} UdmVectorType_t;


/**
 * 要素形状タイプ
 */
typedef enum {
    Udm_ElementTypeUnknown = 0,    //!< 未定
    Udm_NODE = 1,                  //!< ノード（節点）
    Udm_BAR_2 = 2,                 //!< BAR要素
    Udm_TRI_3 = 3,                 //!< Shell:三角形要素
    Udm_QUAD_4 = 4,                //!< Shell:四角形要素
    Udm_TETRA_4 = 5,               //!< Solid:四面体要素
    Udm_PYRA_5 = 6,                //!< Solid:ピラミッド要素
    Udm_PENTA_6 = 7,               //!< Solid:五面体要素
    Udm_HEXA_8 = 8,                //!< Solid:六面体要素
    Udm_MIXED = 9,                 //!< 混合要素
} UdmElementType_t;

/**
 * ノード、要素の仮想ノードタイプ
 */
typedef enum {
    Udm_RealityTypeUnknown,     //!< 未定
    Udm_Virtual,                //!< 仮想ノード、要素
    Udm_Actual                  //!< 実体ノード、要素
} UdmRealityType_t;

/**
 * 物理量データタイプ
 */
typedef enum {
    Udm_SolutionFieldTypeUnknown,     //!< 未定
    Udm_FlowSolution,                //!< 物理量データ
    Udm_Attribute                  //!< 属性データ
} UdmSolutionFieldType_t;


/**
 * 時系列シミュレーションタイプ
 */
typedef enum {
    Udm_SimulationTypeUnknown,     ///< 未定
    Udm_TimeAccurate,         ///< タイムステップ
    Udm_NonTimeAccurate        ///< ワンステップ
} UdmSimulationType_t;


/**
 * メモリ配列タイプ
 */
typedef enum {
    Udm_MemArrayTypeUnknown,     ///< 未定
    Udm_MemSequentialArray,         ///< スカラメモリ配列（成分配列) : { {X0,X1,X2,X3},{Y0,Y1,Y2,Y3},{Z0,Z1,Z2,Z3}}
    Udm_MemIndexesArray        ///< ベクトルメモリ配列（ノード配列) : { {X0,Y0,Z0},{X1,Y1,Z1},{X2,Y2,Z2},{X3,Y3,Z3}}
} UdmMemArrayType_t;


/**
 * 要素クラスタイプ
 */
typedef enum {
    Udm_CellClassUnknown,     ///< 未定
    Udm_CellClass,         ///< 要素（セル）クラス
    Udm_ComponentClass        ///< 部品要素（セル）クラス
} UdmCellClass_t;

/**
 * UDMlibライブラリのバージョンを取得する
 * @param [out] version        UDMlibライブラリのバージョン
 * @return        versionポインタ
 */
inline char* udm_get_version(char *version)
{
    if (version == NULL) return NULL;
    strcpy(version, UDM_VERSION_NO);
    return version;
}


/**
 * UDMlibライブラリのリビジョンを取得する
 * @param [out] revision        UDMlibライブラリのリビジョン
 * @return        revisionポインタ
 */
inline char* udm_get_revision(char *revision)
{
    if (revision == NULL) return NULL;
    strcpy(revision, UDM_REVISION);
    return revision;
}

/**
 * UDMlibライブラリのバージョン情報を出力する.
 */
inline void udm_print_version()
{
    printf("/***********************************************************/ \n");
    printf("  UDMlib : Unstructured Data Management Library : Version=%s, Revision=%s \n",
            UDM_VERSION_NO, UDM_REVISION);
    printf("  Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo. \n");
    printf("  All rights reserved. \n");
    printf("/***********************************************************/ \n");

    return;
}


// C用　API 関数
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * UDMlibハンドラ型.
 * UdmModelクラスオブジェクトのポインタ
 */
typedef void* UdmHanler_t;

// UdmModel生成・破棄
UdmHanler_t udm_create_model();
void udm_delete_model(UdmHanler_t udm_handler);
UdmError_t udm_rebuild_model(UdmHanler_t udm_handler);
UdmError_t udm_transfer_virtualcells(UdmHanler_t udm_handler);

// CGNS
UdmError_t udm_load_model(UdmHanler_t udm_handler, const char* dfi_filename, int timeslice_step);
UdmError_t udm_read_cgns(UdmHanler_t udm_handler, const char* cgns_filename, int timeslice_step);
UdmError_t udm_write_model(UdmHanler_t udm_handler, int timeslice_step, float timeslice_time);
UdmError_t udm_write_model_average(UdmHanler_t udm_handler, int timeslice_step, float timeslice_time, int average_step, float average_time);
int udm_create_zone(UdmHanler_t udm_handler);
int udm_getnum_zones(UdmHanler_t udm_handler);
int udm_create_section(UdmHanler_t udm_handler, int zone_id, UdmElementType_t element_type);
int udm_getnum_sections(UdmHanler_t udm_handler, int zone_id);
UdmError_t udm_insert_rankconnectivity(
                        UdmHanler_t udm_handler,
                        int zone_id,
                        UdmSize_t node_id,
                        int rankno,
                        UdmSize_t localid);
UdmSize_t udm_insert_cellconnectivity(
                        UdmHanler_t udm_handler,
                        int zone_id,
                        UdmElementType_t elem_type,
                        UdmSize_t *node_ids);
UdmSize_t udm_getnum_nodes(UdmHanler_t udm_handler, int zone_id);
UdmSize_t udm_insert_gridcoordinates(UdmHanler_t udm_handler, int zone_id, UdmReal_t x, UdmReal_t y, UdmReal_t z);
UdmError_t udm_get_gridcoordinates(UdmHanler_t udm_handler, int zone_id, UdmSize_t node_id, UdmReal_t *x, UdmReal_t *y, UdmReal_t *z);
UdmError_t udm_set_gridcoordinates(UdmHanler_t udm_handler, int zone_id, UdmSize_t node_id, UdmReal_t x, UdmReal_t y, UdmReal_t z);
UdmSize_t udm_getnum_cells(UdmHanler_t udm_handler, int zone_id);
UdmError_t udm_set_nodesolutions_integer(UdmHanler_t udm_handler, int zone_id, UdmSize_t node_id, const char *solution_name, const UdmInteger_t *values, int size);
UdmError_t udm_set_nodesolutions_real(   UdmHanler_t udm_handler, int zone_id, UdmSize_t node_id, const char *solution_name, const UdmReal_t *values, int size);
UdmError_t udm_get_nodesolutions_integer(UdmHanler_t udm_handler, int zone_id, UdmSize_t node_id, const char *solution_name, UdmInteger_t *values, int *size);
UdmError_t udm_get_nodesolutions_real(   UdmHanler_t udm_handler, int zone_id, UdmSize_t node_id, const char *solution_name, UdmReal_t *values, int *size);
UdmError_t udm_set_nodesolution_integer(UdmHanler_t udm_handler, int zone_id, UdmSize_t node_id, const char *solution_name, UdmInteger_t value);
UdmError_t udm_set_nodesolution_real(   UdmHanler_t udm_handler, int zone_id, UdmSize_t node_id, const char *solution_name, UdmReal_t value);
UdmError_t udm_get_nodesolution_integer(UdmHanler_t udm_handler, int zone_id, UdmSize_t node_id, const char *solution_name, UdmInteger_t *value);
UdmError_t udm_get_nodesolution_real(   UdmHanler_t udm_handler, int zone_id, UdmSize_t node_id, const char *solution_name, UdmReal_t *value);
UdmError_t udm_set_cellsolutions_integer(UdmHanler_t udm_handler, int zone_id, UdmSize_t cell_id, const char *solution_name, const UdmInteger_t *values, int size);
UdmError_t udm_set_cellsolutions_real(   UdmHanler_t udm_handler, int zone_id, UdmSize_t cell_id, const char *solution_name, const UdmReal_t *values, int size);
UdmError_t udm_get_cellsolutions_integer(UdmHanler_t udm_handler, int zone_id, UdmSize_t cell_id, const char *solution_name, UdmInteger_t *values, int *size);
UdmError_t udm_get_cellsolutions_real(   UdmHanler_t udm_handler, int zone_id, UdmSize_t cell_id, const char *solution_name, UdmReal_t *values, int *size);
UdmError_t udm_set_cellsolution_integer(UdmHanler_t udm_handler, int zone_id, UdmSize_t cell_id, const char *solution_name, UdmInteger_t value);
UdmError_t udm_set_cellsolution_real(   UdmHanler_t udm_handler, int zone_id, UdmSize_t cell_id, const char *solution_name, UdmReal_t value);
UdmError_t udm_get_cellsolution_integer(UdmHanler_t udm_handler, int zone_id, UdmSize_t cell_id, const char *solution_name, UdmInteger_t *value);
UdmError_t udm_get_cellsolution_real(   UdmHanler_t udm_handler, int zone_id, UdmSize_t cell_id, const char *solution_name, UdmReal_t *value);

int udm_getnum_cellconnectivity(
                        UdmHanler_t udm_handler,
                        int zone_id,
                        UdmSize_t cell_id);
int udm_get_cellconnectivity(
                        UdmHanler_t udm_handler,
                        int zone_id,
                        UdmSize_t cell_id,
                        UdmElementType_t *elem_type,
                        UdmSize_t *node_ids,
                        int *num_nodes);
void udm_set_partitionweight(   UdmHanler_t udm_handler, int zone_id, UdmSize_t cell_id, const float weight);
void udm_get_partitionweight(   UdmHanler_t udm_handler, int zone_id, UdmSize_t cell_id, float *weight);
void udm_clear_partitionweight(UdmHanler_t udm_handler, int zone_id);
int udm_getnum_nodeconnectivity(
                        UdmHanler_t udm_handler,
                        int zone_id,
                        UdmSize_t node_id);
int udm_get_nodeconnectivity(
                        UdmHanler_t udm_handler,
                        int zone_id,
                        UdmSize_t node_id,
                        UdmSize_t *cell_ids,
                        UdmRealityType_t *cell_types,
                        int *num_cells);
int udm_getnum_neighborcells(
                        UdmHanler_t udm_handler,
                        int zone_id,
                        UdmSize_t cell_id);
int udm_get_neighborcells(
                        UdmHanler_t udm_handler,
                        int zone_id,
                        UdmSize_t cell_id,
                        UdmSize_t *neighbor_cellids,
                        UdmRealityType_t *neighbor_types,
                        int *num_neighbors);
int udm_getnum_neighbornodes(
                        UdmHanler_t udm_handler,
                        int zone_id,
                        UdmSize_t node_id);
int udm_get_neighbornodes(
                        UdmHanler_t udm_handler,
                        int zone_id,
                        UdmSize_t node_id,
                        UdmSize_t *neighbor_nodeids,
                        UdmRealityType_t *neighbor_types,
                        int *num_neighbors);

// UserDefinedData
UdmError_t udm_user_getinfo(
                UdmHanler_t udm_handler,
                int zone_id,
                const char *user_name,
                UdmDataType_t *data_type,
                int *dimension,
                UdmSize_t *dim_sizes );
UdmError_t udm_user_getdata(
                UdmHanler_t udm_handler,
                int zone_id,
                const char *user_name,
                UdmDataType_t data_type,
                void *data);
UdmError_t udm_user_setdata(
                UdmHanler_t udm_handler,
                int zone_id,
                const char *user_name,
                UdmDataType_t data_type,
                int dimension,
                UdmSize_t *dim_sizes,
                void *data);
UdmError_t udm_user_remove(
                UdmHanler_t udm_handler,
                int zone_id,
                const char *user_name);

// DFI-Config
void udm_config_setfileprefix(UdmHanler_t udm_handler, const char *prefix);
void udm_config_setoutputpath(UdmHanler_t udm_handler, const char *path);
const char* udm_config_getfileprefix(UdmHanler_t udm_handler, char *prefix);
const char* udm_config_getoutputpath(UdmHanler_t udm_handler, char *path);
const char* udm_config_getcgnsinputfile(UdmHanler_t udm_handler, char *file_path, int rank_no);
const char* udm_config_getcgnslinkfile(UdmHanler_t udm_handler, char *file_path, int rank_no);
const char* udm_config_getoutputdirectory(UdmHanler_t udm_handler, char *path);
UdmError_t udm_config_getsolution(
                UdmHanler_t udm_handler,
                const char     *solution_name,
                UdmGridLocation_t    *grid_location,
                UdmDataType_t        *data_type,
                UdmVectorType_t      *vector_type,
                int                   *nvector_size,
                bool                  *constant_flag);
UdmError_t udm_config_setsolution(
                UdmHanler_t udm_handler,
                const char     *solution_name,
                UdmGridLocation_t    grid_location,
                UdmDataType_t        data_type,
                UdmVectorType_t        vector_type,
                int                    nvector_size,
                bool                   constant_flag);
UdmError_t udm_config_setscalarsolution(
                UdmHanler_t udm_handler,
                const char     *solution_name,
                UdmGridLocation_t    grid_location,
                UdmDataType_t        data_type);
bool udm_config_existssolution(
                UdmHanler_t udm_handler,
                const char *solution_name);
void udm_config_removesolution(
                UdmHanler_t udm_handler,
                const char *solution_name);
bool udm_config_existsunit(
                UdmHanler_t udm_handler,
                const char *unit_name);
UdmError_t udm_config_setunit(
                UdmHanler_t udm_handler,
                const char *unit_name,
                const char *unit,
                float reference);
UdmError_t udm_config_setunitwithdiff(
                UdmHanler_t udm_handler,
                const char *unit_name,
                const char *unit,
                float reference,
                float difference);
UdmError_t udm_config_getunit(
                UdmHanler_t udm_handler,
                const char *unit_name,
                char *unit,
                float *reference,
                float *difference);
void udm_config_removeunit(
                UdmHanler_t udm_handler,
                const char *unit_name);
void udm_config_setfilecomposition(
                UdmHanler_t udm_handler,
                UdmFileCompositionType_t type);
bool udm_config_existsfilecomposition(
                UdmHanler_t udm_handler,
                UdmFileCompositionType_t type);
bool udm_config_isfilegridconstant(UdmHanler_t udm_handler);
void udm_config_setfielddirectory(
                UdmHanler_t udm_handler,
                const char *directory);
const char* udm_config_getfielddirectory(
                UdmHanler_t udm_handler,
                char *directory);
void udm_config_settimeslicedirectory(
                UdmHanler_t udm_handler,
                bool timeslice_directory);
bool udm_config_istimeslicedirectory(
                UdmHanler_t udm_handler);

// partition
UdmError_t udm_partition_zone(UdmHanler_t udm_handler, int zone_id);
UdmError_t udm_partition_sethypergraph(UdmHanler_t udm_handler, const char *approach);
UdmError_t udm_partition_setgraph(UdmHanler_t udm_handler, const char *approach);
const char*  udm_partition_getparameter(UdmHanler_t udm_handler, const char *name, char *value);
UdmError_t udm_partition_setparameter(UdmHanler_t udm_handler, const char *name, const char *value);
UdmError_t udm_partition_removeparameter(UdmHanler_t udm_handler, const char *name);
UdmError_t udm_partition_setdebuglevel(UdmHanler_t udm_handler, int debug_level);

// STOPWATCH
void udm_stopwatch_start(const char *label);
void udm_stopwatch_stop(const char *label);
void udm_stopwatch_print();
void udm_stopwatch_info(const char *label, const char *info);

// for debug
void udm_debug_setlevel(unsigned int level);
unsigned int udm_debug_getlevel();
void udm_debug_setoutput(unsigned int output);
unsigned int udm_debug_getoutput();
UdmError_t udm_write_rankconnectivity(UdmHanler_t udm_handler, const char* cgns_filename);
UdmError_t udm_write_virtualcells(UdmHanler_t udm_handler, const char* cgns_filename);
bool udm_validate_cgns(UdmHanler_t udm_handler);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef __cplusplus
} /* namespace udm */
#endif

#endif /* _UDMLIB_H_ */
