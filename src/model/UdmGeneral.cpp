/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmGeneral.cpp
 * モデル構成の基底クラスのソースファイル
 */

#include "model/UdmGeneral.h"
#include "model/UdmNode.h"
#include "model/UdmCell.h"
#include "model/UdmComponent.h"

namespace udm {

/**
 * コンストラクタ
 */
UdmGeneral::UdmGeneral()
{
    this->name.empty();
    this->id = 0;
    this->datatype = Udm_DataTypeUnknown;
}

/**
 * コンストラクタ：データ型
 * @param dataType        データ型
 */
UdmGeneral::UdmGeneral(UdmDataType_t datatype)
{
    this->name.empty();
    this->id = 0;
    this->datatype = datatype;
}

/**
 * コンストラクタ: 名前, ID
 * @param name        名前
 * @param id        ID
 */
UdmGeneral::UdmGeneral(const std::string &name, UdmSize_t id)
{
    this->name = name;
    this->id = id;
    this->datatype = Udm_DataTypeUnknown;
}

/**
 * コンストラクタ: 名前, ID
 * @param name        名前
 * @param id        ID
 * @param datatype      データ型
 */
UdmGeneral::UdmGeneral(const std::string &name, UdmSize_t id, UdmDataType_t datatype)
{
    this->name = name;
    this->id = id;
    this->datatype = datatype;
}


/**
 * デストラクタ
 */
UdmGeneral::~UdmGeneral()
{
}

/**
 * CGNSノードのデータ型を取得する.
 * @return        データ型
 */
UdmDataType_t UdmGeneral::getDataType() const
{
    return datatype;
}

/**
 * CGNSノードのデータ型を設定する.
 * @param dataType        データ型
 */
void UdmGeneral::setDataType(UdmDataType_t datatype)
{
    this->datatype = datatype;
}

/**
 * CGNSノードのIDを取得する.
 * @return        CGNSノードID
 */
UdmSize_t UdmGeneral::getId() const
{
    return this->id;
}

/**
 * CGNSノードのIDを設定します.
 * @param id    CGNSノードID
 */
void UdmGeneral::setId(UdmSize_t id)
{
    this->id = id;
}

/**
 * CGNSノードのラベルを取得する.
 * @return        CGNSノードのラベル
 */
const std::string& UdmGeneral::getLabel() const
{
    return label;
}

/**
 * CGNSノードのラベルを設定する.
 * @param label        CGNSノードのラベル
 */
void UdmGeneral::setLabel(const std::string& label)
{
    this->label = label;
}

/**
 * CGNSノードの名前を取得する.
 * @return        CGNSノードの名前
 */
const std::string& UdmGeneral::getName() const
{
    return this->name;
}


/**
 * CGNSノードの名前を設定する.
 * @param name        CGNSノードの名前
 */
void UdmGeneral::setName(const std::string& name)
{
    this->name = name;
}

/**
 * CGNSノードの名前が同じであるかチェックする.
 * 大文字小文字の区別は行わない。
 * @param  dest_name    比較対象名前
 * @return  true=名前は同じ
 */
bool UdmGeneral::equalsName(const std::string &dest_name)
{
    return (strcasecmp(this->name.c_str(), dest_name.c_str()) == 0);
}

/**
 * CGNSデータ型からUDMlib:データ型に変換する.
 * @param cgns_datatype        CGNS:データ型
 * @return        UDMlib:データ型
 */
UdmDataType_t UdmGeneral::toUdmDataType(DataType_t cgns_datatype)
{
    if (cgns_datatype == Integer) return Udm_Integer;
    else if (cgns_datatype == LongInteger) return Udm_LongInteger;
    else if (cgns_datatype == RealSingle) return Udm_RealSingle;
    else if (cgns_datatype == RealDouble) return Udm_RealDouble;

    return Udm_DataTypeUnknown;
}

/**
 * UDMlib:データ型からCGNS:データ型に変換する.
 * @param cgns_datatype        UDMlib:データ型
 * @return        CGNS:データ型
 */
DataType_t UdmGeneral::toCgnsDataType(UdmDataType_t datatype)
{
    if (datatype == Udm_Integer) return Integer;
    else if (datatype == Udm_LongInteger) return LongInteger;
    else if (datatype == Udm_RealSingle) return RealSingle;
    else if (datatype == Udm_RealDouble) return RealDouble;

    return DataTypeNull;
}

/**
 * CGNS定義位置タイプからUDMlib:定義位置タイプに変換する.
 * @param cgns_datatype        CGNS:定義位置
 * @return        UDMlib:定義位置タイプ
 */
UdmGridLocation_t UdmGeneral::toGridLocation(GridLocation_t cgns_location)
{
    if (cgns_location == Vertex) return Udm_Vertex;
    else if (cgns_location == CellCenter) return Udm_CellCenter;

    return Udm_GridLocationUnknown;
}

/**
 * UDMlib:定義位置タイプからCGNS:定義位置タイプに変換する.
 * @param grid_location        UDMlib:定義位置
 * @return        CGNS:定義位置タイプ
 */
GridLocation_t UdmGeneral::toCgnsGridLocation(UdmGridLocation_t grid_location)
{
    if (grid_location == Udm_Vertex) return Vertex;
    else if (grid_location == Udm_CellCenter) return CellCenter;

    return GridLocationNull;
}

/**
 * CGNS要素形状タイプからUDMlib:要素形状タイプに変換する.
 * @param cgns_elementtype        CGNS:要素形状タイプ
 * @return        UDMlib:要素形状タイプ
 */
UdmElementType_t UdmGeneral::toElementType(ElementType_t cgns_elementtype)
{
    if (cgns_elementtype == NODE) return Udm_NODE;
    else if (cgns_elementtype == BAR_2) return Udm_BAR_2;
    else if (cgns_elementtype == TRI_3) return Udm_TRI_3;
    else if (cgns_elementtype == QUAD_4) return Udm_QUAD_4;
    else if (cgns_elementtype == TETRA_4) return Udm_TETRA_4;
    else if (cgns_elementtype == PYRA_5) return Udm_PYRA_5;
    else if (cgns_elementtype == PENTA_6) return Udm_PENTA_6;
    else if (cgns_elementtype == HEXA_8) return Udm_HEXA_8;
    else if (cgns_elementtype == MIXED) return Udm_MIXED;

    return Udm_ElementTypeUnknown;
}


/**
 * CGNSシミュレーションタイプからUDMlib:シミュレーションタイプに変換する.
 * @param cgns_elementtype        CGNS:シミュレーションタイプ
 * @return        UDMlib:シミュレーションタイプ
 */
UdmSimulationType_t UdmGeneral::toSimulationType(SimulationType_t cgns_simulationtype)
{

    if (cgns_simulationtype == TimeAccurate) return Udm_TimeAccurate;
    else if (cgns_simulationtype == NonTimeAccurate) return Udm_NonTimeAccurate;
    else return Udm_SimulationTypeUnknown;
}


/**
 * UDMlib:要素形状タイプからCGNS要素形状タイプに変換する.
 * @param elementtype        UDMlib:要素形状タイプ
 * @return        CGNS:要素形状タイプ
 */
ElementType_t UdmGeneral::toCgnsElementType(UdmElementType_t elementtype)
{
    if (elementtype == Udm_NODE) return NODE;
    else if (elementtype == Udm_BAR_2) return BAR_2;
    else if (elementtype == Udm_TRI_3) return TRI_3;
    else if (elementtype == Udm_QUAD_4) return QUAD_4;
    else if (elementtype == Udm_TETRA_4) return TETRA_4;
    else if (elementtype == Udm_PYRA_5) return PYRA_5;
    else if (elementtype == Udm_PENTA_6) return PENTA_6;
    else if (elementtype == Udm_HEXA_8) return HEXA_8;
    else if (elementtype == Udm_MIXED) return MIXED;

    return ElementTypeNull;
}

/**
 * 要素形状タイプの頂点（ノード）数を取得する
 * @param datatype        要素形状タイプ
 * @return        頂点（ノード）数 : 不明、エラーの場合は0を返す。
 */
int UdmGeneral::getNumVertexOfElementType(UdmElementType_t elementtype)
{
    if (elementtype == Udm_NODE) return 1;
    else if (elementtype == Udm_BAR_2) return 2;
    else if (elementtype == Udm_TRI_3) return 3;
    else if (elementtype == Udm_QUAD_4) return 4;
    else if (elementtype == Udm_TETRA_4) return 4;
    else if (elementtype == Udm_PYRA_5) return 5;
    else if (elementtype == Udm_PENTA_6) return 6;
    else if (elementtype == Udm_HEXA_8) return 8;
    else if (elementtype == Udm_MIXED) return 0;

    return 0;
}


/**
 * 要素形状タイプの面数（部品要素数）を取得する.
 * Solid(3D)要素のShell(2D)要素、Shell(2D)要素のBar(1D)要素の構成数を取得する.
 * @param datatype        要素形状タイプ
 * @return        面、線数 : 不明、エラーの場合は0を返す。
 */
int UdmGeneral::getNumComponentOfElementType(UdmElementType_t elementtype)
{
    if (elementtype == Udm_NODE) return 0;
    else if (elementtype == Udm_BAR_2) return 0;
    else if (elementtype == Udm_TRI_3) return 3;
    else if (elementtype == Udm_QUAD_4) return 4;
    else if (elementtype == Udm_TETRA_4) return 4;
    else if (elementtype == Udm_PYRA_5) return 5;
    else if (elementtype == Udm_PENTA_6) return 5;
    else if (elementtype == Udm_HEXA_8) return 6;
    else if (elementtype == Udm_MIXED) return 0;

    return 0;
}

/**
 * 要素形状タイプの文字列を取得する.
 * @param [in]  datatype        要素形状タイプ
 * @param [out] buf        要素形状タイプ文字列
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGeneral::toStringElementType(UdmElementType_t elementtype, std::string &buf)
{
    if (elementtype == Udm_NODE)          buf = "NODE";
    else if (elementtype == Udm_BAR_2)    buf = "BAR_2";
    else if (elementtype == Udm_TRI_3)    buf = "TRI_3";
    else if (elementtype == Udm_QUAD_4)   buf = "QUAD_4";
    else if (elementtype == Udm_TETRA_4)  buf = "TETRA_4";
    else if (elementtype == Udm_PYRA_5)   buf = "PYRA_5";
    else if (elementtype == Udm_PENTA_6)  buf = "PENTA_6";
    else if (elementtype == Udm_HEXA_8)   buf = "HEXA_8";
    else if (elementtype == Udm_MIXED)    buf = "MIXED";
    else return UDM_ERROR;

    return UDM_OK;
}


/**
 * データ型のデータ配列を作成する.
 * @param datatype        データ型
 * @param size            データサイズ
 * @return        データ配列
 */
void* UdmGeneral::createDataArray(UdmDataType_t datatype, UdmSize_t size)
{
    void* value = NULL;
    if (datatype == Udm_Integer) {
        value = new int[size];
        memset(value, 0x00, sizeof(int)*size);
    }
    else if (datatype == Udm_LongInteger) {
        value = new long long[size];
        memset(value, 0x00, sizeof(long long)*size);
    }
    else if (datatype == Udm_RealSingle) {
        value = new float[size];
        memset(value, 0x00, sizeof(float)*size);
    }
    else if (datatype == Udm_RealDouble) {
        value = new double[size];
        memset(value, 0x00, sizeof(double)*size);
    }

    return value;
}


/**
 * データ型のデータ配列を破棄する.
 * @param array         破棄データ
 * @param datatype        データ型
 */
void UdmGeneral::deleteDataArray(void* array, UdmDataType_t datatype)
{
    if (array == NULL) return;
    if (datatype == Udm_Integer) {
        delete[] (int*)array;
    }
    else if (datatype == Udm_LongInteger) {
        delete[] (long long*)array;
    }
    else if (datatype == Udm_RealSingle) {
        delete[] (float*)array;
    }
    else if (datatype == Udm_RealDouble) {
        delete[] (double*)array;
    }
    else {
        delete[] (char*)array;
    }
    return;
}

/**
 * データ型のサイズを取得する.
 * @param datatype        データ型
 * @return        データ型のサイズ
 */
unsigned int UdmGeneral::sizeofDataType(UdmDataType_t datatype)
{
    if (datatype == Udm_Integer) return sizeof(int);
    else if (datatype == Udm_LongInteger) return sizeof(long long);
    else if (datatype == Udm_RealSingle) return sizeof(float);
    else if (datatype == Udm_RealDouble) return sizeof(double);

    return 0;
}

/**
 * CGNS:BaseIterativeDataからステップ数と同じIDリストを取得する.
 * IDリストは１～とする。
 * @param [out] iterative_ids        CGNS:BaseIterativeData IDリスト.
 * @param [in] index_file        CGNSファイルインデックス
 * @param [in] index_base        CGNSベースインデックス
 * @param [in] timeslice_step    ステップ数 (default = -1)
 * @return            IDリスト数
 */
int UdmGeneral::getCgnsBaseIterativeDataIds(std::vector<UdmSize_t> &iterative_ids, int index_file, int index_base, int timeslice_step)
{
    char bitername[33], arrayname[33];
    int nsteps, narrays;
    int dataDimension;
    cgsize_t n, i, dimensionVector[3];
    DataType_t idatatype;

    if (timeslice_step < 0) {
        return 1;
    }

    // CGNS:BaseIterativeDataの読込 : cg_biter_read
    if (cg_biter_read(index_file,index_base,bitername,&nsteps) != CG_OK) {
        return 0;
    }
    if (nsteps <= 0) {
        return 0;
    }
    if (cg_goto(index_file,index_base,"BaseIterativeData_t",1,"end") != CG_OK) {
        return 0;
    }
    // CGNS:BaseIterativeData/DataArray_tの数の取得 : cg_narrays
    narrays = 0;
    cg_narrays(&narrays);
    for (n=1; n<=narrays; n++) {
        // CGNS:BaseIterativeData/DataArray_tの名前,データ数の取得 : cg_array_info
        cg_array_info(n, arrayname, &idatatype,&dataDimension, dimensionVector);
        if (strcmp(arrayname, "IterationValues") == 0) {
            if (dataDimension != 1 && dimensionVector[0] != nsteps) {
                UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "invalid DataDimension or DimensionVector.");
                return 0;
            }
            // int iterationValues[dimensionVector[0]];
            int *iterationValues = new int[dimensionVector[0]];
            // IterationValuesの繰返回数の取得 : cg_array_read_as
            cg_array_read_as(n, Integer, iterationValues);
            for (i=0; i<dimensionVector[0]; i++) {
                if (iterationValues[i] == timeslice_step) {
                    iterative_ids.push_back(i+1);
                }
            }
            delete []iterationValues;
        }
    }
    return iterative_ids.size();
}


