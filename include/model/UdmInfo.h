/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

#ifndef _UDMINFO_H_
#define _UDMINFO_H_

#include <cgnslib.h>
#include "UdmBase.h"

/**
 * @file UdmInfo.h
 * CGNS:UdmInfo情報クラスのヘッダーファイル
 */

namespace udm
{
class UdmModel;

/**
 * CGNS:UdmInfo情報クラス
 */
class UdmInfo : public UdmBase
{
private:
    std::string udmlib_version;            ///< UDMlibバージョン情報
    int info_rankno;                    ///< 出力MPIランク番号
    int info_process;                    ///< 出力MPIプロセス数
    UdmModel *parent_model;                ///< 親モデル（ベース）

public:
    UdmInfo();
    UdmInfo(UdmModel *model);
    virtual ~UdmInfo();

    // UdmInfo情報
    int getUdminfoProcessSize() const;
    void setUdminfoProcessSize(int process);
    int getUdminfoRankno() const;
    void setUdminfoRankno(int rankno);
    const std::string& getUdmlibVersion() const;
    void setUdmlibVersion(const std::string& version);

    // UdmModel
    UdmModel* getParentModel() const;
    UdmError_t setParentModel(UdmModel *model);

    // CGNS
    UdmError_t readCgns(int index_file, int index_base);
    UdmError_t writeCgns(int index_file, int index_base);

private:
    void initialize();
};

} /* namespace udm */


#endif /* _UDMINFO_H_ */
