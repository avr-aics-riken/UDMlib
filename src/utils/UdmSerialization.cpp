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

#include "utils/UdmSerialization.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmSerializeBuffer::UdmSerializeBuffer() : buffer_size(0), buffer(NULL), overflow_size(0)
{
    setbuf(NULL, 0);
}


/**
 * コンストラクタ
 * @param buf        シリアライズバッファー
 * @param length    バッファーサイズ
 */
UdmSerializeBuffer::UdmSerializeBuffer(char *buf, size_t length)
                    : buffer_size(length), buffer(NULL), overflow_size(0)
{
    this->initialize(buf, length);
}

/**
 * デストラクタ
 */
UdmSerializeBuffer::~UdmSerializeBuffer()
{
}

/**
 * 初期化を行う.
 * @param buf        シリアライズバッファー
 * @param length    バッファーサイズ
 */
void UdmSerializeBuffer::initialize(char *buf, size_t length)
{
    this->buffer_size = length;
    this->buffer = buf;
    this->overflow_size = 0;
    char* bufferEnd = this->buffer + length;
    setp(this->buffer, bufferEnd);
    setg(this->buffer, this->buffer, bufferEnd);
}

/**
 * バッファーをクリアする.
 */
void UdmSerializeBuffer::clear()
{
    this->buffer_size = 0;
    this->buffer = NULL;
    this->overflow_size = 0;
    setbuf(NULL, 0);
}

/**
 * 入力、出力時のポインタを移動する.
 * @param __off        シークする相対位置
 * @param __way        シークの開始点 (先頭=beg、末尾=end、現在位置=cur)
 * @param __which    シークする I/O の方向（入力=in、出力=out）
 * @return        シーク位置
 */
std::ios::pos_type UdmSerializeBuffer::seekoff(
                    std::ios::off_type __off,
                    std::ios_base::seekdir __way,
                    std::ios_base::openmode __which)
{
    std::ios::pos_type result;
    // 入出力の処理対象を取得します。
    bool isIn = (__which & std::ios_base::in) == std::ios_base::in;
    bool isOut = (__which & std::ios_base::out) == std::ios_base::out;

    if (!isIn && !isOut) {
        // 入力にも出力にも該当しない場合はエラーとします。
        result = std::ios::pos_type(std::ios::off_type(-1));
        return result;
    }
    else if ((isIn && isOut) && (__way == std::ios_base::cur)) {
        // 入出力両方が対象で、現在位置からの移動の場合はエラーとします。
        result = std::ios::pos_type(std::ios::off_type(-1));
        return result;
    }

    // 基本的な入出力指定に誤りがなかった場合の処理です。
    // 入出力それぞれの、バッファの移動先の位置を示すポインタをここに揃えます。
    char* targetIn;
    char* targetOut;
    // 入出力それぞれの、先頭からの移動先の位置を示すインデックスをここに揃えます。
    std::ios::pos_type posIn;
    std::ios::pos_type posOut;
    // 入力であれば、入力位置を計算します。
    if (isIn) {
        // どこからの相対位置かによって、移動先の位置を特定します。
        if (__way == std::ios_base::beg) {
            targetIn = this->eback() + __off;
        }
        else if (__way == std::ios_base::end) {
            targetIn = this->egptr() + __off;
        }
        else if (__way == std::ios_base::cur) {
            targetIn = this->gptr() + __off;
        }
        // 移動先がバッファの範囲の収まるかどうかを調べます。
        if (this->eback() <= targetIn && targetIn <= this->egptr()) {
            // 収まるときは、先頭からのインデックスを計算します。
            posIn = targetIn - this->eback();
        }
        else {
            // 収まらない場合は、エラーの状態にします。
            targetIn = NULL;
            posIn = std::ios::pos_type(std::ios::off_type(-1));
        }
    }
    else {
        // 入力が対象でないときは、とりあえず入力についてはエラーの状態にします。
        targetIn = NULL;
        posIn = std::ios::pos_type(std::ios::off_type(-1));
    }

    // 出力であれば、出力位置を計算します。
    if (isOut) {
        // どこからの相対位置かによって、移動先の位置を特定します。
        if (__way == std::ios_base::beg) {
            targetOut = this->pbase() + __off;
        }
        else if (__way == std::ios_base::end) {
            targetOut = this->epptr() + __off;
        }
        else if (__way ==  std::ios_base::cur) {
            targetOut = this->pptr() + __off;
        }
        // 移動先がバッファの範囲の収まるかどうかを調べます。
        if (this->pbase() <= targetOut && targetOut <= this->epptr()) {
            // 収まるときは、先頭からのインデックスを計算します。
            posOut = targetOut - this->pbase();
        }
        else {
            // 収まらない場合は、エラーの状態にします。
            targetOut = NULL;
            posOut = std::ios::pos_type(std::ios::off_type(-1));
        }
    }
    else {
        // 出力が対象でないときは、とりあえず出力についてはエラーの状態にします。
        targetOut = NULL;
        posOut = std::ios::pos_type(std::ios::off_type(-1));
    }

    // 実際の移動の前に、エラーがないか確認します。
    if ((isIn && !targetIn) || (isOut && !targetOut)) {
        // 入力位置が特定できなかったか、出力位置が特定できなかった場合はエラーとします。
        result = std::ios::pos_type(std::ios::off_type(-1));
        return result;
    }
    else if ((isIn && isOut) && (posIn != posOut)) {
        // 入出力双方で位置が異なった場合はエラーとします。
        result = std::ios::pos_type(std::ios::off_type(-1));
        return result;
    }

    // 戻り値を確定します。In も Out も同じ値なので、有効な方を取得します。
    result = (isIn ? posIn : posOut);
    // 入力指定で、現在位置が異なる場合に再設定します。
    if (isIn && (gptr() != targetIn)) {
        this->setg(this->eback(), targetIn, this->egptr());
    }
    // 出力指定で、現在位置が異なる場合に再設定します。
    if (isOut && (pptr() != targetOut)) {
        this->pbump(targetOut - this->pptr());
    }

    return result;
};

