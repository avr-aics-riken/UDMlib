/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmDomainConfig.cpp
 * proc.dfiファイルのDomainデータクラスのソースファイル
 */

#include "config/UdmDomainConfig.h"

namespace udm {

/**
 * コンストラクタ
 */
UdmDomainConfig::UdmDomainConfig() : UdmConfigBase(NULL)
{
    this->initialize();
}

/**
 * コンストラクタ:TextParser
 * @param  parser        TextParser
 */
UdmDomainConfig::UdmDomainConfig(TextParser *parser) : UdmConfigBase(parser)
{
    this->initialize();
}

/**
 * デストラクタ
 */
UdmDomainConfig::~UdmDomainConfig()
{
}

/**
 * 初期化を行う.
 */
void UdmDomainConfig::initialize()
{
}


/**
 * DFIファイルからDomain要素を読みこむ.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmDomainConfig::read()
{
    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }

    std::string label, value;
    UdmError_t ierror;

    // Domain/CellDimension
    label = "/Domain/CellDimension";
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->cell_dimension = this->convertLong(value, &ierror);
    }
    else {
        // エラー
        return UDM_ERROR_INVALID_CELLDIMENSION;
    }

    // Domain/VertexSize
    label = "/Domain/VertexSize";
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->vertex_size = this->convertLong(value, &ierror);
    }
    else {
        // エラー
        return UDM_ERROR_INVALID_VERTEXSIZE;
    }

    // Domain/CellSize
    label = "/Domain/CellSize";
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->cell_size = this->convertLong(value, &ierror);
    }
    else {
        // エラー
        return UDM_ERROR_INVALID_CELLSIZE;
    }

    return UDM_OK;
}


/**
 * Domain要素をDFIファイルに出力する.
 * @param fp        出力ファイルポインタ
 * @param indent    出力インデント
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmDomainConfig::write(FILE *fp, unsigned int indent)
{
    std::string label, value, buf;

    // Domain : open
    label = "Domain";
    if (this->writeLabel(fp, indent, label) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }
    indent++;

    // Domain/CellDimension
    label = "CellDimension";
    this->writeNodeValue(fp, indent, label, this->cell_dimension);

    // Domain/VertexSize
    label = "VertexSize";
    this->writeNodeValue(fp, indent, label, this->vertex_size);

    // Domain/CellSize
    label = "CellSize";
    this->writeNodeValue(fp, indent, label, this->cell_size);

    // Domain : close
    if (this->writeCloseTab(fp, --indent) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }

    return UDM_OK;
}

/**
 * 非構造格子モデルの次元数（=1D, 2D, 3D）を取得する.
 * @return        非構造格子モデルの次元数（=1D, 2D, 3D）
 */
UdmSize_t UdmDomainConfig::getCellDimension() const
{
    return this->cell_dimension;
}

/**
 * 非構造格子モデルの次元数（=1D, 2D, 3D）を設定する
 * @param cell_dimension        非構造格子モデルの次元数（=1D, 2D, 3D）
 */
void UdmDomainConfig::setCellDimension(int cell_dimension)
{
    this->cell_dimension = cell_dimension;
}


/**
 * ノード数を取得する.
 * @return        ノード数
 */
UdmSize_t UdmDomainConfig::getVertexSize() const
{
    return this->vertex_size;
}

/**
 * ノード数を設定する.
 * @param vertex_size        ノード数
 */
void UdmDomainConfig::setVertexSize(UdmSize_t vertex_size)
{
    this->vertex_size = vertex_size;
}

/**
 * セル（要素）数を取得する.
 * @return        セル（要素）数
 */
UdmSize_t UdmDomainConfig::getCellSize() const
{
    return this->cell_size;
}

/**
 * セル（要素）数を設定する.
 * @param    cell_size        セル（要素）数
 */
void UdmDomainConfig::setCellSize(UdmSize_t cell_size)
{
    this->cell_size = cell_size;
}


/**
 * DFIラベルパスのパラメータの設定値を取得する.
 * 数値は文字列に変換して返す。
 * 複数の設定値のあるパラメータの場合はUDM_ERRORを返す。
 * @param [in]  label_path        DFIラベルパス
 * @param [out] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDomainConfig::getDfiValue(const std::string &label_path, std::string &value) const
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
    if (strcasecmp(label.c_str(), UDM_DFI_DOMAIN)) {
        return UDM_ERROR;
    }

    // Domain/CellDimension
    if (!strcasecmp(name.c_str(), UDM_DFI_CELLDIMENSION)) {
        error = this->toString((long long)this->cell_dimension, value);
        return error;
    }
    // Domain/VertexSize
    else if (!strcasecmp(name.c_str(), UDM_DFI_VERTEXSIZE)) {
        error = this->toString(this->vertex_size, value);
        return error;
    }
    // Domain/CellSize
    else if (!strcasecmp(name.c_str(), UDM_DFI_CELLSIZE)) {
        error = this->toString(this->cell_size, value);
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
 * @return        パラメータの設定値数
 */
int UdmDomainConfig::getNumDfiValue(const std::string &label_path) const
{
    std::vector<std::string> labels;
    std::string label, name;
    int count;

    // Domain : ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 0) return 0;
    label = labels[0];
    if (strcasecmp(label.c_str(), UDM_DFI_DOMAIN)) {
        return 0;
    }
    // Domainのパラメータ数を返す
    if (count == 1) {
        return 3;
    }
    else if (count != 2) {
        return 0;
    }
    name = labels[1];

    // Domain/CellDimension
    if (!strcasecmp(name.c_str(), UDM_DFI_CELLDIMENSION)) {
        return 1;
    }
    // Domain/VertexSize
    else if (!strcasecmp(name.c_str(), UDM_DFI_VERTEXSIZE)) {
        return 1;
    }
    // Domain/CellSize
    else if (!strcasecmp(name.c_str(), UDM_DFI_CELLSIZE)) {
        return 1;
    }
    else {
        return 0;
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
UdmError_t UdmDomainConfig::setDfiValue(const std::string &label_path, const std::string &value)
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
    if (strcasecmp(label.c_str(), UDM_DFI_DOMAIN)) {
        return UDM_ERROR;
    }

    // Domain/CellDimension
    if (!strcasecmp(name.c_str(), UDM_DFI_CELLDIMENSION)) {
        this->cell_dimension = (int)this->convertLong(value, &error);
        return error;
    }
    // Domain/VertexSize
    else if (!strcasecmp(name.c_str(), UDM_DFI_VERTEXSIZE)) {
        this->vertex_size = (UdmSize_t)this->convertLong(value, &error);
        return error;
    }
    // Domain/CellSize
    else if (!strcasecmp(name.c_str(), UDM_DFI_CELLSIZE)) {
        this->cell_size = (UdmSize_t)this->convertLong(value, &error);
        return error;
    }
    else {
        return UDM_ERROR;
    }

    return UDM_OK;
}


} /* namespace udm */