/**
 * ステップ数に設定されている時系列CGNS:FlowSolution名を取得する
 * @param [out] solution_names      CGNS:FlowSolution名リスト
 * @param [in] index_file        CGNSファイルインデックス
 * @param [in] index_base        CGNSベースインデックス
 * @param [in] index_zone        CGNSゾーンインデックス
 * @param [in] timeslice_step    ステップ数 (default = -1)
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGeneral::getCgnsIterativeFlowSolutionNames(std::vector<std::string> &solution_names, int index_file, int index_base, int index_zone, int timeslice_step)
{
    char arrayname[33], zitername[33];
    int narrays;
    int dimension, num_iterative, i;
    cgsize_t n, vectors[3];
    UdmSize_t  iterative_index;
    DataType_t idatatype;

    // CGNS:BaseIterativeData,ZoneIterativeDataが存在するかチェックする.
    if (!this->existsCgnsIterativeData(index_file, index_base, index_zone)) {
        return UDM_WARNING_CGNS_NOTEXISTS_ITERATIVEDATA;
    }

    // CGNS:BaseIterativeDataからステップ数と一致するIterativeIDを取得する.
    std::vector<UdmSize_t> iterative_ids;
    num_iterative = this->getCgnsBaseIterativeDataIds(
                                iterative_ids,
                                index_file,
                                index_base,
                                timeslice_step>=0?timeslice_step:0);

    if (timeslice_step < 0 && num_iterative <= 0) {
        // すべてのCGNS:FlowSolutionを取得する.
        int nsols;
        cg_nsols(index_file, index_base, index_zone, &nsols);
        for (int solution_index=1; solution_index<=nsols; solution_index++) {
            char name[33] = {0x00};
            GridLocation_t cgns_location;
            // FlowSolution名称の取得
            if (cg_sol_info(index_file, index_base, index_zone, solution_index, name, &cgns_location) != CG_OK) {
                return UDM_ERROR_CGNS_INVALID_FLOWSOLUTION;
            }
            solution_names.push_back(name);
        }
        return UDM_OK;
    }

    if (iterative_ids.size() <= 0) {
        return UDM_WARNING_CGNS_NOTFOUND_FLOWSOLUTION;
    }

    // CGNS:ZoneIterativeDataの読込
    if (cg_ziter_read(index_file,index_base,index_zone,zitername) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "failure:cg_ziter_read(index_file=%d,index_base=%d,index_zone=%d).", index_file,index_base,index_zone);
    }
    if (cg_goto(index_file,index_base,"Zone_t",index_zone,"ZoneIterativeData_t",1,"end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "failure:cg_goto(ZoneIterativeData_t).");
    }

    // CGNS:ZoneIterativeData/DataArray_tの数の取得 : cg_narrays
    narrays = 0;
    cg_narrays(&narrays);
    for (n=1; n<=narrays; n++) {
        // CGNS:ZoneIterativeData/DataArray_tの名前,データ数の取得 : cg_array_info
        cg_array_info(n, arrayname, &idatatype,&dimension,vectors);
        if (dimension != 2) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "invalid ZoneIterativeData[dataDimension=%d].", dimension);
        }
        if (strcmp(arrayname, "FlowSolutionPointers")==0) {
            // char sol_names[vectors[1]][vectors[0]];
            char *sol_names = new char[vectors[1]*vectors[0]];
            memset(sol_names, 0x00, vectors[1]*vectors[0]);
            // 時系列FlowSolution名リストの取得 : FlowSolutionPointers
            cg_array_read_as(n,Character,sol_names);

            for (i=0; i<iterative_ids.size(); i++) {
                // 時系列インデックスの名前の取得
                char name[33] = {0x00};
                // strncpy(name, sol_names[iterative_ids[i]-1], vectors[0]);
                strncpy(name, sol_names+(iterative_ids[i]-1)*vectors[0], vectors[0]);
                trim(name);
                solution_names.push_back(name);
            }
            delete []sol_names;
            return UDM_OK;
        }
    }
    // return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "not found ZoneIterativeData[timeslice_step=%d,iterative_index=%d]", timeslice_step, iterative_index);
    return UDM_ERROR_CGNS_INVALID_ITERATIVEDATA;
}


/**
 * CGNS:FlowSolution名からCGNS:FlowSolutionのIDを取得する.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param solution_name        CGNS:FlowSolution名
 * @return
 */
