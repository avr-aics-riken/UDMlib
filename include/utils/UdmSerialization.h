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

#ifndef _UDMSERIALIZATION_H_
#define _UDMSERIALIZATION_H_

/**
 * @file UdmSerialization.h
 * シリアライズ・デシリアライズクラスのヘッダファイル
 */

#include "UdmBase.h"

namespace udm
{
class UdmSerializeArchive;

/**
 * シリアライズインターフェイス
 */
class UdmISerializable
{
public:
    /**
     * デストラクタ
     */
    virtual ~UdmISerializable() {};

    /**
     * シリアライズを行う.
     * @param archive        シリアライズ・デシリアライズクラス
     */
    virtual UdmSerializeArchive& serialize(UdmSerializeArchive &archive) const = 0;

    /**
     * デシリアライズを行う.
     * @param archive        シリアライズ・デシリアライズクラス
     */
    virtual UdmSerializeArchive& deserialize(UdmSerializeArchive &archive) = 0;
};

/**
 * シリアライズバッファークラス
 */
class UdmSerializeBuffer : public std::streambuf
{
private:
    /// バッファーサイズ
    size_t buffer_size;
    /// シリアライズバッファー
    char *buffer;
    /// オーバーフローサイズ
    size_t overflow_size;

    typedef std::char_traits<char> traits;

public:
    UdmSerializeBuffer();
    UdmSerializeBuffer(char *buf, size_t length);
    virtual ~UdmSerializeBuffer();
    void initialize(char *buf, size_t length);
    std::ios::pos_type seekoff(std::ios::off_type __off, std::ios_base::seekdir __way, std::ios_base::openmode __which);
    std::ios::pos_type seekpos(pos_type __sp, std::ios_base::openmode __which);
    size_t getBufferSize() const;
    char* getBuffer() const;
    size_t getOverflowSize() const;
    void clear();

protected:
    int overflow( int ch = EOF );
    int underflow();
};

/**
 * シリアライズ・デシリアライズクラス
 */
class UdmSerializeArchive : public std::basic_iostream<char>
{
private:
    UdmSerializeBuffer *m_streambuf;        ///< シリアライズバッファー
    std::istream m_in;                        ///< 入力ストリーム
    std::ostream m_out;                        ///< 出力ストリーム

    /**
     * 入力ストリームを取得する.
     * @return        入力ストリーム
     */
    std::istream& in() { return this->m_in; }

    /**
     * 出力ストリームを取得する.
     * @return        出力ストリーム
     */
    std::ostream& out() { return this->m_out; }

public:
    UdmSerializeArchive(UdmSerializeBuffer *buf);
    virtual ~UdmSerializeArchive();
    void close();
    void clear();
    void initialize(char *buf, size_t length);

