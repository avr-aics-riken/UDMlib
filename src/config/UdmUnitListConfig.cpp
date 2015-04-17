/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmUnitList.cpp
 * index.dfiファイルのUnitListデータのソースファイル.
 * UdmUnitConfig, UdmUnitListConfigクラスのソースファイル
 */

#include "TextParserTree.h"
#include "config/UdmUnitListConfig.h"

namespace udm {


/*******************************************************
 * class UdmUnitConfig
 ******************************************************/

/**
 * コンストラクタ
 */
UdmUnitConfig::UdmUnitConfig(): UdmConfigBase(NULL), reference(NULL), difference(NULL)
{
}

/**
 * コンストラクタ:TextParser
 * @param  parser        TextParser
 */
UdmUnitConfig::UdmUnitConfig(TextParser *parser) : UdmConfigBase(parser), reference(NULL), difference(NULL)
{
    this->initialize();
}


/**
 * コピーコンストラクタ
 * @param  src_unit_config        UdmUnitConfig
 */
UdmUnitConfig::UdmUnitConfig(const UdmUnitConfig &src_unit_config) : reference(NULL), difference(NULL)
{
    this->initialize();

    this->setUnitName(src_unit_config.unit_name);        ///< 単位系名称
    this->setUnit(src_unit_config.unit);            ///< 単位
    if (src_unit_config.reference != NULL) {
        this->reference = new UdmDfiValue<float>(*src_unit_config.reference);        ///< 基準値（実数）
    }
    if (src_unit_config.difference != NULL) {
        this->difference = new UdmDfiValue<float>(*src_unit_config.difference);    ///< 差分値（実数）
    }
}

/**
 * コンストラクタ:単位系名称、単位、基準値
 * @param unit_name        単位系名称
 * @param unit            単位
 * @param reference        基準値
 */
UdmUnitConfig::UdmUnitConfig(const std::string &unit_name, const std::string &unit, float reference) : reference(NULL), difference(NULL)
{
    this->initialize();

    this->setUnitName(unit_name);
    this->setUnit(unit);
    this->setReference(reference);
}

/**
 * コンストラクタ:単位系名称、単位、基準値、差分値
 * @param unit_name        単位系名称
 * @param unit            単位
 * @param reference        基準値
 * @param difference    差分値
 */
UdmUnitConfig::UdmUnitConfig(const std::string &unit_name, const std::string &unit, float reference, float difference) : reference(NULL), difference(NULL)
{
    this->initialize();

    this->setUnitName(unit_name);
    this->setUnit(unit);
    this->setReference(reference);
    this->setDifference(difference);
}


/**
 * 初期化を行う.
 */
void UdmUnitConfig::initialize()
{
    this->unit_name.clear();
    this->unit.clear();
    this->reference = NULL;
    this->difference = NULL;
}

/**
 * デストラクタ
 */
UdmUnitConfig::~UdmUnitConfig()
{
    if (this->reference != NULL) delete this->reference;
    if (this->difference != NULL) delete this->difference;
}

/**
 * DFIファイルから/UnitList/[単位系名称]要素を読みこむ.
 * @param     unit_label         単位系名称
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUnitConfig::read(const std::string& unit_label)
{
    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }

    std::string label, value, unit_name, label_leaf;
    UdmError_t ierror = UDM_OK;
    std::vector<std::string> child_list;

    // UnitList/[単位系名称]
    std::vector<std::string> path_list;
    int count = this->split(unit_label, '/', path_list);
    if (count != 2) {
        return UDM_ERROR_INVALID_DFI_ELEMENTPATH;
    }
    if (strcasecmp(path_list[0].c_str(), UDM_DFI_UNITLIST)) {
        return UDM_ERROR_INVALID_DFI_ELEMENTPATH;
    }
    this->unit_name = path_list[1];

    // UnitList/[単位系名称]/Unit
    label = unit_label + "/" + UDM_DFI_UNIT;
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->unit = value;
    }
    else {
        // エラー
        return UDM_ERROR_INVALID_UNITLIST_UNIT;
    }

    // UnitList/[単位系名称]/Reference
    label = unit_label + "/" + UDM_DFI_REFERENCE;
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->reference = new UdmDfiValue<float>(UDM_DFI_REFERENCE, this->convertDouble(value, &ierror));
    }

    // UnitList/[単位系名称]/Difference
    label = unit_label + "/" + UDM_DFI_DIFFERENCE;
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->difference = new UdmDfiValue<float>(UDM_DFI_DIFFERENCE, this->convertDouble(value, &ierror));
    }

    return UDM_OK;
}

/**
 * UnitList/[単位系名称]要素をDFIファイルに出力する.
 * @param fp        出力ファイルポインタ
 * @param indent    出力インデント
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUnitConfig::write(FILE *fp, unsigned int indent)
{
    std::string label, value;
    UdmError_t error;

    // UnitList : open
    label = this->unit_name;
    if (this->writeLabel(fp, indent, label) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }
    indent++;

    // UnitList/[単位系名称]/Unit
    label = UDM_DFI_UNIT;
    this->writeNodeValue(fp, indent, label, this->unit );

    // UnitList/[単位系名称]/Reference
    if (this->reference != NULL && this->reference->isSetValue()) {
        label = UDM_DFI_REFERENCE;
        this->writeNodeValue(fp, indent, label, this->reference->getValue() );
    }

    // UnitList/[単位系名称]/Difference
    if (this->difference != NULL && this->difference->isSetValue()) {
        label = UDM_DFI_DIFFERENCE;
        this->writeNodeValue(fp, indent, label, this->difference->getValue() );
    }

    // UnitList : close
    if (this->writeCloseTab(fp, --indent) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }

    return UDM_OK;
}

/**
 * 単位系名称を取得する.
 * @return        単位系名称
 */
const std::string& UdmUnitConfig::getUnitName() const
{
    return this->unit_name;
}


/**
 * 単位を取得する.
 * @return        単位
 */
const std::string& UdmUnitConfig::getUnit() const
{
    return this->unit;
}


/**
 * 基準値を取得する.
 * @return        基準値
 */
float UdmUnitConfig::getReference() const
{
    if (this->reference == NULL) return 0.0;
    return this->reference->getValue();
}

/**
 * 基準値がセットされているかチェックする.
 * @return        true=基準値がセットされている.
 */
bool UdmUnitConfig::isSetReference() const
{
    return (this->reference != NULL);
}


/**
 * 差分値を取得する.
 * @return        差分値
 */
float UdmUnitConfig::getDifference() const
{
    if (this->difference == NULL) return 0.0;
    return this->difference->getValue();
}

/**
 * 差分値を設定する.
 * @param value        差分値
 */
void UdmUnitConfig::setDifference(float value)
{
    if (this->difference == NULL) {
        this->difference = new UdmDfiValue<float>(UDM_DFI_DIFFERENCE, value);
    }
    else {
        this->difference->setValue(value);
    }
}

/**
 * 差分値を削除する.
 * 差分値（difference）をdeleteする.
 */
void UdmUnitConfig::unsetDifference()
{
    if (this->difference != NULL) {
        delete this->difference;
        this->difference = NULL;
    }
}


/**
 * 基準値を設定する.
 * @param value        基準値
 */
void UdmUnitConfig::setReference(float value)
{
    if (this->reference == NULL) {
        this->reference = new UdmDfiValue<float>(UDM_DFI_REFERENCE, value);
    }
    else {
        this->reference->setValue(value);
    }
}

/**
 * 基準値を削除する.
 * 基準値reference）をdeleteする.
 */
void UdmUnitConfig::unsetReference()
{
    if (this->reference != NULL) {
        delete this->reference;
        this->reference = NULL;
    }
}


/**
 * 単位を設定する.
 * @param unit        単位
 */
void UdmUnitConfig::setUnit(const std::string& unit)
{
    this->unit = unit;
}

/**
 * 単位系名称を設定する.
 * @param unit_name        単位系名称
 */
void UdmUnitConfig::setUnitName(const std::string& unit_name)
{
    this->unit_name = unit_name;
}

/**
 * 差分値がセットされているかチェックする.
 * @return        true=差分値がセットされている.
 */
bool UdmUnitConfig::isSetDifference() const
{
    return (this->difference != NULL);
}


/**
 * DFIラベルパスのパラメータの設定値を取得する.
 * 数値は文字列に変換して返す。
 * 複数の設定値のあるパラメータの場合はUDM_ERRORを返す。
 * @param [in]  label_path        DFIラベルパス
 * @param [out] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmUnitConfig::getDfiValue(const std::string &label_path, std::string &value) const
{
    std::vector<std::string> labels;
    std::string label, unit_name, name;
    int count;
    UdmError_t error = UDM_OK;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count != 3) return UDM_ERROR;
    label = labels[0];
    unit_name = labels[1];
    name = labels[2];
    if (strcasecmp(label.c_str(), UDM_DFI_UNITLIST)) {
        return UDM_ERROR;
    }
    if (strcasecmp(unit_name.c_str(), this->unit_name.c_str())) {
        return UDM_ERROR;
    }

    // UnitList/[単位系名称]/Unit
    if (!strcasecmp(name.c_str(), UDM_DFI_UNIT)) {
        value = this->unit;
        return UDM_OK;
    }
    // UnitList/[単位系名称]/Reference
    else if (!strcasecmp(name.c_str(), UDM_DFI_REFERENCE)) {
        if (!this->isSetReference()) return UDM_ERROR;
        error = this->toString(this->getReference(), value);
        return error;
    }
    // UnitList/[単位系名称]/Difference
    else if (!strcasecmp(name.c_str(), UDM_DFI_DIFFERENCE)) {
        if (!this->isSetDifference()) return UDM_ERROR;
        error = this->toString(this->getDifference(), value);
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
 * @param [out] value                設定値
 * @return        パラメータの設定値数
 */
int UdmUnitConfig::getNumDfiValue(const std::string &label_path) const
{
    std::vector<std::string> labels;
    std::string label, unit_name, name;
    int count;
    UdmError_t error = UDM_OK;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 1) return 0;
    label = labels[0];
    unit_name = labels[1];
    if (strcasecmp(label.c_str(), UDM_DFI_UNITLIST)) {
        return 0;
    }
    if (strcasecmp(unit_name.c_str(), this->unit_name.c_str())) {
        return 0;
    }
    // UnitList[unit name]のパラメータ数を返す
    if (count == 2) {
        return  ( 1
                 + (this->isSetReference()?1:0)
                 + (this->isSetDifference()?1:0)
                );
    }
    if (count != 3) return 0;
    name = labels[2];

