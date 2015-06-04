/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmErrorHandler.cpp
 * エラーハンドラクラスのソースファイル
 */

#include "UdmErrorHandler.h"
#if defined(_WIN32) && !defined(__NUTC__)
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

namespace udm
{

/// UdmErrorHandler スタティックメンバーの初期化
UdmErrorHandler *UdmErrorHandler::m_instance = NULL;

/**
 * コンストラクタ
 */
UdmErrorHandler::UdmErrorHandler()
{
    this->debug_level = UDM_DEFAULT_DEBUG_LEVEL;
    this->output_target = 0x01;
}


/**
 * デストラクタ
 */
UdmErrorHandler::~UdmErrorHandler()
{

}

/**
 * エラーハンドラクラスのインスタンスを取得する.
 * @return        エラーハンドラクラス
 */
UdmErrorHandler* UdmErrorHandler::getInstance()
{
    if (m_instance == NULL) {
        m_instance = new UdmErrorHandler();
    }
    return m_instance;
}

/**
 * エラーメッセージを出力する.
 * @param error_no            エラー番号
 * @param filename            エラーファイル名
 * @param lineno            エラーライン番号
 * @param funcname            エラー関数名
 * @return        エラー番号
 */
UdmError_t UdmErrorHandler::error(UdmError_t error_no, const char* filename, int lineno, const char* funcname)
{
    this->setMessage(Udm_ErrorLevel_Error, error_no, filename, lineno, funcname, NULL);
    if (this->isDebugLevelPrintError()) {
        this->printMessage();
    }
    return error_no;
}

/**
 * エラーメッセージを出力する.
 * @param error_no            エラー番号
 * @param filename            エラーファイル名
 * @param lineno            エラーライン番号
 * @param funcname            エラー関数名
 * @param format            エラー情報書式
 * @param ...                エラー情報パラメータ
 * @return        エラー番号
 */
UdmError_t UdmErrorHandler::error(UdmError_t error_no, const char* filename, int lineno, const char* funcname, const char* format, ...)
{
    // 詳細メッセージを取得する。
    if (strlen(format) < UDM_ERROR_BUFFER_SIZE) {
    va_list ap;
    va_start(ap, format);
        char buf[UDM_ERROR_BUFFER_SIZE] = {0x00};
        vsnprintf(buf, UDM_ERROR_BUFFER_SIZE, format, ap);
        this->setMessage(Udm_ErrorLevel_Error, error_no, filename, lineno, funcname, buf);
        va_end( ap );
    }
    else {
        this->setMessage(Udm_ErrorLevel_Error, error_no, filename, lineno, funcname, format);
    }

    if (this->isDebugLevelPrintError()) {
        this->printMessage();
    }
    return error_no;
}

/**
 * エラーメッセージを出力する.
 * @param error_no            エラー番号
 * @param info            エラー情報
 * @return        エラー番号
 */
UdmError_t UdmErrorHandler::error_info(UdmError_t error_no, const char* info)
{
    this->setMessage(Udm_ErrorLevel_Error, error_no, NULL, 0, NULL, info);
    if (this->isDebugLevelPrintError()) {
        this->printMessage();
    }
    return error_no;
}

/**
 * 警告メッセージを出力する.
 * @param error_no            エラー番号
 * @param filename            エラーファイル名
 * @param lineno            エラーライン番号
 * @param funcname            エラー関数名
 * @return        エラー番号
 */
UdmError_t UdmErrorHandler::warning(UdmError_t error_no, const char* filename, int lineno, const char* funcname)
{
    if (!this->isDebugLevelPrintWarning()) return error_no;
    this->printMessage(Udm_ErrorLevel_Warning, error_no, filename, lineno, funcname, NULL);

    return error_no;
}

/**
 * 警告メッセージを出力する.
 * @param error_no            エラー番号
 * @param filename            エラーファイル名
 * @param lineno            エラーライン番号
 * @param funcname            エラー関数名
 * @param format            エラー情報書式
 * @param ...                エラー情報パラメータ
 * @return        エラー番号
 */
UdmError_t UdmErrorHandler::warning(UdmError_t error_no, const char* filename, int lineno, const char* funcname, const char* format, ...)
{
    if (!this->isDebugLevelPrintWarning()) return error_no;

    // 詳細メッセージを取得する。
    if (strlen(format) < UDM_ERROR_BUFFER_SIZE) {
    va_list ap;
    va_start(ap, format);
        char buf[UDM_ERROR_BUFFER_SIZE] = {0x00};
        vsnprintf(buf, UDM_ERROR_BUFFER_SIZE, format, ap);

    this->printMessage(Udm_ErrorLevel_Warning, error_no, filename, lineno, funcname, buf);
        va_end( ap );
    }
    else {
        this->printMessage(Udm_ErrorLevel_Warning, error_no, filename, lineno, funcname, format);
    }

    return error_no;
}

/**
 * 警告メッセージを出力する.
 * @param error_no            エラー番号
 * @param info            エラー情報
 * @return        エラー番号
 */
UdmError_t UdmErrorHandler::warning_info(UdmError_t error_no, const char* info)
{
    if (!this->isDebugLevelPrintWarning()) return error_no;
    this->printMessage(Udm_ErrorLevel_Error, error_no, NULL, 0, NULL, info);
    return error_no;
}

/**
 * 情報メッセージを出力する.
 * @param filename            エラーファイル名
 * @param lineno            エラーライン番号
 * @param funcname            エラー関数名
 * @param format            エラー情報書式
 * @param ...                エラー情報パラメータ
 * @return        エラー番号
 */
void UdmErrorHandler::info(const char* filename, int lineno,  const char* funcname, const char* format, ...)
{
    if (!this->isDebugLevelPrintInfo()) return;
    // 詳細メッセージを取得する。
    if (strlen(format) < UDM_ERROR_BUFFER_SIZE) {
    va_list ap;
    va_start(ap, format);
        char buf[UDM_ERROR_BUFFER_SIZE] = {0x00};
        vsnprintf(buf, UDM_ERROR_BUFFER_SIZE, format, ap);
    this->printMessage(Udm_ErrorLevel_Info, (UdmError_t)0, filename, lineno, funcname, buf);
        va_end( ap );
    }
    else {
        if (this->isOutputTargetFile()) {
            this->writeFile(format);
        }
    }

    return;
}

/**
 * 情報メッセージを出力する.
 * @param info            情報メッセージ
 * @return        エラー番号
 */
void UdmErrorHandler::info(const char* format, ...)
{
    if (format == NULL) return;
    if (!this->isDebugLevelPrintInfo()) return;

    // 詳細メッセージを取得する。
    if (strlen(format) < UDM_ERROR_BUFFER_SIZE) {
        va_list ap;
        char buf[UDM_ERROR_BUFFER_SIZE] = {0x00};
        vsnprintf(buf, UDM_ERROR_BUFFER_SIZE, format, ap);
        this->printMessage(Udm_ErrorLevel_Info, (UdmError_t)0, NULL, 0, NULL, buf);
        va_end( ap );
    }
    else {
        if (this->isOutputTargetFile()) {
            this->writeFile(format);
        }
    }

    return;
}

/**
 * デバッグメッセージを出力する.
 * @param error_no            エラー番号
 * @param filename            エラーファイル名
 * @param lineno            エラーライン番号
 * @param funcname            エラー関数名
 * @param format            エラー情報書式
 * @param ...                エラー情報パラメータ
 * @return        エラー番号
 */
void UdmErrorHandler::debug(const char* filename, int lineno, const char* funcname, const char* format, ...)
{
    if (!this->isDebugLevelPrintDebug()) return;

    // 詳細メッセージを取得する。
    if (strlen(format) < UDM_ERROR_BUFFER_SIZE) {
    va_list ap;
    va_start(ap, format);
        char buf[UDM_ERROR_BUFFER_SIZE] = {0x00};
        vsnprintf(buf, UDM_ERROR_BUFFER_SIZE, format, ap);

    this->printMessage(Udm_ErrorLevel_Debug, (UdmError_t)0, filename, lineno, funcname, buf);
        va_end( ap );
    }
    else {
        if (this->isOutputTargetFile()) {
            this->writeFile(format);
        }
    }
    return;
}

/**
 * デバッグメッセージを出力する.
 * @param info            情報メッセージ
 * @return        エラー番号
 */
void UdmErrorHandler::debug(const char* format, ...)
{
    if (format == NULL) return;
    if (!this->isDebugLevelPrintDebug()) return;

    // 詳細メッセージを取得する。
    if (strlen(format) < UDM_ERROR_BUFFER_SIZE) {
        va_list ap;
        va_start(ap, format);
        char buf[UDM_ERROR_BUFFER_SIZE] = {0x00};
        vsnprintf(buf, UDM_ERROR_BUFFER_SIZE, format, ap);
        this->printMessage(Udm_ErrorLevel_Debug, (UdmError_t)0, NULL, 0, NULL, buf);
        va_end( ap );
    }
    else {
        if (this->isOutputTargetFile()) {
            this->writeFile(format);
        }
    }

    return;
}

/**
 * メッセージを取得する.
 * エラーメッセージ文字列を取得して返す.
 * @param [out] buf            メッセージ
 */
void UdmErrorHandler::getMessage(char *buf)
{
    this->getMessage(buf, this->level, this->error_no, this->error_filename.c_str(), this->error_lineno, this->error_funcname.c_str(), this->message.c_str());
}

/**
 * エラーメッセージを取得する.
 * @param [out] buf   エラーメッセージ
 * @param level            エラーレベル
 * @param error_no        エラー番号
 * @param filename        エラーファイル名
 * @param lineno        エラー行番号
 * @param funcname        エラー関数名
 * @param message        エラー追加情報
 */
void UdmErrorHandler::getMessage(char* buf, UdmErrorLevel_t level, UdmError_t error_no, const char* filename, int lineno, const char* funcname, const char* message)
{
    char stream[512] = {0x00};
    char tmp_buf[512] = {0x00};

    int rank = -1, nprocs = 0, flag;
    udm_mpi_initialized(&flag);
    if (flag) {
        udm_mpi_comm_rank(MPI_COMM_WORLD,&rank);
        udm_mpi_comm_size(MPI_COMM_WORLD,&nprocs);
    }

    if (nprocs > 0 && rank >= 0) {
        sprintf(tmp_buf, "[rank:%d]", rank);
        strcat(stream, tmp_buf);
    }
    if (filename != NULL && strlen(filename) > 0) {
        strcat(stream, filename);
    }
    if (lineno > 0) {
        sprintf(tmp_buf, ":%d", lineno);
        strcat(stream, tmp_buf);
    }
    if (funcname != NULL && strlen(funcname) > 0) {
        sprintf(tmp_buf, "[%s]", funcname);
        strcat(stream, tmp_buf);
    }

    if (strlen(stream) > 0) {
        strcat(stream, " : ");
    }
    if (level == Udm_ErrorLevel_Error) {
        strcat(stream, "Error ");
    }
    else if (level == Udm_ErrorLevel_Warning) {
        strcat(stream, "Warning ");
    }
    else if (level == Udm_ErrorLevel_Info) {
        strcat(stream, "Info ");
    }
    else if (level == Udm_ErrorLevel_Debug) {
        strcat(stream, "Debug ");
    }

    if (error_no > 0) {
        sprintf(tmp_buf, "[%d] %s", (int)error_no, udm_strerror((UdmError_t)error_no));
        strcat(stream, tmp_buf);
    }

    if (message != NULL && strlen(message) > 0) {
        sprintf(tmp_buf, " : %s", message);
        strcat(stream, tmp_buf);
    }
    strcat(stream, "\n");

    strncpy(buf, stream, strlen(stream));

    return;
}


/**
 * エラーメッセージを出力する.
 * @param level            エラーレベル
 * @param error_no        エラー番号
 * @param filename        エラーファイル名
 * @param lineno        エラー行番号
 * @param funcname        エラー関数名
 * @param message        エラー追加情報
 */
void UdmErrorHandler::printMessage(UdmErrorLevel_t level, UdmError_t error_no,
        const char* filename, int lineno, const char* funcname,
        const char* message)
{
    int len = 256;
    if (message != NULL) {
        len += strlen(message) + 256;
    }
    char *buf = new char[len];
    memset(buf, 0x00, len);
    getMessage(buf, level, error_no, filename, lineno, funcname, message);

    if (this->isOutputTargetStdout()) {
        printf(buf);
    }
    if (this->isOutputTargetFile()) {
        this->writeFile(buf);
    }

    delete []buf;
    return;
}


/**
 * エラー番号、メッセージをクリアする.
 */
void UdmErrorHandler::clear()
{
    this->error_no = (UdmError_t)0;
    this->message = "";
    this->level = Udm_ErrorHandlerUnknown;
    this->error_filename = "";
    this->error_lineno = 0;
    this->error_funcname = "";
}

/**
 * エラーメッセージを標準出力を行う.
 */
void UdmErrorHandler::printMessage()
{
    int len = this->message.length() + 256;
    char *buf = new char[len];
    memset(buf, 0x00, len);
    this->getMessage(buf);
    if (this->isOutputTargetStdout()) {
        printf(buf);
    }
    if (this->isOutputTargetFile()) {
        this->writeFile(buf);
    }

    delete []buf;
    return;
}

/**
 * エラーメッセージ情報を設定する.
 * @param level            エラーレベル
 * @param error_no        エラー番号
 * @param filename        エラーファイル名
 * @param lineno        エラー行番号
 * @param funcname        エラー関数名
 * @param message        エラー追加情報
 */
void UdmErrorHandler::setMessage(UdmErrorLevel_t level, UdmError_t error_no, const char* filename, int lineno, const char* funcname, const char* message)
{

    this->level = level;
    this->error_no = error_no;

    if (message != NULL) this->message = std::string(message);
    else this->message = std::string();

    if (filename != NULL) this->error_filename = std::string(filename);
    else this->error_filename = std::string();

    this->error_lineno = lineno;

    if (funcname != NULL)  this->error_funcname = std::string(funcname);
    else this->error_funcname = std::string();
}

/**
 * デバッグレベル：エラー出力なしであるかチェックする.
 * @return        true=デバッグレベル：エラー出力なし
 */
bool UdmErrorHandler::isDebugLevelSilent() const
{
    int level = this->debug_level & UDM_DEBUG_LEVEL_ERROR_MASK;
    return (level == 0x00);
}

/**
 * デバッグレベル：エラー出力であるかチェックする.
 * @return        true=デバッグレベル：エラー出力
 */
bool UdmErrorHandler::isDebugLevelPrintError() const
{
    if (isDebugLevelSilent()) return false;
    return true;
}

/**
 * デバッグレベル：エラー,警告出力であるかチェックする.
 * @return        true=デバッグレベル：エラー出力
 */
bool UdmErrorHandler::isDebugLevelPrintWarning() const
{
    if (isDebugLevelPrintDebug()) return true;
    if (isDebugLevelPrintInfo()) return true;
    int level = this->debug_level & UDM_DEBUG_LEVEL_ERROR_MASK;
    return (level == 0x02);
}

/**
 * デバッグレベル：エラー, 情報出力であるかチェックする.
 * @return        true=デバッグレベル：エラー, 情報出力
 */
bool UdmErrorHandler::isDebugLevelPrintInfo() const
{
    if (isDebugLevelPrintDebug()) return true;
    int level = this->debug_level & UDM_DEBUG_LEVEL_ERROR_MASK;
    return (level == 0x03);
}

/**
 * デバッグレベル：エラー,情報,デバッグ出力であるかチェックする.
 * @return        true=デバッグレベル：エラー,情報,デバッグ出力
 */
bool UdmErrorHandler::isDebugLevelPrintDebug() const
{
    int level = this->debug_level & UDM_DEBUG_LEVEL_ERROR_MASK;
    return (level == 0x04);
}

/**
 * デバッグレベルを取得する.
 * デバッグレベル : デフォルト 0x0002
 * 0x0000                エラーメッセージ出力なし：エラーメッセージはUdmErrorHandler::messageにセットするのみ。
 * 0x0001                エラーメッセージを常時出力する.
 * 0x0002                エラー、警告メッセージを常時出力する.
 * 0x0003                エラー、警告、情報メッセージを常時出力する.
 * 0x0004                エラー、警告、情報、デバッグメッセージを常時出力する.
 * @return        デバッグレベル
 */
unsigned int UdmErrorHandler::getDebugLevel() const
{
    return this->debug_level;
}

/**
 * デバッグレベルを設定する.
 * デバッグレベル : デフォルト 0x0002
 * 0x0000                エラーメッセージ出力なし：エラーメッセージはUdmErrorHandler::messageにセットするのみ。
 * 0x0001                エラーメッセージを常時出力する.
 * 0x0002                エラー、警告メッセージを常時出力する.
 * 0x0003                エラー、警告、情報メッセージを常時出力する.
 * 0x0004                エラー、警告、情報、デバッグメッセージを常時出力する.
 * @param debug_level        デバッグレベル
 */
void UdmErrorHandler::setDebugLevel(unsigned int debug_level)
{
    this->debug_level = debug_level;
}


/**
 * 出力先（デフォルト 0x01）を取得する.
 * 出力先 : デフォルト 0x01
 * 0x00                    標準出力、ファイル出力なし
 * 0x01                    標準出力のみに出力
 * 0x02                    ファイルのみに出力
 * 0x03                    標準出力、ファイルに出力
 * @return        出力先
 */
unsigned int UdmErrorHandler::getOutputTarget() const
{
    return this->output_target;
}

/**
 * 出力先を設定する.
 * 出力先 : デフォルト 0x01
 * 0x00                    標準出力、ファイル出力なし
 * 0x01                    標準出力のみに出力
 * 0x02                    ファイルのみに出力
 * 0x03                    標準出力、ファイルに出力
 * @param output_target            出力先
 */
void UdmErrorHandler::setOutputTarget(unsigned int output_target)
{
    this->output_target = output_target;
}

/**
 * 標準出力に出力を行うかチェックする.
 * @return        true=標準出力に出力を行う
 */
bool UdmErrorHandler::isOutputTargetStdout() const
{
    return (bool)(this->output_target & 0x01);
}

/**
 * ファイル出力を行うかチェックする.
 * @return        true=ファイル出力を行う.
 */
bool UdmErrorHandler::isOutputTargetFile() const
{
    return (bool)(this->output_target & 0x02);
}

/**
 * ファイル出力を行う
 * @param msg        出力メッセージ
 */
void UdmErrorHandler::writeFile(const char *msg)
{
    char filename[256] = {0x00};

#if defined(_WIN32) && !defined(__NUTC__)
    int pid = GetCurrentProcessId();
#else
    pid_t pid = getpid();
#endif

    // 出力ログファイル名
    sprintf(filename, "udm_pid%06d.log", pid);
    FILE *fp = fopen(filename, "a");
    if (fp == NULL) return;
    fprintf( fp, "%s", msg);
    fclose(fp);

    return;
}

} /* namespace udm */
