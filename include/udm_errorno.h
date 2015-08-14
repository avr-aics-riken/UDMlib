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

#ifndef _UDM_ERRORNO_H_
#define _UDM_ERRORNO_H_

/**
 * @file udm_errorno.h
 * エラー番号、エラーメッセージの定義を行う。
 */

/**
 * エラーメッセージ長さ
 */
#define UDM_ERROR_MSG_LENGTH     64

/**
 * エラーコード
 */
typedef enum {
    UDM_OK = 0,                        ///< Success
    UDM_ERROR =      1,                        ///< Failure
    UDM_ERROR_PARAMETER =      100,            ///< パラメータエラー
    UDM_ERROR_READ_DFI =      1000,            ///< DFI読込エラー
    UDM_ERROR_READ_INDEXFILE_OPENERROR =        1001,    ///< index.dfi読込エラー
    UDM_ERROR_READ_PROCFILE_OPENERROR =        1002,    ///< proc.dfi読込エラー
    UDM_ERROR_READ_UDMDFIFILE_OPENERROR =        1003,    ///< udm.dfi読込エラー
    UDM_ERROR_TEXTPARSER =            1004,    ///< TextParserエラー
    UDM_ERROR_TEXTPARSER_NULL =        1005,    ///< TextParser NULLエラー
    UDM_ERROR_TEXTPARSER_GETVALUE =        1006,    ///< TextParser getValueエラー
    UDM_ERROR_TEXTPARSER_SETVALUE =        1007,    ///< TextParser setValueエラー
    UDM_ERROR_WRITE_INDEXFILE_OPENERROR =        1008,    ///< index.dfi書込エラー
    UDM_ERROR_WRITE_PROCFILE_OPENERROR =        1009,    ///< proc.dfi書込エラー
    UDM_ERROR_WRITE_UDMDFIFILE_OPENERROR =        1010,    ///< udm.dfi書込エラー
    UDM_ERROR_WRITE_DFIFILE_WRITENODE =        1011,    ///< DFIファイルノード出力エラー
    UDM_ERROR_INVALID_ELEMENTTYPE =        1012,        ///< ElementTypeエラー
    UDM_ERROR_INVALID_PARAMETERS =        1013,    ///< 引数がNULL又は不正
    UDM_ERROR_INVALID_VARIABLE =        1014,    ///< 変数が不正値
    UDM_ERROR_NULL_VARIABLE =        1015,        ///< 変数がNULL
    UDM_ERROR_SERIALIZE =        1016,        ///< シリアライズエラー
    UDM_ERROR_DESERIALIZE =        1017,        ///< デシリアライズエラー
    UDM_ERROR_PARTITION_IMPORTS =        1016,        ///< 分割要素（セル）インポートエラー
    UDM_ERROR_PARTITION_EXPORTS =        1017,        ///< 分割要素（セル）エクスポートエラー
    UDM_ERROR_WRITE_DFI =      1018,            ///< DFI書込エラー

    // DFI-IO
    UDM_ERROR_INVALID_DFITYPE =        1100,            ///< DFITYPEエラー
    UDM_ERROR_INVALID_RANKNO =        1101,            ///< ランク番号エラー
    UDM_ERROR_INVALID_FIELDFILENAMEFORMAT =        1102,            ///< /FileInfo/FieldFilenameFormatエラー
    UDM_ERROR_INVALID_PREFIX =        1103,        ///< /FileInfo/Prefixエラー
    UDM_ERROR_INVALID_FILEFORMAT =    1104,        ///< /FileInfo/FileFormatエラー
    UDM_ERROR_INVALID_FILEPATH_PROCESS =    1105,            ///< /FilePath/Processエラー
    UDM_ERROR_INVALID_DFI_ELEMENTPATH =         1106,            ///< DFI要素パスエラー
    UDM_ERROR_INVALID_UNITLIST_UNIT =        1107,            ///< /UnitList/[unit name]/Unitのパースエラー
    UDM_ERROR_INVALID_UNITLIST_UNITNAME =        1108,            ///< /UnitList/[unit name]エラー
    UDM_ERROR_INVALID_FLOWSOLUTION_GRIDLOCATION = 1109,        ///< /FlowSolutionList/[solution name]/GridLocationエラー
    UDM_ERROR_INVALID_CELLDIMENSION = 1110,        ///< proc.dfi::CellDimensionエラー
    UDM_ERROR_INVALID_VERTEXSIZE = 1111,        ///< proc.dfi::VertexSizeエラー
    UDM_ERROR_INVALID_CELLSIZE = 1112,        ///< proc.dfi::CellSizeエラー
    UDM_ERROR_INVALID_NUMBEROFRANK = 1113,        ///< proc.dfi::/MPI/NumberOfRankエラー
    UDM_ERROR_INVALID_FILECOMPOSITIONTYPE =        1114,        ///< /FileInfo/FileCompositionTypeエラー
    UDM_ERROR_TOSTRING_DOUBLE =        1115,        ///< double値の文字列変換エラー
    UDM_ERROR_TOSTRING_INTEGER =        1116,        ///< integer値の文字列変換エラー
    UDM_ERROR_INVALID_FLOWSOLUTION_VECTOR = 1117,    ///< dfi:FLowSolution vector設定エラー
    UDM_ERROR_NOTFOUND_FLOWSOLUTION_NAME = 1118,        ///< /FlowSolutionList/[solution name]エラー

    // CGNS
    UDM_ERROR_READ_CGNSFILE_OPENERROR =        1200,        ///< CGNSファイル読込エラー
    UDM_ERROR_CGNS_OPENERROR =        1201,        ///< CGNS:openエラー
    UDM_ERROR_CGNS_INVALID_BASE =        1202,        ///< CGNS:Baseエラー
    UDM_ERROR_CGNS_INVALID_ZONE =        1203,        ///< CGNS:Zoneエラー
    UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES =        1204,        ///< CGNS:GridCoordinatesエラー
    UDM_ERROR_CGNS_INVALID_ELEMENTS =        1205,        ///< CGNS:Elementsエラー
    UDM_ERROR_CGNS_INVALID_FLOWSOLUTION =        1206,        ///< CGNS:FlowSolutionエラー
    UDM_ERROR_CGNS_CREATE_CELL =        1207,        ///< CELLの作成エラー
    UDM_ERROR_CGNS_INVALID_ITERATIVEDATA =        1208,        ///< CGNS:BaseIterativeData,ZoneIterativeDataエラー
    UDM_ERROR_CGNS_INVALID_GOTO =        1209,        ///< CGNS:cg_gotoエラー
    UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA =        1210,        ///< CGNS:UserDefinedDataエラー
    UDM_ERROR_CGNS_INVALID_NODE =        1211,        ///< NODEのエラー
    UDM_ERROR_CGNS_WRITE =        1212,        ///< CGNSファイルwriteエラー
    UDM_ERROR_CGNS_READ =        1213,        ///< CGNSファイルreadエラー

    // Zoltan
    UDM_ERROR_ZOLTAN_LB_PARTITION =        1250,        ///< Zoltan:LB_Partitionエラー
    UDM_ERROR_ZOLTAN_MIGRATE =        1251,        ///< Zoltan:Migrateエラー

    // MPI
    UDM_ERROR_INVALID_MPI =        1300,        ///< MPIエラー
    UDM_ERROR_MPI_ACK =        1301,        ///< MPI ACKエラー

    // WARNING
    UDM_WARNING_CGNS_REJECT_SOLUTIONFIELD =        2500,        ///< CGNS除外物理量エラー
    UDM_WARNING_INVALID_NODE =                     2501,        ///< 節点（ノード）IDエラー
    UDM_WARNING_INVALID_CELL =                     2502,        ///< 要素（セル）IDエラー
    UDM_WARNING_NOTFOUND_CGNSNODE =                2503,        ///< CGNS:ノードが存在しない。
    UDM_WARNING_CGNS_VALIDATE_RANKCONNECTIVITY =   2504,        ///< 内部境界検証エラー
    UDM_WARNING_ZOLTAN_NOCHANGE =                  2505,        ///< Zoltan:分割変更なし
    UDM_WARNING_CGNS_SIMULATIONTYPEUNKNOWN =       2506,        ///< CGNS:BaseのSimulationTypeが未設定
    UDM_WARNING_CGNS_NOTSUPPORT_ELEMENTTYPE =      2507,        ///< サポート対象外CGNS:ElementType
    UDM_WARNING_CGNS_EMPTYCELL =                   2508,        ///< CGNS:出力要素が存在しない
    UDM_WARNING_CGNS_NOTFOUND_FLOWSOLUTION =       2509,        ///< CGNS:対象物理量が存在しない.
    UDM_WARNING_DFI_NOTFOUND_NODE =                2510,        ///< DFI:ノードが存在しない
    UDM_WARNING_CGNS_NOTEXISTS_ITERATIVEDATA =      2511         ///< CGNS:BaseIterativeData,ZoneIterativeDataが存在しない。

} UdmError_t;