int UdmGeneral::getCgnsFlowSolutionId(int index_file, int index_base, int index_zone, const std::string &solution_name)
{
    int num_solutions = 0;
    char name[33];
    GridLocation_t cgns_location;
    int solution_index;

    // 物理量の読込
    if (cg_nsols(index_file, index_base, index_zone, &num_solutions) != CG_OK) {
        return 0;
    }

    for (solution_index=1; solution_index<=num_solutions; solution_index++) {
        // FlowSolution名称の取得
        if (cg_sol_info(index_file, index_base, index_zone, solution_index, name, &cgns_location) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_FLOWSOLUTION, "failure:cg_sol_info");
        }
        if (strcmp(solution_name.c_str(), name) == 0) {
            return solution_index;
        }
    }

    return 0;
}

/**
 * データ型を変更してコピー元データをコピー先にデータをコピーする.
 * @param src            コピー先データポインタ
 * @param src_type        コピー先データ型
 * @param dest            コピー元データポインタ
 * @param dest_type        コピー元データ型
 * @param size            コピーデータサイズ
 * @return    コピー先データポインタ
 */
void* UdmGeneral::copyDataArray(void* src, UdmDataType_t src_type, const void* dest, UdmDataType_t dest_type, unsigned int size)
{
    unsigned int n;

    if (src == NULL) return NULL;
    if (dest == NULL) return NULL;
    if (size <= 0) return NULL;

    if (src_type == dest_type) {
        unsigned int sizeofdata = UdmGeneral::sizeofDataType(src_type);
        memcpy(src, dest, size*sizeofdata);
        return src;
    }

    for (n=0; n<size; n++) {
        // Udm_Integer
        if (src_type == Udm_Integer && dest_type == Udm_LongInteger) {
            *((int*)src+n*sizeof(int)) = *(long long*)((char*)dest+n*sizeof(long long));
        }
        else if (src_type == Udm_Integer && dest_type == Udm_RealSingle) {
            *((int*)src+n*sizeof(int)) = *(float*)((char*)dest+n*sizeof(float));
        }
        else if (src_type == Udm_Integer && dest_type == Udm_RealDouble) {
            *((int*)src+n*sizeof(int)) = *(double*)((char*)dest+n*sizeof(double));
        }
        // Udm_LongInteger
        else if (src_type == Udm_LongInteger && dest_type == Udm_Integer) {
            *((long long*)src+n*sizeof(long long)) = *(int*)((char*)dest+n*sizeof(int));
        }
        else if (src_type == Udm_LongInteger && dest_type == Udm_RealSingle) {
            *((long long*)src+n*sizeof(long long)) = *(float*)((char*)dest+n*sizeof(float));
        }
        else if (src_type == Udm_LongInteger && dest_type == Udm_RealDouble) {
            *((long long*)src+n*sizeof(long long)) = *(double*)((char*)dest+n*sizeof(double));
        }
        // Udm_RealSingle
        else if (src_type == Udm_RealSingle && dest_type == Udm_Integer) {
            *((float*)src+n*sizeof(float)) = *(int*)((char*)dest+n*sizeof(int));
        }
        else if (src_type == Udm_RealSingle && dest_type == Udm_LongInteger) {
            *((float*)src+n*sizeof(float)) = *(long long*)((char*)dest+n*sizeof(long long));
        }
        else if (src_type == Udm_RealSingle && dest_type == Udm_RealDouble) {
            *((float*)src+n*sizeof(float)) = *(double*)((char*)dest+n*sizeof(double));
        }
        // Udm_RealDouble
        else if (src_type == Udm_RealDouble && dest_type == Udm_Integer) {
            *((double*)src+n*sizeof(double)) = *(int*)((char*)dest+n*sizeof(int));
        }
        else if (src_type == Udm_RealDouble && dest_type == Udm_LongInteger) {
            *((double*)src+n*sizeof(double)) = *(long long*)((char*)dest+n*sizeof(long long));
        }
        else if (src_type == Udm_RealDouble && dest_type == Udm_RealSingle) {
            *((double*)src+n*sizeof(double)) = *(float*)((char*)dest+n*sizeof(float));
        }
    }
    return src;
}


