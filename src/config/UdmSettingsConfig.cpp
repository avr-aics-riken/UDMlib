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
 * @file UdmSettingsConfig.cpp
 * udmliib.dfiファイルの入出力クラスのヘッダーファイル
 */

#include "config/UdmSettingsConfig.h"

namespace udm {

/**
 * コンストラクタ
 */
UdmSettingsConfig::UdmSettingsConfig() : UdmConfigBase(NULL)
{
    this->initialize();
}

/**
 * コンストラクタ:TextParser
 * @param  parser        TextParser
 */
UdmSettingsConfig::UdmSettingsConfig(TextParser *parser) : UdmConfigBase(parser)
{
    this->initialize();
}

/**
 * デストラクタ
 */
UdmSettingsConfig::~UdmSettingsConfig()
{
    this->zoltan_parameters.clear();
}


/**
 * 初期化を行う.
 */
void UdmSettingsConfig::initialize()
{
    this->debug_level = 0;
    this->mxm_partition = udm_disable;
    this->clearZoltanParameters();
    this->filename_udm_dfi = std::string();

    // Zoltanデフォルトパラメータの設定
    this->setZoltanParameter("LB_METHOD", "HYPERGRAPH");
    this->setZoltanParameter("GRAPH_PACKAGE", "PHG");
    this->setZoltanParameter("LB_APPROACH", ZOLTAN_DEFAULT_LB_APPROACH);
    char param_value[32] = {0x00};
    sprintf(param_value, "%d", ZOLTAN_DEFAULT_DEBUG_LEVEL);
    this->setZoltanParameter("DEBUG_LEVEL", param_value);
    this->setDebugLevel(ZOLTAN_DEFAULT_DEBUG_LEVEL);
}

/**
 * udm.dfiファイルからUDMlib動作設定、分割パラメータの読込を行う.
 * @param filename        udm.dfiファイル
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSettingsConfig::read(const char* filename)
{
    UdmError_t ierror = UDM_OK;
    std::string label, value;
    FILE*fp = NULL;

    if( !(fp=fopen(filename, "rb")) ) {
        printf("Can't open file. (%s)\n", filename);
        return UDM_ERROR_READ_INDEXFILE_OPENERROR;
    }
    fclose(fp);

    // 初期化を行う.
    this->initialize();

    // udmlib.tpファイル名
    this->filename_udm_dfi = filename;

    // TextParser
    TextParser *parser = new TextParser();
    TextParserError parser_ret = parser->read(filename);
    if (parser_ret != TP_NO_ERROR) {
        delete parser;
        return UDM_ERROR_TEXTPARSER;
    }
    this->setTextParser(parser);

    ierror = UDM_OK;
    std::vector<std::string> param_labels;
    label = "UDMlib";

    // UDMlib/partition
    label = "/UDMlib/partition";
    if (this->getLabels(label, param_labels) != UDM_OK) {
        std::vector<std::string>::iterator itr;
        for (itr = param_labels.begin(); itr != param_labels.end(); itr++) {
            // DEBUG_LEVEL
            if ( !strcasecmp((*itr).c_str(), UDM_DFI_DEBUG_LEVEL) ) {
                if ( this->getValue((*itr), value ) == UDM_OK ) {
                    this->debug_level = this->convertLong(value, &ierror);
                }
            }
            // MxM_PARTITION
            else if ( !strcasecmp((*itr).c_str(), UDM_DFI_MxM_PARTITION) ) {
                if ( this->getValue((*itr), value ) == UDM_OK ) {
                    if (this->convertBool(value, &ierror)) this->mxm_partition = udm_enable;
                    else  this->mxm_partition = udm_disable;
                }
            }
            else {
                if ( this->getValue((*itr), value ) == UDM_OK ) {
                    this->setZoltanParameter((*itr), value);
                }
            }
        }
    }

    delete parser;
    parser = NULL;

    if (ierror > 0) {
        return UDM_ERROR;
    }
    return UDM_OK;

}


/**
 * UDMlib動作設定、分割パラメータをudm.dfiファイルへ出力する.
 * @param filename        udm.dfiファイル
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSettingsConfig::write(const char* filename)
{
    std::string label, value, buf;
    UdmError_t error;
    FILE* fp = NULL;
    char str[128];
    unsigned int indent = 0;
    char dirname[256] = {0x00};

    if (filename == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "filename is null.");
    }

    udm_get_dirname(filename, dirname);
    if (strlen(dirname) > 0) {
        if (udm_make_directories(dirname) != 0) {
            return UDM_ERROR_HANDLER(UDM_ERROR, "make directory(%s).", dirname);
        }
    }

    if( !(fp = fopen(filename, "w")) ) {
        return UDM_ERROR_HANDLER(UDM_ERROR_WRITE_INDEXFILE_OPENERROR, "filename=%s.", filename);
    }

    // ヘッダー出力
    this->writeText(fp, 0, UDM_CGNS_TEXT_UDMINFO_DESCRIPTION);

    // UDMlib : open
    label = UDM_DFI_UDMLIB;
    if (this->writeLabel(fp, indent, label) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_WRITE_DFIFILE_WRITENODE, "filename=%s.", filename);
    }
    indent++;

    // PARTITION : open
    label = UDM_DFI_PARTITION;
    if (this->writeLabel(fp, indent, label) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }
    indent++;

    // debug_lebel
    label = UDM_DFI_DEBUG_LEVEL;
    sprintf(str, "%d", this->debug_level);
    value = std::string(str);
    this->writeNodeValue(fp, indent, label, value);

    // MxM_PARTITION:分割数が同じ場合の分割実行
    label = UDM_DFI_MxM_PARTITION;
    error = this->toString(this->mxm_partition, value);
    this->writeNodeValue(fp, indent, label, value);

    // Zoltanパラメータ
    std::map<std::string, std::string>::const_iterator itr;
    for (itr = this->zoltan_parameters.begin(); itr != this->zoltan_parameters.end(); itr++) {
        label = itr->first;
        value = itr->second;
        this->writeNodeValue(fp, indent, label, value);
    }

    // PARTITION : close
    if (this->writeCloseTab(fp, --indent) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_WRITE_DFIFILE_WRITENODE, "filename=%s.", filename);
    }
    // FileInfo : close
    if (this->writeCloseTab(fp, --indent) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_WRITE_DFIFILE_WRITENODE, "filename=%s.", filename);
    }

    fclose(fp);

    return UDM_OK;
}


/**
 * udm.dfiファイル名を取得する.
 * read(filename)に渡されたファイル名を返す.
 * @param [out] filepath        udm.dfiファイル名
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmSettingsConfig::getUdmDfiPath(std::string &filepath) const
{
    filepath = this->filename_udm_dfi;
    return UDM_OK;
}

/**
 * デバッグレベルを取得する.
 * @return        デバッグレベル
 */
int UdmSettingsConfig::getDebugLevel() const
{
    return this->debug_level;
}

/**
 * デバッグレベルを設定する.
 * @param value        デバッグレベル
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSettingsConfig::setDebugLevel(int value)
{
    this->debug_level = value;
    return UDM_OK;
}

/**
 * MxM_PARTITIONパラメータの設定値を取得する.
 * MxM_PARTITION = 分割数が同じ場合の分割実行設定
 * @return    true=分割数が同じ場合の分割実行を行う.
 */
bool UdmSettingsConfig::isMxMPartition() const
{
    return (this->mxm_partition == udm_enable);
}

/**
 * MxM_PARTITIONパラメータの設定値を設定する.
 * MxM_PARTITION = 分割数が同じ場合の分割実行設定
 * @param mxm_partition        true=分割数が同じ場合の分割実行を行う.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSettingsConfig::setMxMPartition(bool mxm_partition)
{
    if (mxm_partition) {
        this->mxm_partition = udm_enable;
    }

    return UDM_OK;
}

/**
 * Zoltanパラメータを取得する
 * @param name        Zoltanパラメータ名
 * @param value        Zoltanパラメータ設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSettingsConfig::getZoltanParameter(const std::string &name, std::string &value) const
{
    if (this->zoltan_parameters.size() <= 0) return UDM_ERROR;
    std::map<std::string, std::string>::const_iterator  itr;
    itr = this->zoltan_parameters.find(name);
    if (itr != this->zoltan_parameters.end()) {
        value = (*itr).second;
        return UDM_OK;
    }
    return UDM_ERROR;
}


/**
 * Zoltanパラメータを設定する.
 * Zoltanパラメータ名が設定済みであれば置き換える.
 * Zoltanパラメータ名が存在しなければ追加する.
 * @param name        Zoltanパラメータ名
 * @param value        Zoltanパラメータ設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSettingsConfig::setZoltanParameter(const std::string &name, const std::string &value)
{
    std::map<std::string, std::string>::iterator  itr;
    itr = this->zoltan_parameters.find(name);
    if (itr != this->zoltan_parameters.end()) {
        (*itr).second = value;
        return UDM_OK;
    }
    else {
        this->zoltan_parameters.insert(std::make_pair(name, value));
    }
    return UDM_OK;

}

/**
 * Zoltanパラメータを削除する.
 * "MxM_PARTITION"を指定した場合は、setMxMPartition(false)と同じとする。
 * @param param_name        Zoltanパラメータ名
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSettingsConfig::removeZoltanParameter(const std::string& param_name)
{
    if (param_name.empty()) return UDM_ERROR;
    if (!strcasecmp(param_name.c_str(), UDM_DFI_MxM_PARTITION)) {
        this->setMxMPartition(false);
        return UDM_OK;
    }

    std::map<std::string, std::string>::iterator  itr;
    itr = this->zoltan_parameters.find(param_name);
    if (itr != this->zoltan_parameters.end()) {
        this->zoltan_parameters.erase(itr);
        return UDM_OK;
    }
    else {
        return UDM_ERROR;
    }
}

/**
 * Zoltanパラメータをすべて削除する.
 * setMxMPartition(false)も行う。
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSettingsConfig::clearZoltanParameters()
{
    this->zoltan_parameters.clear();
    return UDM_OK;
}

/**
 * DFIラベルパスのパラメータの設定値を取得する.
 * 数値は文字列に変換して返す。
 * 複数の設定値のあるパラメータの場合はUDM_ERRORを返す。
 * @param [in]  label_path        DFIラベルパス
 * @param [out] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmSettingsConfig::getDfiValue(const std::string &label_path, std::string &value) const
{
    std::vector<std::string> labels;
    std::string label, param_name, name;
    int count;
    UdmError_t error = UDM_OK;

    // UDMlib
    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 1) return UDM_ERROR;
    label = labels[0];
    if (strcasecmp(label.c_str(), UDM_DFI_UDMLIB)) {
        return UDM_ERROR;
    }

    if (count == 2) {
        // debug_lebel
        name = labels[1];
        if (!strcasecmp(name.c_str(), UDM_DFI_DEBUG_LEVEL)) {
            error = this->toString((long long)this->debug_level, value);
            return error;
        }
    }
    else if (count == 3) {
        param_name = labels[1];
        name = labels[2];

        // MxM_PARTITION:分割数が同じ場合の分割実行
        if (!strcasecmp(name.c_str(), UDM_DFI_MxM_PARTITION)) {
            error = this->toString(this->mxm_partition, value);
            return error;
        }

        // Zoltanパラメータ
        std::map<std::string, std::string>::const_iterator itr;
        itr = this->zoltan_parameters.find(name);
        if (itr != this->zoltan_parameters.end()) {
            value = itr->second;
            return UDM_OK;
        }
    }

    return UDM_ERROR;
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
int UdmSettingsConfig::getNumDfiValue(const std::string &label_path) const
{
    std::vector<std::string> labels;
    std::string label, param_name, name;
    int count;
    UdmError_t error = UDM_OK;

    // UDMlib : ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 0) return 0;
    label = labels[0];
    if (strcasecmp(label.c_str(), UDM_DFI_UDMLIB)) {
        return 0;
    }
    // UDMlibのパラメータ数を返す
    if (count == 1) {
        return 2 + this->zoltan_parameters.size();
    }
    else if (count == 2) {
        // debug_lebel
        name = labels[1];
        if (!strcasecmp(name.c_str(), UDM_DFI_DEBUG_LEVEL)) {
            return 1;
        }
    }
    else if (count == 3) {
        param_name = labels[1];
        name = labels[2];

        // MxM_PARTITION:分割数が同じ場合の分割実行
        if (!strcasecmp(name.c_str(), UDM_DFI_MxM_PARTITION)) {
            return 1;
        }

        // Zoltanパラメータ
        std::map<std::string, std::string>::const_iterator itr;
        itr = this->zoltan_parameters.find(name);
        if (itr != this->zoltan_parameters.end()) {
            return 1;
        }
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
UdmError_t UdmSettingsConfig::setDfiValue(const std::string &label_path, const std::string &value)
{
    std::vector<std::string> labels;
    std::string label, param_name, name;
    int count;
    UdmError_t error = UDM_OK;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count != 2 && count != 3) return UDM_ERROR;
    label = labels[0];
    name = labels[1];
    if (strcasecmp(label.c_str(), UDM_DFI_UDMLIB)) {
        return UDM_ERROR;
    }

    if (count == 2) {
        // debug_lebel
        name = labels[1];
        if (!strcasecmp(name.c_str(), UDM_DFI_DEBUG_LEVEL)) {
            this->debug_level = this->convertLong(value, &error);
            return error;
        }
    }
    else if (count == 3) {
        param_name = labels[1];
        name = labels[2];

        // MxM_PARTITION:分割数が同じ場合の分割実行
        if (!strcasecmp(name.c_str(), UDM_DFI_MxM_PARTITION)) {
            if (this->convertBool(value, &error)) this->mxm_partition = udm_enable;
            else  this->mxm_partition = udm_disable;
            return error;
        }

        // Zoltanパラメータ
        return setZoltanParameter(name, value);
    }

    return UDM_ERROR;
}

/**
 * UDMlibのDFIパラメータを取得する.
 * @param [in] name        パラメータ名
 * @param [out] value        設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmSettingsConfig::getParameter(const std::string& name, std::string& value) const
{
    char label_path[128] = {0x00};
    if (!strcasecmp(name.c_str(), UDM_DFI_DEBUG_LEVEL)) {
        sprintf(label_path, "/%s/%s", UDM_DFI_UDMLIB, name.c_str());
    }
    else {
        sprintf(label_path, "/%s/%s/%s", UDM_DFI_UDMLIB, UDM_DFI_PARTITION, name.c_str());
    }
    return this->getDfiValue(std::string(label_path), value);
}

/**
 * UDMlibのDFIパラメータを設定する.
 * @param [in] name        パラメータ名
 * @param [in] value        設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmSettingsConfig::setParameter(const std::string& name, const std::string& value)
{
    char label_path[128] = {0x00};
    if (!strcasecmp(name.c_str(), UDM_DFI_DEBUG_LEVEL)) {
        sprintf(label_path, "/%s/%s", UDM_DFI_UDMLIB, name.c_str());
    }
    else {
        sprintf(label_path, "/%s/%s/%s", UDM_DFI_UDMLIB, UDM_DFI_PARTITION, name.c_str());
    }
    return this->setDfiValue(std::string(label_path), value);
}

/**
 * UDMlibのDFIパラメータを削除、又はデフォルト値とする.
 * @param [in] name        パラメータ名
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmSettingsConfig::removeParameter(const std::string& name)
{
    if (!strcasecmp(name.c_str(), UDM_DFI_DEBUG_LEVEL)) {
        this->debug_level = 0;
        return UDM_OK;
    }
    else {
        return this->removeZoltanParameter(name);
    }
}

/**
 * Zoltanパラメータ数を取得する.
 * @return        Zoltanパラメータ数
 */
int UdmSettingsConfig::getNumZoltanParameters() const
{
    return this->zoltan_parameters.size();
}

/**
 * ZoltanパラメータIDのZoltanパラメータ名、設定値を取得する.
 * @param param_id    ZoltanパラメータID
 * @param name        Zoltanパラメータ名
 * @param value        Zoltanパラメータ設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmSettingsConfig::getZoltanParameterById(int param_id, std::string& name, std::string& value) const
{
    if (param_id <= 0) return UDM_ERROR_INVALID_PARAMETERS;
    if (param_id > this->getNumZoltanParameters()) return UDM_ERROR_INVALID_PARAMETERS;

    std::map<std::string, std::string>::const_iterator itr = this->zoltan_parameters.begin();
    std::advance( itr, param_id-1 );            // イテレータの移動
    name = itr->first;
    value = itr->second;

    return UDM_OK;

}
} /* namespace udm */