#ifdef __cplusplus
namespace udm {
#endif

/**
 * エラーメッセージ情報構造体定義
 */
struct    UdmErrorInfo
{
    int        error_no;        // エラー番号
    char    error_msg[UDM_ERROR_MSG_LENGTH];
};

/**
 * エラーメッセージ情報の宣言、初期化
 */
static struct UdmErrorInfo   error_infos[] = {
    {UDM_OK, "Success!"},
    {UDM_ERROR, "Failure!"},
    {UDM_ERROR_PARAMETER, "invalid parameter."},
    {UDM_ERROR_READ_DFI, "can not read DFI file."},
    {UDM_ERROR_READ_INDEXFILE_OPENERROR, "can not open index.dfi in read mode."},
    {UDM_ERROR_READ_PROCFILE_OPENERROR, "can not open prooc.dfi in read mode."},
    {UDM_ERROR_READ_UDMDFIFILE_OPENERROR, "can not open udm.dfi in read mode."},
    {UDM_ERROR_TEXTPARSER, "TextParser error."},
    {UDM_ERROR_TEXTPARSER_NULL, "invalid TextParser pointer."},
    {UDM_ERROR_TEXTPARSER_GETVALUE, "can not get value (TextParser::getValue)."},
    {UDM_ERROR_TEXTPARSER_SETVALUE, "can not set value (UdmConfigBase::setValue)."},
    {UDM_ERROR_WRITE_INDEXFILE_OPENERROR, "can not open index.dfi in write mode."},
    {UDM_ERROR_WRITE_PROCFILE_OPENERROR, "can not open prooc.dfi in write mode."},
    {UDM_ERROR_WRITE_UDMDFIFILE_OPENERROR, "can not open udm.dfi in write mode."},
    {UDM_ERROR_WRITE_DFIFILE_WRITENODE, "can not write to dfi node."},
    {UDM_ERROR_INVALID_ELEMENTTYPE, "invalid UdmElementType."},
    {UDM_ERROR_INVALID_PARAMETERS, "invalid parameter."},
    {UDM_ERROR_INVALID_VARIABLE, "invalid variable."},
    {UDM_ERROR_NULL_VARIABLE, "variable is null."},
    {UDM_ERROR_SERIALIZE, "serialize error."},
    {UDM_ERROR_DESERIALIZE, "deserialize error."},
    {UDM_ERROR_PARTITION_IMPORTS, "can not import partitioning cells."},
    {UDM_ERROR_PARTITION_EXPORTS, "can not export partitioning cells."},
    {UDM_ERROR_WRITE_DFI, "can not write DFI file."},

