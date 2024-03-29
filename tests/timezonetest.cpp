/*
 * Copyright (C) 2012  Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "timezonetest.h"
#include <conversion/timezoneconverter.h>
#include <kolabformat/kolabobject.h>
#include <kolabformat/errorhandler.h>
#include "testutils.h"

#include <QTest>
// #include <unicode/uversion.h>
// #include <unicode/timezone.h>
// #include <iostream>
#include <kdebug.h>
#include <kcalcore/event.h>
#include <kcalcore/icalformat.h>

// void icuFoo()
// {
//     icu::UnicodeString s;
//     UErrorCode error;
// //     icu::TimeZone::getCanonicalID("GMT+01.00) Sarajevo/Warsaw/Zagreb", s, error);
// //     icu::TimeZone::getCanonicalID(icu::UnicodeString::fromUTF8("Europe/Zurich"), s, error);
//     icu::TimeZone::getCanonicalID(icu::UnicodeString::fromUTF8("GMT+01.00"), s, error);
//     std::string cs;
//     s.toUTF8String(cs);
//     std::cout << "This is the new timezone: " << cs  << std::endl << u_errorName(error) << std::endl;
// // 
// // 
// //     icu::TimeZone *tz = icu::TimeZone::createTimeZone("GMT-8:00");
// //     icu::UnicodeString result;
// //     tz->getDisplayName(result);
// //     std::string stringresult;
// //     result.toUTF8String(stringresult);
// //     std::cout << stringresult;
//     
// //     icu::TimeZone *tz = icu::TimeZone::getStaticClassID();
// 
// }

void TimezoneTest::testFromName()
{
    TimezoneConverter converter;
    const QString timezone = converter.normalizeTimezone("(GMT+01.00) Sarajevo/Warsaw/Zagreb");
    QCOMPARE(timezone, QLatin1String("Europe/Sarajevo"));
}

void TimezoneTest::testFromHardcodedList_data()
{
    QTest::addColumn<QString>( "timezone" );
    
    QTest::newRow( "1" ) << QString::fromLatin1("(GMT+01:00) West Central Africa");
    QTest::newRow( "2" ) << QString::fromLatin1("(GMT-04:00) Atlantic Time (Canada)");
    QTest::newRow( "3" ) << QString::fromLatin1("(GMT-06:00) Saskatchewan");
    QTest::newRow( "4" ) << QString::fromLatin1("(GMT-01:00) Cape Verde Islands");
    QTest::newRow( "5" ) << QString::fromLatin1("(GMT-06:00) Central America");
    QTest::newRow( "6" ) << QString::fromLatin1("(GMT-06:00) Central Time (US and Canada)");
//     QTest::newRow( "7" ) << QString::fromLatin1("(GMT-12:00) International Date Line West"); //Not mappable
    QTest::newRow( "8" ) << QString::fromLatin1("(GMT-05:00) Eastern Time (US and Canada)");
//     QTest::newRow( "9" ) << QString::fromLatin1("(GMT-02:00) Mid-Atlantic"); //Not mappable
    QTest::newRow( "10" ) << QString::fromLatin1("(GMT-07:00) Mountain Time (US and Canada)");
    QTest::newRow( "11" ) << QString::fromLatin1("(GMT-03:30) Newfoundland and Labrador");
    QTest::newRow( "12" ) << QString::fromLatin1("(GMT-08:00) Pacific Time (US and Canada); Tijuana");
    QTest::newRow( "13" ) << QString::fromLatin1("(GMT-11:00) Midway Island, Samoa");
    QTest::newRow( "14" ) << QString::fromLatin1("W. Europe Standard Time");
}

void TimezoneTest::testFromHardcodedList()
{
    TimezoneConverter converter;
    QFETCH(QString, timezone);
    const QString tz = converter.normalizeTimezone(timezone);
    kDebug() << tz;
    QVERIFY(!tz.isEmpty());
    QVERIFY(tz != timezone);
}

void TimezoneTest::testKolabObjectWriter()
{
    KCalCore::Event::Ptr event(new KCalCore::Event());
    event->setOrganizer(KCalCore::Person::Ptr());
    event->setDtStart(KDateTime(QDate(2012,11,11), QTime(1,1), KDateTime::Spec(KTimeZone("(GMT+01:00) West Central Africa"))));
    KMime::Message::Ptr msg = Kolab::KolabObjectWriter::writeEvent(event);
    Kolab::KolabObjectReader reader(msg);
    KCalCore::Event::Ptr result = reader.getEvent();
    kDebug() << result->dtStart().timeZone().name();
    QCOMPARE(result->dtStart().timeZone().name(), KTimeZone(QLatin1String("Africa/Lagos")).name());
}

void TimezoneTest::testKolabObjectReader()
{
    const Kolab::Version version = Kolab::KolabV3;
    const Kolab::ObjectType type = Kolab::EventObject;
    QString icalFileName = TESTFILEDIR+QString::fromLatin1("timezone/windowsTimezone.ics"); //To compare
    QString mimeFileName = TESTFILEDIR+QString::fromLatin1("timezone/windowsTimezoneV3.mime"); //For parsing

    //Parse mime message
    bool ok = false;
    const KMime::Message::Ptr &msg = readMimeFile( mimeFileName, ok );
    QVERIFY(ok);
    Kolab::KolabObjectReader reader;
    Kolab::ObjectType t = reader.parseMimeMessage(msg);
    QCOMPARE(t, type);
    QCOMPARE(reader.getVersion(), version);
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Debug);

    KCalCore::Incidence::Ptr convertedIncidence = reader.getIncidence();
    kDebug() << "read incidence";

    //Parse ICalFile for comparison
    QFile icalFile( icalFileName );
    QVERIFY( icalFile.open( QFile::ReadOnly ) );
    KCalCore::ICalFormat format;
    KCalCore::Incidence::Ptr realIncidence( format.fromString( QString::fromUtf8( icalFile.readAll() ) ) );

    // fix up the converted incidence for comparisson
    normalizeIncidence(convertedIncidence);
    normalizeIncidence(realIncidence);

    // recurrence objects are created on demand, but KCalCore::Incidence::operator==() doesn't take that into account
    // so make sure both incidences have one
    realIncidence->recurrence();
    convertedIncidence->recurrence();

    realIncidence->setLastModified(convertedIncidence->lastModified());

    //The following test is just for debugging and not really relevant
    if ( *(realIncidence.data()) != *(convertedIncidence.data()) ) {
        showDiff(format.toString( realIncidence ), format.toString( convertedIncidence ));
    }
    QVERIFY( *(realIncidence.data()) ==  *(convertedIncidence.data()) );
}

QTEST_MAIN( TimezoneTest )

#include "timezonetest.moc"
