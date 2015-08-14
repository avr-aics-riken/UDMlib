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
 * @file UdmTimeSlice.cpp
 * index.dfiファイルのTimeSlice,Sliceデータクラスのソースファイル
 */
#include "config/UdmTimeSliceConfig.h"

namespace udm {

/*******************************************************
 * class UdmSliceConfig
 ******************************************************/

/**
 * コンストラクタ
 */
UdmSliceConfig::UdmSliceConfig()  : UdmConfigBase(NULL), step(NULL), time(NULL), average_step(NULL), average_time(NULL)
{
    this->initialize();
}

/**
 * コンストラクタ:TextParser
 * @param  parser        TextParser
 */
UdmSliceConfig::UdmSliceConfig(TextParser *parser) : UdmConfigBase(parser), step(NULL), time(NULL), average_step(NULL), average_time(NULL)
{
    this->initialize();
}

/**
 * コンストラクタ (時系列ステップ番号,時系列ステップ時間).
 * @param step        時系列ステップ番号
 * @param time        時系列ステップ時間
 */
UdmSliceConfig::UdmSliceConfig(UdmSize_t step, float time)
{
    this->initialize();
    this->setStep(step);
    this->setTime(time);
}

/**
 * コンストラクタ (時系列ステップ番号,時系列ステップ時間,平均ステップ番号,平均ステップ時間)
 * @param step        時系列ステップ番号
 * @param time        時系列ステップ時間
 * @param average_step    平均ステップ番号
 * @param average_time    平均ステップ時間
 */
UdmSliceConfig::UdmSliceConfig(UdmSize_t step, float time, UdmSize_t average_step, float average_time)
{
    this->initialize();
    this->setStep(step);
    this->setTime(time);
    this->setAverageStep(average_step);
    this->setAverageTime(average_time);
}


/**
 * 初期化を行う.
 */
void UdmSliceConfig::initialize()
{
    this->step = NULL;
    this->time = NULL;
    this->average_step = NULL;
    this->average_time = NULL;
}


/**
 * デストラクタ
 */
UdmSliceConfig::~UdmSliceConfig()
{
    // 時系列ステップ番号
    if (this->step != NULL) delete this->step;
    // 時系列ステップ時間（実数）
    if (this->time != NULL) delete this->time;
    // 平均ステップ番号（整数）
    if (this->average_step != NULL) delete this->average_step;
    // 平均ステップ時間（実数）
    if (this->average_time != NULL) delete this->average_time;
}

/**
 * DFIファイルからTimeSlice/Slice要素を読みこむ.
 * @param    slice_label            Sliceラベル
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSliceConfig::read(const std::string& slice_label)
{
    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }

    std::string label, value, slice_node, label_leaf;
    UdmError_t ierror = UDM_OK;
    std::vector<std::string> child_list;

    // TimeSlice/Slice[@]
    std::vector<std::string> path_list;
    int count = this->split(slice_label, '/', path_list);
    if (count != 2) {
        return UDM_ERROR_INVALID_DFI_ELEMENTPATH;
    }
    if (strcasecmp(path_list[0].c_str(), UDM_DFI_TIMESLICE)) {
        return UDM_ERROR_INVALID_DFI_ELEMENTPATH;
    }

    // TimeSlice/Slice[@]/Step : 時系列ステップ番号
    label = slice_label + "/" + UDM_DFI_STEP;
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->step = new UdmDfiValue<UdmSize_t>(UDM_DFI_STEP, this->convertLong(value, &ierror));
    }

    // TimeSlice/Slice[@]/Time : 時系列ステップ時間
    label = slice_label + "/" + UDM_DFI_TIME;
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->time = new UdmDfiValue<float>(UDM_DFI_TIME, this->convertDouble(value, &ierror));
    }

    // TimeSlice/Slice[@]/AverageStep  : 平均ステップ番号
    label = slice_label + "/" + UDM_DFI_AVERAGESTEP;
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->average_step = new UdmDfiValue<UdmSize_t>(UDM_DFI_AVERAGESTEP, this->convertLong(value, &ierror));
    }

    // TimeSlice/Slice[@]/AverageTime : 平均ステップ時間
    label = slice_label + "/" + UDM_DFI_AVERAGETIME;
    if ( this->getValue(label, value ) == UDM_OK ) {
        this->average_time = new UdmDfiValue<float>(UDM_DFI_AVERAGETIME, this->convertDouble(value, &ierror));
    }

    return UDM_OK;
}

/**
 * TimeSlice/Slice要素をDFIファイルに出力する.
 * @param fp        出力ファイルポインタ
 * @param indent    出力インデント
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSliceConfig::write(FILE *fp, unsigned int indent)
{
    std::string label, value;
    UdmError_t error;

    // TimeSlice/Slice[@] : open
    label = "Slice[@]";
    if (this->writeLabel(fp, indent, label) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }
    indent++;

    // TimeSlice/Slice[@]/Step : 時系列ステップ番号
    if (this->step != NULL && this->step->isSetValue()) {
        label = UDM_DFI_STEP;
        this->writeNodeValue(fp, indent, label, this->step->getValue() );
    }

    // TimeSlice/Slice[@]/Time : 時系列ステップ時間
    if (this->time != NULL && this->time->isSetValue()) {
        label = UDM_DFI_TIME;
        this->writeNodeValue(fp, indent, label, this->time->getValue());
    }

    // TimeSlice/Slice[@]/AverageStep  : 平均ステップ番号
    if (this->average_step != NULL && this->average_step->isSetValue()) {
        label = UDM_DFI_AVERAGESTEP;
        this->writeNodeValue(fp, indent, label, this->average_step->getValue());
    }

    // TimeSlice/Slice[@]/AverageTime : 平均ステップ時間
    if (this->average_time != NULL && this->average_time->isSetValue()) {
        label = UDM_DFI_AVERAGETIME;
        this->writeNodeValue(fp, indent, label, this->average_time->getValue());
    }

    // TimeSlice/Slice[@] : close
    if (this->writeCloseTab(fp, --indent) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }

    return UDM_OK;
}

/**
 * /TimeSlice/Slice[@]/Step : 時系列ステップ番号を取得する.
 * @return  時系列ステップ番号
 */
UdmSize_t UdmSliceConfig::getStep() const
{
    if (this->step == NULL) return 0;
    return this->step->getValue();
}


/**
 * /TimeSlice/Slice[@]/Time : 時系列ステップ時間を取得する.
 * @return  時系列ステップ時間
 */
float UdmSliceConfig::getTime() const
{
    if (this->time == NULL) return 0.0;
    return this->time->getValue();
}

/**
 * /TimeSlice/Slice[@]/AverageStep  : 平均ステップ番号を取得する.
 * @return  平均ステップ番号
 */
UdmSize_t UdmSliceConfig::getAverageStep() const
{
    if (this->average_step == NULL) return 0;
    return this->average_step->getValue();
}


/**
 * TimeSlice/Slice[@]/AverageTime : 平均ステップ時間を取得する.
 * @return  平均ステップ時間
 */
float UdmSliceConfig::getAverageTime() const
{
    if (this->average_time == NULL) return 0.0;
    return this->average_time->getValue();

}


/**
 * /TimeSlice/Slice[@]/Step : 時系列ステップ番号が設定されているかチェックする.
 * @return  true=時系列ステップ番号が設定されている.
 */
bool UdmSliceConfig::isSetStep() const
{
    return (this->step != NULL);
}

/**
 * /TimeSlice/Slice[@]/Time : 時系列ステップ時間が設定されているかチェックする.
 * @return  true=時系列ステップ時間が設定されている.
 */
bool UdmSliceConfig::isSetTime() const
{
    return (this->time != NULL);
}


/**
 * /TimeSlice/Slice[@]/AverageStep  : 平均ステップ番号が設定されているかチェックする.
 * @return  true=平均ステップ番号が設定されている.
 */
bool UdmSliceConfig::isSetAverageStep() const
{
    return (this->average_step != NULL);
}


/**
 * TimeSlice/Slice[@]/AverageTime : 平均ステップ時間が設定されているかチェックする.
 * @return  true=平均ステップ時間が設定されている.
 */
bool UdmSliceConfig::isSetAverageTime() const
{
    return (this->average_time != NULL);
}


/**
 * /TimeSlice/Slice[@]/Step : 時系列ステップ番号を設定する.
 * @param value        時系列ステップ番号
 */
void UdmSliceConfig::setStep(UdmSize_t value)
{
    if (this->step == NULL) {
        this->step = new UdmDfiValue<UdmSize_t>(UDM_DFI_STEP, value);
    }
    else {
        this->step->setValue(value);
    }
}


/**
 * /TimeSlice/Slice[@]/Time : 時系列ステップ時間を設定する.
 * @param value        時系列ステップ時間
 */
void UdmSliceConfig::setTime(float value)
{
    if (this->time == NULL) {
        this->time = new UdmDfiValue<float>(UDM_DFI_TIME, value);
    }
    else {
        this->time->setValue(value);
    }
}

/**
 * /TimeSlice/Slice[@]/AverageStep  : 平均ステップ番号を設定する.
 * @param value        平均ステップ番号
 */
void UdmSliceConfig::setAverageStep(UdmSize_t value)
{
    if (this->average_step == NULL) {
        this->average_step = new UdmDfiValue<UdmSize_t>(UDM_DFI_AVERAGESTEP, value);
    }
    else {
        this->average_step->setValue(value);
    }
}


/**
 * /TimeSlice/Slice[@]/AverageTime : 平均ステップ時間を設定する.
 * @param value        平均ステップ時間
 */
void UdmSliceConfig::setAverageTime(float value)
{
    if (this->average_time == NULL) {
        this->average_time = new UdmDfiValue<float>(UDM_DFI_AVERAGETIME, value);
    }
    else {
        this->average_time->setValue(value);
    }
}

/**
 * /TimeSlice/Slice[@]/Step : 時系列ステップ番号をを削除する.
 * stepをdeleteする.
 */
void UdmSliceConfig::unsetStep()
{
    if (this->step != NULL) {
        delete this->step;
        this->step = NULL;
    }
}

/**
 * /TimeSlice/Slice[@]/Time : 時系列ステップ時間をを削除する.
 * timeをdeleteする.
 */
void UdmSliceConfig::unsetTime()
{
    if (this->time != NULL) {
        delete this->time;
        this->time = NULL;
    }
}

/**
 * /TimeSlice/Slice[@]/AverageStep  : 平均ステップ番号をを削除する.
 * average_stepをdeleteする.
 */
void UdmSliceConfig::unsetAverageStep()
{
    if (this->average_step != NULL) {
        delete this->average_step;
        this->average_step = NULL;
    }
}

/**
 * /TimeSlice/Slice[@]/AverageTime  : 平均ステップ時間をを削除する.
 * average_timeをdeleteする.
 */
void UdmSliceConfig::unsetAverageTime()
{
    if (this->average_time != NULL) {
        delete this->average_time;
        this->average_time = NULL;
    }
}


/**
 * DFIラベルパスのパラメータの設定値を取得する.
 * 数値は文字列に変換して返す。
 * 複数の設定値のあるパラメータの場合はUDM_ERRORを返す。
 * @param [in]  label_path        DFIラベルパス
 * @param [out] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmSliceConfig::getDfiValue(const std::string &label_path, std::string &value) const
{
    std::vector<std::string> labels;
    std::string label, slice_name, name;
    int count;
    UdmError_t error = UDM_OK;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count != 3) return UDM_ERROR;
    label = labels[0];
    slice_name = labels[1];
    name = labels[2];
    if (strcasecmp(label.c_str(), UDM_DFI_TIMESLICE)) {
        return UDM_ERROR;
    }

    // Sliceチェック
    if (slice_name.length() <= std::string(UDM_DFI_SLICE).length()+2) return UDM_ERROR;
    if (strcasecmp(slice_name.substr(0, std::string(UDM_DFI_SLICE).length()).c_str(), UDM_DFI_SLICE)) return UDM_ERROR;

    // TimeSlice/Slice[@]/Step : 時系列ステップ番号
    if (!strcasecmp(name.c_str(), UDM_DFI_STEP)) {
        if (!this->isSetStep()) return UDM_ERROR;
        error = this->toString(this->getStep(), value);
        return error;
    }
    // TimeSlice/Slice[@]/Time : 時系列ステップ時間
    else if (!strcasecmp(name.c_str(), UDM_DFI_TIME)) {
        if (!this->isSetTime()) return UDM_ERROR;
        error = this->toString(this->getTime(), value);
        return error;
    }
    // TimeSlice/Slice[@]/AverageStep  : 平均ステップ番号
    else if (!strcasecmp(name.c_str(), UDM_DFI_AVERAGESTEP)) {
        if (!this->isSetAverageStep()) return UDM_ERROR;
        error = this->toString(this->getAverageStep(), value);
        return error;
    }
    // TimeSlice/Slice[@]/AverageTime : 平均ステップ時間
    else if (!strcasecmp(name.c_str(), UDM_DFI_AVERAGETIME)) {
        if (!this->isSetAverageTime()) return UDM_ERROR;
        error = this->toString(this->getAverageTime(), value);
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
int UdmSliceConfig::getNumDfiValue(const std::string &label_path) const
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
    if (strcasecmp(label.c_str(), UDM_DFI_TIMESLICE)) {
        return 0;
    }

    // Sliceチェック
    if (name.length() <= std::string(UDM_DFI_SLICE).length()+2) return 0;
    if (strcasecmp(name.substr(0, std::string(UDM_DFI_SLICE).length()).c_str(), UDM_DFI_SLICE)) return 0;

    // TimeSlice/Slice[@]のパラメータ数を返す
    if (count == 2) {
        return  (  (this->isSetStep()?1:0)
                 + (this->isSetTime()?1:0)
                 + (this->isSetAverageStep()?1:0)
                 + (this->isSetAverageTime()?1:0)
                );
    }
    if (count != 3) return 0;
    name = labels[2];

    // TimeSlice/Slice[@]/Step : 時系列ステップ番号
    if (!strcasecmp(name.c_str(), UDM_DFI_STEP)) {
        if (!this->isSetStep()) return 0;
        return 1;
    }
    // TimeSlice/Slice[@]/Time : 時系列ステップ時間
    else if (!strcasecmp(name.c_str(), UDM_DFI_TIME)) {
        if (!this->isSetTime()) return 0;
        return 1;
    }
    // TimeSlice/Slice[@]/AverageStep  : 平均ステップ番号
    else if (!strcasecmp(name.c_str(), UDM_DFI_AVERAGESTEP)) {
        if (!this->isSetAverageStep()) return 0;
        return 1;
    }
    // TimeSlice/Slice[@]/AverageTime : 平均ステップ時間
    else if (!strcasecmp(name.c_str(), UDM_DFI_AVERAGETIME)) {
        if (!this->isSetAverageTime()) return 0;
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
UdmError_t UdmSliceConfig::setDfiValue(const std::string &label_path, const std::string &value)
{
    std::vector<std::string> labels;
    std::string label, slice_name, name;
    int count;
    UdmError_t error = UDM_OK;

    // TimeSlice
    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count != 3) return UDM_ERROR;
    label = labels[0];
    slice_name = labels[1];
    name = labels[2];
    if (strcasecmp(label.c_str(), UDM_DFI_TIMESLICE)) {
        return UDM_ERROR;
    }

    // Sliceチェック
    if (slice_name.length() <= std::string(UDM_DFI_SLICE).length()+2) return UDM_ERROR;
    if (strcasecmp(slice_name.substr(0, std::string(UDM_DFI_SLICE).length()).c_str(), UDM_DFI_SLICE)) return UDM_ERROR;

    // TimeSlice/Slice[@]/Step : 時系列ステップ番号
    if (!strcasecmp(name.c_str(), UDM_DFI_STEP)) {
        this->setStep(this->convertLong(value, &error));
        return error;
    }
    // TimeSlice/Slice[@]/Time : 時系列ステップ時間
    else if (!strcasecmp(name.c_str(), UDM_DFI_TIME)) {
        this->setTime(this->convertDouble(value, &error));
        return error;
    }
    // TimeSlice/Slice[@]/AverageStep  : 平均ステップ番号
    else if (!strcasecmp(name.c_str(), UDM_DFI_AVERAGESTEP)) {
        this->setAverageStep(this->convertLong(value, &error));
        return error;
    }
    // TimeSlice/Slice[@]/AverageTime : 平均ステップ時間
    else if (!strcasecmp(name.c_str(), UDM_DFI_AVERAGETIME)) {
        this->setAverageTime(this->convertDouble(value, &error));
        return error;
    }
    else {
        return UDM_ERROR;
    }

    return UDM_OK;
}


/*******************************************************
 * class UdmTimeSliceConfig
 ******************************************************/

/**
 * コンストラクタ
 */
UdmTimeSliceConfig::UdmTimeSliceConfig()  : UdmConfigBase(NULL)
{
    this->initialize();
}

/**
 * コンストラクタ:TextParser
 * @param  parser        TextParser
 */
UdmTimeSliceConfig::UdmTimeSliceConfig(TextParser *parser) : UdmConfigBase(parser)
{
    this->initialize();
}

/**
 * 初期化を行う.
 */
void UdmTimeSliceConfig::initialize()
{
    this->previous_timeslices.clear();
}


/**
 * デストラクタ
 */
UdmTimeSliceConfig::~UdmTimeSliceConfig()
{
    std::vector<UdmSliceConfig*>::iterator itr;
    for (itr = this->previous_timeslices.begin(); itr != this->previous_timeslices.end(); itr++) {
        if (*itr) delete *itr;
    }

    for (itr = this->current_timeslices.begin(); itr != this->current_timeslices.end(); itr++) {
        if (*itr) delete *itr;
    }
}


/**
 * DFIファイルからTimeSlice要素を読みこむ.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmTimeSliceConfig::read()
{
    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }

    std::string label, value, label_leaf;
    std::vector<std::string> child_list;

    // TimeSlice
    label = "/TimeSlice";
    if (!this->checkNode(label)) {
        return UDM_WARNING_DFI_NOTFOUND_NODE;
    }
    if ( this->getChildLabels(label, child_list) != UDM_OK) {
        return UDM_ERROR;
    }

    std::vector<std::string>::iterator itr;
    for (itr=child_list.begin(); itr<child_list.end(); itr++) {
        UdmSliceConfig *slice = new UdmSliceConfig(this->parser);
        if( slice->read(*itr) == UDM_OK ) {
            this->previous_timeslices.push_back(slice);
        }
    }

    return UDM_OK;
}

/**
 * TimeSlice要素をDFIファイルに出力する.
 * @param fp        出力ファイルポインタ
 * @param indent    出力インデント
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmTimeSliceConfig::write(FILE *fp, unsigned int indent)
{
    std::string label, value;
    UdmError_t error;

    // TimeSlice : open
    label = "TimeSlice";
    if (this->writeLabel(fp, indent, label) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }
    indent++;

    // TimeSlice/Slice[@]
    std::vector<UdmSliceConfig*>::const_iterator itr;
    for (itr = this->current_timeslices.begin(); itr != this->current_timeslices.end(); itr++) {
        error = (*itr)->write(fp, indent);
        if (error != UDM_OK) {
            return error;
        }
    }

    // TimeSlice : close
    if (this->writeCloseTab(fp, --indent) != UDM_OK) {
        return UDM_ERROR_WRITE_DFIFILE_WRITENODE;
    }

    return UDM_OK;
}

/**
 * index.dfiから読込の/TimeSlice/Slice[@]要素数を取得する.
 * @return        /TimeSlice/Slice[@]要素数
 */
UdmSize_t UdmTimeSliceConfig::getNumPreviousTimeSlices() const
{
    return this->previous_timeslices.size();
}

/**
 * index.dfiから読込の/TimeSlice/Slice[@]の添字の要素を取得する:const.
 * @param slice_id        Slice[@]添字（１～）
 * @return        Slice情報クラス
 */
const UdmSliceConfig* UdmTimeSliceConfig::getPreviousSliceConfig(UdmSize_t slice_id) const
{
    if (slice_id <= 0) return NULL;
    if (slice_id > this->previous_timeslices.size()) return NULL;
    return this->previous_timeslices[slice_id-1];
}


/**
 * index.dfiから読込の/TimeSlice/Slice[@]のStep, Timeを取得する.
 * @param [in]  slice_id        Slice[@]添字（１～）
 * @param [out] step            時系列ステップ番号
 * @param [out] time            時系列ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmTimeSliceConfig::getPreviousTimeSlice(UdmSize_t slice_id, UdmSize_t &step, float &time) const
{
    const UdmSliceConfig* slice = this->getPreviousSliceConfig(slice_id);
    if (slice == NULL)  return UDM_ERROR;
    step = slice->getStep();
    time = slice->getTime();

    return UDM_OK;
}

/**
 * index.dfiから読込の/TimeSlice/Slice[@]のStep, Time, AverageStep, AverageTimeを取得する.
 * @param [in] slice_id        Slice[@]添字（１～）
 * @param [out] step            時系列ステップ番号
 * @param [out] time            時系列ステップ時間
 * @param [out] average_step        平均ステップ番号
 * @param [out] average_time        平均ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmTimeSliceConfig::getPreviousTimeSlice(UdmSize_t slice_id, UdmSize_t &step, float &time, UdmSize_t &average_step, float &average_time) const
{
    const UdmSliceConfig* slice = this->getPreviousSliceConfig(slice_id);
    if (slice == NULL)  return UDM_ERROR;
    step = slice->getStep();
    time = slice->getTime();
    average_step = slice->getAverageStep();
    average_time = slice->getAverageTime();

    return UDM_OK;
}

/**
 * index.dfiから読込の/TimeSlice/Slice[@]のTimeを取得する.
 * 時系列ステップ番号にて設定の時系列情報を取得する.
 * @param [in]  step            時系列ステップ番号
 * @param [out] time            時系列ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmTimeSliceConfig::getPreviousTimeSliceByStep(UdmSize_t step, float &time) const
{
    const UdmSliceConfig* config = this->getPreviousSliceConfigByStep(step);
    if (config == NULL) return UDM_ERROR;
    time = config->getTime();

    return UDM_OK;
}


/**
 * index.dfiから読込の/TimeSlice/Slice[@]のTime, AverageStep, AverageTimeを取得する.
 * 時系列ステップ番号にて設定の時系列情報を取得する.
 * @param [in]  step            時系列ステップ番号
 * @param [out] time            時系列ステップ時間
 * @param [out] average_step        平均ステップ番号
 * @param [out] average_time        平均ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmTimeSliceConfig::getPreviousTimeSliceByStep(UdmSize_t step, float &time, UdmSize_t &average_step, float &average_time) const
{
    const UdmSliceConfig* config = this->getPreviousSliceConfigByStep(step);
    if (config == NULL) return UDM_ERROR;
    time = config->getTime();
    average_step = config->getAverageStep();
    average_time = config->getAverageTime();

    return UDM_OK;
}


/**
 * 現在実行中の/TimeSlice/Slice[@]要素数を取得する.
 * @return        /TimeSlice/Slice[@]要素数
 */
UdmSize_t UdmTimeSliceConfig::getNumCurrentTimeSlices() const
{
    return this->current_timeslices.size();
}

/**
 * 現在実行中の/TimeSlice/Slice[@]の添字の要素を取得する:const.
 * @param slice_id        Slice[@]添字（１～）
 * @return        Slice情報クラス
 */
UdmSliceConfig* UdmTimeSliceConfig::getCurrentSliceConfig(UdmSize_t slice_id) const
{
    if (slice_id <= 0) return NULL;
    if (slice_id > this->current_timeslices.size()) return NULL;
    return this->current_timeslices[slice_id-1];
}


/**
 * 現在実行中の/TimeSlice/Slice[@]のStep, Timeを取得する.
 * @param slice_id        Slice[@]添字（１～）
 * @param step            時系列ステップ番号
 * @param time            時系列ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmTimeSliceConfig::getCurrentTimeSlice(UdmSize_t slice_id, UdmSize_t &step, float &time) const
{
    const UdmSliceConfig* slice = this->getCurrentSliceConfig(slice_id);
    if (slice == NULL)  return UDM_ERROR;
    step = slice->getStep();
    time = slice->getTime();

    return UDM_OK;
}

/**
 * 現在実行中の/TimeSlice/Slice[@]のStep, Time, AverageStep, AverageTimeを取得する.
 * @param slice_id        Slice[@]添字（１～）
 * @param step            時系列ステップ番号
 * @param time            時系列ステップ時間
 * @param average_step        平均ステップ番号
 * @param average_time        平均ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmTimeSliceConfig::getCurrentTimeSlice(UdmSize_t slice_id, UdmSize_t &step, float &time, UdmSize_t &average_step, float &average_time) const
{
    const UdmSliceConfig* slice = this->getCurrentSliceConfig(slice_id);
    if (slice == NULL)  return UDM_ERROR;
    step = slice->getStep();
    time = slice->getTime();
    average_step = slice->getAverageStep();
    average_time = slice->getAverageTime();

    return UDM_OK;
}

/**
 * 現在実行中の/TimeSlice/Slice[@]のTimeを取得する.
 * 時系列ステップ番号にて設定の時系列情報を取得する.
 * @param [in]  step            時系列ステップ番号
 * @param [out] time            時系列ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmTimeSliceConfig::getCurrentTimeSliceByStep(UdmSize_t step, float &time) const
{
    const UdmSliceConfig* config = this->getCurrentSliceConfigByStep(step);
    if (config == NULL) return UDM_ERROR;
    time = config->getTime();

    return UDM_OK;
}


/**
 * 現在実行中の/TimeSlice/Slice[@]のTime, AverageStep, AverageTimeを取得する.
 * 時系列ステップ番号にて設定の時系列情報を取得する.
 * @param [in]  step            時系列ステップ番号
 * @param [out] time            時系列ステップ時間
 * @param [out] average_step        平均ステップ番号
 * @param [out] average_time        平均ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmTimeSliceConfig::getCurrentTimeSliceByStep(UdmSize_t step, float &time, UdmSize_t &average_step, float &average_time) const
{
    const UdmSliceConfig* config = this->getCurrentSliceConfigByStep(step);
    if (config == NULL) return UDM_ERROR;
    time = config->getTime();
    average_step = config->getAverageStep();
    average_time = config->getAverageTime();

    return UDM_OK;
}

/**
 * 現在実行中の/TimeSlice/Slice[@]のStep, Timeを設定する.
 * 時系列ステップ番号が追加済みの場合は、上書きを行う.
 * 時系列ステップ番号が未設定の場合は、追加を行う.
 * @param step            時系列ステップ番号
 * @param time            時系列ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmTimeSliceConfig::setCurrentTimeSlice(UdmSize_t step, float time)
{
    UdmSliceConfig* slice = this->getCurrentSliceConfigByStep(step);
    if (slice == NULL)  {
        slice = new UdmSliceConfig(step, time);
        this->insertCurrentSliceConfig(slice);
    }
    else {
        slice->setStep(step);
        slice->setTime(time);
        slice->unsetAverageStep();
        slice->unsetAverageTime();
    }
    return UDM_OK;
}

/**
 * 現在実行中の/TimeSlice/Slice[@]のStep, Time, AverageStep, AverageTimeを設定する.
 * 時系列ステップ番号が追加済みの場合は、上書きを行う.
 * 時系列ステップ番号が未設定の場合は、追加を行う.
 * @param step            時系列ステップ番号
 * @param time            時系列ステップ時間
 * @param average_step        平均ステップ番号
 * @param average_time        平均ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmTimeSliceConfig::setCurrentTimeSlice(UdmSize_t step, float time, UdmSize_t average_step, float average_time)
{
    UdmSliceConfig* slice = this->getCurrentSliceConfigByStep(step);
    if (slice == NULL)  {
        slice = new UdmSliceConfig(step, time, average_step, average_time);
        this->insertCurrentSliceConfig(slice);
    }
    else {
        slice->setStep(step);
        slice->setTime(time);
        slice->setAverageStep(average_step);
        slice->setAverageTime(average_time);
    }

    return UDM_OK;
}

/**
 * 時系列設定を追加する.
 * @param config            時系列設定
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmTimeSliceConfig::insertCurrentSliceConfig(UdmSliceConfig *config)
{
    if (config == NULL) return UDM_ERROR;
    this->current_timeslices.push_back(config);
    return UDM_OK;
}


/**
 * 現在実行中の時系列ステップ番号の時系列情報を削除する
 * @param step            時系列ステップ番号
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmTimeSliceConfig::removeCurrentTimeSlice(UdmSize_t step)
{
    std::vector<UdmSliceConfig*>::iterator itr;
    for (itr = this->current_timeslices.begin(); itr != this->current_timeslices.end(); itr++) {
        if ((*itr)->getStep() == step) {
            delete (*itr);
            this->current_timeslices.erase(itr);
            return UDM_OK;
        }
    }

    return UDM_ERROR;
}

/**
 * 時系列ステップ番号のindex.dfi読込の時系列設定を取得する.
 * @param step            時系列ステップ番号
 * @return        index.dfi読込の時系列設定
 */
const UdmSliceConfig* UdmTimeSliceConfig::getPreviousSliceConfigByStep(UdmSize_t step) const
{
    std::vector<UdmSliceConfig*>::const_iterator itr;
    for (itr = this->previous_timeslices.begin(); itr != this->previous_timeslices.end(); itr++) {
        if ((*itr)->getStep() == step) {
            return (*itr);
        }
    }

    return NULL;
}

/**
 * 時系列ステップ番号の現在実行中の時系列設定を取得する.
 * @param step            時系列ステップ番号
 * @return        現在実行中の時系列設定
 */
UdmSliceConfig* UdmTimeSliceConfig::getCurrentSliceConfigByStep(UdmSize_t step) const
{
    std::vector<UdmSliceConfig*>::const_iterator itr;
    for (itr = this->current_timeslices.begin(); itr != this->current_timeslices.end(); itr++) {
        if ((*itr)->getStep() == step) {
            return (*itr);
        }
    }

    return NULL;
}


/**
 * /TimeSlice/Slice[@]をすべて削除する
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmTimeSliceConfig::clearCurrentTimeSlices()
{
    std::vector<UdmSliceConfig*>::iterator itr;
    for (itr = this->current_timeslices.begin(); itr != this->current_timeslices.end(); itr++) {
        if (*itr) delete *itr;
    }
    this->current_timeslices.clear();
    return UDM_OK;
}

/**
 * index.dfi読込の時系列情報のDFIラベルパスのパラメータの設定値を取得する.
 * 数値は文字列に変換して返す。
 * 複数の設定値のあるパラメータの場合はUDM_ERRORを返す。
 * @param [in]  label_path        DFIラベルパス
 * @param [out] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmTimeSliceConfig::getDfiValue(const std::string &label_path, std::string &value) const
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
    if (strcasecmp(label.c_str(), UDM_DFI_TIMESLICE)) {
        return UDM_ERROR;
    }

    // Sliceチェック
    if (name.length() <= std::string(UDM_DFI_SLICE).length()+2) return UDM_ERROR;
    if (strcasecmp(name.substr(0, std::string(UDM_DFI_SLICE).length()).c_str(), UDM_DFI_SLICE)) return UDM_ERROR;

    // TimeSlice/Slice[@]
    int slice_id = this->splitLabelIndex(name);
    if (slice_id < 0) return UDM_ERROR;
    if (slice_id >= this->previous_timeslices.size()) return UDM_ERROR;
    return this->previous_timeslices[slice_id]->getDfiValue(label_path, value);
}

/**
 * index.dfi読込の時系列情報のDFIラベルパスのパラメータの設定値数を取得する.
 * 複数の設定値のあるパラメータの場合は設定値数を返す。
 * １つのみ設定値のパラメータの場合は1を返す。
 * 設定値されていないパラメータの場合は0を返す。
 * 子ノードが存在している場合は、ノード数又はパラメータ数を返す.
 * @param [in]  label_path        DFIラベルパス
 * @param [out] value                設定値
 * @return        パラメータの設定値数
 */
int UdmTimeSliceConfig::getNumDfiValue(const std::string &label_path) const
{
    std::vector<std::string> labels;
    std::string label, name;
    int count;
    UdmError_t error = UDM_OK;

    // TimeSlice : ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 0) return 0;
    label = labels[0];
    if (strcasecmp(label.c_str(), UDM_DFI_TIMESLICE)) {
        return 0;
    }
    // TimeSliceのパラメータ数を返す
    if (count == 1) {
        return this->previous_timeslices.size();
    }

    name = labels[1];
    // /TimeSlice/Sliceの場合、TimeSliceのパラメータ数を返す
    if (strcasecmp(name.c_str(), UDM_DFI_SLICE)) {
        return this->previous_timeslices.size();
    }

    // Sliceチェック
    if (name.length() <= std::string(UDM_DFI_SLICE).length()+2) return UDM_ERROR;
    if (strcasecmp(name.substr(0, std::string(UDM_DFI_SLICE).length()).c_str(), UDM_DFI_SLICE)) return UDM_ERROR;

    // TimeSlice/Slice[@]
    int slice_id = this->splitLabelIndex(name);
    if (slice_id < 0) return UDM_ERROR;
    if (slice_id >= this->previous_timeslices.size()) return UDM_ERROR;
    return this->previous_timeslices[slice_id]->getNumDfiValue(label_path);

}

/**
 * 現在実行中の時系列情報のDFIラベルパスのパラメータの設定値を設定する.
 * 数値は文字列から変換して設定する。
 * 複数の設定値を持つことの可能なパラメータの場合は１つのみ設定する.
 * @param [in]  label_path        DFIラベルパス
 * @param [in] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmTimeSliceConfig::setDfiValue(const std::string &label_path, const std::string &value)
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
    if (strcasecmp(label.c_str(), UDM_DFI_TIMESLICE)) {
        return UDM_ERROR;
    }

    // Sliceチェック
    if (name.length() <= std::string(UDM_DFI_SLICE).length()+2) return UDM_ERROR;
    if (strcasecmp(name.substr(0, std::string(UDM_DFI_SLICE).length()).c_str(), UDM_DFI_SLICE)) return UDM_ERROR;

    // TimeSlice/Slice[@]
    if (!strcasecmp(name.c_str(), (std::string(UDM_DFI_SLICE)+std::string("[@]")).c_str())) {
        UdmSliceConfig *slice = new UdmSliceConfig();
        error = slice->setDfiValue(label_path, value);
        if (error != UDM_OK) {
            delete slice;
            return error;
        }
        this->current_timeslices.push_back(slice);
        return error;
    }
    // FileCompositionType[0..n]
    else if (!strcasecmp(name.substr(0, strlen(UDM_DFI_SLICE)).c_str(), UDM_DFI_SLICE)) {
        int slice_id = this->splitLabelIndex(name);
        if (slice_id < 0) return UDM_ERROR;
        if (slice_id >= this->current_timeslices.size()) return UDM_ERROR;
        return this->current_timeslices[slice_id]->setDfiValue(label_path, value);
    }

    return UDM_ERROR;
}
} /* namespace udm */