template<>
UdmDataType_t UdmGeneral::getDataType<int>()
{
    return Udm_Integer;
}

template<>
UdmDataType_t UdmGeneral::getDataType<long long>()
{
    return Udm_LongInteger;
}

template<>
UdmDataType_t UdmGeneral::getDataType<float>()
{
    return Udm_RealSingle;
}

template<> UdmDataType_t
UdmGeneral::getDataType<double>()
{
    return Udm_RealDouble;
}

/**
 * モデル構成基本情報のシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 * @param id            ID
 * @param datatype        データ型
 * @param name            名前
 * @return                シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmGeneral::serializeGeneralBase(
                        UdmSerializeArchive& archive,
                        UdmSize_t id,
                        UdmDataType_t datatype,
                        const std::string &name)
{
    char len = 0;
    archive << id;                // ID
    archive << datatype;        // データ型
    // 名前
    if (name.empty()) {
        archive << len;
    }
    else {
        // len = UDM_CGNS_NAME_LENGTH;
        len = name.size() + 1;
        archive << len;
        archive.write(name, len);
    }
    return archive;
}


/**
 * モデル構成基本情報のシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 * @param datatype        データ型
 * @param name            名前
 * @return                シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmGeneral::serializeGeneralBase(
                        UdmSerializeArchive& archive,
                        UdmDataType_t datatype,
                        const std::string &name)
{
    char len = 0;
    archive << datatype;        // データ型
    // 名前
    if (name.empty()) {
        archive << len;
    }
    else {
        // len = UDM_CGNS_NAME_LENGTH;
        len = name.size() + 1;
        archive << len;
        archive.write(name, len);
    }
    return archive;
}


/**
 * モデル構成基本情報のデシリアライズを行う.
 * @param [inout] archive        シリアライズ・デシリアライズクラス
 * @param [out] id                ID
 * @param [out] datatype        データ型
 * @param [out] name            名前
 * @return                シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmGeneral::deserializeGeneralBase(
                        UdmSerializeArchive& archive,
                        UdmSize_t& id,
                        UdmDataType_t& datatype,
                        std::string& name)
{
    char len = 0;
    archive >> id;            // ID
    archive >> datatype;    // データ型
    archive >> len;            // 名前
    if (len > 0) {
        archive.read(name, len);
    }

    return archive;
}

/**
 * モデル構成基本情報のデシリアライズを行う.
 * @param [inout] archive        シリアライズ・デシリアライズクラス
 * @param [out] datatype        データ型
 * @param [out] name            名前
 * @return                シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmGeneral::deserializeGeneralBase(
                        UdmSerializeArchive& archive,
                        UdmDataType_t& datatype,
                        std::string& name)
{
    char len = 0;
    archive >> datatype;    // データ型
    archive >> len;            // 名前
    if (len > 0) {
        archive.read(name, len);
    }

    return archive;
}


/**
 * 節点（ノード）,要素（セル）リストからIDの節点（ノード）,要素（セル）を検索する.
 * 二分探索による検索を行う。
 * @param list           節点（ノード）,要素（セル）リスト
 * @param node_id        ID
 * @return        検索イテレータ（検索失敗の場合は、endイテレータを返す）
 */