    // DFI-IO
    {UDM_ERROR_INVALID_DFITYPE, "invalid DFITYPE."},
    {UDM_ERROR_INVALID_RANKNO, "invalid rankno."},
    {UDM_ERROR_INVALID_FIELDFILENAMEFORMAT, "invalid FieldFilenameFormat."},
    {UDM_ERROR_INVALID_PREFIX, "invalid Prefix."},
    {UDM_ERROR_INVALID_FILEPATH_PROCESS, "invalid /FilePath/Process."},
    {UDM_ERROR_INVALID_DFI_ELEMENTPATH, "invalid element path for DFI."},
    {UDM_ERROR_INVALID_UNITLIST_UNIT, "invalid /UnitList/[unit name]/Unit."},
    {UDM_ERROR_INVALID_UNITLIST_UNITNAME, "invalid unit name."},
    {UDM_ERROR_INVALID_FLOWSOLUTION_GRIDLOCATION, "invalid /FlowSolutionList/[solution name]/GridLocation."},
    {UDM_ERROR_INVALID_CELLDIMENSION, "invalid proc.dfi::CellDimension."},
    {UDM_ERROR_INVALID_VERTEXSIZE, "invalid proc.dfi::VertexSize."},
    {UDM_ERROR_INVALID_CELLSIZE, "invalid proc.dfi::CellSize."},
    {UDM_ERROR_INVALID_NUMBEROFRANK, "invalid proc.dfi::/MPI/NumberOfRank."},
    {UDM_ERROR_INVALID_FILECOMPOSITIONTYPE, "invalid FileCompositionType."},
    {UDM_ERROR_TOSTRING_DOUBLE, "can not parse to string from double value."},
    {UDM_ERROR_TOSTRING_INTEGER, "can not parse to string from integer value."},
    {UDM_ERROR_INVALID_FLOWSOLUTION_VECTOR, "invalid FlowSolution:VectorType in index.dfi."},
    {UDM_ERROR_NOTFOUND_FLOWSOLUTION_NAME, "not found FlowSolution name."},

