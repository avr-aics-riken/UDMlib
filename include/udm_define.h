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

#ifndef _UDM_DEFINE_H_
#define _UDM_DEFINE_H_


/**
 * @file udm_define.h
 * UDM、CGNS文字列定義
 */

/**
 * サイズ型のデータ型
 */
#ifdef UDM_SIZE64
typedef  size_t        UdmSize_t;
#else
typedef  unsigned int        UdmSize_t;
#endif

#ifdef UDM_REAL8
typedef  double        UdmReal_t;
#else
typedef  float        UdmReal_t;
#endif

#ifdef UDM_INT8
typedef  long long        UdmInteger_t;
#else
typedef  int        UdmInteger_t;
#endif

// CGNS
#define      UDM_CGNS_NAME_BASE                    "UdmBase"
#define      UDM_CGNS_NAME_ZONE                    "UdmZone#%d"
#define      UDM_CGNS_NAME_FLOWSOLUTION        "udmflow"
#define      UDM_CGNS_NAME_ATTRIBUTE           "udmattr"
#define      UDM_CGNS_NAME_FIELD_VERTEX        "vertex"
#define      UDM_CGNS_NAME_FIELD_CELLCENTER    "cellcn"
#define      UDM_CGNS_NAME_GRIDCOORDINATES    "GridCoordinates"
#define      UDM_CGNS_NAME_FLOWSOLUTION_VERTEX    "UdmSol_Node"
#define      UDM_CGNS_NAME_FLOWSOLUTION_CELLCENTER    "UdmSol_Cell"
#define      UDM_CGNS_NAME_FLOWSOLUTION_VERTEX_CONST        "UdmSol_Node_Const"
#define      UDM_CGNS_NAME_FLOWSOLUTION_CELLCENTER_CONST    "UdmSol_Cell_Const"
#define      UDM_CGNS_NAME_UDMINFO                  "UdmInfo"
#define      UDM_CGNS_NAME_UDMINFO_VERSION              "UDMLibaryVersion"
#define      UDM_CGNS_NAME_UDMINFO_DESCRIPTION          "Description"
#define      UDM_CGNS_NAME_UDMINFO_RANKINFO          "OutputRankInfo"
#define      UDM_CGNS_NAME_RANKCONNECTIVITY          "UdmRankConnectivity"
#define      UDM_CGNS_NAME_RANKCONNECTIVITY_ARRAY          "RankConnectivity"
#define      UDM_CGNS_NAME_LENGTH                    33
#define      UDM_CGNS_TEXT_UDMINFO_DESCRIPTION          "/*\n\
 * UDMlib - Unstructured Data Management Library\n\
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.\n\
 * All rights reserved.\n\
 */"
#define      UDM_CGNS_NAME_USERDEFINEDDATAS          "UdmUserDefinedData"

// CGNS書式出力要素
#define UDM_CGNS_FORMAT_GRIDCOORDINATES    "GridCoordinates_%010d"
#define UDM_CGNS_FORMAT_ELEMENTS    "UdmElements_%s"
#define UDM_CGNS_FORMAT_FLOWSOLUTION_VERTEX    "UdmSol_Node_%010d"
#define UDM_CGNS_FORMAT_FLOWSOLUTION_CELLCENTER    "UdmSol_Cell_%010d"

// DFI
#define UDM_DFI_FILENAME_INDEX         "index.dfi"        ///< DFIファイル名:index.dfi
#define UDM_DFI_FILENAME_PROC         "proc.dfi"        ///< DFIファイル名:proc.dfi
#define UDM_DFI_FILENAME_UDMLIB         "udmlib.tp"        ///< DFIファイル名:udmlib.tp
#define UDM_DFI_OUTPUT_PATH         "output"        ///< デフォルト出力パス

