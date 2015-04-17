/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

#ifndef _UDMITERATIVEDATA_H_
#define _UDMITERATIVEDATA_H_

/**
 * @file UdmIterativeData.h
 * 時系列データクラスのヘッダーファイル
 */

#include "model/UdmGeneral.h"

namespace udm
{

/**
 * 時系列クラス
 */
class UdmIterativeData: public UdmGeneral
{
private:
    UdmSize_t     step;        ///< 出力ステップ
    double        time;        ///< 出力時間（実数）
    UdmSize_t     average_step;    ///< 平均ステップ数（整数）
    double        average_time;    ///< 平均時間（実数）
    bool isset_averages;

public:
    UdmIterativeData();
    virtual ~UdmIterativeData();
    UdmSize_t getAverageStep() const;
    double getAverageTime() const;
    UdmSize_t getStep() const;
    double getTime() const;
    bool isSetAverages() const;
    UdmError_t setIterativeData(UdmSize_t step, double time);
    UdmError_t setIterativeData(UdmSize_t step, double time, UdmSize_t average_step, double average_time);

private:
    void initialize();
};

} /* namespace udm */

#endif /* _UDMITERATIVEDATA_H_ */
