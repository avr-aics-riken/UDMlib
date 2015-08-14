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
 * @file UdmStopWatch.cpp
 * 時間測定クラスのソースファイル
 */

#include "utils/UdmStopWatch.h"

namespace udm
{

/// sphStopWatch スタティックメンバーの初期化
UdmStopWatch *UdmStopWatch::instance = NULL;


/**
 * コンストラクタ.
 */
UdmTimeMeasure::UdmTimeMeasure() : label(""), start_time(0.0)
{
}

/**
 * コンストラクタ.
 * @param   label
 */
UdmTimeMeasure::UdmTimeMeasure(const std::string &label) : start_time(0.0)
{
    this->label = label;
    this->measure_times.clear();
}

/**
 * デストラクタ.
 */
UdmTimeMeasure::~UdmTimeMeasure()
{
    this->measure_times.clear();
}

/**
 * 初期化を行う。
 */
void UdmTimeMeasure::clear()
{
    this->measure_times.clear();
    this->label.clear();
}

/**
 * 測定スタート.
 */
void UdmTimeMeasure::start()
{
    this->start_time = getTime();
}

/**
 * 測定ストップ.
 */
void UdmTimeMeasure::stop()
{
    double stoptime = this->getTime();
    double interval = stoptime - this->start_time;
    this->measure_times.push_back(interval);
}

/**
 * 測定回数
 */
int UdmTimeMeasure::getCount() const
{
    return this->measure_times.size();
}

/**
 * 測定時間を取得する。
 * @param  id        取得測定回数（１～）
 * @return     測定時間
 */
double UdmTimeMeasure::getTime(int  id) const
{
    if (id <= 0) return 0.0;
    if ((unsigned)id > this->measure_times.size()) return 0.0;
    return this->measure_times[id-1];
}

/**
 * 測定時間の合計値を取得する。
 * @return    合計値
 */
double UdmTimeMeasure::getTotalTime() const
{
    double total = 0;
    for (unsigned int i=0; i<this->measure_times.size(); i++) {
        total += this->measure_times[i];
    }
    return total;
}

/**
 * 測定時間の平均を取得する。
 * @return    平均値
 */
double UdmTimeMeasure::getAverageTime() const
{
    if (this->measure_times.size() <= 0) return 0.0;
    double total = this->getTotalTime();
    return total/this->measure_times.size();
}

/**
 * 測定時間の最大値を取得する。
 * @return    最大値
 */
double UdmTimeMeasure::getMaximumTime() const
{
    if (this->measure_times.size() <= 0) return 0.0;
    double max = 0.0;
    for (unsigned int i=0; i<this->measure_times.size(); i++) {
        if (max < this->measure_times[i]) {
            max = this->measure_times[i];
        }
    }
    return max;
}

/**
 * 測定時間の最小値を取得する。
 * @return    最小値
 */
double UdmTimeMeasure::getMinimumTime() const
{
    if (this->measure_times.size() <= 0) return 0.0;
    double min = DBL_MAX;
    for (unsigned int i=0; i<this->measure_times.size(); i++) {
        if (min > this->measure_times[i]) {
            min = this->measure_times[i];
        }
    }
    return min;
}

/**
 * 測定時間の標準偏差を取得する。
 * @return    標準偏差
 */
double UdmTimeMeasure::getDeviationTime() const
{
    if (this->measure_times.size() <= 0) return 0.0;
    double avr = this->getAverageTime();
    double dev = 0;
    for (unsigned int i=0; i<this->measure_times.size(); i++) {
        dev += (this->measure_times[i]-avr)*(this->measure_times[i]-avr);
    }
    dev = dev/this->measure_times.size();
    dev = sqrt(dev);
    return dev;
}

/**
 * 測定結果を出力する.
 * @param [out] buf            測定結果
 */
void UdmTimeMeasure::toString(std::string &buf) const
{
    char output[128] = {0x00};
    if (this->getCount() > 1) {
        sprintf(output, "label = %s\n", this->label.c_str());
        buf += output;
        sprintf(output, "count, sum,    average, maximum, minimum, deviation\n");
        buf += output;
        sprintf(output, "%d, %12.6e, %12.6e, %12.6e, %12.6e, %12.6e\n",
                this->getCount(),
                this->getTotalTime(),
                this->getAverageTime(),
                this->getMaximumTime(),
                this->getMinimumTime(),
                this->getDeviationTime());
        buf += output;
    }
    else if (this->getCount() == 1) {
        sprintf(output, "label : %s = %12.6e\n", this->label.c_str(), this->getTotalTime());
        buf += output;
    }

    return;
}

/**
 * 測定結果を出力する.
 * @param [out] buf            測定結果
 */
void UdmTimeMeasure::toStringLine(std::string &buf) const
{
    // label,  count, sum,    average, maximum, minimum, deviation
    char format[] = "%-32s: %6d, %12.6e, %12.6e, %12.6e, %12.6e, %12.6e \n";
    char output[512] = {0x00};
    sprintf(output, format,
            this->label.c_str(),
            this->getCount(),
            this->getTotalTime(),
            this->getAverageTime(),
            this->getMaximumTime(),
            this->getMinimumTime(),
            this->getDeviationTime());

    buf += output;
    int num_info = this->getNumInformations();
    int n = 0;
    char info_format[32] = "%32s: %s\n";
    for (n=1; n<=num_info; n++) {
        const std::string info = this->getInformation(n);
        std::string out_info = info.substr(0, 256);
        sprintf(output, info_format, " ", info.c_str());
        buf += output;
    }

    return;
}

/**
 * 測定ラベルを取得する.
 * @return        測定ラベル
 */
const std::string& UdmTimeMeasure::getLabel() const
{
    return this->label;
}

/**
 * 測定ラベルを取得する.
 * @return        測定ラベル
 */
void UdmTimeMeasure::setLabel(const std::string& label)
{
    this->label = label;
}

/**
 * 測定時間(秒)リストを取得する.
 * @return        測定時間(秒)リスト
 */
const std::vector<double>& UdmTimeMeasure::getMeasureTimes() const
{
    return this->measure_times;
}

/**
 * 開始時間を取得する.
 * @return        開始時間
 */
double UdmTimeMeasure::getStartTime() const
{
    return this->start_time;
}

/**
 * 追加情報数を取得する.
 * @return        追加情報
 */
const std::vector<std::string> &UdmTimeMeasure::getInformations() const
{
    return this->informations;
}


/**
 * 追加情報数を取得する.
 * @return        追加情報
 */
int UdmTimeMeasure::getNumInformations() const
{
    return this->informations.size();
}

/**
 * 追加情報を取得する.
 * @param  info_id        追加情報ID（１～）
 * @return        追加情報
 */
const std::string& UdmTimeMeasure::getInformation(int info_id) const
{
	static std::string ret;
    if (info_id <= 0) return ret;
    if ((unsigned)info_id > this->informations.size()) return ret;
    return this->informations[info_id-1];
}

/**
 * 追加情報を設定する.
 * @param information        追加情報
 */
void UdmTimeMeasure::addInformation(const std::string& information)
{
    this->informations.push_back(information);
}

/**
 * 追加情報をクリアする.
 */
void UdmTimeMeasure::clearInformations()
{
    this->informations.clear();
    return;
}

/** 時刻を取得.
 *   Unix/Linux: gettimeofdayシステムコールを使用.
 *   @return 時刻値(秒)
 **/
double UdmTimeMeasure::getTime() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1.0e-6;
}