    // UnitList/[単位系名称]/Unit
    if (!strcasecmp(name.c_str(), UDM_DFI_UNIT)) {
        return 1;
    }
    // UnitList/[単位系名称]/Reference
    if (!strcasecmp(name.c_str(), UDM_DFI_REFERENCE)) {
        if (!this->isSetReference()) return 0;
        else return 1;
    }
    // UnitList/[単位系名称]/Difference
    else if (!strcasecmp(name.c_str(), UDM_DFI_DIFFERENCE)) {
        if (!this->isSetDifference()) return 0;
        else return 1;
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
UdmError_t UdmUnitConfig::setDfiValue(const std::string &label_path, const std::string &value)
{
    std::vector<std::string> labels;
    std::string label, unit_name, name;
    int count;
    UdmError_t error = UDM_OK;

    // UnitList
    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count != 3) return UDM_ERROR;
    label = labels[0];
    unit_name = labels[1];
    name = labels[2];
    if (strcasecmp(label.c_str(), UDM_DFI_UNITLIST)) {
        return UDM_ERROR;
    }
    if (strcasecmp(unit_name.c_str(), this->unit_name.c_str())) {
        return UDM_ERROR;
    }

    // UnitList/[単位系名称]/Unit
    if (!strcasecmp(name.c_str(), UDM_DFI_UNIT)) {
        this->unit = value;
        return UDM_OK;
    }
    // UnitList/[単位系名称]/Reference
    else if (!strcasecmp(name.c_str(), UDM_DFI_REFERENCE)) {
        this->setReference(this->convertDouble(value, &error));
        return error;
    }
    // UnitList/[単位系名称]/Difference
    else if (!strcasecmp(name.c_str(), UDM_DFI_DIFFERENCE)) {
        this->setDifference(this->convertDouble(value, &error));
        return error;
    }
    else {
        return UDM_ERROR;
    }

    return UDM_OK;
}

/*******************************************************
 * class UdmUnitListConfig
 ******************************************************/

/**
 * コンストラクタ
 */
UdmUnitListConfig::UdmUnitListConfig(): UdmConfigBase(NULL)
{
}

/**
 * コンストラクタ:TextParser
 * @param  parser        TextParser
 */
UdmUnitListConfig::UdmUnitListConfig(TextParser *parser) : UdmConfigBase(parser)
{
    this->initialize();
}

/**
 * 初期化を行う.
 */
void UdmUnitListConfig::initialize()
{
}

/**
 * デストラクタ
 */
UdmUnitListConfig::~UdmUnitListConfig()
{
    std::map<std::string, UdmUnitConfig*>::iterator itr;
    for (itr = this->unit_list.begin(); itr != this->unit_list.end(); itr++) {
        if (itr->second != NULL) delete itr->second;
    }
}

/**
 * DFIファイルからUnitList要素を読みこむ.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUnitListConfig::read()
{
    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }

    std::string label, value, unit_name, label_leaf;
    std::vector<std::string> child_list;

    // UnitList
    label = "/UnitList";
    if (!this->checkNode(label)) {
        return UDM_WARNING_DFI_NOTFOUND_NODE;
    }
    if ( this->getChildLabels(label, child_list) != UDM_OK) {
        return UDM_ERROR;
    }

    std::vector<std::string>::iterator itr;
    for (itr=child_list.begin(); itr<child_list.end(); itr++) {
        UdmUnitConfig *unit = new UdmUnitConfig(this->parser);
        if( unit->read(*itr) == UDM_OK ) {
            this->unit_list.insert(std::make_pair(unit->getUnitName(), unit));
        }
        else {
            delete unit;
        }
    }

    return UDM_OK;
}

/**
 * UnitList要素をDFIファイルに出力する.
 * @param fp        出力ファイルポインタ
 * @param indent    出力インデント
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUnitListConfig::write(FILE *fp, unsigned int indent)
{
    std::string label, value;
    UdmError_t error;

    // UnitList : open
    label = "UnitList";
    if (this->writeLabel(fp, indent, label) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }
    indent++;

    // UnitList/[unit name]
    std::map<std::string, UdmUnitConfig*>::const_iterator itr;
    for (itr = this->unit_list.begin(); itr != this->unit_list.end(); itr++) {
        label = "/UnitList/" + itr->first;
        error = itr->second->write(fp, indent);
        if (error != UDM_OK) {
            return error;
        }
    }

    // UnitList : close
    if (this->writeCloseTab(fp, --indent) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }

    return UDM_OK;
}

/**
 * 単位系クラス(UdmUnitConfig)を取得する : const.
 * @param unit_name        単位系名称
 * @return        単位系クラス(UdmUnitConfig)
 */
const UdmUnitConfig* UdmUnitListConfig::getUnitConfig(const std::string &unit_name) const
{
    std::map<std::string, UdmUnitConfig*>::const_iterator itr;
    itr = this->unit_list.find(unit_name);
    if (itr != this->unit_list.end()) {
        return itr->second;
    }

    return NULL;
}

/**
 * 単位系クラス(UdmUnitConfig)を取得する.
 * @param unit_name        単位系名称
 * @return        単位系クラス(UdmUnitConfig)
 */
UdmUnitConfig* UdmUnitListConfig::getUnitConfig(const std::string &unit_name)
{
    std::map<std::string, UdmUnitConfig*>::iterator itr;
    itr = this->unit_list.find(unit_name);
    if (itr != this->unit_list.end()) {
        return itr->second;
    }
    return NULL;
}

/**
 * 単位系を設定する.
 * 既に同名の単位系が存在している場合は、上書きする。
 * 同名の単位系が存在しない場合は、追加する。
 * @param         単位系クラス(UdmUnitConfig)
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmUnitListConfig::setUnitConfig(const UdmUnitConfig* unit_config)
{
    if (unit_config == NULL) {
        return UDM_ERROR_INVALID_PARAMETERS;
    }
    std::string unit_name = unit_config->getUnitName();
    UdmUnitConfig *src_unit_config = this->getUnitConfig(unit_name);
    if (src_unit_config == NULL) {
        // 新規に追加する
        this->unit_list.insert(std::make_pair(unit_name, new UdmUnitConfig(*unit_config)));
    }
    else {
        // 設定値を上書きする。
        src_unit_config->setUnitName(unit_config->getUnitName());
        src_unit_config->setUnit(unit_config->getUnit());
        if (unit_config->isSetReference()) {
            src_unit_config->setReference(unit_config->getReference());
        }
        else {
            src_unit_config->unsetReference();
        }
        if (unit_config->isSetDifference()) {
            src_unit_config->setDifference(unit_config->getDifference());
        }
        else {
            src_unit_config->unsetDifference();
        }

    }
    return UDM_OK;
}

/**
 * 単位系の設定数を取得する.
 * @return        単位系の設定数
 */
unsigned int UdmUnitListConfig::getNumUnitConfig() const
{
    return this->unit_list.size();
}

/**
 * 単位系名称のリストを取得する.
 * @param unit_names        単位系名称リスト
 * @return        単位系名称リスト数
 */
unsigned int UdmUnitListConfig::getUnitNameList(std::vector<std::string> &unit_names) const
{
    unsigned int count = 0;
    std::map<std::string, UdmUnitConfig*>::const_iterator itr;
    for (itr = this->unit_list.begin(); itr != this->unit_list.end(); itr++) {
        std::string unit_name = itr->first;
        unit_names.push_back(unit_name);
        count++;
    }

    return count;
}

/**
 * 単位系名称の単位系の単位を取得する.
 * @param unit_name        単位系名称
 * @param  unit            単位
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUnitListConfig::getUnit(const std::string &unit_name, std::string& unit) const
{
    const UdmUnitConfig *unit_config = this->getUnitConfig(unit_name);
    if (unit_config == NULL) return UDM_ERROR;
    unit = unit_config->getUnit();
    return UDM_OK;
}

/**
 * 単位系名称の単位系の基準値を取得する.
 * @param unit_name        単位系名称
 * @param reference        基準値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUnitListConfig::getReference(const std::string &unit_name, float &reference) const
{
    const UdmUnitConfig *unit_config = this->getUnitConfig(unit_name);
    if (unit_config == NULL) return UDM_ERROR;
    reference = unit_config->getReference();
    return UDM_OK;
}

/**
 * 単位系名称の単位系の差分値を取得する.
 * @param unit_name        単位系名称
 * @param difference        差分値
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmUnitListConfig::getDifference(const std::string &unit_name, float &difference) const
{
    const UdmUnitConfig *unit_config = this->getUnitConfig(unit_name);
    if (unit_config == NULL) return UDM_ERROR;
    difference = unit_config->getDifference();
    return UDM_OK;
}


/**
 * 単位系を設定する（差分値を除く）.
 * 差分値を除き、単位系を設定する.
 * 同名の単位系が存在していた場合、上書きする.
 * 同名の単位系が存在しない場合、追加する.
 * @param unit_name        単位系名称
 * @param unit            単位
 * @param reference        基準値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmUnitListConfig::setUnitConfig(const std::string &unit_name, const std::string &unit, float reference)
{
    if (unit_name.empty()) {
        return UDM_ERROR_INVALID_UNITLIST_UNITNAME;
    }
    if (unit.empty()) {
        return UDM_ERROR_INVALID_UNITLIST_UNIT;
    }
    UdmUnitConfig *src_unit_config = this->getUnitConfig(unit_name);
    if (src_unit_config == NULL) {
        // 新規に追加する
        this->unit_list.insert(std::make_pair(unit_name, new UdmUnitConfig(unit_name, unit, reference)));
    }
    else {
        // 設定値を上書きする。
        src_unit_config->setUnitName(unit_name);
        src_unit_config->setUnit(unit);
        src_unit_config->setReference(reference);
        src_unit_config->unsetDifference();
    }

    return UDM_OK;
}

/**
 * 単位系を設定する（差分値を含む）.
 * 差分値を含めて、単位系を設定する.
 * 同名の単位系が存在していた場合、上書きする.
 * 同名の単位系が存在しない場合、追加する.
 * @param unit_name        単位系名称
 * @param unit            単位
 * @param reference        基準値
 * @param difference    差分値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmUnitListConfig::setUnitConfig(const std::string &unit_name, const std::string &unit, float reference, float difference)
{

    if (unit_name.empty()) {
        return UDM_ERROR_INVALID_UNITLIST_UNITNAME;
    }
    if (unit.empty()) {
        return UDM_ERROR_INVALID_UNITLIST_UNIT;
    }
    UdmUnitConfig *src_unit_config = this->getUnitConfig(unit_name);
    if (src_unit_config == NULL) {
        // 新規に追加する
        this->unit_list.insert(std::make_pair(unit_name, new UdmUnitConfig(unit_name, unit, reference, difference)));
    }
    else {
        // 設定値を上書きする。
        src_unit_config->setUnitName(unit_name);
        src_unit_config->setUnit(unit);
        src_unit_config->setReference(reference);
        src_unit_config->setDifference(difference);
    }

    return UDM_OK;
}

/**
 * 単位系が存在するかチェックする.
 * @param unit_name        単位系名称
 * @return        true=単位系が存在する
 */
bool UdmUnitListConfig::existsUnitConfig(const std::string &unit_name) const
{
    const UdmUnitConfig *unit_config = this->getUnitConfig(unit_name);
    if (unit_config == NULL) return false;
    return true;

}

/**
 * 単位系を削除する.
 * @param unit_name        単位系名称
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmUnitListConfig::removeUnitConfig(const std::string &unit_name)
{
    std::map<std::string, UdmUnitConfig*>::iterator itr;
    itr = this->unit_list.find(unit_name);
    if (itr != this->unit_list.end()) {
        if (itr->second != NULL) delete itr->second;
        this->unit_list.erase(itr);
        return UDM_OK;
    }
    return UDM_ERROR;
}

/**
 * 単位系をすべて削除する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmUnitListConfig::clear()
{
    std::map<std::string, UdmUnitConfig*>::iterator itr;
    for (itr = this->unit_list.begin(); itr != this->unit_list.end(); itr++) {
        if (itr->second != NULL) delete itr->second;
    }
    this->unit_list.clear();
    return UDM_ERROR;
}



/**
 * DFIラベルパスのパラメータの設定値を取得する.
 * 数値は文字列に変換して返す。
 * 複数の設定値のあるパラメータの場合はUDM_ERRORを返す。
 * @param [in]  label_path        DFIラベルパス
 * @param [out] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmUnitListConfig::getDfiValue(const std::string &label_path, std::string &value) const
{
    std::vector<std::string> labels;
    std::string label, name;
    int count;
    UdmError_t error = UDM_OK;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 2) return UDM_ERROR;
    label = labels[0];
    name = labels[1];
    if (strcasecmp(label.c_str(), UDM_DFI_UNITLIST)) {
        return UDM_ERROR;
    }

    // UnitList/[unit name]
    std::map<std::string, UdmUnitConfig*>::const_iterator itr;
    itr = this->unit_list.find(name);
    if (itr != this->unit_list.end()) {
        return itr->second->getDfiValue(label_path, value);
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
int UdmUnitListConfig::getNumDfiValue(const std::string &label_path) const
{
    std::vector<std::string> labels;
    std::string label, name;
    int count;
    UdmError_t error = UDM_OK;

    // UnitList : ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 0) return 0;
    label = labels[0];
    if (strcasecmp(label.c_str(), UDM_DFI_UNITLIST)) {
        return 0;
    }
    // UnitListのパラメータ数を返す
    if (count == 1) {
        return this->unit_list.size();
    }

    name = labels[1];

    // UnitList/[unit name]
    std::map<std::string, UdmUnitConfig*>::const_iterator itr;
    itr = this->unit_list.find(name);
    if (itr != this->unit_list.end()) {
        return itr->second->getNumDfiValue(label_path);
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
UdmError_t UdmUnitListConfig::setDfiValue(const std::string &label_path, const std::string &value)
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
    if (strcasecmp(label.c_str(), UDM_DFI_UNITLIST)) {
        return UDM_ERROR;
    }

    // UnitList/[unit name]
    std::map<std::string, UdmUnitConfig*>::const_iterator itr;
    itr = this->unit_list.find(name);
    if (itr != this->unit_list.end()) {
        return itr->second->setDfiValue(label_path, value);
    }
    else {
        // 新規に追加する
        UdmUnitConfig *unit = new UdmUnitConfig();
        unit->setUnitName(name);
        error = unit->setDfiValue(label_path, value);
        if (error != UDM_OK) {
            delete unit;
            return error;
        }
        this->unit_list.insert(std::make_pair(name, unit));
    }

    return UDM_OK;
}

} /* namespace udm */
