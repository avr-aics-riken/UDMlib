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
 * @file UdmUserDefinedDatas.cpp
 * ユーザ定義データクラスのソースファイル
 */

#include "model/UdmUserDefinedDatas.h"
#include "model/UdmZone.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmUserDefinedDatas::UdmUserDefinedDatas()
{
    this->initialize();
}

/**
 * コンストラクタ
 * @param zone        ゾーン
 */
UdmUserDefinedDatas::UdmUserDefinedDatas(UdmZone* zone)
{
    this->initialize();
    this->parent_zone = zone;
}

/**
 * デストラクタ
 */
UdmUserDefinedDatas::~UdmUserDefinedDatas()
{
/*******************
    std::vector<UdmUserData*>::reverse_iterator ritr = this->user_datas.rbegin();
    while (ritr != this->user_datas.rend()) {
        UdmUserData* data = *(--(ritr.base()));
        if (data != NULL) {
            delete data;
        }
        this->user_datas.erase((++ritr).base());
        ritr = this->user_datas.rbegin();
    }
***********************/

    std::vector<UdmUserData*>::iterator itr = this->user_datas.begin();
    for (itr = this->user_datas.begin(); itr != this->user_datas.end(); itr++) {
        UdmUserData* data = (*itr);
        if (data != NULL) {
            delete data;
        }
    }

    this->user_datas.clear();

}

/**
 * 初期化を行う
 */
void UdmUserDefinedDatas::initialize()
{
    this->user_datas.clear();
    this->parent_zone = NULL;
}

/**
 * ユーザ定義データリスト数を取得する.
 * @return        ユーザ定義データリスト数
 */
int UdmUserDefinedDatas::getNumUserDatas() const
{
    return this->user_datas.size();
}

/**
 * ユーザ定義データ名を取得する.
 * @param defined_id            定義データID（１～）
 * @return        ユーザ定義データ名
 */
const std::string& UdmUserDefinedDatas::getUserDataName(int defined_id) const
{
    if (defined_id <= 0) return this->getName();		// nameは空文字
    if (defined_id > this->user_datas.size()) return this->getName();	// nameは空文字
    return this->user_datas[defined_id-1]->getName();
}