    /**
     * シリアライズバッファーから読込を行う:プリミティブ型.
     * @param [out] value            取得データ
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& read(char &value) { return this->readPrimitive(value, sizeof(char)); }
    UdmSerializeArchive& read(short &value) { return this->readPrimitive(value, sizeof(short)); }
    UdmSerializeArchive& read(int &value) { return this->readPrimitive(value, sizeof(int)); }
    UdmSerializeArchive& read(long &value) { return this->readPrimitive(value, sizeof(long)); }
    UdmSerializeArchive& read(unsigned char &value) { return this->readPrimitive(value, sizeof(unsigned char)); }
    UdmSerializeArchive& read(unsigned short &value) { return this->readPrimitive(value, sizeof(unsigned short)); }
    UdmSerializeArchive& read(unsigned int &value) { return this->readPrimitive(value, sizeof(unsigned int)); }
    UdmSerializeArchive& read(unsigned long &value) { return this->readPrimitive(value, sizeof(unsigned long)); }
    UdmSerializeArchive& read(float &value) { return this->readPrimitive(value, sizeof(float)); }
    UdmSerializeArchive& read(double &value) { return this->readPrimitive(value, sizeof(double)); }
    UdmSerializeArchive& read(long long &value) { return this->readPrimitive(value, sizeof(long long)); }
    UdmSerializeArchive& read(unsigned long long &value) { return this->readPrimitive(value, sizeof(unsigned long long)); }

    /**
     * オペレータ:シリアライズバッファーから読込を行う:プリミティブ型.
     * @param [out] value        取得データ
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& operator >> (char &value) { return this->readPrimitive(value, sizeof(char)); }
    UdmSerializeArchive& operator >> (short &value) { return this->readPrimitive(value, sizeof(short)); }
    UdmSerializeArchive& operator >> (int &value) { return this->readPrimitive(value, sizeof(int)); }
    UdmSerializeArchive& operator >> (long &value) { return this->readPrimitive(value, sizeof(long)); }
    UdmSerializeArchive& operator >> (unsigned char &value) { return this->readPrimitive(value, sizeof(unsigned char)); }
    UdmSerializeArchive& operator >> (unsigned short &value) { return this->readPrimitive(value, sizeof(unsigned short)); }
    UdmSerializeArchive& operator >> (unsigned int &value) { return this->readPrimitive(value, sizeof(unsigned int)); }
    UdmSerializeArchive& operator >> (unsigned long &value) { return this->readPrimitive(value, sizeof(unsigned long)); }
    UdmSerializeArchive& operator >> (float &value) { return this->readPrimitive(value, sizeof(float)); }
    UdmSerializeArchive& operator >> (double &value) { return this->readPrimitive(value, sizeof(double)); }
    UdmSerializeArchive& operator >> (long long &value) { return this->readPrimitive(value, sizeof(long long)); }
    UdmSerializeArchive& operator >> (unsigned long long &value) { return this->readPrimitive(value, sizeof(unsigned long long)); }
    UdmSerializeArchive& operator >> (UdmDataType_t &value) { return this->readPrimitive(value, sizeof(UdmDataType_t)); }

    /**
     * シリアライズバッファーから読込を行う:サイズ指定.
     * @param [out] value            取得データ
     * @param [in] size            取得サイズ
     * @return        シリアライズ・デシリアライズクラス
     */
    template<typename DATA_TYPE>
    UdmSerializeArchive& read(DATA_TYPE &value, int size)
    {
        return this->readPrimitive(value, size);
    }

