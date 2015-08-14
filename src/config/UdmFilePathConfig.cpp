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
 * @file UdmFilePath.cpp
 * index.dfiファイルのFilePathデータクラスのソースファイル
 */

#include "config/UdmFilePathConfig.h"

namespace udm {

/**
 * コンストラクタ
 */
UdmFilePathConfig::UdmFilePathConfig()
{
    this->initialize();
}

/**
 * コンストラクタ:TextParser
 * @param  parser        TextParser
 */
UdmFilePathConfig::UdmFilePathConfig(TextParser *parser) : UdmConfigBase(parser)
{
    this->initialize();
}

/**
 * 初期化を行う.
 */
void UdmFilePathConfig::initialize()
{
    // デフォルトproc.dfiファイル名を設定する
    this->process = UDM_DFI_FILENAME_PROC;
}

/**
 * デストラクタ
 */
UdmFilePathConfig::~UdmFilePathConfig() {
}


/**
 * DFIファイルからFilePath要素を読みこむ.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFilePathConfig::read()
{
    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }

    std::string label, value;

    //Process : proc.dfiファイル名
    label = "/FilePath/Process";
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->process = value;
    }
    else {
        // エラー
        return UDM_ERROR_INVALID_FILEPATH_PROCESS;
    }

    return UDM_OK;
}

/**
 * FilePath要素をDFIファイルに出力する.
 * @param fp        出力ファイルポインタ
 * @param indent    出力インデント
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFilePathConfig::write(FILE *fp, unsigned int indent)
{
    std::string label, value;
    UdmError_t error;

    // FilePath : open
    label = "FilePath";
    if (this->writeLabel(fp, indent, label) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }
    indent++;

    // Process : proc.dfiファイル名
    if (!this->process.empty()) {
        label = "Process";
        this->writeNodeValue(fp, indent, label, this->process);
    }

    // FilePath : close
    if (this->writeCloseTab(fp, --indent) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }

    return UDM_OK;
}


/**
 * proc.dfiファイル名を取得する.
 * @param [out] process        proc.dfiファイル名
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFilePathConfig::getProcess(std::string& process) const
{
    process = this->process;
    return UDM_OK;
}

/**
 * proc.dfiファイル名を設定する.
 * @param process        proc.dfiファイル名
 */
void UdmFilePathConfig::setProcess(const std::string& process)
{
    this->process = process;
}

/**
 * DFIラベルパスのパラメータの設定値を取得する.
 * 数値は文字列に変換して返す。
 * 複数の設定値のあるパラメータの場合はUDM_ERRORを返す。
 * @param [in]  label_path        DFIラベルパス
 * @param [out] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmFilePathConfig::getDfiValue(const std::string &label_path, std::string &value) const
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
    if (strcasecmp(label.c_str(), UDM_DFI_FILEPATH)) {
        return UDM_ERROR;
    }

    // Process : proc.dfiファイル名
    if (!strcasecmp(name.c_str(), UDM_DFI_PROCESS)) {
        value = this->process;
    }
    else {
        return UDM_ERROR;
    }

    return UDM_OK;
}

/**
 * DFIラベルパスのパラメータの設定値数を取得する.
 * 複数の設定値のあるパラメータの場合は設定値数を返す。
 * １つのみ設定値のパラメータの場合は1を返す。
 * 設定値されていないパラメータの場合は0を返す。
 * 子ノードが存在している場合は、ノード数又はパラメータ数を返す.
 * @param [in]  label_path        DFIラベルパス
 * @param [out] value                設定値
 * @return        パラメータの設定値数
 */
int UdmFilePathConfig::getNumDfiValue(const std::string &label_path) const
{
    std::vector<std::string> labels;
    std::string label, name;
    int count;
    UdmError_t error = UDM_OK;

    // FilePath : ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 0) return 0;
    label = labels[0];
    if (strcasecmp(label.c_str(), UDM_DFI_FILEPATH)) {
        return UDM_ERROR;
    }
    // FilePathのパラメータ数を返す
    if (count == 0) {
        return 1;
    }
    name = labels[1];

    // Process : proc.dfiファイル名
    if (!strcasecmp(name.c_str(), UDM_DFI_PROCESS)) {
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
UdmError_t UdmFilePathConfig::setDfiValue(const std::string &label_path, const std::string &value)
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
    if (strcasecmp(label.c_str(), UDM_DFI_FILEPATH)) {
        return UDM_ERROR;
    }

    // Process : proc.dfiファイル名
    if (!strcasecmp(name.c_str(), UDM_DFI_PROCESS)) {
        this->process = value;
    }
    else {
        return UDM_ERROR;
    }

    return UDM_OK;
}
} /* namespace udm */
