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

#ifndef _UDMSTOPWATCH_H_
#define _UDMSTOPWATCH_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <deque>
#include <vector>
#include <iostream>
#include <map>
#include <sstream>
#include <cmath>
#include <float.h>
#include <sys/time.h>


#define     UDM_STOPWATCH_START(label)        UdmStopWatch::getInstance()->start(label)
#define     UDM_STOPWATCH_STOP(label)         UdmStopWatch::getInstance()->stop(label)
#define     UDM_STOPWATCH_PRINT()             UdmStopWatch::getInstance()->print()
#define     UDM_STOPWATCH_INFORMATION(label,information)        UdmStopWatch::getInstance()->addInformation(label,information)

/**
 * @file UdmStopWatch.h
 * 時間測定クラスのヘッダファイル
 */

namespace udm
{

/**
 * 測定時間クラス.
 */
class UdmTimeMeasure {

private:
    std::vector<double>  measure_times;         ///< 測定時間(秒)リスト
    std::string label;   ///< 測定ラベル
    double start_time;       ///< 開始時間
    std::vector<std::string>    informations;            ///< 追加情報

public:
    UdmTimeMeasure();
    UdmTimeMeasure(const std::string &label);
    ~UdmTimeMeasure();
    void clear();
    void start();
    void stop();
    int getCount() const;
    double getTime(int  id) const;
    double getTotalTime() const;
    double getAverageTime() const;
    double getMaximumTime() const;
    double getMinimumTime() const;
    double getDeviationTime() const;
    void toString(std::string &buf) const;
    void toStringLine(std::string &buf) const;
    const std::string& getLabel() const;
    void setLabel(const std::string& label);
    const std::vector<double>& getMeasureTimes() const;
    double getStartTime() const;
    const std::vector<std::string> &getInformations() const;
    int getNumInformations() const;
    const std::string& getInformation(int info_id) const;
    void addInformation(const std::string& information);
    void clearInformations();

private:
    double getTime();

};


/**
 * 時間測定管理クラス
 */
class UdmStopWatch
{
private:
    /**
     * 時間測定テーブル
     */
    std::map<std::string, UdmTimeMeasure*> stopwatch_list;

    /**
     * sphStopWatchクラスのシングルトンオブジェクト
     */
    static UdmStopWatch* instance;

public:
    static UdmStopWatch* getInstance();
    static void destruct();
    void clear();
    void start(const std::string &key);
    void stop(const std::string &key);
    int getNumTimeMeasure() const;
    const std::string &getTimeMeasureLabel(int measure_id) const;
    bool existsTimeMeasure(const std::string &key);
    void removeTimeMeasure(const std::string &key);
    const UdmTimeMeasure* getTimeMeasure(const std::string &key) const;
    double getTotalTime(const std::string &key) const;
    double getAverageTime(const std::string &key) const;
    double getMaximumTime(const std::string &key) const;
    double getMinimumTime(const std::string &key) const;
    double getDeviationTime(const std::string &key) const;
    const std::vector<std::string> &getInformations(const std::string &key) const;
    void addInformation(const std::string &key, const std::string &information) const;
    void toString(std::string &buf) const;
    void print() const;

private:
    UdmStopWatch();
    UdmStopWatch(const UdmStopWatch &src);
    virtual ~UdmStopWatch();
    UdmTimeMeasure* findTimeMeasure(const std::string &key) const;

};

} /* namespace udm */

#endif /* _UDMSTOPWATCH_H_ */