    // CGNS
    {UDM_ERROR_READ_CGNSFILE_OPENERROR, "can not open CGNS file in read mode."},
    {UDM_ERROR_CGNS_OPENERROR, "can not open CGNS file in CGNS OPEN."},
    {UDM_ERROR_CGNS_INVALID_BASE, "invalid UdmModel(CGNS:Base)."},
    {UDM_ERROR_CGNS_INVALID_ZONE, "invalid UdmZone(CGNS:Zone)."},
    {UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "invalid UdmGridCoordinates(CGNS:GridCoordinates)."},
    {UDM_ERROR_CGNS_INVALID_ELEMENTS, "invalid UdmSections(CGNS:Elements)."},
    {UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "invalid UdmFlowSolutions(CGNS:FlowSolution)."},
    {UDM_ERROR_CGNS_CREATE_CELL, "invalid create UdmCell(CGNS:Elements)."},
    {UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "invalid CGNS:BaseIterativeData|ZoneIterativeData."},
    {UDM_ERROR_CGNS_INVALID_GOTO, "invalid cg_goto."},
    {UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "CGNS:UserDefinedData"},
    {UDM_ERROR_CGNS_INVALID_NODE, "invalid node."},
    {UDM_ERROR_CGNS_WRITE, "CGNS:Write Error."},
    {UDM_ERROR_CGNS_READ, "CGNS Read Error."},

    // Zoltan
    {UDM_ERROR_ZOLTAN_LB_PARTITION, "error Zoltan::LB_Partition."},
    {UDM_ERROR_ZOLTAN_MIGRATE, "error Zoltan::Migrate."},

    // MPI
    {UDM_ERROR_INVALID_MPI, "invalid MPI."},
    {UDM_ERROR_MPI_ACK, "NACK."},

    // WARNING
    {UDM_WARNING_CGNS_REJECT_SOLUTIONFIELD, "reject solution filed in CGNS:FlowSolution."},
    {UDM_WARNING_INVALID_NODE, "invalid node."},
    {UDM_WARNING_INVALID_CELL, "invalid cell."},
    {UDM_WARNING_NOTFOUND_CGNSNODE, "not found cgns node."},
    {UDM_WARNING_CGNS_VALIDATE_RANKCONNECTIVITY, "invalid RankConnectivity."},
    {UDM_WARNING_ZOLTAN_NOCHANGE, "Zoltan No changes"},
    {UDM_WARNING_CGNS_SIMULATIONTYPEUNKNOWN, "CGNS:SimulationType is null."},
    {UDM_WARNING_CGNS_NOTSUPPORT_ELEMENTTYPE, "not support CGNS:ElementType"},
    {UDM_WARNING_CGNS_EMPTYCELL, "CGNS:Cell is empty."},
    {UDM_WARNING_CGNS_NOTFOUND_FLOWSOLUTION, "not found CGNS:FlowSolution ."},
    {UDM_WARNING_DFI_NOTFOUND_NODE, "not found DFI:NODE."},
    {UDM_WARNING_CGNS_NOTEXISTS_ITERATIVEDATA, "not exists CGNS:BaseIterativeData,ZoneIterativeData"}
};

/**
 * エラーメッセージを取得する.
 * @param error_no        エラー番号
 * @return                エラーメッセージ
 */

inline char *udm_strerror (UdmError_t error_no)
{
    int i;
    int num_info = sizeof(error_infos)/sizeof(struct UdmErrorInfo);

    for (i=0; i<num_info; i++) {
        if (error_no == error_infos[i].error_no) {
            return (char *) error_infos[i].error_msg;
        }
    }
    return (char *) "Unknown Error!";
}

#ifdef __cplusplus
} /* namespace udm */
#endif

#endif /* _UDM_ERRORNO_H_ */
