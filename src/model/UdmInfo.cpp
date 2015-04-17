/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmInfo.cpp
 */

#include "model/UdmInfo.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmInfo::UdmInfo() : parent_model(NULL)
{
    this->initialize();
}

/**
 * コンストラクタ
 * @param model        親モデル（ベース）
 */
UdmInfo::UdmInfo(UdmModel* model) : parent_model(model)
{
    this->initialize();
}

/**
 * デストラクタ
 */
UdmInfo::~UdmInfo()
{
}

/**
 * 出力MPIプロセス数を取得する.
 * @return        出力MPIプロセス数
 */
int UdmInfo::getUdminfoProcessSize() const
{
    return this->info_process;
}

/**
 * 出力MPIプロセス数を設定する.
 * @param process        出力MPIプロセス数
 */
void UdmInfo::setUdminfoProcessSize(int process)
{
    this->info_process = process;
}

/**
 * 出力MPIランク番号を取得する
 * @return        出力MPIランク番号
 */
int UdmInfo::getUdminfoRankno() const
{
    return this->info_rankno;
}

/**
 * 出力MPIランク番号を設定する.
 * @param rankno        出力MPIランク番号
 */
void UdmInfo::setUdminfoRankno(int rankno)
{
    this->info_rankno = rankno;
}

/**
 * UDMlibバージョン情報を取得する.
 * @return        UDMlibバージョン情報
 */
const std::string& UdmInfo::getUdmlibVersion() const
{
    return this->udmlib_version;
}

/**
 * UDMlibバージョン情報を設定する.
 * @param version        UDMlibバージョン情報
 */
void UdmInfo::setUdmlibVersion(const std::string& version)
{
    this->udmlib_version = version;
}

/**
 * 親モデル（ベース）を取得する
 * @return        親モデル（ベース）
 */
UdmModel* UdmInfo::getParentModel() const
{
    return this->parent_model;
}

/**
 * 親モデル（ベース）を設定する.
 * @param model        親モデル（ベース）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmInfo::setParentModel(UdmModel* model)
{
    this->parent_model = model;
    return UDM_OK;
}

/**
 * 初期化を行う.
 */
void UdmInfo::initialize()
{
    this->info_rankno = -1;
    this->info_process = 0;
    char version[33] = {0x00};
    udm_get_version(version);
    this->udmlib_version = version;
}


/**
 * CGNS:UdmInfoを読込を行う.
 * @param index_file        CGNS:ファイルインデックス
 * @param index_base        CGNS:ベースインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmInfo::readCgns(int index_file, int index_base)
{
    cgsize_t user_dims[2] = {0, 0};
    char version[33] = {0x00};
    int rank_info[2] = {0, 0};
    int n;
    char array_name[33] = {0x00};
    DataType_t cgns_datatype;
    int dimension;
    cgsize_t dimension_vector[3] = {1};

    // "UdmInfo" : UDMlib情報
    if (cg_goto(index_file, index_base, "end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO, "index_file=%d, cgns_error=%s", index_file, cg_get_error());
    }

    int num_arrays, len = 0, index_array = 0;
    cg_narrays(&num_arrays);
    for (n=1; n<=num_arrays; n++) {
        if (cg_array_info(n, array_name, &cgns_datatype, &dimension, dimension_vector) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_info(%d)", n);
        }

        // UDMlib:バージョン : CGNS:DataArray[@name="UDMlibVersion"]
        if (strncmp(array_name, UDM_CGNS_NAME_UDMINFO_VERSION, strlen(UDM_CGNS_NAME_UDMINFO_VERSION)) == 0) {
            if (cg_array_read_as(n, Character, &version) != CG_OK) {
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_read_as(%d=%s)", n, UDM_CGNS_NAME_RANKCONNECTIVITY_ARRAY);
            }
            this->setUdmlibVersion(version);
        }

        // 出力ランク番号 : CGNS:DataArray[@name="OutputRankInfo"]
        if (strncmp(array_name, UDM_CGNS_NAME_UDMINFO_RANKINFO, strlen(UDM_CGNS_NAME_UDMINFO_RANKINFO)) == 0) {
            if (cg_array_read_as(n, Integer, &rank_info) != CG_OK) {
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_read_as(%d=%s)", n, UDM_CGNS_NAME_RANKCONNECTIVITY_ARRAY);
            }
            this->setUdminfoProcessSize(rank_info[0]);
            this->setUdminfoRankno(rank_info[1]);
        }
    }

    return UDM_OK;
}

/**
 * CGNS:UdmInfo(UDMlib情報)を出力する
 * @param index_file        CGNS:ファイルインデックス
 * @param index_base        CGNS:ベースインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmInfo::writeCgns(int index_file, int index_base)
{
    cgsize_t user_dims[2] = {0, 0};
    char version[33] = {0x00};
    char description[] = UDM_CGNS_TEXT_UDMINFO_DESCRIPTION;
    int rank_info[2] = { this->info_process, this->info_rankno};

    // UDMlibバージョン
    strcpy(version, this->udmlib_version.c_str());

    // "UdmInfo" : UDMlib情報
    if (cg_goto(index_file, index_base, "end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO, "index_file=%d, cgns_error=%s", index_file, cg_get_error());
    }

    // CGNS:UserDefinedData[@name="UdmInfo"]の作成 : cg_user_data_write
    if (cg_user_data_write(UDM_CGNS_NAME_UDMINFO) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_user_data_write(%s), cgns_error=%s", UDM_CGNS_NAME_UDMINFO, cg_get_error());
    }
    if (cg_goto(index_file, index_base, UDM_CGNS_NAME_UDMINFO, 0, "end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO, "index_file=%d", index_file);
    }

    // UDMlib:バージョン
    // CGNS:DataArray[@name="UDMlibVersion"]の作成 : cg_array_write
    user_dims[0] = 32;
    user_dims[1] = 1;
    if (cg_array_write( UDM_CGNS_NAME_UDMINFO_VERSION,
                        Character,
                        2,
                        user_dims,
                        version) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_write(%s), cgns_error=%s", UDM_CGNS_NAME_UDMINFO_VERSION, cg_get_error());
    }

    // UDMlib:説明
    // CGNS:DataArray[@name="UDMlibDescription"]の作成 : cg_array_write
    user_dims[0] = strlen(description);
    user_dims[1] = 1;
    if (cg_array_write( UDM_CGNS_NAME_UDMINFO_DESCRIPTION,
                        Character,
                        2,
                        user_dims,
                        description) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_write(%s), cgns_error=%s", UDM_CGNS_NAME_UDMINFO_DESCRIPTION, cg_get_error());
    }

    // 出力ランク番号
    // CGNS:DataArray[@name="OutputRankInfo"]の作成 : cg_array_write
    user_dims[0] = 2;
    user_dims[1] = 0;
    if (cg_array_write( UDM_CGNS_NAME_UDMINFO_RANKINFO,
                        Integer,
                        1,
                        user_dims,
                        rank_info) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_write(%s), cgns_error=%s", UDM_CGNS_NAME_UDMINFO_RANKINFO, cg_get_error());
    }

    return UDM_OK;
}


} /* namespace udm */