// index.dfi
#define UDM_CGNS_EXT         "cgns"        ///< CGNSファイル拡張子
#define UDM_DFI_DFITYPE_UNS     "Uns"        ///< DFI種別
#define UDM_DFI_FILEINFO    "FileInfo"        ///< DFIラベル : ファイル情報
#define UDM_DFI_DFITYPE        "DFIType"        ///< DFIラベル : DFI種別
#define UDM_DFI_DIRECTORYPATH        "DirectoryPath"        ///< DFIラベル : フィールドデータディレクトリ
#define UDM_DFI_TIMESLICEDIRECTORY        "TimeSliceDirectory"        ///< DFIラベル : 時刻ディレクトリ作成オプション
#define UDM_DFI_PROCESSDIRECTORY        "ProcessDirectory"        ///< DFIラベル : プロセスディレクトリ作成オプション
#define UDM_DFI_PREFIX        "Prefix"            ///< DFIラベル : ベースファイル名
#define UDM_DFI_FILEFORMAT        "FileFormat"    ///< DFIラベル : ファイルフォーマット
#define UDM_DFI_FIELDFILENAMEFORMAT        "FieldFilenameFormat"        ///< DFIラベル : ファイル命名書式
#define UDM_DFI_FIELDFILENAME        "FieldFilename"        ///< DFIラベル : ファイル名
#define UDM_DFI_DATATYPE        "DataType"        ///< DFIラベル : データ型
#define UDM_DFI_ELEMENTPATH        "ElementPath"    ///< DFIラベル : CGNS要素パス
#define UDM_DFI_FILECOMPOSITIONTYPE        "FileCompositionType"        ///< DFIラベル : ファイル構成タイプ
#define UDM_DFI_FILEPATH    "FilePath"        ///< ファイルパス情報
#define UDM_DFI_PROCESS        "Process"        ///< proc.dfiファイルラベル
#define UDM_DFI_UNITLIST    "UnitList"        ///< 単位系情報
#define UDM_DFI_UNIT        "Unit"            ///< 単位（文字列）
#define UDM_DFI_REFERENCE    "Reference"        ///< 基準値（実数）
#define UDM_DFI_DIFFERENCE    "Difference"    ///< 差分値（実数）
#define UDM_DFI_TIMESLICE    "TimeSlice"        ///< 時系列情報
#define UDM_DFI_SLICE        "Slice"            ///< ステップ数分データ
#define UDM_DFI_STEP         "Step"            ///< 出力ステップ
#define UDM_DFI_TIME         "Time"            ///< 出力時刻
#define UDM_DFI_AVERAGESTEP     "AverageStep"    ///< 平均化ステップ数
#define UDM_DFI_AVERAGETIME     "AverageTime"    ///< 平均時間
#define UDM_DFI_FLOWSOLUTIONLIST    "FlowSolutionList"        ///< 物理変数情報
#define UDM_DFI_GRIDLOCATION        "GridLocation"        ///< 物理量適用位置
#define UDM_DFI_VECTORTYPE        "VectorType"        ///< ベクトルデータタイプ
#define UDM_DFI_NVECTORSIZE        "NvectorSize"        ///< N成分データ数
#define UDM_DFI_CONSTANT    "Constant"        ///< 固定値フラグ
// proc.dfi
#define UDM_DFI_DOMAIN            "Domain"            ///< ドメイン情報
#define UDM_DFI_CELLDIMENSION    "CellDimension"        /// 非構造格子モデルの次元数（=1D, 2D, 3D）
#define UDM_DFI_VERTEXSIZE        "VertexSize"        /// ノード数
#define UDM_DFI_CELLSIZE        "CellSize"            /// セル（要素）数
#define UDM_DFI_MPI                "MPI"                /// 並列実行情報
#define UDM_DFI_NUMBEROFRANK    "NumberOfRank"        /// 並列実行数
#define UDM_DFI_NUMBEROFGROUP    "NumberOfGroup"        /// 並列実行グループ数
#define UDM_DFI_PROCESS            "Process"            ///< プロセス情報
#define UDM_DFI_RANK            "Rank"                ///< プロセス別情報
#define UDM_DFI_ID                "ID"                ///< ランク番号
// udmlib.tp
#define UDM_DFI_UDMLIB            "UDMlib"            ///< UDMlib設定項目
#define UDM_DFI_DEBUG_LEVEL        "DEBUG_LEVEL"        ///< Zoltan:デバッグレベル
#define UDM_DFI_PARTITION        "partition"            ///< 分割情報
#define UDM_DFI_MxM_PARTITION    "MxM_PARTITION"        ///< 分割数が同じ場合の分割実行
// DFI:write
#define UDM_DFI_INDENT         "    "        ///< DFIファイル出力インデント
// index.dfi/FileInfo/フォーマット
#define UDM_DFI_FILEFORMAT_RANK_STEP       "%s_id%06d_%010d.%s"        ///< CGNS:rank_stepファイルフォーマット
#define UDM_DFI_FILEFORMAT_STEP_RANK        "%s_%010d_id%06d.%s"        ///< CGNS:step_rankファイルフォーマット
#define UDM_DFI_FILEFORMAT_RANK        "%s_id%06d.%s"        ///< CGNS:rankファイルフォーマット
#define UDM_DFI_FILEFORMAT_STEP        "%s_%010d.%s"        ///< CGNS:stepファイルフォーマット
#define UDM_DFI_LINKFORMAT_RANK_STEP       "%s_link_id%06d_%010d.%s"        ///< CGNS:GrdiCoordinates:rank_stepファイルフォーマット
#define UDM_DFI_LINKFORMAT_STEP_RANK        "%s_link_%010d_id%06d.%s"        ///< CGNS:GrdiCoordinates:step_rankファイルフォーマット
#define UDM_DFI_LINKFORMAT_RANK        "%s_link_id%06d.%s"        ///< CGNS:GrdiCoordinates:rankファイルフォーマット
#define UDM_DFI_LINKFORMAT_STEP        "%s_link_%010d.%s"        ///< CGNS:GrdiCoordinates:stepファイルフォーマット
#define UDM_DFI_GRIDFORMAT_RANK_STEP       "%s_grid_id%06d_%010d.%s"        ///< CGNS:GrdiCoordinates:rank_stepファイルフォーマット
#define UDM_DFI_GRIDFORMAT_STEP_RANK        "%s_grid_%010d_id%06d.%s"        ///< CGNS:GrdiCoordinates:step_rankファイルフォーマット
#define UDM_DFI_GRIDFORMAT_RANK        "%s_grid_id%06d.%s"        ///< CGNS:GrdiCoordinates:rankファイルフォーマット
#define UDM_DFI_GRIDFORMAT_STEP        "%s_grid_%010d.%s"        ///< CGNS:GrdiCoordinates:stepファイルフォーマット
#define UDM_DFI_SOLUTIONFORMAT_RANK_STEP       "%s_solution_id%06d_%010d.%s"        ///< CGNS:FLowSolution:rank_stepファイルフォーマット
#define UDM_DFI_SOLUTIONFORMAT_STEP_RANK        "%s_solution_%010d_id%06d.%s"        ///< CGNS:FLowSolution:step_rankファイルフォーマット
#define UDM_DFI_SOLUTIONFORMAT_RANK        "%s_solution_id%06d.%s"        ///< CGNS:FLowSolution:rankファイルフォーマット
#define UDM_DFI_SOLUTIONFORMAT_STEP        "%s_solution_%010d.%s"        ///< CGNS:FLowSolution:stepファイルフォーマット
#define UDM_DFI_FILEFORMAT_NONE        "%s.%s"        ///< CGNS:ファイルフォーマット
#define UDM_DFI_FORMAT_TIMESLICE        "%010d"        ///< 時系列ステップ番号フォーマット
#define UDM_DFI_FORMAT_PROCESS        "id%06d"        ///< プロセス番号フォーマット

// Zoltan:Migrate
#define UDM_MIGRATION_MAXSIZE         0x40000000    ///< Zoltan:Migrate転送最大サイズ : INT_MAX以下とすること


#endif /* _UDM_DEFINE_H_ */