template<class DATA_TYPE>
typename std::vector<DATA_TYPE*>::const_iterator
UdmGeneral::searchEntityId(const std::vector<DATA_TYPE*>& list, UdmSize_t entity_id) const
{
    if (entity_id <= 0) return list.end();
    if (list.size() == 0) return list.end();

    int left = 0;
    int right = list.size();
    int mid;
    int size = list.size();
    UdmEntity *first = (UdmEntity*)list[left];
    UdmEntity *last = (UdmEntity*)list[right-1];
    UdmEntity *mid_entity = NULL;
    UdmSize_t first_id = first->getId();
    UdmSize_t last_id = last->getId();
    UdmSize_t value;
    if (first_id > entity_id) return list.end();        // 先頭ノードより小さいことはない。
    if (last_id < entity_id) return list.end();        // 最終ノードより大きいことはない。

    // entity_id位置で検索
    if (entity_id>=first_id && entity_id-first_id < list.size()) {
        if (list[entity_id-first_id]->getId() == entity_id) {
            return list.begin() + (entity_id-first_id);
        }
    }

    while (left <= right) {
        mid = (left + right) / 2;
        if (mid >= size) {
            break;
        }
        mid_entity = (UdmEntity*)list[mid];
        value = mid_entity->getId();
        if (value == entity_id) {
            return list.begin() + mid;
        }
        else if (value < entity_id) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }

    return list.end();

}

template std::vector<UdmCell*>::const_iterator
UdmGeneral::searchEntityId<UdmCell>(const std::vector<UdmCell*>& list, UdmSize_t entity_id) const;
template std::vector<UdmNode*>::const_iterator
UdmGeneral::searchEntityId<UdmNode>(const std::vector<UdmNode*>& list, UdmSize_t entity_id) const;



/**
 * 節点（ノード）,要素（セル）リストからIDの節点（ノード）,要素（セル）を検索する.
 * 二分探索による検索を行う。
 * @param list           節点（ノード）,要素（セル）リスト
 * @param node_id        ID
 * @return        検索イテレータ（検索失敗の場合は、endイテレータを返す）
 */

std::vector<UdmComponent*>::const_iterator
UdmGeneral::searchEntityId(const std::vector<UdmComponent*>& list, UdmSize_t entity_id) const
{
    if (entity_id <= 0) return list.end();
    if (list.size() == 0) return list.end();

    int left = 0;
    int right = list.size();
    int mid;
    int size = list.size();
    UdmComponent *first = (UdmComponent*)list[left];
    UdmComponent *last = (UdmComponent*)list[right-1];
    UdmComponent *mid_entity = NULL;
    UdmSize_t first_id = first->getId();
    UdmSize_t last_id = last->getId();
    UdmSize_t value;
    if (first_id > entity_id) return list.end();        // 先頭ノードより小さいことはない。
    if (last_id < entity_id) return list.end();        // 最終ノードより大きいことはない。

    // entity_id位置で検索
    if (entity_id>=first_id && entity_id-first_id < list.size()) {
        if (list[entity_id-first_id]->getId() == entity_id) {
            return list.begin() + (entity_id-first_id);
        }
    }

    while (left <= right) {
        mid = (left + right) / 2;
        if (mid >= size) {
            break;
        }
        mid_entity = (UdmComponent*)list[mid];
        value = mid_entity->getId();
        if (value == entity_id) {
            return list.begin() + mid;
        }
        else if (value < entity_id) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }

    return list.end();

}

/**
 * 節点（ノード）,要素（セル）リストからIDの節点（ノード）,要素（セル）を検索する.
 * 二分探索による検索を行う。
 * @param list           節点（ノード）,要素（セル）リスト
 * @param src            検索対象節点（ノード）,要素（セル）
 * @return        検索イテレータ（検索失敗の場合は、endイテレータを返す）
 */
