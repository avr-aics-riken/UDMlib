/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmProcessConfig.cpp
 * proc.dfiファイルのProcessデータクラスのソースファイル
 */

#include "config/UdmProcessConfig.h"

namespace udm {

/*******************************************************
 * class UdmRankConfig
 ******************************************************/

/**
 * コンストラクタ
 */
UdmRankConfig::UdmRankConfig() : UdmConfigBase(NULL), rankid(0), vertex_size(0), cell_size(0)
{
    this->initialize();
}

/**
 * コンストラクタ:TextParser
 * @param  parser        TextParser
 */
UdmRankConfig::UdmRankConfig(TextParser *parser) : UdmConfigBase(parser), rankid(0), vertex_size(0), cell_size(0)
{
    this->initialize();
}

/**
 * コピーコンストラクタ
 * @param src_rank_config        コピー元コンストラクタ
 */
UdmRankConfig::UdmRankConfig(const UdmRankConfig &src_rank_config)
{
    this->initialize();
    this->setRankConfig(src_rank_config);
}

/**
 * コンストラクタ
 * @param rankid        ランク番号
 * @param vertex_size    プロセスの領域のノード数
 * @param cell_size        プロセスの領域のセル（要素）数
 */
UdmRankConfig::UdmRankConfig(int rankid, UdmSize_t vertex_size, UdmSize_t cell_size)
{
    this->initialize();
    this->rankid = rankid;
    this->vertex_size = vertex_size;
    this->cell_size = cell_size;
}

/**
 * デストラクタ
 */
UdmRankConfig::~UdmRankConfig()
{
}

/**
 * 初期化を行う.
 */
void UdmRankConfig::initialize()
{
    this->rankid = 0;
    this->vertex_size = 0;
    this->cell_size = 0;
}

/**
 * DFIファイルからProcess/Rank[@]要素を読みこむ.
 * @param    rank_label            Rankラベル
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConfig::read(const std::string& rank_label)
{
    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }

    std::string label, value, slice_node, label_leaf;
    UdmError_t ierror = UDM_OK;
    std::vector<std::string> child_list;

    // Process/Rank[@]
    std::vector<std::string> path_list;
    int count = this->split(rank_label, '/', path_list);
    if (count != 2) {
        return UDM_ERROR_INVALID_DFI_ELEMENTPATH;
    }
    if (strcasecmp(path_list[0].c_str(), UDM_DFI_PROCESS)) {
        return UDM_ERROR_INVALID_DFI_ELEMENTPATH;
    }

    // Process/Rank[@]/ID : ランク番号
    label = rank_label + "/" + UDM_DFI_ID;
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->rankid = this->convertLong(value, &ierror);
    }

    // Process/Rank[@]/VertexSize : プロセスの領域のノード数
    label = rank_label + "/" + UDM_DFI_VERTEXSIZE;
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->vertex_size = this->convertLong(value, &ierror);
    }

    // Process/Rank[@]/CellSize : プロセスの領域のセル（要素）数
    label = rank_label + "/" + UDM_DFI_CELLSIZE;
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->cell_size = this->convertLong(value, &ierror);
    }

    return UDM_OK;
}

/**
 * Process/Rank[@]要素をDFIファイルに出力する.
 * @param fp        出力ファイルポインタ
 * @param indent    出力インデント
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConfig::write(FILE *fp, unsigned int indent)
{
    std::string label, value;
    UdmError_t error;

    // Process/Rank[@] : open
    label = "Rank[@]";
    if (this->writeLabel(fp, indent, label) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }
    indent++;

    // Process/Rank[@]/ID : ランク番号
    label = UDM_DFI_ID;
    this->writeNodeValue(fp, indent, label, this->rankid );

    // Process/Rank[@]/VertexSize : プロセスの領域のノード数
    label = UDM_DFI_VERTEXSIZE;
    this->writeNodeValue(fp, indent, label, this->vertex_size );

    // Process/Rank[@]/CellSize : プロセスの領域のセル（要素）数
    label = UDM_DFI_CELLSIZE;
    this->writeNodeValue(fp, indent, label, this->cell_size );

    // Process/Rank[@] : close
    if (this->writeCloseTab(fp, --indent) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }

    return UDM_OK;
}

/**
 * ランク番号を取得する.
 * @return        ランク番号
 */
int UdmRankConfig::getRankId() const
{
    return this->rankid;
}

/**
 * ランク番号を設定する。
 * @param rankid        ランク番号
 */
void UdmRankConfig::setRankId(int rankid)
{
    this->rankid = rankid;
}

/**
 * ノード数を取得する.
 * @return        ノード数
 */
UdmSize_t UdmRankConfig::getVertexSize() const
{
    return this->vertex_size;
}

/**
 * ノード数を設定する.
 * @param vertex_size        ノード数
 */
void UdmRankConfig::setVertexSize(UdmSize_t vertex_size)
{
    this->vertex_size = vertex_size;
}

/**
 * セル（要素）数を取得する.
 * @return        セル（要素）数
 */
UdmSize_t UdmRankConfig::getCellSize() const
{
    return this->cell_size;
}

/**
 * セル（要素）数を設定する.
 * @param    cell_size        セル（要素）数
 */
void UdmRankConfig::setCellSize(UdmSize_t cell_size)
{
    this->cell_size = cell_size;
}

/**
 * ランク情報を設定する.
 * @param src_rank_config        元ランク情報
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmRankConfig::setRankConfig(const UdmRankConfig &src_rank_config)
{
    this->initialize();
    this->rankid = src_rank_config.rankid;
    this->vertex_size = src_rank_config.vertex_size;
    this->cell_size = src_rank_config.cell_size;

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
UdmError_t UdmRankConfig::getDfiValue(const std::string &label_path, std::string &value) const
{
    std::vector<std::string> labels;
    std::string label, rank_name, name;
    int count;
    UdmError_t error = UDM_OK;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count != 3) return UDM_ERROR;
    label = labels[0];
    rank_name = labels[1];
    name = labels[2];
    if (strcasecmp(label.c_str(), UDM_DFI_PROCESS)) {
        return UDM_ERROR;
    }

    // Rankチェック
    if (rank_name.length() <= std::string(UDM_DFI_RANK).length()+2) return UDM_ERROR;
    if (strcasecmp(rank_name.substr(0, std::string(UDM_DFI_RANK).length()).c_str(), UDM_DFI_RANK)) return UDM_ERROR;

    // Process/Rank[@]/ID : ランク番号
    if (!strcasecmp(name.c_str(), UDM_DFI_ID)) {
        error = this->toString((long long)this->getRankId(), value);
        return error;
    }
    // Process/Rank[@]/VertexSize : プロセスの領域のノード数を記述します。
    else if (!strcasecmp(name.c_str(), UDM_DFI_VERTEXSIZE)) {
        error = this->toString(this->vertex_size, value);
        return error;
    }
    // Process/Rank[@]/CellSize  : プロセスの領域のセル（要素）数
    else if (!strcasecmp(name.c_str(), UDM_DFI_CELLSIZE)) {
        error = this->toString(this->cell_size, value);
        return error;
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
int UdmRankConfig::getNumDfiValue(const std::string &label_path) const
{
    std::vector<std::string> labels;
    std::string label, slice_name, name;
    int count;
    UdmError_t error = UDM_OK;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 1) return 0;
    label = labels[0];
    slice_name = labels[1];
    if (strcasecmp(label.c_str(), UDM_DFI_PROCESS)) {
        return 0;
    }

    // Rankチェック
    if (name.length() <= std::string(UDM_DFI_RANK).length()+2) return 0;
    if (strcasecmp(name.substr(0, std::string(UDM_DFI_RANK).length()).c_str(), UDM_DFI_RANK)) return 0;

    // Process/Rank[@]のパラメータ数を返す
    if (count == 2) {
        return 3;
    }
    if (count != 3) return 0;
    name = labels[2];

    // Process/Rank[@]/ID : ランク番号
    if (!strcasecmp(name.c_str(), UDM_DFI_ID)) {
        return 1;
    }
    // Process/Rank[@]/VertexSize : プロセスの領域のノード数を記述します。
    else if (!strcasecmp(name.c_str(), UDM_DFI_VERTEXSIZE)) {
        return 1;
    }
    // Process/Rank[@]/CellSize  : プロセスの領域のセル（要素）数
    else if (!strcasecmp(name.c_str(), UDM_DFI_CELLSIZE)) {
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
UdmError_t UdmRankConfig::setDfiValue(const std::string &label_path, const std::string &value)
{
    std::vector<std::string> labels;
    std::string label, rank_name, name;
    int count;
    UdmError_t error = UDM_OK;

    // TimeSlice
    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count != 3) return UDM_ERROR;
    label = labels[0];
    rank_name = labels[1];
    name = labels[2];
    if (strcasecmp(label.c_str(), UDM_DFI_PROCESS)) {
        return UDM_ERROR;
    }

    // Rankチェック
    if (rank_name.length() <= std::string(UDM_DFI_RANK).length()+2) return UDM_ERROR;
    if (strcasecmp(rank_name.substr(0, std::string(UDM_DFI_RANK).length()).c_str(), UDM_DFI_RANK)) return UDM_ERROR;

    // Process/Rank[@]/ID : ランク番号
    if (!strcasecmp(name.c_str(), UDM_DFI_ID)) {
        this->rankid = this->convertLong(value, &error);
        return error;
    }
    // Process/Rank[@]/VertexSize : プロセスの領域のノード数を記述します。
    else if (!strcasecmp(name.c_str(), UDM_DFI_VERTEXSIZE)) {
        this->vertex_size = this->convertLong(value, &error);
        return error;
    }
    // Process/Rank[@]/CellSize  : プロセスの領域のセル（要素）数
    else if (!strcasecmp(name.c_str(), UDM_DFI_CELLSIZE)) {
        this->cell_size = this->convertLong(value, &error);
        return error;
    }
    else {
        return UDM_ERROR;
    }

    return UDM_OK;
}

/*******************************************************
 * class UdmProcessConfig
 ******************************************************/

/**
 * コンストラクタ
 */
UdmProcessConfig::UdmProcessConfig(): UdmConfigBase(parser)
{
    this->initialize();
}

/**
 * コンストラクタ:TextParser
 * @param  parser        TextParser
 */
UdmProcessConfig::UdmProcessConfig(TextParser *parser) : UdmConfigBase(parser)
{

}


/**
 * デストラクタ
 */
UdmProcessConfig::~UdmProcessConfig()
{
    std::map<int, UdmRankConfig*>::iterator itr;
    for (itr = this->rank_list.begin(); itr != this->rank_list.end(); itr++) {
        if (itr->second != NULL) delete itr->second;
    }
    this->rank_list.clear();
}

/**
 * 初期化を行う.
 */
void UdmProcessConfig::initialize()
{
    this->rank_list.clear();
}


/**
 * DFIファイルからProcess要素を読みこむ.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmProcessConfig::read()
{
    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }

    std::string label, value, unit_name, label_leaf;
    std::vector<std::string> child_list;

    // Process
    label = "/Process";
    if ( this->getChildLabels(label, child_list) != UDM_OK) {
        return UDM_ERROR;
    }

    std::vector<std::string>::iterator itr;
    for (itr=child_list.begin(); itr<child_list.end(); itr++) {
        UdmRankConfig *rank = new UdmRankConfig(this->parser);
        if( rank->read(*itr) == UDM_OK ) {
            this->setRankConfig(rank);
        }
        else {
            delete rank;
        }
    }

    return UDM_OK;
}


/**
 * Process要素をDFIファイルに出力する.
 * @param fp        出力ファイルポインタ
 * @param indent    出力インデント
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmProcessConfig::write(FILE *fp, unsigned int indent)
{
    std::string label, value;
    UdmError_t error;

    // Process : open
    label = "Process";
    if (this->writeLabel(fp, indent, label) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }
    indent++;

    // Process/Rank[@]
    std::map<int, UdmRankConfig*>::const_iterator itr;
    for (itr = this->rank_list.begin(); itr != this->rank_list.end(); itr++) {
        error = itr->second->write(fp, indent);
        if (error != UDM_OK) {
            return error;
        }
    }

    // Process : close
    if (this->writeCloseTab(fp, --indent) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }

    return UDM_OK;
}

/**
 * proc.dfi:/Process/Rank[@]のリスト数を取得する.
 * @return        /Process/Rank[@]のリスト数
 */
unsigned int  UdmProcessConfig::getNumRankConfig() const
{
    return this->rank_list.size();
}


/**
 * proc.dfi:/Process/Rank[@]/IDのリストを取得する.
 * @param [out] rankids        ID(ランク番号)のリスト
 * @return        IDのリスト数
 */
unsigned int UdmProcessConfig::getRankIdList(std::vector<int> &rankids) const
{
    int count = 0;
    std::map<int, UdmRankConfig*>::const_iterator itr;
    for (itr = this->rank_list.begin(); itr != this->rank_list.end(); itr++) {
        rankids.push_back(itr->first);
        count++;
    }
    return count;
}


/**
 * ランク番号が存在するかチェックする。
 * @param rankid    ランク番号
 * @return        true=存在する
 */
bool  UdmProcessConfig::existsRankId(int rankid) const
{
    return (this->getRankConfig(rankid) != NULL);
}

/**
 * ランク情報を取得する:const.
 * @param rankid        ランク番号
 * @return        ランク情報:UdmRankConfig
 */
const UdmRankConfig* UdmProcessConfig::getRankConfig(int rankid) const
{
    std::map<int, UdmRankConfig*>::const_iterator itr = this->rank_list.find(rankid);
    if (itr != this->rank_list.end()) {
        return itr->second;
    }
    return NULL;
}

/**
 * ランク情報を取得する.
 * @param rankid        ランク番号
 * @return        ランク情報:UdmRankConfig
 */
UdmRankConfig* UdmProcessConfig::getRankConfig(int rankid)
{
    std::map<int, UdmRankConfig*>::iterator itr = this->rank_list.find(rankid);
    if (itr != this->rank_list.end()) {
        return itr->second;
    }
    return NULL;

}

/**
 * ランク情報を取得する.
 * @param [in]  rankid        ランク番号
 * @param [out] vertex_size    プロセスの領域のノード数
 * @param [out] cell_size        プロセスの領域のセル（要素）数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmProcessConfig::getRankConfig(int rankid, UdmSize_t& vertex_size, UdmSize_t& cell_size) const
{
    const UdmRankConfig* rank = this->getRankConfig(rankid);
    if (rank != NULL) {
        vertex_size = rank->getVertexSize();
        cell_size = rank->getCellSize();
        return UDM_OK;
    }
    return UDM_ERROR;
}


/**
 * ランク情報を設定する.
 * ランク番号が既に存在する場合は上書きする.
 * ランク番号が存在しない場合は、追加する.
 * @param rank_config        ランク情報
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmProcessConfig::setRankConfig(UdmRankConfig* rank_config)
{
    if (rank_config == NULL) {
        return UDM_ERROR_INVALID_PARAMETERS;
    }
    int rankid = rank_config->getRankId();
    UdmRankConfig *src_rank = this->getRankConfig(rankid);
    if (src_rank != NULL) {
        src_rank->setRankConfig(*rank_config);
    }
    else {
        this->rank_list.insert(std::make_pair(rankid, rank_config));
    }

    return UDM_OK;
}


/**
 * ランク情報を設定する.
 * ランク番号が既に存在する場合は上書きする.
 * ランク番号が存在しない場合は、追加する.
 * @param rankid        ランク番号
 * @param vertex_size    プロセスの領域のノード数
 * @param cell_size        プロセスの領域のセル（要素）数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmProcessConfig::setRankConfig(int rankid, UdmSize_t vertex_size, UdmSize_t cell_size)
{
    UdmRankConfig *src_rank = this->getRankConfig(rankid);
    if (src_rank != NULL) {
        src_rank->setRankId(rankid);
        src_rank->setVertexSize(vertex_size);
        src_rank->setCellSize(cell_size);
    }
    else {
        this->rank_list.insert(std::make_pair(rankid, new UdmRankConfig(rankid, vertex_size, cell_size)));
    }

    return UDM_OK;
}

/**
 * ランク情報を削除する.
 * @param rankid        ランク番号
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmProcessConfig::removeRankConfig(int rankid)
{
    std::map<int, UdmRankConfig*>::iterator itr;
    itr = this->rank_list.find(rankid);
    if (itr != this->rank_list.end()) {
        if (itr->second != NULL) delete itr->second;
        this->rank_list.erase(itr);
        return UDM_OK;
    }
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
UdmError_t UdmProcessConfig::getDfiValue(const std::string &label_path, std::string &value) const
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
    if (strcasecmp(label.c_str(), UDM_DFI_PROCESS)) {
        return UDM_ERROR;
    }

    // Rankチェック
    if (name.length() <= std::string(UDM_DFI_RANK).length()+2) return UDM_ERROR;
    if (strcasecmp(name.substr(0, std::string(UDM_DFI_RANK).length()).c_str(), UDM_DFI_RANK)) return UDM_ERROR;

    // Process/Rank[@]
    int rank_id = this->splitLabelIndex(name);
    if (rank_id < 0) return UDM_ERROR;
    if ((size_t)rank_id >= this->rank_list.size()) return UDM_ERROR;
    std::map<int, UdmRankConfig*>::const_iterator itr;
    itr = this->rank_list.find(rank_id);
    if (itr == this->rank_list.end()) return UDM_ERROR;

    return itr->second->getDfiValue(label_path, value);
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
int UdmProcessConfig::getNumDfiValue(const std::string &label_path) const
{
    std::vector<std::string> labels;
    std::string label, name;
    int count;

    // Process : ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 0) return 0;
    label = labels[0];
    if (strcasecmp(label.c_str(), UDM_DFI_PROCESS)) {
        return 0;
    }
    // Processのパラメータ数を返す
    if (count == 1) {
        return this->rank_list.size();
    }

    name = labels[1];
    // /Process/Rankの場合、Rank数を返す
    if (strcasecmp(name.c_str(), UDM_DFI_RANK)) {
        return this->rank_list.size();
    }

    // Sliceチェック
    if (name.length() <= std::string(UDM_DFI_RANK).length()+2) return 0;
    if (strcasecmp(name.substr(0, std::string(UDM_DFI_RANK).length()).c_str(), UDM_DFI_RANK)) return 0;

    // /Process/Rank[@]    // Process/Rank[@]
    int rank_id = this->splitLabelIndex(name);
    if (rank_id < 0) return 0;
    if ((size_t)rank_id >= this->rank_list.size()) return 0;
    std::map<int, UdmRankConfig*>::const_iterator itr;
    itr = this->rank_list.find(rank_id);
    if (itr == this->rank_list.end()) return 0;

    return itr->second->getNumDfiValue(label_path);
}

/**
 * DFIラベルパスのパラメータの設定値を設定する.
 * 数値は文字列から変換して設定する。
 * 複数の設定値を持つことの可能なパラメータの場合は１つのみ設定する.
 * @param [in]  label_path        DFIラベルパス
 * @param [in] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmProcessConfig::setDfiValue(const std::string &label_path, const std::string &value)
{
    std::vector<std::string> labels;
    std::string label, rank_label, name;
    int count;
    UdmError_t error;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 2) return UDM_ERROR;
    label = labels[0];
    rank_label = labels[1];
    name = labels[2];
    if (strcasecmp(label.c_str(), UDM_DFI_PROCESS)) {
        return UDM_ERROR;
    }

    // Rankチェック
    if (rank_label.length() <= std::string(UDM_DFI_RANK).length()+2) return UDM_ERROR;
    if (strcasecmp(rank_label.substr(0, std::string(UDM_DFI_RANK).length()).c_str(), UDM_DFI_RANK)) return UDM_ERROR;

    // Process/Rank[@]/ID
    if (!strcasecmp(rank_label.c_str(), (std::string(UDM_DFI_RANK)+std::string("[@]")).c_str())
        && !strcasecmp(name.c_str(), UDM_DFI_ID)) {
        UdmRankConfig *rank = new UdmRankConfig();
        error = rank->setDfiValue(label_path, value);
        if (error != UDM_OK) {
            delete rank;
            return error;
        }
        this->rank_list.insert(std::make_pair(rank->getRankId(), rank));
        return error;
    }
    // Process/Rank[0..n]
    else if (!strcasecmp(rank_label.substr(0, strlen(UDM_DFI_RANK)).c_str(), UDM_DFI_RANK)) {
        int rank_id = this->splitLabelIndex(rank_label);
        if (rank_id < 0) return UDM_ERROR;
        if ((size_t)rank_id >= this->rank_list.size()) return UDM_ERROR;
        std::map<int, UdmRankConfig*>::const_iterator itr;
        itr = this->rank_list.find(rank_id);
        if (itr == this->rank_list.end()) return UDM_ERROR;

        return itr->second->setDfiValue(label_path, value);
    }

    return UDM_ERROR;
}

/**
 * プロセス領域情報リストをクリアする.
 */
void UdmProcessConfig::clearRankConfig()
{
    std::map<int, UdmRankConfig*>::iterator itr;
    for (itr = this->rank_list.begin(); itr != this->rank_list.end(); itr++) {
        if (itr->second != NULL) delete itr->second;
    }

    this->rank_list.clear();

    return;
}

} /* namespace udm */