/**
 * コンストラクタ
 */
UdmStopWatch::UdmStopWatch()
{
    this->clear();
}

/**
 * デストラクタ
 */
UdmStopWatch::~UdmStopWatch()
{
    std::map<std::string, UdmTimeMeasure*>::iterator itr;
    for(itr=this->stopwatch_list.begin();itr!=this->stopwatch_list.end();itr++) {
        UdmTimeMeasure* measure = itr->second;
        if (measure != NULL) delete measure;
        measure = NULL;
    }
    this->stopwatch_list.clear();
    this->clear();
}

/**
 * コピーコンストラクタ
 * @param src        元測定管理クラス
 */
UdmStopWatch::UdmStopWatch(const UdmStopWatch& src)
{
}

/**
 * 時間測定クラスのインスタンスを取得する.
 * @return        時間測定クラスのインスタンス
 */
UdmStopWatch* UdmStopWatch::getInstance()
{
    if (UdmStopWatch::instance == NULL) {
        UdmStopWatch::instance = new UdmStopWatch();

        // プログラムが終了時に破棄処理を呼ぶ。
        atexit(&destruct);
    }

    return UdmStopWatch::instance;
}

/**
 * シングルトンクラスであるUdmStopWatchのオブジェクトの破棄を行う。
 */
void UdmStopWatch::destruct()
{
    if (UdmStopWatch::instance != NULL) delete UdmStopWatch::instance;
    UdmStopWatch::instance = NULL;

    return;
}