template<class DATA_TYPE>
typename std::vector<DATA_TYPE*>::const_iterator
UdmGeneral::searchGlobalId(const std::vector<DATA_TYPE*>& list, const UdmEntity *src) const
{
    if (src == NULL) return list.end();

    UdmSize_t src_entityid = src->getId();
    int src_myrankno = src->getMyRankno();
    if (src_entityid <= 0) return list.end();
    if (list.size() == 0) return list.end();

    return this->searchCurrentGlobalId(list, src_myrankno, src_entityid);
}

template std::vector<UdmCell*>::const_iterator
UdmGeneral::searchGlobalId<UdmCell>(const std::vector<UdmCell*>& list, const UdmEntity *src) const;
template std::vector<UdmNode*>::const_iterator
UdmGeneral::searchGlobalId<UdmNode>(const std::vector<UdmNode*>& list, const UdmEntity *src) const;

/**
 * 節点（ノード）,要素（セル）リストから現在のランク番号, IDの節点（ノード）,要素（セル）を検索する.
 * 二分探索による検索を行う。
 * @param list           節点（ノード）,要素（セル）リスト
 * @param src_myrankno         ランク番号
 * @param src_entityid       ID
 * @return        検索イテレータ（検索失敗の場合は、endイテレータを返す）
 */
template<class DATA_TYPE>
typename std::vector<DATA_TYPE*>::const_iterator
UdmGeneral::searchCurrentGlobalId(const std::vector<DATA_TYPE*>& list, int src_myrankno, UdmSize_t src_entityid) const
{
    if (src_entityid <= 0) return list.end();
    if (list.size() == 0) return list.end();

    int left = 0;
    int right = list.size();
    int mid;
    int size = list.size();
    UdmEntity *first = (UdmEntity*)list[left];
    UdmEntity *last = (UdmEntity*)list[right-1];
    UdmEntity *mid_entity = NULL;
    UdmSize_t first_id = first->getId();
    UdmSize_t last_id = last->getId();
    UdmSize_t value;
    if (first->compareGlobalId(src_myrankno, src_entityid) > 0) return list.end();        // 先頭ノードより小さいことはない。
    if (last->compareGlobalId(src_myrankno, src_entityid) < 0) return list.end();        // 最終ノードより大きいことはない。

    // entity_id位置で検索
    if (src_entityid>=first_id && src_entityid-first_id < list.size()) {
        UdmEntity *list_entity = list[src_entityid-first_id];
        int compare = list_entity->compareGlobalId(src_myrankno, src_entityid);
        if (compare == 0) {
            return list.begin() + (src_entityid-first_id);
        }
        else if (compare > 0) {
            right = src_entityid-first_id;
        }
        else {
            left = src_entityid-first_id;
        }
    }

    while (left <= right) {
        mid = (left + right) / 2;
        if (mid >= size) {
            break;
        }
        mid_entity = (UdmEntity*)list[mid];
        value = mid_entity->getId();
        int diff = mid_entity->compareGlobalId(src_myrankno, src_entityid);
        if (diff == 0) {
            return list.begin() + mid;
        }
        else if (diff < 0) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }

    return list.end();
}
template std::vector<UdmCell*>::const_iterator
UdmGeneral::searchCurrentGlobalId<UdmCell>(const std::vector<UdmCell*>& list, int rankno, UdmSize_t entity_id) const;
template std::vector<UdmNode*>::const_iterator
UdmGeneral::searchCurrentGlobalId<UdmNode>(const std::vector<UdmNode*>& list, int rankno, UdmSize_t entity_id) const;


/**
 * 節点（ノード）,要素（セル）リストから以前のランク番号, IDの節点（ノード）,要素（セル）を検索する.
 * 二分探索による検索を行う。
 * @param list           節点（ノード）,要素（セル）リスト
 * @param src_myrankno         以前のランク番号
 * @param src_entityid       以前のID
 * @return        検索イテレータ（検索失敗の場合は、endイテレータを返す）
 */
template<class DATA_TYPE>
typename std::vector<DATA_TYPE*>::const_iterator
UdmGeneral::searchPreviousGlobalId(const std::vector<DATA_TYPE*>& list, int src_myrankno, UdmSize_t src_entityid) const
{
    if (src_entityid <= 0) return list.end();
    if (list.size() == 0) return list.end();

    UdmSize_t n;
    UdmSize_t num_list = list.size();
    for (n=0; n<num_list; n++) {
        UdmEntity *entity = static_cast<UdmEntity*>(list[n]);
        if (entity == NULL) continue;
        if (entity->existsPreviousRankInfo(src_myrankno, src_entityid)) {
            return list.begin() + n;
        }
    }

    return list.end();
}
template std::vector<UdmCell*>::const_iterator
UdmGeneral::searchPreviousGlobalId<UdmCell>(const std::vector<UdmCell*>& list, int rankno, UdmSize_t entity_id) const;
template std::vector<UdmNode*>::const_iterator
UdmGeneral::searchPreviousGlobalId<UdmNode>(const std::vector<UdmNode*>& list, int rankno, UdmSize_t entity_id) const;


/**
 * 節点（ノード）リストから節点（ノード）IDを検索する:upper_bound.
 * 節点（ノード）IDが見つかったら、そのイテレータを返す。
 * 節点（ノード）IDが見つからなかったら、検索節点（ノード）IDの挿入位置のイテレータを返す。
 * @param list               節点（ノード）リスト
 * @param elem_id            節点（ノード）ID
 * @return        検索イテレータ（検索失敗の場合は、endイテレータを返す）
 */