/**
 * ユーザ定義データ情報を取得する.
 * @param [in]  array_name        ユーザ定義データ名
 * @param [out] data_type            データ型
 * @param [out] dimension            データ次元数
 * @param [out] dim_sizes            データ次元毎のデータ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUserDefinedDatas::getUserDataInfo(
                        const std::string& array_name,
                        UdmDataType_t& data_type,
                        int& dimension,
                        UdmSize_t* dim_sizes) const
{
    if (dim_sizes == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "dim_sizes is null.");
    }
    std::vector<UdmUserData*>::const_iterator itr;
    for (itr=this->user_datas.begin(); itr!=this->user_datas.end(); itr++) {
        if ((*itr)->getName() == array_name) {
            data_type = (*itr)->getDataType();
            dimension = (*itr)->getDataDimension();
            memcpy(dim_sizes, (*itr)->getDimensionSizes(), dimension*sizeof(UdmSize_t));
            return UDM_OK;
        }
    }
    return UDM_ERROR;
}


/**
 * ユーザ定義データを取得する.
 * @param [in] array_name        ユーザ定義データ名
 * @param [in] data_type            取得データ型
 * @param [out] data                ユーザ定義データ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUserDefinedDatas::getUserDataArray(
                const std::string& array_name,
                UdmDataType_t data_type,
                void* data) const
{
    if (data == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "data is null.");
    }

    UdmSize_t n, size;
    std::vector<UdmUserData*>::const_iterator itr;
    for (itr=this->user_datas.begin(); itr!=this->user_datas.end(); itr++) {
        if ((*itr)->getName() != array_name) continue;

        size = (*itr)->getDataArraySize();
        void *src_data = (*itr)->getDataArray();
        if (data_type == (*itr)->getDataType()) {
            memcpy(  data,
                    (*itr)->getDataArray(),
                    size*this->sizeofDataType((*itr)->getDataType()));
        }
        else if (data_type == Udm_Integer) {
            int *dest_data = (int*)data;
            if ((*itr)->getDataType() == Udm_LongInteger) {
                cglong_t *src_data = (cglong_t *)((*itr)->getDataArray());
                for (n = 0; n < size; n++) dest_data[n] = (cglong_t)src_data[n];
            }
            else if ((*itr)->getDataType() == Udm_RealSingle) {
                float *src_data = (float *)((*itr)->getDataArray());
                for (n = 0; n < size; n++) dest_data[n] = (float)src_data[n];
            }
            else if ((*itr)->getDataType() == Udm_RealDouble) {
                double *src_data = (double *)((*itr)->getDataArray());
                for (n = 0; n < size; n++) dest_data[n] = (double)src_data[n];
            }
        }
        else if (data_type == Udm_LongInteger) {
            cglong_t *dest_data = (cglong_t*)data;
            if ((*itr)->getDataType() == Udm_Integer) {
                int *src_data = (int *)((*itr)->getDataArray());
                for (n = 0; n < size; n++) dest_data[n] = (int)src_data[n];
            }
            else if ((*itr)->getDataType() == Udm_RealSingle) {
                float *src_data = (float *)((*itr)->getDataArray());
                for (n = 0; n < size; n++) dest_data[n] = (float)src_data[n];
            }
            else if ((*itr)->getDataType() == Udm_RealDouble) {
                double *src_data = (double *)((*itr)->getDataArray());
                for (n = 0; n < size; n++) dest_data[n] = (double)src_data[n];
            }
        }
        else if (data_type == Udm_RealSingle) {
            cglong_t *dest_data = (cglong_t*)data;
            if ((*itr)->getDataType() == Udm_Integer) {
                int *src_data = (int *)((*itr)->getDataArray());
                for (n = 0; n < size; n++) dest_data[n] = (int)src_data[n];
            }
            else if ((*itr)->getDataType() == Udm_LongInteger) {
                cglong_t *src_data = (cglong_t *)((*itr)->getDataArray());
                for (n = 0; n < size; n++) dest_data[n] = (cglong_t)src_data[n];
            }
            else if ((*itr)->getDataType() == Udm_RealDouble) {
                double *src_data = (double *)((*itr)->getDataArray());
                for (n = 0; n < size; n++) dest_data[n] = (double)src_data[n];
            }
        }
        else if (data_type == Udm_RealDouble) {
            cglong_t *dest_data = (cglong_t*)data;
            if ((*itr)->getDataType() == Udm_Integer) {
                int *src_data = (int *)((*itr)->getDataArray());
                for (n = 0; n < size; n++) dest_data[n] = (int)src_data[n];
            }
            else if ((*itr)->getDataType() == Udm_LongInteger) {
                cglong_t *src_data = (cglong_t *)((*itr)->getDataArray());
                for (n = 0; n < size; n++) dest_data[n] = (cglong_t)src_data[n];
            }
            else if ((*itr)->getDataType() == Udm_RealSingle) {
                float *src_data = (float *)((*itr)->getDataArray());
                for (n = 0; n < size; n++) dest_data[n] = (float)src_data[n];
            }
        }

        return UDM_OK;
    }

    return UDM_ERROR;
}

/**
 * ユーザ定義データを設定する.
 * @param array_name        ユーザ定義データ名
 * @param data_type            データ型
 * @param dimension            データ次元数
 * @param dim_sizes            データ次元毎のデータ数
 * @param data                ユーザ定義データ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUserDefinedDatas::setUserData(
                const std::string& array_name,
                UdmDataType_t data_type,
                int dimension,
                UdmSize_t* dim_sizes,
                void* data)
{
    if (array_name.empty()) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "array_name is empty.");
    }
    if (dimension <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "dimension is zero.");
    }
    if (dim_sizes == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "dim_sizes is null.");
    }
    if (data == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "data is null.");
    }
    // 既存ユーザ定義データを削除する
    this->removeUserData(array_name);

    // ユーザ定義データの追加
    UdmUserData *user_data = new UdmUserData(array_name, data_type, dimension, dim_sizes, data);
    if (user_data->getDataArray() == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "data is null.");
    }
    this->insertUserData(user_data);

    return UDM_OK;
}

/**
 * ユーザ定義データを追加する.
 * @param user_data        ユーザ定義データ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUserDefinedDatas::insertUserData(UdmUserData *user_data)
{
    this->user_datas.push_back(user_data);
    return UDM_OK;
}

/**
 * ユーザ定義データを削除する.
 * @param array_name        ユーザ定義データ名
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUserDefinedDatas::removeUserData(const std::string &array_name)
{
    std::vector<UdmUserData*>::iterator itr;
    for (itr=this->user_datas.begin(); itr!=this->user_datas.end(); itr++) {
        if ((*itr)->getName() != array_name) continue;

        // ユーザ定義データを削除する
        delete (*itr);
        this->user_datas.erase(itr);
        return UDM_OK;
    }

    return UDM_ERROR;
}

/**
 * ユーザ定義データをCGNSファイルから読み込む.
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUserDefinedDatas::readCgns(
                int index_file,
                int index_base,
                int index_zone)
{
    int num_array = 0;
    int i, n;
    char array_name[33];
    DataType_t cgns_datatype;
    int dimension;
    cgsize_t cgns_sizes[12];
    UdmDataType_t udm_datatype;
    UdmSize_t len = 0;
    UdmSize_t udm_sizes[12] = {0x00};

    // CGNS:UdmUserDefinedData
    if (cg_goto(index_file, index_base, "Zone_t", index_zone,
            UDM_CGNS_NAME_USERDEFINEDDATAS, 0, "end") != CG_OK) {
        return UDM_OK;        // ユーザ定義データはない
    }

    // ユーザ定義データ数
    cg_narrays(&num_array);
    if (num_array == 0) {
        return UDM_OK;
    }

    // ユーザ定義データの読込
    for (n=1; n<=num_array; n++) {
        if (cg_array_info(n, array_name, &cgns_datatype, &dimension, cgns_sizes) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_info(index_file=%d,index_base=%d,index_zone=%d).", index_file, index_base, index_zone);
        }
        udm_datatype = this->toUdmDataType(cgns_datatype);
        len = this->sizeofDataType(udm_datatype);
        for (i=0; i<dimension; i++) {
            len *= cgns_sizes[i];
            udm_sizes[i] = cgns_sizes[i];
        }
        if (len == 0) {
            continue;
        }
        char *data = new char[len];
        memset(data, 0x00, len*sizeof(char));
        if (cg_array_read(n, data) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_read(index_file=%d,index_base=%d,index_zone=%d,array_name=%s)", index_file, index_base, index_zone, array_name);
        }
        // ユーザ定義データの追加
        this->setUserData(std::string(array_name), udm_datatype, dimension, udm_sizes, data);
        delete[] data;
    }

    return UDM_OK;
}

/**
 * ユーザ定義データをCGNSファイルに出力する
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUserDefinedDatas::writeCgns(
                int index_file,
                int index_base,
                int index_zone)
{
    // 出力ユーザ定義データチェック
    if (this->user_datas.size() <= 0) {
        return UDM_OK;
    }

    // CGNS:UdmUserDefinedData
    if (cg_goto(index_file, index_base, "Zone_t", index_zone, "end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO, "index_file=%d,index_base=%d,index_zone=%d", index_file, index_base, index_zone);
    }

    // CGNS:UserDefinedData[@name="UdmUserDefinedData"]の作成 : cg_user_data_write
    if (cg_user_data_write(UDM_CGNS_NAME_USERDEFINEDDATAS) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_user_data_write(%s)", UDM_CGNS_NAME_USERDEFINEDDATAS);
    }
    if (cg_goto(index_file, index_base, "Zone_t", index_zone, UDM_CGNS_NAME_USERDEFINEDDATAS, 0, "end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO, "index_file=%d,index_base=%d,index_zone=%d", index_file, index_base, index_zone);
    }

    // ユーザ定義データの書込
    int dimension;
    cgsize_t cgns_sizes[12] = {0x00};
    int n;
    const UdmSize_t *dim_sizes;
    std::vector<UdmUserData*>::const_iterator itr;
    for (itr=this->user_datas.begin(); itr!=this->user_datas.end(); itr++) {
        // CGNS:DataArrayの作成 : cg_array_write
        dimension = (*itr)->getDataDimension();
        dim_sizes = (*itr)->getDimensionSizes();
        for (n=0; n<dimension; n++) {
            cgns_sizes[n] = dim_sizes[n];
        }
        if (cg_array_write( (*itr)->getName().c_str(),
                            this->toCgnsDataType((*itr)->getDataType()),
                            (*itr)->getDataDimension(),
                            cgns_sizes,
                            (*itr)->getDataArray()) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_write(%s)", (*itr)->getName().c_str());
        }
    }

    return UDM_OK;
}

/**
 * ユーザ定義データをCGNSリンクファイルに出力する
 * @param index_file        CGNSファイルインデックス
 * @param index_base        CGNSベースインデックス
 * @param index_zone        CGNSゾーンインデックス
 * @param link_output_path        リンク出力ファイル
 * @param linked_files            リンクファイルリスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUserDefinedDatas::writeCgnsLinkFile(
                int index_file,
                int index_base,
                int index_zone,
                const std::string &link_output_path,
                const std::vector<std::string>& linked_files)
{

    UdmError_t error = UDM_OK;
    char basename[33] = {0x00};
    char zonename[33] = {0x00};
    char array_name[33] = {0x00};
    int index_linkfile;
    std::vector<std::string>::const_iterator itr;
    char path[256] = {0x00};
    std::string filename;
    int cell_dim, phys_dim, dimension;
    cgsize_t sizes[9] = {0x00};
    cgsize_t cgns_sizes[12] = {0x00};
    DataType_t cgns_datatype;
    int n, linked_size, num_array;
    std::vector<std::string> linked_paths;
    std::vector<std::string> data_files;
    std::vector<std::string> link_names;


    // リンクファイルにCGNS:UdmUserDefinedDataが存在するかチェックする.
    filename.clear();
    linked_paths.clear();
    data_files.clear();
    link_names.clear();
    for (itr=linked_files.begin(); itr!= linked_files.end(); itr++) {
        filename = (*itr);
        // リンクファイルオープン
        if (cg_open(filename.c_str(), CG_MODE_READ, &index_linkfile) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_OPENERROR, "filename=%s, cgns_error=%s", filename.c_str(), cg_get_error());
        }
        // 出力リンクファイルと同じCGNSベースインデックス, CGNSゾーンインデックスとする.
        cg_base_read(index_linkfile, index_base, basename, &cell_dim, &phys_dim);
        cg_zone_read(index_linkfile, index_base, index_zone, zonename, sizes);

        // UdmUserDefinedDataが存在するかチェックする.
        if (cg_goto(index_linkfile, index_base, "Zone_t", index_zone,
                UDM_CGNS_NAME_USERDEFINEDDATAS, 0, "end") != CG_OK) {
            continue;        // ユーザ定義データはない
        }

        // ユーザ定義データ数
        cg_narrays(&num_array);
        if (num_array == 0) {
            continue;        // ユーザ定義データはない
        }

        // ユーザ定義データの読込
        for (n=1; n<=num_array; n++) {
            if (cg_array_info(n, array_name, &cgns_datatype, &dimension, cgns_sizes) != CG_OK) {
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_info(index_file=%d,index_base=%d,index_zone=%d).", index_file, index_base, index_zone);
            }
            sprintf(path, "/%s/%s/%s/%s", basename, zonename, UDM_CGNS_NAME_USERDEFINEDDATAS, array_name);
            linked_paths.push_back(std::string(path));
            data_files.push_back(filename);
            link_names.push_back(std::string(array_name));
        }

        cg_close(index_linkfile);
    }
    // リンク元CGNS:UdmUserDefinedData/ユーザ定義データ名称
    linked_size = linked_paths.size();
    if (linked_size <= 0) {
        // CGNS:UdmUserDefinedDataはリンクしない。
        return UDM_OK;
    }

    // CGNSリンクの作成
    if (cg_goto(index_file, index_base, "Zone_t", index_zone, UDM_CGNS_NAME_USERDEFINEDDATAS, 0, "end") != CG_OK) {
        // UdmUserDefinedData未作成
        if (cg_goto(index_file, index_base, "Zone_t", index_zone, "end") != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO, "index_file=%d,index_base=%d,index_zone=%d", index_file, index_base, index_zone);
        }
        // CGNS:UserDefinedData[@name="UdmUserDefinedData"]の作成 : cg_user_data_write
        if (cg_user_data_write(UDM_CGNS_NAME_USERDEFINEDDATAS) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_user_data_write(%s)", UDM_CGNS_NAME_USERDEFINEDDATAS);
        }
        if (cg_goto(index_file, index_base, "Zone_t", index_zone, UDM_CGNS_NAME_USERDEFINEDDATAS, 0, "end") != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO, "index_file=%d,index_base=%d,index_zone=%d", index_file, index_base, index_zone);
        }
    }

    // CGNS:UdmUserDefinedData/ユーザ定義データ名称リンクの作成
    for (n=0; n<linked_size; n++) {

        // リンクファイルをリンク出力ファイルからの相対パスに変換する.
        std::string linked_relativepath;
        this->getLinkedRelativePath(link_output_path, data_files[n], linked_relativepath);

        if (cg_link_write(link_names[n].c_str(), linked_relativepath.c_str(), linked_paths[n].c_str()) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ELEMENTS, "failure:cg_link_write(%s,%s,%s)",link_names[n].c_str(), data_files[n].c_str(), linked_paths[n].c_str());
        }
    }

    return UDM_OK;
}

/**
 * 親ゾーンを取得する.
 * @return        親ゾーン
 */