/**
 * 測定時間をクリアする.
 */
void UdmStopWatch::clear()
{
    this->stopwatch_list.clear();
}

/**
 * 時間測定を開始する.
 * @param key        時間測定ラベル
 */
void UdmStopWatch::start(const std::string& key)
{
    UdmTimeMeasure* measure = this->findTimeMeasure(key);
    if (measure == NULL) {
        measure = new UdmTimeMeasure(key);
        // 要素を追加する
        this->stopwatch_list.insert( std::make_pair( key, measure ) );
    }

    // 時間測定開始
    measure->start();

    return;
}

/**
 * 時間測定を停止する.
 * @param key        時間測定ラベル
 */
void UdmStopWatch::stop(const std::string& key)
{
    UdmTimeMeasure *measure = this->findTimeMeasure(key);
    if (measure == NULL) {
        return;
    }

    // 時間測定終了
    measure->stop();

    return;
}


/**
 * 時間測定が存在するかチェックする.
 * @param key        時間測定ラベル
 * @return   true=時間測定が存在する
 */
bool UdmStopWatch::existsTimeMeasure(const std::string &key)
{
    UdmTimeMeasure *measure = this->findTimeMeasure(key);
    return (measure != NULL);
}

/**
 * 時間測定を削除する.
 * @param key        時間測定ラベル
 */
void UdmStopWatch::removeTimeMeasure(const std::string& key)
{
    std::map<std::string, UdmTimeMeasure*>::iterator itr;
    itr = this->stopwatch_list.find(key);
    if (itr == this->stopwatch_list.end()) {
        return;
    }
    if (itr->second != NULL) delete itr->second;
    this->stopwatch_list.erase(itr);
    return;
}

/**
 * 時間測定クラスを取得する.
 * @param key        時間測定ラベル
 * @return        時間測定クラス
 */
const UdmTimeMeasure* UdmStopWatch::getTimeMeasure(const std::string& key) const
{
    UdmTimeMeasure *measure = this->findTimeMeasure(key);
    return measure;
}

/**
 * 測定合計時間を取得する.
 * @param key        時間測定ラベル
 * @return        測定合計時間
 */
double UdmStopWatch::getTotalTime(const std::string& key) const
{
    UdmTimeMeasure *measure = this->findTimeMeasure(key);
    if (measure == NULL) {
        return 0.0;
    }
    return measure->getTotalTime();
}

/**
 * 測定平均時間を取得する.
 * @param key        時間測定ラベル
 * @return        測定平均時間
 */
double UdmStopWatch::getAverageTime(const std::string& key) const
{
    UdmTimeMeasure *measure = this->findTimeMeasure(key);
    if (measure == NULL) {
        return 0.0;
    }
    return measure->getAverageTime();
}

/**
 * 測定最大時間を取得する.
 * @param key        時間測定ラベル
 * @return        測定最大時間
 */