    /**
     * シリアライズバッファーから読込を行う:std::string文字列.
     * @param [out] value            取得文字列データ
     * @param [in] size            取得サイズ
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& read(std::string &value, int size)
    {
        char *str = new char[size];
        memset(str, 0x00, size*sizeof(char));
        this->readString(str, size);
        value = std::string(str);
        delete []str;
        return *this;
    }

    /**
     * シリアライズバッファーから読込を行う:char*文字列.
     * @param [out] value            取得文字列データ
     * @param [in] size            取得サイズ
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& read(char *value, int size)
    {
        return this->readString(value, size);
    }


    /**
     * シリアライズバッファーに書込を行う:プリミティブ型.
     * @param [in] value        書込データ
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& write(const char &value) { return this->writePrimitive(value, sizeof(char)); }
    UdmSerializeArchive& write(const short &value) { return this->writePrimitive(value, sizeof(short)); }
    UdmSerializeArchive& write(const int &value) { return this->writePrimitive(value, sizeof(int)); }
    UdmSerializeArchive& write(const long &value) { return this->writePrimitive(value, sizeof(long)); }
    UdmSerializeArchive& write(const unsigned char &value) { return this->writePrimitive(value, sizeof(unsigned char)); }
    UdmSerializeArchive& write(const unsigned short &value) { return this->writePrimitive(value, sizeof(unsigned short)); }
    UdmSerializeArchive& write(const unsigned int &value) { return this->writePrimitive(value, sizeof(unsigned int)); }
    UdmSerializeArchive& write(const unsigned long &value) { return this->writePrimitive(value, sizeof(unsigned long)); }
    UdmSerializeArchive& write(const float &value) { return this->writePrimitive(value, sizeof(float)); }
    UdmSerializeArchive& write(const double &value) { return this->writePrimitive(value, sizeof(double)); }
    UdmSerializeArchive& write(const long long &value) { return this->writePrimitive(value, sizeof(long long)); }
    UdmSerializeArchive& write(const unsigned long long &value) { return this->writePrimitive(value, sizeof(unsigned long long)); }

    /**
     * オペレータ:シリアライズバッファーへ書込を行う:プリミティブ型.
     * @param [in] value        書込データ
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& operator << (const char &value) { return this->writePrimitive(value, sizeof(char)); }
    UdmSerializeArchive& operator << (const short &value) { return this->writePrimitive(value, sizeof(short)); }
    UdmSerializeArchive& operator << (const int &value) { return this->writePrimitive(value, sizeof(int)); }
    UdmSerializeArchive& operator << (const long &value) { return this->writePrimitive(value, sizeof(long)); }
    UdmSerializeArchive& operator << (const unsigned char &value) { return this->writePrimitive(value, sizeof(unsigned char)); }
    UdmSerializeArchive& operator << (const unsigned short &value) { return this->writePrimitive(value, sizeof(unsigned short)); }
    UdmSerializeArchive& operator << (const unsigned int &value) { return this->writePrimitive(value, sizeof(unsigned int)); }
    UdmSerializeArchive& operator << (const unsigned long &value) { return this->writePrimitive(value, sizeof(unsigned long)); }
    UdmSerializeArchive& operator << (const float &value) { return this->writePrimitive(value, sizeof(float)); }
    UdmSerializeArchive& operator << (const double &value) { return this->writePrimitive(value, sizeof(double)); }
    UdmSerializeArchive& operator << (const long long &value) { return this->writePrimitive(value, sizeof(long long)); }
    UdmSerializeArchive& operator << (const unsigned long long &value) { return this->writePrimitive(value, sizeof(unsigned long long)); }
    UdmSerializeArchive& operator << (const UdmDataType_t &value) { return this->writePrimitive(value, sizeof(UdmDataType_t)); }

    /**
     * シリアライズバッファーに書込を行う:サイズ指定.
     * @param [in] value        書込値
     * @param [in] size        書込サイズ
     * @return        シリアライズ・デシリアライズクラス
     */
    template<typename DATA_TYPE>
    UdmSerializeArchive& write(const DATA_TYPE &value, int size)
    {
        return this->writePrimitive(value, size);
    }

    /**
     * シリアライズバッファーに文字列ポインタ指定の文字列を書き込む:std::string。
     * @param [in] value         書込文字列
     * @param [in] size        書込文字列サイズ
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& write(const std::string &value, int size)
    {
        return this->writeString(value.c_str(), size);
    }

    /**
     * シリアライズバッファーに文字列ポインタ指定の文字列を書き込む。
     * @param [in] value         書込文字列
     * @param [in] size        書込文字列サイズ
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& write(const char *value, int size)
    {
        return this->writeString(value, size);
    }


    /**
     * オペレータ:シリアライズバッファーへ書込を行う.
     * @param manip        出力ストリーム
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& operator << (std::ostream&(*manip)(std::ostream&))
    {
        ((*manip)(out()));
        return *this;
    }

    /**
     * オペレータ:シリアライズバッファーから読込を行う:std::string.
     * NULL終端文字まで読み込む
     * @param [out] value        読込文字列
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& operator >> (std::string &value)
    {
        value.clear();
        char ch;
        do
        {
            *this>>ch;
            value.push_back(ch);
        }while(ch&&!in().eof());

        // basic_stringはNULL終端じゃないので
        // 最後に受け取った\0を消す
        if(!ch)
            value.erase(value.end()-1);
        return *this;
    }



    /**
     * オペレータ:シリアライズバッファーへ書込を行う:文字列(basic_string)
     * @param [in] value        書込文字列
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& operator << (const std::string &value)
    {
        return *this<<value.c_str();   // char*として書込みを行う
    }

    /**
     * オペレータ:シリアライズバッファーへ書込を行う:文字列ポインタ(const char*).
     * 末尾にNULLを挿入する.
     * @param [in] value        書込文字列ポインタ
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& operator << (const char*value)
    {
        unsigned int len = strlen(value);
        this->writeString(value, len);
        return *this << std::ends;
    }

    /**
     * シリアライズ可能クラスをシリアライズを行い、書込データを作成する.
     * @param [in] value         シリアライズ可能クラス
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& write(const UdmISerializable&value)
    {
        return this->writeObject(value);
    }

    /**
     * シリアライズ可能クラスをシリアライズを行い、書込データを作成する.
     * @param [in] value         シリアライズ可能クラス
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& read(UdmISerializable&value)
    {
        return this->readObject(value);
    }


    /**
     * オペレータ:シリアライズ可能クラスをシリアライズを行い、書込データを作成する.
     * @param [in] value        シリアライズ可能クラス
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& operator << (const UdmISerializable&value)
    {
        return this->writeObject(value);
    }

    /**
     * オペレータ:シリアライズ可能クラスを読込データからデシリアライズを行う.
     * @param [in] value        シリアライズ可能クラス
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& operator >> (UdmISerializable&value)
    {
        return this->readObject(value);
    }

    /**
     * シリアライズバッファーに書込を行う:プリミティブ型.
     * @param [in] value        書込値
     * @param [in] size        書込サイズ
     * @return        シリアライズ・デシリアライズクラス
     */
    template<typename DATA_TYPE>
    UdmSerializeArchive& writePrimitive(const DATA_TYPE &value, int size)
    {
        this->out().write((char*)&value, size);
        return *this;
    }