UdmZone* UdmUserDefinedDatas::getParentZone() const
{
    return this->parent_zone;
}


/**
 * 親ゾーンを設定する.
 * @param zone        親ゾーン
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUserDefinedDatas::setParentZone(UdmZone* zone)
{
    this->parent_zone = zone;
    return UDM_OK;
}

/**
 * CGNS:UserDefinedデータを結合する
 * @param dest_user_datas        結合ユーザデータ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUserDefinedDatas::joinCgnsUserDefinedDatas(const UdmUserDefinedDatas* dest_user_datas)
{

    UdmSize_t n;
    if (dest_user_datas == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "dest_user_datas is null.");
    }

    UdmSize_t num_datas = dest_user_datas->getNumUserDatas();
    for (n=1; n<=num_datas; n++) {
        UdmUserData *user_data = dest_user_datas->getUserData(n);
        UdmUserData *dest_data = new UdmUserData();
        dest_data->cloneUserData(*user_data);
        this->insertUserData(dest_data);
    }

    return UDM_OK;
}

/**
 * ユーザ定義データを取得する.
 * @param defined_id        定義ID（１～）
 * @return        ユーザ定義データ
 */
UdmUserData* UdmUserDefinedDatas::getUserData(int defined_id) const
{
    if (defined_id <= 0) return NULL;
    if (defined_id > this->user_datas.size()) return NULL;
    return this->user_datas[defined_id-1];
}

} /* namespace udm */
