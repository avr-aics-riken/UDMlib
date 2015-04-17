/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmMpiConfig.cpp
 * proc.dfiファイルのMPIデータクラスのソースファイル
 */

#include "config/UdmMpiConfig.h"

namespace udm {

/**
 * コンストラクタ
 */
UdmMpiConfig::UdmMpiConfig() : UdmConfigBase(NULL)
{
    this->initialize();
}


/**
 * コンストラクタ:TextParser
 * @param  parser        TextParser
 */
UdmMpiConfig::UdmMpiConfig(TextParser *parser) : UdmConfigBase(parser)
{
    this->initialize();
}

/**
 * デストラクタ
 */
UdmMpiConfig::~UdmMpiConfig()
{

}


/**
 * 初期化を行う.
 */
void UdmMpiConfig::initialize()
{
    this->number_group = 0;
}


/**
 * DFIファイルからMPI要素を読みこむ.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmMpiConfig::read()
{
    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }

    std::string label, value;
    UdmError_t ierror;

    // MPI/NumberOfRank
    label = "/MPI/NumberOfRank";
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->number_rank = this->convertLong(value, &ierror);
    }
    else {
        // エラー
        return UDM_ERROR_INVALID_NUMBEROFRANK;
    }

    // MPI/NumberOfGroup
    label = "/MPI/NumberOfGroup";
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->number_group = this->convertLong(value, &ierror);
    }

    return UDM_OK;
}

/**
 * MPI要素をDFIファイルに出力する.
 * @param fp        出力ファイルポインタ
 * @param indent    出力インデント
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmMpiConfig::write(FILE *fp, unsigned int indent)
{
    std::string label, value, buf;

    // MPI : open
    label = "MPI";
    if (this->writeLabel(fp, indent, label) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }
    indent++;

    // MPI/NumberOfRank
    label = "NumberOfRank";
    this->writeNodeValue(fp, indent, label, this->number_rank);

    // MPI/NumberOfGroup
    label = "NumberOfGroup";
    this->writeNodeValue(fp, indent, label, this->number_group);

    // MPI : close
    if (this->writeCloseTab(fp, --indent) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }

    return UDM_OK;
}

/**
 * 並列実行グループ数を取得する.
 * @return        並列実行グループ数
 */
int UdmMpiConfig::getNumberGroup() const
{
    return this->number_group;
}

/**
 * 並列実行グループ数を設定する.
 * @param number_group        並列実行グループ数
 */
void UdmMpiConfig::setNumberGroup(int number_group)
{
    this->number_group = number_group;
}

/**
 * 並列実行数を取得する.
 * @return        並列実行数
 */
int UdmMpiConfig::getNumberRank() const
{
    return this->number_rank;
}

/**
 * 並列実行数を設定する.
 * @param number_rank        並列実行数
 */
void UdmMpiConfig::setNumberRank(int number_rank)
{
    this->number_rank = number_rank;
}


/**
 * DFIラベルパスのパラメータの設定値を取得する.
 * 数値は文字列に変換して返す。
 * 複数の設定値のあるパラメータの場合はUDM_ERRORを返す。
 * @param [in]  label_path        DFIラベルパス
 * @param [out] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmMpiConfig::getDfiValue(const std::string &label_path, std::string &value) const
{
    std::vector<std::string> labels;
    std::string label, name;
    int count;
    UdmError_t error = UDM_OK;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 1) return UDM_ERROR;
    label = labels[0];
    name = labels[1];
    if (strcasecmp(label.c_str(), UDM_DFI_MPI)) {
        return UDM_ERROR;
    }

    // MPI/NumberOfRank
    if (!strcasecmp(name.c_str(), UDM_DFI_NUMBEROFRANK)) {
        error = this->toString((long long)this->number_rank, value);
        return error;
    }
    // MPI/NumberOfGroup
    else if (!strcasecmp(name.c_str(), UDM_DFI_NUMBEROFGROUP)) {
        error = this->toString((long long)this->number_group, value);
        return error;
    }
    else {
        return UDM_ERROR;
    }
}

/**
 * DFIラベルパスのパラメータの設定値数を取得する.
 * 複数の設定値のあるパラメータの場合は設定値数を返す。
 * １つのみ設定値のパラメータの場合は1を返す。
 * 設定値されていないパラメータの場合は0を返す。
 * 子ノードが存在している場合は、ノード数又はパラメータ数を返す.
 * @param [in]  label_path        DFIラベルパス
 * @return        パラメータの設定値数
 */
int UdmMpiConfig::getNumDfiValue(const std::string &label_path) const
{
    std::vector<std::string> labels;
    std::string label, name;
    int count;

    // MPI : ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 0) return 0;
    label = labels[0];
    if (strcasecmp(label.c_str(), UDM_DFI_MPI)) {
        return 0;
    }
    // MPIのパラメータ数を返す
    if (count == 1) {
        return 2;
    }
    else if (count != 2) {
        return 0;
    }
    name = labels[1];

    // MPI/NumberOfRank
    if (!strcasecmp(name.c_str(), UDM_DFI_NUMBEROFRANK)) {
        return 1;
    }
    // MPI/NumberOfGroup
    else if (!strcasecmp(name.c_str(), UDM_DFI_NUMBEROFGROUP)) {
        return 1;
    }

    return 0;
}

/**
 * DFIラベルパスのパラメータの設定値を設定する.
 * 数値は文字列から変換して設定する。
 * 複数の設定値を持つことの可能なパラメータの場合は１つのみ設定する.
 * @param [in]  label_path        DFIラベルパス
 * @param [in] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmMpiConfig::setDfiValue(const std::string &label_path, const std::string &value)
{
    std::vector<std::string> labels;
    std::string label, name;
    int count;
    UdmError_t error = UDM_OK;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count != 2) return UDM_ERROR;
    label = labels[0];
    name = labels[1];
    if (strcasecmp(label.c_str(), UDM_DFI_MPI)) {
        return UDM_ERROR;
    }

    // MPI/NumberOfRank
    if (!strcasecmp(name.c_str(), UDM_DFI_NUMBEROFRANK)) {
        this->number_rank = (int)this->convertLong(value, &error);
        return error;
    }
    // MPI/NumberOfGroup
    else if (!strcasecmp(name.c_str(), UDM_DFI_NUMBEROFGROUP)) {
        this->number_group = (int)this->convertLong(value, &error);
        return error;
    }

    return UDM_ERROR;
}

} /* namespace udm */