    /**
     * シリアライズバッファーに文字列ポインタ指定の文字列を書き込む。
     * @param [in] value         書込文字列
     * @param [in] size        書込文字列サイズ
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& writeString(const char *value, int size)
    {
        int len = strlen(value);
        int write_len = len;
        if (len >= size) {
            write_len = size;
        }
        this->out().write((char*)value, write_len);

        // 余分な文字数を0x00埋めする.
        if (len < size) {
            int n;
            char null_value = 0x00;
            for (n=len; n<size; n++) {
                this->out().write((char*)&null_value, 1);
            }
        }

        return *this;
    }

    /**
     * シリアライズ可能クラスをシリアライズを行い、書込データを作成する.
     * @param [in] value         シリアライズ可能クラス
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& writeObject(const UdmISerializable&value)
    {
        return value.serialize(*this);
    }

    /**
     * シリアライズバッファーから読込を行う:プリミティブ型.
     * @param [out] value            取得値
     * @param [in] size            取得サイズ
     * @return        シリアライズ・デシリアライズクラス
     */
    template<typename DATA_TYPE>
    UdmSerializeArchive& readPrimitive(DATA_TYPE &value, int size)
    {
        this->in().read((char*)&value, size);

        // 正しく読めたか確認
        int sz=in().gcount();
        if(sz != sizeof(DATA_TYPE)) {
            this->setstate(std::ios_base::failbit);
        }
        return *this;
    }
    /**
     * シリアライズ可能クラスをシリアライズを行い、書込データを作成する.
     * @param [in] value         シリアライズ可能クラス
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& readObject(UdmISerializable&value)
    {
        value.deserialize(*this);
        return *this;
    }

    /**
     * シリアライズバッファーから文字サイズ分の読込を行う:文字列.
     * @param [out] value         取得文字列
     * @param [in] size        取得文字列サイズ
     * @return        シリアライズ・デシリアライズクラス
     */
    UdmSerializeArchive& readString(char *value, int size)
    {
        this->in().read(value, size);

        // 正しく読めたか確認
        int sz=in().gcount();
        if (sz != size) {
            setstate(std::ios_base::failbit);
        }
        return *this;
    }


    /**
     * ストリームバッファーのオーバーフローサイズを取得する.
     * @return        オーバーフローサイズ
     */
    size_t getOverflowSize() const
    {
        if (this->m_streambuf == NULL) return 0;
        return this->m_streambuf->getOverflowSize();
    }

    /**
     * ストリームからの入出力が正常に終了したかチェックする.
     * @return        true=正常終了
     */
    bool validateFinish()
    {
        if (this->m_streambuf == NULL) return false;
        if (this->rdstate() != std::ios_base::goodbit) {
            return false;
        }
        return true;
    }
};


} /* namespace udm */

#endif /* _UDMSERIALIZATION_H_ */
