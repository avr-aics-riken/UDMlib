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

#ifndef _UDMUSERDEFINEDDATAS_H_
#define _UDMUSERDEFINEDDATAS_H_

/**
 * @file UdmUserDefinedDatas.h
 * ユーザ定義データクラスのヘッダーファイル
 */

#include "model/UdmGeneral.h"

namespace udm
{
class UdmZone;

/**
 * ユーザ定義データクラス
 */
class UdmUserData: public UdmGeneral
{
private:
    int data_dimension;                ///< データ次元数
    UdmSize_t dimension_sizes[12];        ///< データ次元毎のデータ数
    void *data_array;                ///< ユーザ定義データ

public:
    /**
     * コンストラクタ
     */
    UdmUserData() : UdmGeneral()
    {
        this->initialize();
    };

    /**
     * コンストラクタ
     * @param array_name        ユーザ定義データ名
     * @param data_type            データ型
     * @param dimension            データ次元数
     * @param dim_sizes            データ次元毎のデータ数
     * @param data                ユーザ定義データ
     */
    UdmUserData(    const std::string &array_name,
                    UdmDataType_t data_type,
                    int dimension,
                    UdmSize_t *dim_sizes,
                    void *data) : UdmGeneral()
    {
        this->initialize();

        if (array_name.empty()) return;
        if (dimension <= 0) return;
        if (dim_sizes == NULL) return;
        if (data == NULL) return;

        this->setName(array_name);
        this->setDataType(data_type);
        this->data_dimension = dimension;
        memcpy(this->dimension_sizes, dim_sizes, dimension*sizeof(UdmSize_t));

        int i;
        UdmSize_t len = this->sizeofDataType(data_type);
        for (i=0; i<dimension; i++) {
            len *= dim_sizes[i];
        }
        if (len > 0) {
            this->data_array = new unsigned char[len];
            memcpy(this->data_array, data, len);
        }
    }

    /**
     * デストラクタ
     */
    virtual ~UdmUserData()
    {
        if (this->data_array != NULL) {
            delete []((unsigned char*)this->data_array);
            this->data_array = NULL;
        }
    }

    /**
     * ユーザ定義データを取得する.
     */
    void* getDataArray() const
    {
        return this->data_array;
    }

    /**
     * ユーザ定義データを設定する.
     * @param data        ユーザ定義データ
     */
    void setDataArray(void* data)
    {
        this->data_array = data;
    }

    /**
     * データ次元数を取得する.
     * @return        データ次元数
     */
    int getDataDimension() const
    {
        return this->data_dimension;
    }

    /**
     * データ次元数を設定する.
     * @param dimension        データ次元数
     */
    void setDataDimension(int dimension)
    {
        this->data_dimension = dimension;
    }

    /**
     * データ次元毎のデータ数を取得する.
     * @return        データ次元毎のデータ数
     */
    const UdmSize_t* getDimensionSizes() const
    {
        return this->dimension_sizes;
    }

    /**
     * データ次元毎のデータ数を設定する.
     * @param dimension        データ次元数
     * @param dim_sizes        データ次元毎のデータ数
     */
    void setDimensionSizes(int dimension, UdmSize_t* dim_sizes)
    {
        memcpy(this->dimension_sizes, dim_sizes, dimension*sizeof(UdmSize_t));
    }

    /**
     * ユーザ定義データサイズを取得する.
     * @return        ユーザ定義データサイズ
     */
    UdmSize_t getDataArraySize() const
    {
        int n;
        UdmSize_t size = 1;
        if (this->dimension_sizes == NULL) return 0;
        if (this->data_dimension <= 0) return 0;

        for (n=0; n<this->data_dimension; n++) {
            size *= this->dimension_sizes[n];
        }
        return size;
    }
    /**
     * ユーザ定義データのコピーを行う.
     * @param src        コピー元ユーザ定義データ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
     */
    UdmError_t cloneUserData(const UdmUserData& src)
    {
        this->data_dimension = src.data_dimension;                ///< データ次元数
        memcpy(this->dimension_sizes, src.dimension_sizes, sizeof(UdmSize_t)*12);        ///< データ次元毎のデータ数
        UdmDataType_t data_type = this->getDataType();
        UdmSize_t len = this->sizeofDataType(data_type);
        len *= this->getDataArraySize();
        if (len <= 0) return UDM_ERROR;
        memcpy(this->data_array, src.data_array, len);                ///< ユーザ定義データ

        return UDM_OK;
    }

private:
    /**
     * 初期化を行う.
     */
    void initialize()
    {
        this->data_dimension = 0;
        memset(this->dimension_sizes, 0x00, 12*sizeof(UdmSize_t));
        this->data_array = NULL;
    }
};


/**
 * ユーザ定義データ管理クラス
 */
class UdmUserDefinedDatas: public UdmGeneral
{
private:
    std::vector<UdmUserData*> user_datas;        ///< ユーザ定義データリスト

    /**
     * 親ゾーン
     */
    UdmZone *parent_zone;

public:
    UdmUserDefinedDatas();
    UdmUserDefinedDatas(UdmZone* zone);
    virtual ~UdmUserDefinedDatas();
    int getNumUserDatas() const;
    const std::string& getUserDataName(int defined_id) const;
    UdmError_t getUserDataInfo(
                    const std::string &array_name,
                    UdmDataType_t &data_type,
                    int &dimension,
                    UdmSize_t *dim_sizes ) const;
    UdmError_t getUserDataArray(
                    const std::string &array_name,
                    UdmDataType_t data_type,
                    void *data) const;
    UdmError_t setUserData(
                    const std::string &array_name,
                    UdmDataType_t data_type,
                    int dimension,
                    UdmSize_t *dim_sizes,
                    void *data);
    UdmError_t removeUserData(const std::string &array_name);

    // CGNSアクセス
    UdmError_t readCgns(int index_file, int index_base, int index_zone);
    UdmError_t writeCgns(int index_file, int index_base, int index_zone);
    UdmError_t writeCgnsLinkFile(int index_file, int index_base, int index_zone, const std::string &link_output_path, const std::vector<std::string> &linked_files);
    UdmError_t joinCgnsUserDefinedDatas(const UdmUserDefinedDatas *dest_user_datas);

    // UdmZone
    UdmZone* getParentZone() const;
    UdmError_t setParentZone(UdmZone *zone);

private:
    void initialize();
    UdmError_t insertUserData(UdmUserData *user_data);
    UdmUserData* getUserData(int defined_id) const;
};

} /* namespace udm */

#endif /* _UDMUSERDEFINEDDATAS_H_ */
