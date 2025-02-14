// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0


#include <QTest>
#include <QtCore/qendian.h>
#include <QtCore/private/qendian_p.h>
#include <QtCore/qsysinfo.h>

class tst_QtEndian: public QObject
{
    Q_OBJECT
public:
    enum Signedness {
        Unsigned,
        Signed
    };
    Q_ENUM(Signedness);

private slots:
    void fromBigEndian();
    void fromLittleEndian();
    void fromBigEndianRegion_data();
    void fromBigEndianRegion();
    void fromLittleEndianRegion_data() { fromBigEndianRegion_data(); }
    void fromLittleEndianRegion();

    void toBigEndian();
    void toLittleEndian();
    void toBigEndianRegion_data() { fromBigEndianRegion_data(); }
    void toBigEndianRegion();
    void toLittleEndianRegion_data() { fromBigEndianRegion_data(); }
    void toLittleEndianRegion();

    void endianIntegers_data();
    void endianIntegers();

    void endianBitfields();

    void endianBitfieldUnions_data();
    void endianBitfieldUnions();
};

struct TestData
{
    quint64 data64;
    quint32 data32;
    quint16 data16;
    quint8 data8;

    float dataFloat;
    double dataDouble;

    quint8 reserved;
};

template <typename T> T getData(const TestData &d);
template <> quint8 getData(const TestData &d) { return d.data8; }
template <> quint16 getData(const TestData &d) { return d.data16; }
template <> quint32 getData(const TestData &d) { return d.data32; }
template <> quint64 getData(const TestData &d) { return d.data64; }
template <> float getData(const TestData &d) { return d.dataFloat; }

union RawTestData
{
    uchar rawData[sizeof(TestData)];
    TestData data;
};

template <typename Float>
Float int2Float(typename QIntegerForSizeof<Float>::Unsigned i)
{
    Float result = 0;
    memcpy(reinterpret_cast<char *>(&result), reinterpret_cast<const char *>(&i), sizeof (Float));
    return result;
}

static const TestData inNativeEndian = {
    Q_UINT64_C(0x0123456789abcdef),
    0x00c0ffee,
    0xcafe,
    0xcf,
    int2Float<float>(0x00c0ffeeU),
    int2Float<double>(Q_UINT64_C(0x0123456789abcdef)),
    '\0'
};
static const RawTestData inBigEndian = {
    "\x01\x23\x45\x67\x89\xab\xcd\xef"
    "\x00\xc0\xff\xee"
    "\xca\xfe"
    "\xcf"
    "\x00\xc0\xff\xee"
    "\x01\x23\x45\x67\x89\xab\xcd\xef"
};
static const RawTestData inLittleEndian = {
    "\xef\xcd\xab\x89\x67\x45\x23\x01"
    "\xee\xff\xc0\x00"
    "\xfe\xca"
    "\xcf"
    "\xee\xff\xc0\x00"
    "\xef\xcd\xab\x89\x67\x45\x23\x01"
};

#define EXPAND_ENDIAN_TEST(endian)          \
    do {                                    \
        /* Unsigned tests */                \
        ENDIAN_TEST(endian, quint, 64);     \
        ENDIAN_TEST(endian, quint, 32);     \
        ENDIAN_TEST(endian, quint, 16);     \
        ENDIAN_TEST(endian, quint, 8);      \
                                            \
        /* Signed tests */                  \
        ENDIAN_TEST(endian, qint, 64);      \
        ENDIAN_TEST(endian, qint, 32);      \
        ENDIAN_TEST(endian, qint, 16);      \
        ENDIAN_TEST(endian, qint, 8);       \
    } while (false)                         \
    /**/