/**
 * 指定された位置までシークします。
 * @param __sp            ストリームの先頭からのオフセット
 * @param __which        シークする I/O の方向（入力=in、出力=out）
 * @return        シーク位置
 */
std::ios::pos_type UdmSerializeBuffer::seekpos(pos_type __sp, std::ios_base::openmode __which)
{
    return this->seekoff(off_type(__sp), std::ios_base::beg, __which);
}


/**
 * 書込バッファーのポインタが最後まできた時に呼び出される。
 * @param ch        書き込みデータ
 * @return            書き込みデータ又はエラー
 */
int UdmSerializeBuffer::overflow( int ch)
{
    // 書き込みバッファーが存在しない場合は、バッファーサイズをインクリメントする.
    if (this->buffer == NULL) {
        this->overflow_size++;
    }
    return ch;
}

/**
 * 読込バッファーが空になった時に呼び出される。
 * @return        EOFを返す.
 */
int UdmSerializeBuffer::underflow()
{
    return traits::eof();
}

/**
 * バッファーサイズを取得する.
 * @return        バッファーサイズ
 */
size_t UdmSerializeBuffer::getBufferSize() const
{
    return this->buffer_size;
}

/**
 * バッファーを取得する.
 * @return        バッファー
 */
char* UdmSerializeBuffer::getBuffer() const
{
    return this->buffer;
}

/**
 * オーバーフローサイズを取得する.
 * @return        オーバーフローサイズ
 */
size_t UdmSerializeBuffer::getOverflowSize() const
{
    return this->overflow_size;
}


/**
 * コンストラクタ
 * @param buf        シリアライズバッファー
 */
UdmSerializeArchive::UdmSerializeArchive(UdmSerializeBuffer *buf)
                : std::basic_iostream<char>(buf), 
                  m_streambuf(buf), m_in (buf), m_out(buf)
{
    this->init(m_streambuf);
    this->clear();
}


/**
 * デストラクタ
 */
UdmSerializeArchive::~UdmSerializeArchive()
{
    this->close();
}

/**
 * 破棄処理を行う.
 */
void UdmSerializeArchive::close()
{
    this->clear();
}


/**
 * バッファーをクリアする.
 */
void UdmSerializeArchive::clear()
{
    if (this->m_streambuf != NULL) {
        this->m_streambuf->clear();
    }
    std::basic_iostream<char>::clear();
}


void UdmSerializeArchive::initialize(char* buf, size_t length)
{
    if (this->m_streambuf != NULL) {
        this->m_streambuf->initialize(buf, length);
    }
    std::basic_iostream<char>::clear();
}
} /* namespace udm */