template<class DATA_TYPE>
typename std::vector<DATA_TYPE*>::const_iterator
UdmGeneral::searchUpperGlobalId(const std::vector<DATA_TYPE*>& list, const UdmEntity *src) const
{
    if (src == NULL) return list.end();

    UdmSize_t src_id = src->getId();
    if (src_id <= 0) return list.end();
    if (list.size() == 0) return list.end();

    int left = 0;
    int right = list.size();
    int mid;
    int size = list.size();
    UdmEntity *first = list[left];
    UdmEntity *last = list[right-1];
    UdmEntity *mid_node = NULL;
    UdmSize_t first_id = first->getId();
    UdmSize_t last_id = last->getId();
    UdmSize_t value;
    if (first->compareGlobalId(src) > 0) return list.begin();        // 挿入位置は先頭
    if (last->compareGlobalId(src) < 0) return list.end();        // 挿入位置は最終

    // node_id位置で検索
    if (src_id>first_id && src_id-first_id < list.size()) {
        if (list[src_id-first_id]->compareGlobalId(src) ==0) {
            // node_idが重複
            UDM_WARNING_HANDLER(UDM_WARNING_INVALID_NODE, "duplicate node_id[=%ld]", src_id);
            return list.begin() + (src_id-first_id) + 1;
        }
    }

    while (left <= right) {
        mid = (left + right) / 2;
        if (mid >= size) {
            break;
        }
        mid_node = list[mid];
        value = mid_node->getId();
        int diff = mid_node->compareGlobalId(src);
        if (diff == 0) {
            // 同一node_idであるので、次を返す。（本来は同一node_idはありえない）
            // idが重複
            UDM_WARNING_HANDLER(UDM_WARNING_INVALID_NODE, "duplicate id[=%ld]", src_id);
            return list.begin() + mid + 1;
        }
        else if (diff < 0) {
            left = mid + 1;
            if (left < list.size()) {
                value = list[left]->getId();
                // 現在位置がnode_idより小さく、+1位置がnode_idより大きい位置を返す。
                if (list[left]->compareGlobalId(src) >= 0) {
                    return list.begin() + left;
                }
            }
        }
        else {
            right = mid - 1;
        }
    }

    return list.end();

}

template std::vector<UdmCell*>::const_iterator
UdmGeneral::searchUpperGlobalId<UdmCell>(const std::vector<UdmCell*>& list, const UdmEntity *src) const;
template std::vector<UdmNode*>::const_iterator
UdmGeneral::searchUpperGlobalId<UdmNode>(const std::vector<UdmNode*>& list, const UdmEntity *src) const;


/**
 * 要素（セル）リストから管理CGNS要素IDを検索する.
 * @param list                要素（セル）リスト
 * @param elem_id            管理CGNS要素ID
 * @return        検索イテレータ（検索失敗の場合は、endイテレータを返す）
 */
std::vector<UdmCell*>::const_iterator
UdmGeneral::searchElemId(const std::vector<UdmCell*>& list, UdmSize_t elem_id) const
{
    if (elem_id <= 0) return list.end();
    if (list.size() == 0) return list.end();

    int left = 0;
    int right = list.size();
    int mid;
    int size = list.size();
    UdmCell *first = list[left];
    UdmCell *last = list[right-1];
    UdmCell *mid_node = NULL;
    UdmSize_t first_id = first->getElemId();
    UdmSize_t last_id = last->getElemId();
    UdmSize_t value;
    if (first_id > elem_id) return list.end();        // 先頭ノードより小さいことはない。
    if (last_id < elem_id) return list.end();        // 最終ノードより大きいことはない。

    // elem_id位置で検索
    if (elem_id>first_id && elem_id-first_id < list.size()) {
        if (list[elem_id-first_id]->getElemId() == elem_id) {
            return list.begin() + (elem_id-first_id);
        }

        // elem_id位置より大きいことはない。
        right = elem_id-first_id;
    }

    while (left <= right) {
        mid = (left + right) / 2;
        if (mid >= size) {
            break;
        }
        mid_node = list[mid];
        value = mid_node->getElemId();
        if (value == elem_id) {
            return list.begin() + mid;
        }
        else if (value < elem_id) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }

    return list.end();

}


/**
 * 要素（セル）リストから管理CGNS要素IDを検索する:upper_bound.
 * 管理CGNS要素IDが見つかったら、そのイテレータを返す。
 * 管理CGNS要素IDが見つからなかったら、検索管理CGNS要素IDの挿入位置のイテレータを返す。
 * @param list                要素（セル）リスト
 * @param elem_id            管理CGNS要素ID
 * @return        検索イテレータ（検索失敗の場合は、endイテレータを返す）
 */
std::vector<UdmCell*>::const_iterator
UdmGeneral::searchUpperElemId(const std::vector<UdmCell*>& list, UdmSize_t elem_id) const
{
    if (elem_id <= 0) return list.end();
    if (list.size() == 0) return list.end();

    int left = 0;
    int right = list.size();
    int mid;
    int size = list.size();
    UdmCell *first = list[left];
    UdmCell *last = list[right-1];
    UdmCell *mid_node = NULL;
    UdmSize_t first_id = first->getElemId();
    UdmSize_t last_id = last->getElemId();
    UdmSize_t value;
    if (first_id > elem_id) return list.begin();        // 挿入位置は先頭
    if (last_id < elem_id) return list.end();        // 挿入位置は最終

    // elem_id位置で検索
    if (elem_id>first_id && elem_id-first_id < list.size()) {
        if (list[elem_id-first_id]->getElemId() == elem_id) {
            // 同一管理CGNS要素IDであるので、次を返す。（本来は同一管理CGNS要素IDはありえない）
            // elem_idが重複
            UDM_WARNING_HANDLER(UDM_WARNING_INVALID_CELL, "duplicate elem_id[=%ld]", elem_id);
            return list.begin() + (elem_id-first_id) + 1;
        }

        // elem_id位置より大きいことはない。
        right = elem_id-first_id;
    }

    while (left <= right) {
        mid = (left + right) / 2;
        if (mid >= size) {
            break;
        }
        mid_node = list[mid];
        value = mid_node->getElemId();
        if (value == elem_id) {
            // 同一管理CGNS要素IDであるので、次を返す。（本来は同一管理CGNS要素IDはありえない）
            // elem_idが重複
            UDM_WARNING_HANDLER(UDM_WARNING_INVALID_CELL, "duplicate elem_id[=%ld]", elem_id);
            return list.begin() + mid + 1;
        }
        else if (value < elem_id) {
            left = mid + 1;
            if (left < list.size()) {
                value = list[left]->getElemId();
                // 現在位置がelem_idより小さく、+1位置がelem_idより大きい位置を返す。
                if (value > elem_id) {
                    return list.begin() + left;
                }
            }
        }
        else {
            right = mid - 1;
        }
    }

    return list.end();

}


