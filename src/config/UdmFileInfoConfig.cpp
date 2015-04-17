/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmFileInfo.cpp
 * index.dfiファイルのFileInfoデータクラスのソースファイル
 */
#include <errno.h>
#if defined(_WIN32)
#include <direct.h>
#else
#include <sys/stat.h>
#endif
#include "config/UdmFileInfoConfig.h"

namespace udm {

/**
 * コンストラクタ
 */
UdmFileInfoConfig::UdmFileInfoConfig() : UdmConfigBase(NULL)
{
    this->initialize();
}

/**
 * コンストラクタ:TextParser
 * @param  parser        TextParser
 */
UdmFileInfoConfig::UdmFileInfoConfig(TextParser *parser) : UdmConfigBase(parser)
{
    this->initialize();
}

/**
 * デストラクタ
 */
UdmFileInfoConfig::~UdmFileInfoConfig()
{
}

/**
 * 初期化を行う.
 */
void UdmFileInfoConfig::initialize()
{
    this->timeslice_directory = false;
    this->dfi_type = Udm_UnsDfiType;
    this->fileformat = "cgns";
    this->fieldfilename_format = Udm_step_rank;
    this->setFileCompositionType(Udm_IncludeGrid);
    this->setFileCompositionType(Udm_EachStep);
}

/**
 * DFIファイルからFileInfo要素を読みこむ.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFileInfoConfig::read()
{
    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }

    std::string label, value;
    UdmError_t ierror;

    //DFIType
    label = "/FileInfo/DFIType";
    if ( this->getValue(label, value ) == UDM_OK ) {
        if (strcasecmp(value.c_str(), UDM_DFI_DFITYPE_UNS) != 0) {
            this->dfi_type = Udm_DfiTypeUnknown;
            // エラー
            return UDM_ERROR_INVALID_DFITYPE;
        }
        this->dfi_type = Udm_UnsDfiType;
    }
    else {
        // エラー
        return UDM_ERROR_INVALID_DFITYPE;
    }

    // DirectoryPath
    label = "/FileInfo/DirectoryPath";
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->directory_path = value;
    }
    else {
        this->directory_path = "./";
    }

    //FieldFilenameFormat
    label = "/FileInfo/FieldFilenameFormat";
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->fieldfilename_format = this->convertFieldFilenameFormat(value, &ierror);
        if (ierror != UDM_OK) {
            // エラー
            return UDM_ERROR_INVALID_FIELDFILENAMEFORMAT;
        }
    }

    //FieldFilename
    label = "/FileInfo/FieldFilename";
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->fieldfilename = value;
    }
    else {
        this->fieldfilename = "";
    }

    // TimeSliceDirectory
    label = "/FileInfo/TimeSliceDirectory";
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->timeslice_directory = this->convertBool(value, &ierror);
    }
    else {
        this->timeslice_directory = false;
    }

    // Prefix
    label = "/FileInfo/Prefix";
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->prefix = value;
    }
    else {
        // エラー
        return UDM_ERROR_INVALID_PREFIX;
    }

    // FileFormat
    label = "/FileInfo/FileFormat";
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->fileformat = value;
    }
    else {
        // エラー
        return UDM_ERROR_INVALID_FILEFORMAT;
    }

    // ElementPath
    label = "/FileInfo/ElementPath";
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->element_path = value;
    }
    else {
        this->element_path.clear();
    }

    // FileCompositionType
    label = "/FileInfo/FileCompositionType";
    std::vector<std::string> nodes;
    if ( this->getValueList(label, nodes ) == UDM_OK ) {
        std::vector<std::string>::iterator itr;
        for(itr = nodes.begin(); itr != nodes.end(); ++itr) {
            UdmFileCompositionType_t type = this->convertFileCompositionType(*itr, &ierror);
            if (type != Udm_FileCompositionTypeUnknown) {
                this->setFileCompositionType(type);
            }
        }
    }

    return UDM_OK;
}


/**
 * FileInfo要素をDFIファイルに出力する.
 * @param fp        出力ファイルポインタ
 * @param indent    出力インデント
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFileInfoConfig::write(FILE *fp, unsigned int indent)
{
    std::string label, value, buf;
    UdmError_t error;

    // FileInfo : open
    label = "FileInfo";
    if (this->writeLabel(fp, indent, label) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }
    indent++;

    //DFIType
    if (this->dfi_type != Udm_DfiTypeUnknown) {
        label = "DFIType";
        error = this->toString(this->dfi_type, value);
        if (error == UDM_OK) {
            this->writeNodeValue(fp, indent, label, value);
        }
    }

    // DirectoryPath
    if (!this->directory_path.empty()) {
        label = "DirectoryPath";
        this->writeNodeValue(fp, indent, label, this->directory_path);
    }

    //FieldFilenameFormat
    if (this->fieldfilename_format != Udm_FieldFilenameFormatUnknown) {
        label = "FieldFilenameFormat";
        error = this->toString(this->fieldfilename_format, value);
        if (error == UDM_OK) {
            this->writeNodeValue(fp, indent, label, value);
        }
    }

    //FieldFilename
    if (!this->fieldfilename.empty()) {
        label = "FieldFilename";
        this->writeNodeValue(fp, indent, label, this->fieldfilename);
    }

    // TimeSliceDirectory
    if (this->timeslice_directory != false) {
        label = "TimeSliceDirectory";
        error = this->toString(
                    this->timeslice_directory?Udm_on:Udm_off, value);
        this->writeNodeValue(fp, indent, label, value);
    }

    // Prefix
    if (!this->prefix.empty()) {
        label = "Prefix";
        this->writeNodeValue(fp, indent, label, this->prefix);
    }

    // FileFormat
    if (!this->fileformat.empty()) {
        label = "FileFormat";
        this->writeNodeValue(fp, indent, label, this->fileformat);
    }

    // ElementPath
    if (!this->element_path.empty()) {
        label = "ElementPath";
        this->writeNodeValue(fp, indent, label, this->element_path);
    }

    // FileCompositionType
    label = "FileCompositionType";
    std::vector<std::string> values;
    std::vector<UdmFileCompositionType_t>::const_iterator itr;
    for (itr = this->filecomposition_types.begin(); itr != this->filecomposition_types.end(); itr++) {
        error = this->toString(*itr, value);
        if (error == UDM_OK) {
            values.push_back(value);
        }
        else {
            // エラー
            return UDM_ERROR_INVALID_FILECOMPOSITIONTYPE;
        }
    }
    if (!values.empty()) {
        this->writeNodeValue(fp, indent, label, values);
    }

    // FileInfo : close
    if (this->writeCloseTab(fp, --indent) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }

    return UDM_OK;
}


/**
 * DFI種別を取得する.
 * @return        DFI種別:UdmDfiType_t
 */
UdmDfiType_t UdmFileInfoConfig::getDfiType() const
{
    return this->dfi_type;
}

/**
 * DFI種別を設定する.
 * @param dfi_type                DFI種別
 */
void UdmFileInfoConfig::setDfiType(UdmDfiType_t dfi_type)
{
    this->dfi_type = dfi_type;
}

/**
 * フィールドデータディレクトリを取得する.
 * @param [out] directorypath        フィールドデータディレクトリ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t  UdmFileInfoConfig::getDirectoryPath(std::string& directorypath) const
{
    directorypath = this->directory_path;
    return UDM_OK;
}

/**
 * フィールドデータディレクトリを設定する.
 * @param directory_path        フィールドデータディレクトリ
 */
void UdmFileInfoConfig::setDirectoryPath(const std::string& directory_path)
{
    this->directory_path = directory_path;
}

/**
 * CGNSパス名を取得する.
 * @param [out] elementpath        CGNSパス名
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFileInfoConfig::getElementPath(std::string& elementpath) const
{
    elementpath = this->element_path;
    return UDM_OK;
}

/**
 * CGNSパス名を設定する.
 * @param element_path        CGNSパス名
 */
void UdmFileInfoConfig::setElementPath(const std::string& element_path)
{
    this->element_path = element_path;
}

/**
 * ファイル命名書式を取得する.
 * @return        ファイル命名書式
 */
UdmFieldFilenameFormat_t UdmFileInfoConfig::getFieldfilenameFormat() const
{
    return this->fieldfilename_format;
}

/**
 * ファイル命名書式を設定する.
 * @param fieldfilename_format        ファイル命名書式
 */
void UdmFileInfoConfig::setFieldfilenameFormat(UdmFieldFilenameFormat_t fieldfilename_format)
{
    this->fieldfilename_format = fieldfilename_format;
}


/**
 * CGNSファイル構成タイプ数を取得する.
 * @return        CGNSファイル構成タイプ数
 */
int UdmFileInfoConfig::getNumFileCompositionTypes() const
{
    return this->filecomposition_types.size();
}


/**
 * CGNSファイル構成タイプを取得する.
 * @return        CGNSファイル構成タイプ
 */
const std::vector<UdmFileCompositionType_t>&
UdmFileInfoConfig::getFileCompositionTypes() const
{
    return this->filecomposition_types;
}

/**
 * CGNSファイル構成タイプを設定する:複数.
 * @param types        CGNSファイル構成タイプリスト
 */
void UdmFileInfoConfig::setFileCompositionTypes(
                const std::vector<UdmFileCompositionType_t>& types)
{
    this->filecomposition_types = types;
}


/**
 * CGNSファイル構成タイプを設定する.
 * 同一設定が不可なタイプが存在する場合は、置換を行う.
 * 同一設定が不可なタイプ.
 *         CGNS:GridCoordinates出力方法            : [IncludeGrid | ExcludeGrid]
 *         CGNS:FlowSolutionのステップ出力方法    : [AppendStep | EachStep]
 *         CGNS:GridCoordinatesの時系列出力方法    : [GridConstant | GridTimeSlice]
 * @param type        CGNSファイル構成タイプ
 */
void UdmFileInfoConfig::setFileCompositionType(UdmFileCompositionType_t type)
{
    if (type == Udm_IncludeGrid || type == Udm_ExcludeGrid) {
        this->removeFileCompositionType(Udm_IncludeGrid);
        this->removeFileCompositionType(Udm_ExcludeGrid);
    }
    else if (type == Udm_AppendStep || type == Udm_EachStep) {
        this->removeFileCompositionType(Udm_AppendStep);
        this->removeFileCompositionType(Udm_EachStep);
    }
    else if (type == Udm_GridConstant || type == Udm_GridTimeSlice) {
        this->removeFileCompositionType(Udm_GridConstant);
        this->removeFileCompositionType(Udm_GridTimeSlice);
    }

    this->filecomposition_types.push_back(type);

    return;
}

/**
 * CGNSファイル構成タイプを削除する.
 * @param type        削除CGNSファイル構成タイプ
 */
void UdmFileInfoConfig::removeFileCompositionType(UdmFileCompositionType_t type)
{

    std::vector<UdmFileCompositionType_t>::iterator itr;
    for (itr = this->filecomposition_types.begin(); itr != this->filecomposition_types.end(); itr++) {
        if (*itr == type) {
            this->filecomposition_types.erase(itr);
            return;
        }
    }
    return;
}


/**
 * CGNSファイル構成タイプをが設定されているかチェックする
 * @param type        CGNSファイル構成タイプ
 * @return true=設定済み
 */
bool UdmFileInfoConfig::existsFileCompositionType(UdmFileCompositionType_t type) const
{
    std::vector<UdmFileCompositionType_t>::const_iterator itr;
    for (itr = this->filecomposition_types.begin(); itr != this->filecomposition_types.end(); itr++) {
        if (*itr == type) return true;
    }
    return false;
}

/**
 * ファイルフォーマットを取得する.
 * @param [out]         ファイルフォーマット
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFileInfoConfig::getFileformat(std::string& fileformat) const
{
    fileformat = this->fileformat;
    return UDM_OK;
}


/**
 * ファイルフォーマットを設定する.
 * @param fileformat        ファイルフォーマット
 */
void UdmFileInfoConfig::setFileformat(const std::string& fileformat)
{
    this->fileformat = fileformat;
}

/**
 * ベースファイル名を取得する.
 * @param [out] prefix        ベースファイル名
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmFileInfoConfig::getPrefix(std::string& prefix) const
{
    prefix = this->prefix;
    return UDM_OK;
}

/**
 * ベースファイル名を設定する.
 * @param prefix        ベースファイル名
 */
void UdmFileInfoConfig::setPrefix(const std::string& prefix)
{
    this->prefix = prefix;
}

/**
 * 時刻ディレクトリ作成オプションを取得する.
 * @return        時刻ディレクトリ作成オプション
 */
bool UdmFileInfoConfig::isTimeSliceDirectory() const
{
    return this->timeslice_directory;
}

/**
 * 時刻ディレクトリ作成オプションを設定する.
 * @param timeslice_directory        時刻ディレクトリ作成オプション
 */
void UdmFileInfoConfig::setTimeSliceDirectory(bool timeslice_directory)
{
    this->timeslice_directory = timeslice_directory;
}


/**
 * DFIラベルパスのパラメータの設定値を取得する.
 * 数値は文字列に変換して返す。
 * 複数の設定値のあるパラメータの場合はUDM_ERRORを返す。
 * @param [in]  label_path        DFIラベルパス
 * @param [out] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmFileInfoConfig::getDfiValue(const std::string &label_path, std::string &value) const
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
    if (strcasecmp(label.c_str(), UDM_DFI_FILEINFO)) {
        return UDM_ERROR;
    }
    if (count != 2) return UDM_ERROR;

    //DFIType
    if (!strcasecmp(name.c_str(), UDM_DFI_DFITYPE)) {
        error = this->toString(this->dfi_type, value);
        return error;
    }
    // DirectoryPath
    else if (!strcasecmp(name.c_str(), UDM_DFI_DIRECTORYPATH)) {
        value = this->directory_path;
    }
    //FieldFilenameFormat
    else if (!strcasecmp(name.c_str(), UDM_DFI_FIELDFILENAMEFORMAT)) {
        error = this->toString(this->fieldfilename_format, value);
        return error;
    }
    //FieldFilename
    else if (!strcasecmp(name.c_str(), UDM_DFI_FIELDFILENAME)) {
        value = this->fieldfilename;
    }
    // TimeSliceDirectory
    else if (!strcasecmp(name.c_str(), UDM_DFI_TIMESLICEDIRECTORY)) {
        error = this->toString(this->timeslice_directory?Udm_on:Udm_off, value);
        return error;
    }
    // Prefix
    else if (!strcasecmp(name.c_str(), UDM_DFI_PREFIX)) {
        value = this->prefix;
    }
    // FileFormat
    else if (!strcasecmp(name.c_str(), UDM_DFI_FILEFORMAT)) {
        value = this->fileformat;
    }
    // ElementPath
    else if (!strcasecmp(name.c_str(), UDM_DFI_ELEMENTPATH)) {
        value = this->element_path;
    }
    // FileCompositionType
    else if (!strcasecmp(name.c_str(), UDM_DFI_FILECOMPOSITIONTYPE)) {
        if (this->filecomposition_types.size() != 1) {
            return UDM_ERROR;
        }
        UdmFileCompositionType_t type = this->filecomposition_types[0];
        error = this->toString(type, value);
        return error;
    }
    // FileCompositionType[@]
    else if (!strcasecmp(name.substr(0, strlen(UDM_DFI_FILECOMPOSITIONTYPE)).c_str(), UDM_DFI_FILECOMPOSITIONTYPE)) {
        if (this->filecomposition_types.size() == 0) {
            return UDM_ERROR;
        }
        int type_id = this->splitLabelIndex(name);
        if (type_id < 0) return UDM_ERROR;
        if (type_id >= this->filecomposition_types.size()) return UDM_ERROR;

        UdmFileCompositionType_t type = this->filecomposition_types[type_id];
        error = this->toString(type, value);
        return error;
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
int UdmFileInfoConfig::getNumDfiValue(const std::string &label_path) const
{
    std::vector<std::string> labels;
    std::string label, name;
    int count;
    UdmError_t error = UDM_OK;

    // FileInfo : ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 0) return 0;
    label = labels[0];
    if (strcasecmp(label.c_str(), UDM_DFI_FILEINFO)) {
        return 0;
    }
    // FileInfoのパラメータ数を返す
    if (count == 1) {
        return 9;
    }
    else if (count != 2) {
        return 0;
    }
    name = labels[1];

    //DFIType
    if (!strcasecmp(name.c_str(), UDM_DFI_DFITYPE)) {
        return 1;
    }
    // DirectoryPath
    else if (!strcasecmp(name.c_str(), UDM_DFI_DIRECTORYPATH)) {
        return 1;
    }
    //FieldFilenameFormat
    else if (!strcasecmp(name.c_str(), UDM_DFI_FIELDFILENAMEFORMAT)) {
        return 1;
    }
    // TimeSliceDirectory
    else if (!strcasecmp(name.c_str(), UDM_DFI_TIMESLICEDIRECTORY)) {
        return 1;
    }
    // Prefix
    else if (!strcasecmp(name.c_str(), UDM_DFI_PREFIX)) {
        return 1;
    }
    // FileFormat
    else if (!strcasecmp(name.c_str(), UDM_DFI_FILEFORMAT)) {
        return 1;
    }
    // DataType
    else if (!strcasecmp(name.c_str(), UDM_DFI_DATATYPE)) {
        return 1;
    }
    // ElementPath
    else if (!strcasecmp(name.c_str(), UDM_DFI_ELEMENTPATH)) {
        return 1;
    }
    // FileCompositionType
    else if (!strcasecmp(name.c_str(), UDM_DFI_FILECOMPOSITIONTYPE)) {
        return this->filecomposition_types.size();
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
UdmError_t UdmFileInfoConfig::setDfiValue(const std::string &label_path, const std::string &value)
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
    if (strcasecmp(label.c_str(), UDM_DFI_FILEINFO)) {
        return UDM_ERROR;
    }

    //DFIType
    if (!strcasecmp(name.c_str(), UDM_DFI_DFITYPE)) {
        this->dfi_type = this->convertDfiType(value, &error);
        return error;
    }
    // DirectoryPath
    else if (!strcasecmp(name.c_str(), UDM_DFI_DIRECTORYPATH)) {
        this->directory_path = value;
        return UDM_OK;
    }
    //FieldFilenameFormat
    else if (!strcasecmp(name.c_str(), UDM_DFI_FIELDFILENAMEFORMAT)) {
        this->fieldfilename_format = this->convertFieldFilenameFormat(value, &error);
        return error;
    }
    //FieldFilename
    else if (!strcasecmp(name.c_str(), UDM_DFI_FIELDFILENAME)) {
        this->fieldfilename = value;
        return error;
    }
    // TimeSliceDirectory
    else if (!strcasecmp(name.c_str(), UDM_DFI_TIMESLICEDIRECTORY)) {
        this->timeslice_directory = this->convertBool(value, &error);
        return error;
    }
    // Prefix
    else if (!strcasecmp(name.c_str(), UDM_DFI_PREFIX)) {
        this->prefix = value;
        return UDM_OK;
    }
    // FileFormat
    else if (!strcasecmp(name.c_str(), UDM_DFI_FILEFORMAT)) {
        this->fileformat = value;
    }
    // ElementPath
    else if (!strcasecmp(name.c_str(), UDM_DFI_ELEMENTPATH)) {
        this->element_path = value;
        return UDM_OK;
    }
    // FileCompositionType
    else if (!strcasecmp(name.c_str(), UDM_DFI_FILECOMPOSITIONTYPE)) {
        this->filecomposition_types.clear();
        this->setFileCompositionType(this->convertFileCompositionType(value, &error));
        return error;
    }
    // FileCompositionType[@]
    else if (!strcasecmp(name.c_str(), (std::string(UDM_DFI_FILECOMPOSITIONTYPE)+std::string("[@]")).c_str())) {
        this->setFileCompositionType(this->convertFileCompositionType(value, &error));
        return error;
    }
    // FileCompositionType[0..n]
    else if (!strcasecmp(name.substr(0, strlen(UDM_DFI_FILECOMPOSITIONTYPE)).c_str(), UDM_DFI_FILECOMPOSITIONTYPE)) {
        if (this->filecomposition_types.size() == 0) {
            return UDM_ERROR;
        }
        int type_id = this->splitLabelIndex(name);
        if (type_id < 0) return UDM_ERROR;
        if (type_id >= this->filecomposition_types.size()) return UDM_ERROR;

        UdmFileCompositionType_t type = this->convertFileCompositionType(value, &error);
        if (error != UDM_OK) return error;
        this->filecomposition_types[type_id] = type;
        return UDM_OK;
    }
    else {
        return UDM_ERROR;
    }

    return UDM_OK;
}

/**
 * 入力ファイル名を作成する.
 * @param [out]  file_name        入力ファイル名
 * @param [in] rank_no            MPIランク番号
 * @param [in] timeslice_step    ステップ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmFileInfoConfig::generateCgnsTimeSliceName(std::string& file_name, int rank_no, int timeslice_step) const
{
    if (this->prefix.empty()) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PREFIX, "prefix is empty.");
    }

    char name[256] = {0x00};
    if (rank_no >= 0 && timeslice_step >= 0) {
        if (this->fieldfilename_format == Udm_rank_step) {
            // "%s_id%06d_%010d.%s" = Prefix + rank_no + step_no + FileFormat
            sprintf(name, UDM_DFI_FILEFORMAT_RANK_STEP, this->prefix.c_str(), rank_no, timeslice_step, this->fileformat.c_str());
        }
        else if (this->fieldfilename_format == Udm_step_rank) {
            // "%s_%010d_id%06d.%s" = Prefix + step_no + rank_no + FileFormat
            sprintf(name, UDM_DFI_FILEFORMAT_STEP_RANK, this->prefix.c_str(), timeslice_step, rank_no, this->fileformat.c_str());
        }
    }
    else if (rank_no >= 0) {
        // "%s_%010d_%06d.%s" = Prefix + rank_no + FileFormat
        sprintf(name, UDM_DFI_FILEFORMAT_RANK, this->prefix.c_str(), rank_no, this->fileformat.c_str());
    }
    else if (timeslice_step >= 0) {
        // "%s_%010d_%06d.%s" = Prefix + step_no + FileFormat
        sprintf(name, UDM_DFI_FILEFORMAT_STEP, this->prefix.c_str(), timeslice_step, this->fileformat.c_str());
    }
    else {
        // "%s_%010d_%06d.%s" = Prefix + FileFormat
        sprintf(name, UDM_DFI_FILEFORMAT_NONE, this->prefix.c_str(), this->fileformat.c_str());
    }

    file_name = name;
    if (file_name.empty()) {
        return UDM_ERROR;
    }

    // フィールドデータディレクトリ, 時刻ディレクトリを付加する.
    return this->createTimeSliceDirectoryPath(file_name, timeslice_step);
}


/**
 * CGNSファイル名を生成する
 * @param [out]  file_name        CGNSファイル
 * @param [in] rank_no            MPIランク番号
 * @param [in] timeslice_step    ステップ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmFileInfoConfig::generateCgnsFileName(std::string& file_name, int rank_no) const
{
    return generateCgnsTimeSliceName(file_name, rank_no, -1);
}

/**
 * CGNSリンクファイル名を生成する
 * @param [out]  file_name        CGNSリンクファイル
 * @param [in] rank_no            MPIランク番号
 * @param [in] timeslice_step    ステップ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmFileInfoConfig::generateCgnsLinkName(std::string& file_name, int rank_no) const
{
    if (this->prefix.empty()) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PREFIX, "prefix is empty.");
    }
    char name[256] = {0x00};
    if (rank_no >= 0) {
        // "%s_%010d_%06d.%s" = Prefix + rank_no + FileFormat
        sprintf(name, UDM_DFI_FILEFORMAT_RANK, this->prefix.c_str(), rank_no, this->fileformat.c_str());
    }
    else {
        // "%s_%010d_%06d.%s" = Prefix + FileFormat
        sprintf(name, UDM_DFI_FILEFORMAT_NONE, this->prefix.c_str(), this->fileformat.c_str());
    }
    file_name = name;

    // フィールドデータディレクトリを付加する.
    return this->createTimeSliceDirectoryPath(file_name, -1);
}

/**
 * CGNS:GrdiCoordinates時系列ファイル名を作成する.
 * @param [out]  file_name        CGNS:GrdiCoordinates時系列ファイル名
 * @param [in] rank_no            MPIランク番号
 * @param [in] timeslice_step    ステップ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmFileInfoConfig::generateCgnsTimeSliceGridName(std::string& file_name, int rank_no, int timeslice_step) const
{
    if (this->prefix.empty()) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PREFIX, "prefix is empty.");
    }

    char name[256] = {0x00};
    if (rank_no >= 0 && timeslice_step >= 0) {
        if (this->fieldfilename_format == Udm_rank_step) {
            // "%s_id%06d_%010d.%s" = Prefix + rank_no + step_no + FileFormat
            sprintf(name, UDM_DFI_GRIDFORMAT_RANK_STEP, this->prefix.c_str(), rank_no, timeslice_step, this->fileformat.c_str());
        }
        else if (this->fieldfilename_format == Udm_step_rank) {
            // "%s_%010d_id%06d.%s" = Prefix + step_no + rank_no + FileFormat
            sprintf(name, UDM_DFI_GRIDFORMAT_STEP_RANK, this->prefix.c_str(), timeslice_step, rank_no, this->fileformat.c_str());
        }
    }
    else if (rank_no >= 0) {
        // "%s_%010d_%06d.%s" = Prefix + rank_no + FileFormat
        sprintf(name, UDM_DFI_GRIDFORMAT_RANK, this->prefix.c_str(), rank_no, this->fileformat.c_str());
    }
    else if (timeslice_step >= 0) {
        // "%s_%010d_%06d.%s" = Prefix + step_no + FileFormat
        sprintf(name, UDM_DFI_GRIDFORMAT_STEP, this->prefix.c_str(), timeslice_step, this->fileformat.c_str());
    }
    else {
        // "%s_%010d_%06d.%s" = Prefix + FileFormat
        sprintf(name, UDM_DFI_FILEFORMAT_NONE, this->prefix.c_str(), this->fileformat.c_str());
    }

    file_name = name;

    // フィールドデータディレクトリ, 時刻ディレクトリを付加する.
    return this->createTimeSliceDirectoryPath(file_name, timeslice_step);
}

/**
 * CGNS:FlowSolution時系列ファイル名を作成する.
 * @param [out]  file_name        CGNS:FlowSolution時系列ファイル名
 * @param [in] rank_no            MPIランク番号
 * @param [in] timeslice_step    ステップ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmFileInfoConfig::generateCgnsTimeSliceSolutionName(std::string& file_name, int rank_no, int timeslice_step) const
{
    if (this->prefix.empty()) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PREFIX, "prefix is empty.");
    }

    char name[256] = {0x00};
    if (rank_no >= 0 && timeslice_step >= 0) {
        if (this->fieldfilename_format == Udm_rank_step) {
            // "%s_id%06d_%010d.%s" = Prefix + rank_no + step_no + FileFormat
            sprintf(name, UDM_DFI_SOLUTIONFORMAT_RANK_STEP, this->prefix.c_str(), rank_no, timeslice_step, this->fileformat.c_str());
        }
        else if (this->fieldfilename_format == Udm_step_rank) {
            // "%s_%010d_id%06d.%s" = Prefix + step_no + rank_no + FileFormat
            sprintf(name, UDM_DFI_SOLUTIONFORMAT_STEP_RANK, this->prefix.c_str(), timeslice_step, rank_no, this->fileformat.c_str());
        }
    }
    else if (rank_no >= 0) {
        // "%s_%010d_%06d.%s" = Prefix + rank_no + FileFormat
        sprintf(name, UDM_DFI_SOLUTIONFORMAT_RANK, this->prefix.c_str(), rank_no, this->fileformat.c_str());
    }
    else if (timeslice_step >= 0) {
        // "%s_%010d_%06d.%s" = Prefix + step_no + FileFormat
        sprintf(name, UDM_DFI_SOLUTIONFORMAT_STEP, this->prefix.c_str(), timeslice_step, this->fileformat.c_str());
    }
    else {
        // "%s_%010d_%06d.%s" = Prefix + FileFormat
        sprintf(name, UDM_DFI_FILEFORMAT_NONE, this->prefix.c_str(), this->fileformat.c_str());
    }

    file_name = name;

    // フィールドデータディレクトリ, 時刻ディレクトリを付加する.
    return this->createTimeSliceDirectoryPath(file_name, timeslice_step);
}

/**
 * CGNS:GrdiCoordinatesファイル名を作成する.
 * @param [out]  file_name        CGNS:GrdiCoordinatesファイル名
 * @param [in] rank_no            MPIランク番号
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmFileInfoConfig::generateCgnsGridName(std::string& file_name, int rank_no) const
{
    return generateCgnsTimeSliceGridName(file_name, rank_no, -1);
}

/**
 * CGNS:FlowSolutionファイル名を作成する.
 * @param [out]  file_name        CGNS:FlowSolutionファイル名
 * @param [in] rank_no            MPIランク番号
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmFileInfoConfig::generateCgnsSolutionName(std::string& file_name, int rank_no) const
{
    return generateCgnsTimeSliceSolutionName(file_name, rank_no, -1);
}

/**
 * フィールドデータディレクトリ, 時刻ディレクトリを付加する.
 * @param [out]  file_name        CGNS:ファイル名
 * @param [in] timeslice_step    ステップ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmFileInfoConfig::createTimeSliceDirectoryPath(std::string &file_name, int timeslice_step) const
{
    char name[256] = {0x00};

    // 時刻ディレクトリ作成オプション
    if (this->timeslice_directory && timeslice_step >= 0) {
        // %010d
        sprintf(name, UDM_DFI_FORMAT_TIMESLICE, timeslice_step);
        file_name = std::string(name) + "/" + file_name;
    }

    // フィールドデータディレクトリ
    std::string dirname;
    this->getDirectoryPath(dirname);
    if (dirname.length() >= 1) {
        if (dirname.substr(dirname.length()-1) != "/") {
            file_name = dirname + "/" + file_name;
        }
        else {
            file_name = dirname + file_name;
        }
    }

    return UDM_OK;
}


/**
 * CGNSリンクファイル名を取得する.
 * @param [out]  file_name        CGNS:リンクファイル名
 * @param [in] rank_no            MPIランク番号
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmFileInfoConfig::getCgnsLinkFile(std::string& file_name, int rank_no) const
{
    return generateCgnsLinkName(file_name, rank_no);
}



/**
 * FileInfo/FieldFilenameを取得します。
 * @return        ファイル名
 */
const std::string& UdmFileInfoConfig::getFieldFilename() const
{
    return this->fieldfilename;
}

/**
 * FileInfo/FieldFilenameを設定します.
 * @param filename        ファイル名
 */
void UdmFileInfoConfig::setFieldFilename(const std::string& filename)
{
    this->fieldfilename = filename;
}

/**
 * CGNS:GridCoordinatesが固定タイプ設定であるかチェックする.
 * @return        true=固定タイプ(初回のみ出力する)
 */
bool UdmFileInfoConfig::isFileGridConstant() const
{
    if (this->existsFileCompositionType(Udm_IncludeGrid)
        && this->existsFileCompositionType(Udm_EachStep)) {
        return false;
    }
    if (this->existsFileCompositionType(Udm_GridConstant)) {
        return true;
    }

    return false;
}

/**
 * CGNS:GridCoordinatesのみのファイル出力を行うかチェックする.
 * ファイル構成タイプにExcludeGridが存在しているかチェックする.
 * @return            true=GridCoordinatesファイル出力を行う.
 */
bool UdmFileInfoConfig::isFileGridCoordinates() const
{
    if (this->existsFileCompositionType(Udm_ExcludeGrid)) {
        return true;
    }

    return false;
}

/**
 * CGNS:FlowSolutionのみのファイル出力を行うかチェックする.
 * ファイル構成タイプにExcludeGridが存在しているかチェックする.
 * isFileGridCoordinatesと同意.
 * @return            true=FlowSolutionファイル出力を行う.
 */
bool UdmFileInfoConfig::isFileFlowSolution() const
{
    return this->isFileGridCoordinates();
}


/**
 * １つのCGNSファイルに複数の時系列構成のファイル出力を行うかチェックする.
 * ファイル構成タイプにAppendStepが存在しているかチェックする.
 * @return            true=時系列構成のファイル出力を行う.
 */
bool UdmFileInfoConfig::isFileTimeSlice() const
{
    if (this->existsFileCompositionType(Udm_AppendStep)) {
        return true;
    }

    return false;
}

/**
 * １つのCGNSファイルにGridCordinatesとFlowSolutionのファイル出力を行うかチェックする.
 * ファイル構成タイプにIncludeGridが存在しているかチェックする.
 * @return            true=GridCordinatesとFlowSolutionのファイル出力を行う.
 */
bool UdmFileInfoConfig::isFileIncludeGrid() const
{
    if (this->existsFileCompositionType(Udm_IncludeGrid)) {
        return true;
    }

    return false;
}

} /* namespace udm */

