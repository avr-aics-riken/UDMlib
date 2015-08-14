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
#ifndef _UDMTIMESLICECONFIG_H_
#define _UDMTIMESLICECONFIG_H_

/**
 * @file UdmTimeSliceConfig.h
 * index.dfiファイルのTimeSlice, Sliceデータクラスのヘッダーファイル
 */
#include "config/UdmConfigBase.h"
#include "config/UdmDfiValue.h"

namespace udm {

/**
 * index.dfiファイルのTimeSlice/Sliceデータクラス
 */
class UdmSliceConfig: public UdmConfigBase
{
private:
    UdmDfiValue<UdmSize_t>        *step;        ///< 出力ステップ
    UdmDfiValue<float>        *time;        ///< 出力時間（実数）
    UdmDfiValue<UdmSize_t>        *average_step;    ///< 平均ステップ数（整数）
    UdmDfiValue<float>        *average_time;    ///< 平均時間（実数）

    void initialize();

public:
    UdmSliceConfig();
    UdmSliceConfig(TextParser *parser);
    UdmSliceConfig(const UdmSliceConfig &src_slice_config);
    UdmSliceConfig(UdmSize_t step, float time);
    UdmSliceConfig(UdmSize_t step, float time, UdmSize_t average_step, float average_time);
    virtual ~UdmSliceConfig();
    UdmError_t read(const std::string& label);
    UdmError_t write(FILE *fp, unsigned int indent);
    UdmSize_t getStep() const;
    float getTime() const;
    UdmSize_t getAverageStep() const;
    float getAverageTime() const;
    bool isSetStep() const;
    bool isSetTime() const;
    bool isSetAverageStep() const;
    bool isSetAverageTime() const;
    void setStep(UdmSize_t value);
    void setTime(float value);
    void setAverageStep(UdmSize_t value);
    void setAverageTime(float value);
    void unsetStep();
    void unsetTime();
    void unsetAverageStep();
    void unsetAverageTime();
    UdmError_t getDfiValue(const std::string &label_path, std::string &value) const;
    UdmError_t setDfiValue(const std::string &label_path, const std::string &value);
    int getNumDfiValue(const std::string &label_path) const;
};


/**
 * index.dfiファイルのTimeSliceデータクラス
 */
class UdmTimeSliceConfig: public UdmConfigBase
{
private:
    std::vector<UdmSliceConfig*> previous_timeslices;        ///< 以前の時系列リスト:index.dfiから読込TimeSlice
    std::vector<UdmSliceConfig*> current_timeslices;        ///< 現在実行中の時系列リスト:index.dfiへの書込TimeSlice

public:
    UdmTimeSliceConfig();
    UdmTimeSliceConfig(TextParser *parser);
    virtual ~UdmTimeSliceConfig();
    UdmError_t read();
    UdmError_t write(FILE *fp, unsigned int indent);
    UdmSize_t getNumPreviousTimeSlices() const;
    const UdmSliceConfig* getPreviousSliceConfig(UdmSize_t slice_id) const;
    const UdmSliceConfig* getPreviousSliceConfigByStep(UdmSize_t step) const;
    UdmError_t getPreviousTimeSlice(UdmSize_t slice_id, UdmSize_t &step, float &time) const;
    UdmError_t getPreviousTimeSlice(UdmSize_t slice_id, UdmSize_t &step, float &time, UdmSize_t &average_step, float &average_time) const;
    UdmError_t getPreviousTimeSliceByStep(UdmSize_t step, float &time) const;
    UdmError_t getPreviousTimeSliceByStep(UdmSize_t step, float &time, UdmSize_t &average_step, float &average_time) const;

    UdmSize_t getNumCurrentTimeSlices() const;
    UdmSliceConfig* getCurrentSliceConfig(UdmSize_t slice_id) const;
    UdmSliceConfig* getCurrentSliceConfigByStep(UdmSize_t step) const;
    UdmError_t getCurrentTimeSlice(UdmSize_t slice_id, UdmSize_t &step, float &time) const;
    UdmError_t getCurrentTimeSlice(UdmSize_t slice_id, UdmSize_t &step, float &time, UdmSize_t &average_step, float &average_time) const;
    UdmError_t getCurrentTimeSliceByStep(UdmSize_t step, float &time) const;
    UdmError_t getCurrentTimeSliceByStep(UdmSize_t step, float &time, UdmSize_t &average_step, float &average_time) const;

    UdmError_t setCurrentTimeSlice(UdmSize_t step, float time);
    UdmError_t setCurrentTimeSlice(UdmSize_t step, float time, UdmSize_t average_step, float average_time);
    UdmError_t removeCurrentTimeSlice(UdmSize_t step);
    UdmError_t clearCurrentTimeSlices();
    UdmError_t getDfiValue(const std::string &label_path, std::string &value) const;
    UdmError_t setDfiValue(const std::string &label_path, const std::string &value);
    int getNumDfiValue(const std::string &label_path) const;

private:
    void initialize();
    UdmError_t insertCurrentSliceConfig(UdmSliceConfig *config);
};

} /* namespace udm */

#endif /* _UDMTIMESLICECONFIG_H_ */