#define ENDIAN_TEST(endian, type, size)                                                 \
    do {                                                                                \
        QCOMPARE(qFrom ## endian ## Endian(                                             \
                    (type ## size)(in ## endian ## Endian.data.data ## size)),          \
                (type ## size)(inNativeEndian.data ## size));                           \
        QCOMPARE(qFrom ## endian ## Endian<type ## size>(                               \
                    in ## endian ## Endian.rawData + offsetof(TestData, data ## size)), \
                (type ## size)(inNativeEndian.data ## size));                           \
    } while (false)                                                                     \
    /**/

void tst_QtEndian::fromBigEndian()
{
    EXPAND_ENDIAN_TEST(Big);
}

void tst_QtEndian::fromLittleEndian()
{
    EXPAND_ENDIAN_TEST(Little);
}

#undef ENDIAN_TEST

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wmemset-elt-size")

template <typename T>
void transformRegion_template(T (*transformOne)(T), void (*transformRegion)(const void *, qsizetype, void *))
{
    enum { Size = 64 };
    T source[Size];
    T dest[Size];
    T expected = transformOne(getData<T>(inNativeEndian));
    std::fill_n(source, +Size, getData<T>(inNativeEndian));
    memset(dest, 0, sizeof(dest));

    auto checkBounds = [&](int from) {
        for ( ; from < Size; ++from)
            QCOMPARE(dest[from], T(0));
    };

    transformRegion(source, 1, dest);
    QCOMPARE(dest[0], expected);
    checkBounds(1);
    memset(dest, 0, sizeof(T));

    transformRegion(source, 2, dest);
    QCOMPARE(dest[0], expected);
    QCOMPARE(dest[1], expected);
    checkBounds(2);
    memset(dest, 0, sizeof(T) * 2);

    transformRegion(source, 3, dest);
    QCOMPARE(dest[0], expected);
    QCOMPARE(dest[1], expected);
    QCOMPARE(dest[2], expected);
    checkBounds(3);
    memset(dest, 0, sizeof(T) * 3);

    transformRegion(source, 4, dest);
    QCOMPARE(dest[0], expected);
    QCOMPARE(dest[1], expected);
    QCOMPARE(dest[2], expected);
    QCOMPARE(dest[3], expected);
    checkBounds(4);
    memset(dest, 0, sizeof(T) * 4);

    transformRegion(source, 8, dest);
    for (int i = 0; i < 8; ++i)
        QCOMPARE(dest[i], expected);
    checkBounds(8);
    memset(dest, 0, sizeof(T) * 8);

    transformRegion(source, 16, dest);
    for (int i = 0; i < 16; ++i)
        QCOMPARE(dest[i], expected);
    checkBounds(16);
    memset(dest, 0, sizeof(T) * 16);

    transformRegion(source, 32, dest);
    for (int i = 0; i < 32; ++i)
        QCOMPARE(dest[i], expected);
    checkBounds(32);
    memset(dest, 0, sizeof(T) * 32);

    transformRegion(source, 64, dest);
    for (int i = 0; i < 64; ++i)
        QCOMPARE(dest[i], expected);

    // check transforming in-place
    memcpy(dest, source, sizeof(dest));
    transformRegion(dest, 64, dest);
    for (int i = 0; i < 64; ++i)
        QCOMPARE(dest[i], expected);
}
QT_WARNING_POP

void tst_QtEndian::fromBigEndianRegion_data()
{
    QTest::addColumn<int>("size");
    QTest::newRow("1") << 1;
    QTest::newRow("2") << 2;
    QTest::newRow("4") << 4;
    QTest::newRow("8") << 8;
}

void tst_QtEndian::fromBigEndianRegion()
{
    QFETCH(int, size);
    switch (size) {
    case 1: return transformRegion_template<quint8>(qFromBigEndian<quint8>, qFromBigEndian<quint8>);
    case 2: return transformRegion_template<quint16>(qFromBigEndian<quint16>, qFromBigEndian<quint16>);
    case 4: return transformRegion_template<quint32>(qFromBigEndian<quint32>, qFromBigEndian<quint32>);
    case 8: return transformRegion_template<quint64>(qFromBigEndian<quint64>, qFromBigEndian<quint64>);
    }
}

void tst_QtEndian::fromLittleEndianRegion()
{
    QFETCH(int, size);
    switch (size) {
    case 1: return transformRegion_template<quint8>(qFromLittleEndian<quint8>, qFromLittleEndian<quint8>);
    case 2: return transformRegion_template<quint16>(qFromLittleEndian<quint16>, qFromLittleEndian<quint16>);
    case 4: return transformRegion_template<quint32>(qFromLittleEndian<quint32>, qFromLittleEndian<quint32>);
    case 8: return transformRegion_template<quint64>(qFromLittleEndian<quint64>, qFromLittleEndian<quint64>);
    }
}

#define ENDIAN_TEST(endian, type, size)                                                 \
    do {                                                                                \
        QCOMPARE(qTo ## endian ## Endian(                                               \
                    (type ## size)(inNativeEndian.data ## size)),                       \
                (type ## size)(in ## endian ## Endian.data.data ## size));              \
                                                                                        \
        RawTestData test;                                                               \
        qTo ## endian ## Endian(                                                        \
                (type ## size)(inNativeEndian.data ## size),                            \
                test.rawData + offsetof(TestData, data ## size));                       \
        QCOMPARE(test.data.data ## size, in ## endian ## Endian.data.data ## size );    \
    } while (false)                                                                     \
    /**/

void tst_QtEndian::toBigEndian()
{
    EXPAND_ENDIAN_TEST(Big);
}

void tst_QtEndian::toLittleEndian()
{
    EXPAND_ENDIAN_TEST(Little);
}

#undef ENDIAN_TEST

void tst_QtEndian::toBigEndianRegion()
{
    QFETCH(int, size);
    switch (size) {
    case 1: return transformRegion_template<quint8>(qToBigEndian<quint8>, qToBigEndian<quint8>);
    case 2: return transformRegion_template<quint16>(qToBigEndian<quint16>, qToBigEndian<quint16>);
    case 4: return transformRegion_template<quint32>(qToBigEndian<quint32>, qToBigEndian<quint32>);
    case 8: return transformRegion_template<quint64>(qToBigEndian<quint64>, qToBigEndian<quint64>);
    }
}

void tst_QtEndian::toLittleEndianRegion()
{
    QFETCH(int, size);
    switch (size) {
    case 1: return transformRegion_template<quint8>(qToLittleEndian<quint8>, qToLittleEndian<quint8>);
    case 2: return transformRegion_template<quint16>(qToLittleEndian<quint16>, qToLittleEndian<quint16>);
    case 4: return transformRegion_template<quint32>(qToLittleEndian<quint32>, qToLittleEndian<quint32>);
    case 8: return transformRegion_template<quint64>(qToLittleEndian<quint64>, qToLittleEndian<quint64>);
    }
}

void tst_QtEndian::endianIntegers_data()
{
    QTest::addColumn<int>("val");

    QTest::newRow("-30000") << -30000;
    QTest::newRow("-1") << -1;
    QTest::newRow("0") << 0;
    QTest::newRow("1020") << 1020;
    QTest::newRow("16385") << 16385;
}

void tst_QtEndian::endianIntegers()
{
    QFETCH(int, val);

    qint16 vi16 = val;
    qint32 vi32 = val;
    qint64 vi64 = val;
    quint16 vu16 = val;
    quint32 vu32 = val;
    quint64 vu64 = val;

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    QCOMPARE(*reinterpret_cast<qint16_be*>(&vi16), vi16);
    QCOMPARE(*reinterpret_cast<qint32_be*>(&vi32), vi32);
    QCOMPARE(*reinterpret_cast<qint64_be*>(&vi64), vi64);
    QCOMPARE(*reinterpret_cast<qint16_le*>(&vi16), qbswap(vi16));
    QCOMPARE(*reinterpret_cast<qint32_le*>(&vi32), qbswap(vi32));
    QCOMPARE(*reinterpret_cast<qint64_le*>(&vi64), qbswap(vi64));
    QCOMPARE(*reinterpret_cast<quint16_be*>(&vu16), vu16);
    QCOMPARE(*reinterpret_cast<quint32_be*>(&vu32), vu32);
    QCOMPARE(*reinterpret_cast<quint64_be*>(&vu64), vu64);
    QCOMPARE(*reinterpret_cast<quint16_le*>(&vu16), qbswap(vu16));
    QCOMPARE(*reinterpret_cast<quint32_le*>(&vu32), qbswap(vu32));
    QCOMPARE(*reinterpret_cast<quint64_le*>(&vu64), qbswap(vu64));
#else
    QCOMPARE(*reinterpret_cast<qint16_be*>(&vi16), qbswap(vi16));
    QCOMPARE(*reinterpret_cast<qint32_be*>(&vi32), qbswap(vi32));
    QCOMPARE(*reinterpret_cast<qint64_be*>(&vi64), qbswap(vi64));
    QCOMPARE(*reinterpret_cast<qint16_le*>(&vi16), vi16);
    QCOMPARE(*reinterpret_cast<qint32_le*>(&vi32), vi32);
    QCOMPARE(*reinterpret_cast<qint64_le*>(&vi64), vi64);
    QCOMPARE(*reinterpret_cast<quint16_be*>(&vu16), qbswap(vu16));
    QCOMPARE(*reinterpret_cast<quint32_be*>(&vu32), qbswap(vu32));
    QCOMPARE(*reinterpret_cast<quint64_be*>(&vu64), qbswap(vu64));
    QCOMPARE(*reinterpret_cast<quint16_le*>(&vu16), vu16);
    QCOMPARE(*reinterpret_cast<quint32_le*>(&vu32), vu32);
    QCOMPARE(*reinterpret_cast<quint64_le*>(&vu64), vu64);
#endif
}

void tst_QtEndian::endianBitfields()
{
    union {
        quint32_be_bitfield<21, 11> upper;
        quint32_be_bitfield<10, 11> lower;
        qint32_be_bitfield<0, 10> bottom;
    } u;

    u.upper = 200;
    QCOMPARE(u.upper, 200U);
    u.lower = 1000;
    u.bottom = -8;
    QCOMPARE(u.lower, 1000U);
    QCOMPARE(u.upper, 200U);

    u.lower += u.upper;
    QCOMPARE(u.upper, 200U);
    QCOMPARE(u.lower, 1200U);

    u.upper = 65536 + 7;
    u.lower = 65535;
    QCOMPARE(u.lower, 65535U & ((1<<11) - 1));
    QCOMPARE(u.upper, 7U);

    QCOMPARE(u.bottom, -8);
}

template<template<typename... Accessors> typename Union, template<int, int, typename> typename Member>
void testBitfieldUnion()
{
    using upper = Member<21, 11, uint>;
    using lower = Member<10, 11, uint>;
    using bottom = Member<0, 10, int>;

    using UnionType = Union<upper, lower, bottom>;
    UnionType u;

    u.template set<upper>(200);
    QCOMPARE(u.template get<upper>(), 200U);
    u.template set<lower>(1000);
    u.template set<bottom>(-8);
    QCOMPARE(u.template get<lower>(), 1000U);
    QCOMPARE(u.template get<upper>(), 200U);

    u.template set<lower>(u.template get<lower>() + u.template get<upper>());
    QCOMPARE(u.template get<upper>(), 200U);
    QCOMPARE(u.template get<lower>(), 1200U);

    u.template set<upper>(65536 + 7);
    u.template set<lower>(65535);
    QCOMPARE(u.template get<lower>(), 65535U & ((1<<11) - 1));
    QCOMPARE(u.template get<upper>(), 7U);

    QCOMPARE(u.template get<bottom>(), -8);

    UnionType u2(QSpecialIntegerBitfieldZero);
    QCOMPARE(u2.data(), 0U);

    UnionType u3(42U);
    QCOMPARE(u3.data(), 42U);

    using BEUintAccessor =  QSpecialIntegerAccessor<QBigEndianStorageType<uint>, 21, 11>;
    using LEUintAccessor =  QSpecialIntegerAccessor<QLittleEndianStorageType<uint>, 21, 11>;
    using BEIntAccessor =  QSpecialIntegerAccessor<QBigEndianStorageType<int>, 0, 10>;
    using LEIntAccessor =  QSpecialIntegerAccessor<QLittleEndianStorageType<int>, 0, 10>;

    if constexpr (std::is_same_v<BEUintAccessor, upper>) {
        QCOMPARE(u.template get<BEUintAccessor>(), 7U);
    } else if constexpr (std::is_same_v<LEUintAccessor, upper>) {
        QCOMPARE(u.template get<LEUintAccessor>(), 7U);
    } else if constexpr (std::is_same_v<BEIntAccessor, bottom>) {
        QCOMPARE(u.template get<BEIntAccessor>(), -8);
    } else if constexpr (std::is_same_v<LEIntAccessor, bottom>) {
        QCOMPARE(u.template get<LEIntAccessor>(), -8);
    } else {
        QFAIL("none of the manually defined accessors match");
    }
}

void tst_QtEndian::endianBitfieldUnions_data()
{
    QTest::addColumn<QSysInfo::Endian>("byteOrder");
    QTest::addColumn<Signedness>("signedness");

    QTest::addRow("little endian unsigned") << QSysInfo::LittleEndian << Unsigned;
    QTest::addRow("little endian signed") << QSysInfo::LittleEndian << Signed;
    QTest::addRow("big endian unsigned") << QSysInfo::BigEndian << Unsigned;
    QTest::addRow("big endian signed") << QSysInfo::BigEndian << Signed;
}

void tst_QtEndian::endianBitfieldUnions()
{
    QFETCH(QSysInfo::Endian, byteOrder);
    QFETCH(Signedness, signedness);

    switch (byteOrder) {
    case QSysInfo::LittleEndian:
        switch (signedness) {
        case Unsigned:
            testBitfieldUnion<quint32_le_bitfield_union, quint32_le_bitfield_member>();
            return;
        case Signed:
            testBitfieldUnion<qint32_le_bitfield_union, qint32_le_bitfield_member>();
            return;
        }
        Q_UNREACHABLE();
        return;
    case QSysInfo::BigEndian:
        switch (signedness) {
        case Unsigned:
            testBitfieldUnion<quint32_be_bitfield_union, quint32_be_bitfield_member>();
            return;
        case Signed:
            testBitfieldUnion<qint32_be_bitfield_union, qint32_be_bitfield_member>();
            return;
        }
        Q_UNREACHABLE();
        return;
    }
}


QTEST_MAIN(tst_QtEndian)
#include "tst_qtendian.moc"
