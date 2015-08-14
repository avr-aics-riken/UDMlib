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

#ifndef _UDMPROCESSCONFIG_H_
#define _UDMPROCESSCONFIG_H_

/**
 * @file UdmProcessConfig.h
 * proc.dfiファイルのProcessデータクラスのヘッダーファイル
 */

#include "config/UdmConfigBase.h"

namespace udm {

/**
 * proc.dfiファイルのProcess/Rankデータクラス
 */
class UdmRankConfig: public UdmConfigBase {
private:
    int        rankid;            ///< ランク番号
    UdmSize_t     vertex_size;            ///< プロセスの領域のノード数
    UdmSize_t    cell_size;                ///< プロセスの領域のセル（要素）数

public:
    UdmRankConfig();
    UdmRankConfig(TextParser *parser);
    UdmRankConfig(const UdmRankConfig &src_rank_config);
    UdmRankConfig(int rankid, UdmSize_t vertex_size, UdmSize_t cell_size);
    virtual ~UdmRankConfig();
    UdmError_t read(const std::string &rank_label);
    UdmError_t write(FILE *fp, unsigned int indent);
    int getRankId() const;
    void setRankId(int rankid);
    UdmSize_t getVertexSize() const;
    void setVertexSize(UdmSize_t vertexSize);
    UdmSize_t getCellSize() const;
    void setCellSize(UdmSize_t cellSize);
    UdmError_t setRankConfig(const UdmRankConfig &src_rank_config);
    UdmError_t getDfiValue(const std::string &label_path, std::string &value) const;
    UdmError_t setDfiValue(const std::string &label_path, const std::string &value);
    int getNumDfiValue(const std::string &label_path) const;

private:
    void initialize();
};


/**
 * proc.dfiファイルのProcessデータクラス
 */
class UdmProcessConfig: public UdmConfigBase
{
private:
    std::map<int, UdmRankConfig*>        rank_list;        ///< プロセス領域情報リスト : <ID, UdmRankConfig>

public:
    UdmProcessConfig();
    UdmProcessConfig(TextParser *parser);
    virtual ~UdmProcessConfig();
    UdmError_t read();
    UdmError_t write(FILE *fp, unsigned int indent);
    unsigned int  getNumRankConfig() const;
    unsigned int getRankIdList(std::vector<int> &rankids) const;
    bool  existsRankId(int rankid) const;
    const UdmRankConfig* getRankConfig(int rankid) const;
    UdmRankConfig* getRankConfig(int rankid);
    UdmError_t getRankConfig(int rankid, UdmSize_t& vertex_size, UdmSize_t& cell_size) const;
    UdmError_t setRankConfig(UdmRankConfig* rank_config);
    UdmError_t setRankConfig(int rankid, UdmSize_t vertex_size, UdmSize_t cell_size);
    UdmError_t removeRankConfig(int rankid);
    UdmError_t getDfiValue(const std::string &label_path, std::string &value) const;
    UdmError_t setDfiValue(const std::string &label_path, const std::string &value);
    int getNumDfiValue(const std::string &label_path) const;
    void clearRankConfig();

private:
    void initialize();
};

} /* namespace udm */

#endif /* _UDMPROCESSCONFIG_H_ */