double UdmStopWatch::getMaximumTime(const std::string& key) const
{
    UdmTimeMeasure *measure = this->findTimeMeasure(key);
    if (measure == NULL) {
        return 0.0;
    }
    return measure->getMaximumTime();
}

/**
 * 測定最小時間を取得する.
 * @param key        時間測定ラベル
 * @return        測定最小時間
 */
double UdmStopWatch::getMinimumTime(const std::string& key) const
{
    UdmTimeMeasure *measure = this->findTimeMeasure(key);
    if (measure == NULL) {
        return 0.0;
    }
    return measure->getMinimumTime();
}

/**
 * 測定標準偏差時間を取得する.
 * @param key        時間測定ラベル
 * @return        測定標準偏差時間
 */
double UdmStopWatch::getDeviationTime(const std::string& key) const
{
    UdmTimeMeasure *measure = this->findTimeMeasure(key);
    if (measure == NULL) {
        return 0.0;
    }
    return measure->getDeviationTime();
}

/**
 * 測定時間を文字列で取得する.
 * @param [out] buf        出力文字列
 */
void UdmStopWatch::toString(std::string& buf) const
{
    buf += "label     ,  count, sum,    average, maximum, minimum, deviation\n";
    std::map<std::string, UdmTimeMeasure*>::const_iterator itr;
    for (itr=this->stopwatch_list.begin(); itr!=this->stopwatch_list.end(); itr++) {
        UdmTimeMeasure *measure = itr->second;
        std::string measure_buf;
        measure->toStringLine(measure_buf);
        buf += measure_buf;
    }
    return;
}

/**
 * 測定時間を文字列出力する.
 */
void UdmStopWatch::print() const
{
    std::string buf;
    this->toString(buf);
    printf(buf.c_str());
    return;
}


/**
 * 時間測定数を取得する.
 * @return        時間測定数
 */
int UdmStopWatch::getNumTimeMeasure() const
{
    return this->stopwatch_list.size();
}

/**
 * 時間測定ラベルを取得する.
 * @param measure_id        時間測定インデックス（１～）
 * @return        時間測定ラベル
 */
const std::string& UdmStopWatch::getTimeMeasureLabel(int measure_id) const
{
	static std::string ret;
    if (measure_id <= 0) return ret;
    if (measure_id > this->stopwatch_list.size()) return ret;
    std::map<std::string, UdmTimeMeasure*>::const_iterator itr = this->stopwatch_list.begin();
    std::advance(itr, measure_id-1);
    return itr->first;
}

/**
 * 追加情報を取得する.
 * @param key        時間測定ラベル
 * @return        追加情報
 */
const std::vector<std::string>& UdmStopWatch::getInformations(const std::string& key) const
{
    UdmTimeMeasure *measure = this->findTimeMeasure(key);
    if (measure == NULL) {
        static std::vector<std::string> infos;
        return infos;
    }
    return measure->getInformations();
}

/**
 * 追加情報を設定する
 * @param key        時間測定ラベル
 * @param information        追加情報
 */
void UdmStopWatch::addInformation(
                    const std::string& key,
                    const std::string& information) const
{
    UdmTimeMeasure *measure = this->findTimeMeasure(key);
    if (measure == NULL) {
        return;
    }
    measure->addInformation(information);

    return;
}

/**
 * 時間測定ラベルの時間測定クラスを検索する.
 * @param [in]  key        時間測定ラベル
 * @param [out] measure        時間測定クラス
 * @return        時間測定クラスが存在する.
 */
UdmTimeMeasure* UdmStopWatch::findTimeMeasure(const std::string& key) const
{
    std::map<std::string, UdmTimeMeasure*>::const_iterator itr;
    itr = this->stopwatch_list.find(key);
    if (itr == this->stopwatch_list.end()) {
        return NULL;
    }

    UdmTimeMeasure* measure = itr->second;
    return measure;
}

} /* namespace udm */