/**
 * ID,データ型等をコピーする.
 * @param src        元モデル構成の基底クラス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGeneral::cloneGeneral(const UdmGeneral& src)
{
    // ID : CGNSノードID、要素ローカルID, ノードローカルID
    this->id = src.id;
    // CGNSノードの名前です。
    this->name = src.name;
    // CGNSノードのラベルです。
    this->label = src.label;
    // CGNSノードのデータ型を定義します.
    this->datatype = src.datatype;

    return UDM_OK;
}

/**
 * リンクファイルをリンク出力ファイルからの相対パスに変換する.
 * @param [in] link_output_path        リンク出力ファイル
 * @param [in] linked_files            リンクファイル
 * @param [out] linked_relativepath    相対パス
 */
void UdmGeneral::getLinkedRelativePath(
                        const std::string& link_output_path,
                        const std::string& linked_file,
                        std::string& linked_relativepath) const
{
    char output_dir[512] = {0x00};
    char linked_dir[512] = {0x00};
    char linked_filename[512] = {0x00};
    char relative_path[512] = {0x00};
    char linked_path[512] = {0x00};

    // リンク出力ファイルのディレクトリ
    udm_get_dirname(link_output_path.c_str(), output_dir);
    // リンクファイルのディレクトリ
    udm_get_dirname(linked_file.c_str(), linked_dir);
    // リンクファイルのファイル名
    udm_get_filename(linked_file.c_str(), linked_filename);

    // リンクファイルをリンク出力ファイルからの相対パス取得
    udm_get_relativepath(output_dir, linked_dir, relative_path);

    // 相対パスとリンクファイルのファイル名を結合する.
    udm_connect_path(relative_path, linked_filename, linked_path);

    linked_relativepath = linked_path;

    return;
}

/**
 * サポート対象の要素タイプであるかチェックする.
 * サポート対象の要素タイプ.
 *    Udm_TETRA_4 = 5,   Solid:四面体要素        : CGNS_ENUMV( TETRA_4 ) =10
 *    Udm_PYRA_5 = 6,    Solid:ピラミッド要素 : CGNS_ENUMV( PYRA_5 ) =12
 *    Udm_PENTA_6 = 7,   Solid:五面体要素     : CGNS_ENUMV( PENTA_6 ) =14
 *    Udm_HEXA_8 = 8,    Solid:六面体要素    : CGNS_ENUMV( HEXA_8 ) =17
 *    Udm_MIXED = 9,     混合要素            : CGNS_ENUMV( MIXED ) =20
 * @param elementtype        要素タイプ.
 * @return            true=サポート対象
 */
bool UdmGeneral::isSupportElementType(UdmElementType_t elementtype)
{
    if (elementtype == Udm_TETRA_4) return true;
    if (elementtype == Udm_PYRA_5) return true;
    if (elementtype == Udm_PENTA_6) return true;
    if (elementtype == Udm_HEXA_8) return true;
    if (elementtype == Udm_MIXED) return true;

    return false;
}


/**
 * CGNS:BaseIterativeDataが存在するかチェックする.
 * IDリストは１～とする。
 * @param [in] index_file        CGNSファイルインデックス
 * @param [in] index_base        CGNSベースインデックス
 * @return         true=CGNS:BaseIterativeDataが存在する
 */
bool UdmGeneral::existsCgnsBaseIterativeData(int index_file, int index_base) const
{
    char bitername[33] = {0x00};
    int nsteps;

    // CGNS:BaseIterativeDataの読込 : cg_biter_read
    if (cg_biter_read(index_file,index_base,bitername,&nsteps) != CG_OK) {
        return false;
    }
    if (nsteps <= 0) {
        return false;
    }

    return true;
}


/**
 * CGNS:ZoneIterativeDataが存在するかチェックする.
 * IDリストは１～とする。
 * @param [in] index_file        CGNSファイルインデックス
 * @param [in] index_base        CGNSベースインデックス
 * @param [in] index_zone        CGNSゾーンインデックス
 * @return         true=CGNS:ZoneIterativeDataが存在する
 */
bool UdmGeneral::existsCgnsZoneIterativeData(int index_file, int index_base, int index_zone) const
{
    char zitername[33] = {0x00};

    // CGNS:ZoneIterativeDataの読込
    if (cg_ziter_read(index_file, index_base, index_zone, zitername) != CG_OK) {
        return false;
    }

    return true;
}

/**
 * CGNS:BaseIterativeData, CGNS:ZoneIterativeDataが存在するかチェックする.
 * IDリストは１～とする。
 * @param [in] index_file        CGNSファイルインデックス
 * @param [in] index_base        CGNSベースインデックス
 * @param [in] index_zone        CGNSゾーンインデックス
 * @return         true=CGNS:ZoneIterativeDataが存在する
 */
bool UdmGeneral::existsCgnsIterativeData(int index_file, int index_base, int index_zone) const
{
    // CGNS:BaseIterativeDataが存在するかチェックする.
    if (!this->existsCgnsBaseIterativeData(index_file, index_base)) {
        return false;
    }
    // CGNS:ZoneIterativeDataが存在するかチェックする.
    if (!this->existsCgnsZoneIterativeData(index_file, index_base, index_zone)) {
        return false;
    }

    return true;
}

/**
 * メモリサイズを取得する.
 * @return        メモリサイズ
 */
size_t UdmGeneral::getMemSize() const
{
    UdmSize_t size = sizeof(*this);
#ifdef _DEBUG
    printf("id size=%ld [offset=%ld]\n", sizeof(this->id), offsetof(UdmGeneral, id));
    printf("name size=%ld [offset=%ld]\n", sizeof(this->name), offsetof(UdmGeneral, name));
    printf("label size=%ld [offset=%ld]\n", sizeof(this->label), offsetof(UdmGeneral, label));
    printf("datatype size=%ld [offset=%ld]\n", sizeof(this->datatype), offsetof(UdmGeneral, datatype));
    printf("virtual void initialize(void)\n");
#endif

    return size;
}

} /* namespace udm */
