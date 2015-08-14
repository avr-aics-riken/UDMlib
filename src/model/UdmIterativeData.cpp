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
 * @file UdmIterativeData.cpp
 * 時系列データクラスのソースファイル
 */

#include "model/UdmIterativeData.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmIterativeData::UdmIterativeData()
{
    this->initialize();
}

/**
 * デストラクタ
 */
UdmIterativeData::~UdmIterativeData()
{
}

/**
 * 初期化を行う.
 */
void UdmIterativeData::initialize()
{
    this->step = 0;
    this->time = 0.0;
    this->average_step = 0;
    this->average_time = 0.0;
    this->isset_averages = false;
}

/**
 * 平均ステップ数を取得する.
 * @return        平均ステップ数
 */
UdmSize_t udm::UdmIterativeData::getAverageStep() const
{
    return this->average_step;
}

/**
 * 平均時間を取得する.
 * @return        平均時間
 */
double udm::UdmIterativeData::getAverageTime() const
{
    return this->average_time;
}

/**
 * 平均ステップ数, 平均時間が設定されているかチェックする.
 * @return        true=平均ステップ数, 平均時間が設定されている.
 */
bool udm::UdmIterativeData::isSetAverages() const
{
    return this->isset_averages;
}

/**
 * ステップ数を取得する.
 * @return        ステップ数
 */
UdmSize_t udm::UdmIterativeData::getStep() const
{
    return this->step;
}

/**
 * ステップ時間を取得する.
 * @return        ステップ時間
 */
double udm::UdmIterativeData::getTime() const
{
    return this->time;
}

/**
 * ステップ数、ステップ時間を設定する.
 * @param step        ステップ数
 * @param time        ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmIterativeData::setIterativeData(UdmSize_t step, double time)
{
    this->step = step;
    this->time = time;
    this->isset_averages = false;

    return UDM_OK;
}

/**
 * ステップ数、ステップ時間, 平均ステップ数, 平均ステップ時間を設定する.
 * @param step        ステップ数
 * @param time        ステップ時間
 * @param average_step        平均ステップ数
 * @param average_time        平均ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmIterativeData::setIterativeData(
                    UdmSize_t step, double time,
                    UdmSize_t average_step, double average_time)
{
    this->step = step;
    this->time = time;
    this->average_step = average_step;
    this->average_time = average_time;
    this->isset_averages = true;

    return UDM_OK;
}

} /* namespace udm */
