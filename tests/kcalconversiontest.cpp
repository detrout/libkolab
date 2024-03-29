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

#include "kcalconversiontest.h"
#include <contact.h>

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include <ksystemtimezone.h>
#include <kolabcontact.h>
#include <kcalcore/recurrence.h>
#include <kabc/addressee.h>

#include "conversion/kcalconversion.h"
#include "conversion/kcalconversion.cpp"
#include "conversion/kabcconversion.h"
#include "testhelpers.h"

using namespace Kolab::Conversion;

template <typename T>
void comparePointerVectors(const QVector<T> &list, const QVector<T> &other)
{
    QCOMPARE(list.size(), other.size());
    for (int i = 0 ; i < list.size(); i++) {
        QCOMPARE(*list.at(i), *other.at(i));
    }
}

void compareAttendeesVectors(const KCalCore::Attendee::List &list, const KCalCore::Attendee::List &other)
{
    QCOMPARE(list.size(), other.size());
    for (int i = 0 ; i < list.size(); i++) {
        KCalCore::Attendee::Ptr at1 = list.at(i).constCast<KCalCore::Attendee>();
        at1->setUid(QString());
        KCalCore::Attendee::Ptr at2 = other.at(i).constCast<KCalCore::Attendee>();
        at2->setUid(QString());
        QCOMPARE(*at1, *at2);
    }
}


void KCalConversionTest::testDate_data()
{
    QTest::addColumn<Kolab::cDateTime>( "input" );
    QTest::addColumn<KDateTime>( "result" );

    QTest::newRow( "datetime with tz" ) << Kolab::cDateTime("Europe/Zurich",2006,1,8,12,0,0) << KDateTime(QDate(2006, 1, 8), QTime(12, 0, 0), KSystemTimeZones::zone("Europe/Zurich"));
    QTest::newRow( "floating datetime" ) << Kolab::cDateTime(2006,1,8,12,0,0, false) << KDateTime(QDate(2006, 1, 8), QTime(12, 0, 0), KDateTime::Spec(KDateTime::ClockTime));
    QTest::newRow( "utc datetime" ) << Kolab::cDateTime(2006,1,8,12,0,0, true) << KDateTime(KDateTime(QDate(2006, 1, 8), QTime(12, 0, 0), KDateTime::UTC));
    QTest::newRow( "date only" ) << Kolab::cDateTime(2006,1,8) << KDateTime(QDate(2006, 1, 8));
}


void KCalConversionTest::testDate()
{
    QFETCH(Kolab::cDateTime, input);
    QFETCH(KDateTime, result);
    
    const KDateTime &r = Kolab::Conversion::toDate(input);
    QCOMPARE(r, result);
    
    const Kolab::cDateTime &r2 = Kolab::Conversion::fromDate(result);
    QCOMPARE(r2, input);
}

void KCalConversionTest::testDuration_data()
{
    QTest::addColumn<Kolab::Duration>( "input" );
    QTest::addColumn<KCalCore::Duration>( "result" );
    QTest::addColumn<Kolab::Duration>( "fromResult" );
    
    QTest::newRow( "seconds" ) << Kolab::Duration(0,0,0,30,false) << KCalCore::Duration(30, KCalCore::Duration::Seconds) << Kolab::Duration(0,0,0,30,false);
    QTest::newRow( "minutes" ) << Kolab::Duration(0,0,1,30,false) << KCalCore::Duration(90, KCalCore::Duration::Seconds) << Kolab::Duration(0,0,0,90,false);
    QTest::newRow( "hours" ) << Kolab::Duration(0,1,1,30,false) << KCalCore::Duration(60*60+90, KCalCore::Duration::Seconds) << Kolab::Duration(0,0,0,60*60+90,false);
    QTest::newRow( "days" ) << Kolab::Duration(1,1,1,30,false) << KCalCore::Duration(24*60*60+60*60+90, KCalCore::Duration::Seconds) << Kolab::Duration(0,0,0,24*60*60+60*60+90,false);
    QTest::newRow( "daysonly" ) << Kolab::Duration(30,0,0,0, false) << KCalCore::Duration(30, KCalCore::Duration::Days) << Kolab::Duration(30,0,0,0,false);
    QTest::newRow( "weeks" ) << Kolab::Duration(30,false) << KCalCore::Duration(30*7, KCalCore::Duration::Days) << Kolab::Duration(30*7,0,0,0,false);
}


void KCalConversionTest::testDuration()
{
    QFETCH(Kolab::Duration, input);
    QFETCH(KCalCore::Duration, result);
    QFETCH(Kolab::Duration, fromResult);
    
    const KCalCore::Duration &r = Kolab::Conversion::toDuration(input);
    QCOMPARE(r, result);
    
    const Kolab::Duration &r2 = Kolab::Conversion::fromDuration(result);
    QCOMPARE(r2, fromResult);
}


void KCalConversionTest::testDateTZ_data()
{
    QTest::addColumn<Kolab::cDateTime>( "input" );
    QTest::addColumn<KDateTime>( "result" );
       
    QTest::newRow( "berlin" ) << Kolab::cDateTime("Europe/Berlin",2006,1,8,12,0,0) << KDateTime(QDate(2006, 1, 8), QTime(12, 0, 0), KSystemTimeZones::zone("Europe/Berlin"));
}

void KCalConversionTest::testDateTZ()
{
    QFETCH(Kolab::cDateTime, input);
    QFETCH(KDateTime, result);
    
    const KDateTime &r = Kolab::Conversion::toDate(input);
    QCOMPARE(result.timeZone().name(), QString::fromStdString(input.timezone()));
    QCOMPARE(r.timeZone().currentOffset(), result.timeZone().currentOffset());
    
    const Kolab::cDateTime &r2 = Kolab::Conversion::fromDate(result);
    QCOMPARE(QString::fromStdString(r2.timezone()), result.timeZone().name());
}


void KCalConversionTest::testConversion_data()
{
    QTest::addColumn<KCalCore::Event>( "kcal" );
    QTest::addColumn<Kolab::Event>( "kolab" );
    
    Kolab::cDateTime date(2011,2,2,12,11,10,true);
    Kolab::cDateTime date2(2011,2,2,12,12,10,true);
    Kolab::cDateTime date3(2012,2,2,12,12,10,true);
    std::vector<int> intVector;
    intVector.push_back(1);
    intVector.push_back(-3);
    intVector.push_back(2);
    std::vector<std::string> stringVector;
    stringVector.push_back("cat1");
    stringVector.push_back("cat2");
    stringVector.push_back("parent/child");
    
    {
        KCalCore::Event kcal;
        kcal.setUid("uid");
        kcal.setCreated(toDate(date));
        kcal.setLastModified(toDate(date));
        kcal.setRevision(3);
        kcal.setSecrecy(KCalCore::Incidence::SecrecyConfidential);
        kcal.setCategories(toStringList(stringVector));
        kcal.setDtStart(toDate(date));
        kcal.setDtEnd(toDate(date2));
        kcal.setTransparency(KCalCore::Event::Transparent);
        
        kcal.setRecurrenceId(toDate(date2)); //TODO THISANDFUTURE
        kcal.recurrence()->setDaily(3);
        kcal.recurrence()->setDuration(5);
        kcal.recurrence()->addRDateTime(toDate(date2));
        kcal.recurrence()->addRDate(toDate(date2).date());
        kcal.recurrence()->addExDateTime(toDate(date3));
        kcal.recurrence()->addExDate(toDate(date3).date());
        
        KCalCore::RecurrenceRule *rr = kcal.recurrence()->defaultRRule(true);
        QList<int> intList = QVector<int>::fromStdVector(intVector).toList();
        rr->setBySeconds(intList);
        rr->setByMinutes(intList);
        rr->setByHours(intList);
        rr->setByDays(QList<KCalCore::RecurrenceRule::WDayPos>() << KCalCore::RecurrenceRule::WDayPos(3,1) << KCalCore::RecurrenceRule::WDayPos(5,4));
        rr->setByMonthDays(intList);
        rr->setByYearDays(intList);
        rr->setByMonths(intList);
        rr->setByWeekNumbers(intList);
        
        kcal.setSummary("summary");
        kcal.setDescription("description");
        kcal.setPriority(3);
        kcal.setStatus(KCalCore::Incidence::StatusConfirmed);
        kcal.setLocation("location");
        kcal.setOrganizer(KCalCore::Person::Ptr(new KCalCore::Person("organizer", "organizer@email")));
        KCalCore::Attendee::Ptr att(new KCalCore::Attendee("attendee", "attendee@email", false, KCalCore::Attendee::NeedsAction, KCalCore::Attendee::ReqParticipant));
        att->setDelegate("mailto:delegatee<delegatee@email>");
        att->setDelegator("mailto:delegator<delegator@email>");
        kcal.addAttendee(att);
        kcal.addAttachment(KCalCore::Attachment::Ptr(new KCalCore::Attachment(QString("uri"), "mimetype/mime")));
        KCalCore::Alarm::Ptr alarm = KCalCore::Alarm::Ptr(new KCalCore::Alarm(&kcal));    
        KCalCore::Person::List addressees;
        addressees.append(KCalCore::Person::Ptr(new KCalCore::Person("name", "email@email")));
        alarm->setEmailAlarm("subject", "text", addressees, QStringList()); //No support for attachments
        kcal.addAlarm(alarm);
        //TODO alarms

        kcal.setNonKDECustomProperty("X-KOLAB-key1", "value1");
        kcal.setNonKDECustomProperty("X-KOLAB-key2", "value2");
        kcal.setCustomProperty("SOMEOTHERAPP", "key2", "value2");
        Q_ASSERT(kcal.nonKDECustomProperty("X-KOLAB-key1") == "value1");
        
        Kolab::Event kolab;
        kolab.setUid("uid");
        kolab.setCreated(date);
        kolab.setLastModified(date);
        kolab.setSequence(3);
        kolab.setClassification(Kolab::ClassConfidential);
        kolab.setCategories(stringVector);
        kolab.setStart(date);
        kolab.setEnd(date2);
        kolab.setTransparency(true);
        
        Kolab::RecurrenceRule rrule;
        rrule.setInterval(3);
        rrule.setFrequency(Kolab::RecurrenceRule::Daily);
        rrule.setCount(5);
        rrule.setBysecond(intVector);
        rrule.setByminute(intVector);
        rrule.setByhour(intVector);
        rrule.setByday(std::vector<Kolab::DayPos>() << Kolab::DayPos(3, Kolab::Monday) << Kolab::DayPos(5, Kolab::Thursday));
        rrule.setBymonthday(intVector);
        rrule.setByyearday(intVector);
        rrule.setByweekno(intVector);
        rrule.setBymonth(intVector);
        
        kolab.setRecurrenceRule(rrule);
        kolab.setRecurrenceID(date2, true);
        kolab.setRecurrenceDates(std::vector<Kolab::cDateTime>() << date2 << Kolab::cDateTime(date2.year(), date2.month(), date2.day()));
        kolab.setExceptionDates(std::vector<Kolab::cDateTime>() << date3 << Kolab::cDateTime(date3.year(), date3.month(), date3.day()));
        
        kolab.setSummary("summary");
        kolab.setDescription("description");
        kolab.setPriority(3);
        kolab.setStatus(Kolab::StatusConfirmed);
        kolab.setLocation("location");
        kolab.setOrganizer(Kolab::ContactReference(Kolab::ContactReference::EmailReference,"organizer@email", "organizer")); //TODO uid
        
        Kolab::Attendee a(Kolab::ContactReference(Kolab::ContactReference::EmailReference,"attendee@email", "attendee"));//TODO uid
        a.setDelegatedTo(std::vector<Kolab::ContactReference>() << Kolab::ContactReference(Kolab::ContactReference::EmailReference,"delegatee@email", "delegatee"));
        a.setDelegatedFrom(std::vector<Kolab::ContactReference>() << Kolab::ContactReference(Kolab::ContactReference::EmailReference,"delegator@email", "delegator"));
        a.setCutype(Kolab::CutypeIndividual);
        kolab.setAttendees(std::vector<Kolab::Attendee>() << a);
        
        Kolab::Attachment attach;
        attach.setUri("uri", "mimetype/mime");
        kolab.setAttachments(std::vector<Kolab::Attachment>() << attach);
        
    //     std::vector<std::string> receipents;
    //     receipents.push_back("email@email");
    //     Kolab::Alarm alarm2("summary", "description", receipents);
    //     kolab.setAlarms(std::vector<Kolab::Alarm>() << alarm2);

        //The sorting is random, just sort them here how we think they should arrive so we don't have to sort during compare (due to lazyness).
        std::vector<Kolab::CustomProperty> customproperties;
        customproperties.push_back(Kolab::CustomProperty("X-KDE-SOMEOTHERAPP-key2", "value2"));
        customproperties.push_back(Kolab::CustomProperty("key1", "value1"));
        customproperties.push_back(Kolab::CustomProperty("key2", "value2"));
        
        kolab.setCustomProperties(customproperties);

        QTest::newRow( "with endDate and recurrence duration" ) << kcal << kolab;
    }
    {
        KCalCore::Event kcal;
        kcal.setUid("uid");
        kcal.setCreated(toDate(date));
        kcal.setLastModified(toDate(date));
        kcal.setRevision(3);
        kcal.setDtStart(toDate(date));
        kcal.setDuration(KCalCore::Duration(toDate(date), toDate(date2)));
        kcal.recurrence()->setDaily(3);
        kcal.recurrence()->setEndDateTime(toDate(date3));
        
        Kolab::Event kolab;
        kolab.setUid("uid");
        kolab.setCreated(date);
        kolab.setLastModified(date);
        kolab.setSequence(3);
        kolab.setStart(date);
        kolab.setDuration(Kolab::Duration(0, 0, 1, 0));
        Kolab::RecurrenceRule rrule;
        rrule.setInterval(3);
        rrule.setFrequency(Kolab::RecurrenceRule::Daily);
        rrule.setEnd(date3);
        kolab.setRecurrenceRule(rrule);
        
        QTest::newRow("with duration and recurrence endDate") << kcal << kolab;
    }
    {
        Kolab::cDateTime start(2011,1,1);
        Kolab::cDateTime end(2011,1,3);
        
        KCalCore::Event kcal;
        kcal.setUid("uid");
        kcal.setCreated(toDate(date));
        kcal.setLastModified(toDate(date));
        kcal.setDtStart(toDate(start));
        kcal.setDtEnd(toDate(end));
        kcal.recurrence()->setDaily(3);
        kcal.recurrence()->setEndDateTime(toDate(end));
        
        Kolab::Event kolab;
        kolab.setUid("uid");
        kolab.setCreated(date);
        kolab.setLastModified(date);
        kolab.setStart(start);
        kolab.setEnd(end);
        Kolab::RecurrenceRule rrule;
        rrule.setInterval(3);
        rrule.setFrequency(Kolab::RecurrenceRule::Daily);
        rrule.setEnd(end);
        kolab.setRecurrenceRule(rrule);
        
        QTest::newRow("date only dates") << kcal << kolab;
    }
    {
        KCalCore::Event kcal;
        kcal.setUid("uid");
        kcal.setCreated(toDate(date));
        kcal.setLastModified(toDate(date));
        kcal.setDtStart(toDate(date));
        kcal.setSummary("äöü%@$£é¤¼²°€Š�");
        
        Kolab::Event kolab;
        kolab.setUid("uid");
        kolab.setCreated(date);
        kolab.setLastModified(date);
        kolab.setStart(date);
        kolab.setSummary(std::string(QString("äöü%@$£é¤¼²°€Š�").toUtf8().constData()));
        
        QTest::newRow("latin1+Unicode") << kcal << kolab;

    }
}

void KCalConversionTest::testConversion()
{
    QFETCH(KCalCore::Event, kcal);
    QFETCH(Kolab::Event, kolab);
    
    const KCalCore::Event::Ptr e = toKCalCore(kolab);
    
    QCOMPARE(e->uid(), kcal.uid());
    QCOMPARE(e->created(), kcal.created());
    QCOMPARE(e->lastModified(), kcal.lastModified());
    QCOMPARE(e->revision(), kcal.revision());
    QCOMPARE(e->secrecy(), kcal.secrecy());
    QCOMPARE(e->categories(), kcal.categories());
    QCOMPARE(e->dtStart(), kcal.dtStart());
    QCOMPARE(e->dtEnd(), kcal.dtEnd());
    QCOMPARE(e->duration(), kcal.duration());
    QCOMPARE(e->transparency(), kcal.transparency());
    QCOMPARE(*e->recurrence(), *kcal.recurrence());
    QCOMPARE(e->recurrenceId(), kcal.recurrenceId());
    QCOMPARE(e->recurrenceType(), kcal.recurrenceType());
    QCOMPARE(e->summary(), kcal.summary());
    QCOMPARE(e->description(), kcal.description());
    QCOMPARE(e->priority(), kcal.priority());
    QCOMPARE(e->status(), kcal.status());
    QCOMPARE(e->location(), kcal.location());
    QCOMPARE(e->organizer()->name(), kcal.organizer()->name());
    QCOMPARE(e->organizer()->email(), kcal.organizer()->email());
    compareAttendeesVectors(e->attendees(), kcal.attendees());
    comparePointerVectors(e->attachments(), kcal.attachments());
    
//     QCOMPARE(e->alarms(), kcal.alarms()); //TODO
    QCOMPARE(e->customProperties(), kcal.customProperties());

//     QBENCHMARK {
//         toKCalCore(kolab);
//     }
    
    const Kolab::Event &b = fromKCalCore(kcal);
    QCOMPARE(b.uid(), kolab.uid());
    QCOMPARE(b.created(), kolab.created());
    QCOMPARE(b.lastModified(), kolab.lastModified());
    QCOMPARE(b.sequence(), kolab.sequence());
    QCOMPARE(b.classification(), kolab.classification());
    QCOMPARE(b.categories(), kolab.categories());
    QCOMPARE(b.start(), kolab.start());
    QCOMPARE(b.end(), kolab.end());
    QCOMPARE(b.duration(), kolab.duration());
    QCOMPARE(b.transparency(), kolab.transparency());
    
    QCOMPARE(b.recurrenceRule(), kolab.recurrenceRule());
    QCOMPARE(b.recurrenceID(), kolab.recurrenceID());
    QCOMPARE(b.recurrenceDates(), kolab.recurrenceDates());
    QCOMPARE(b.exceptionDates(), kolab.exceptionDates());
    
    QCOMPARE(b.summary(), kolab.summary());
    QCOMPARE(b.description(), kolab.description());
    QCOMPARE(b.status(), kolab.status());
    QCOMPARE(b.location(), kolab.location());
    QCOMPARE(b.organizer(), kolab.organizer());
    QCOMPARE(b.attendees(), kolab.attendees());
    QCOMPARE(b.attachments(), kolab.attachments());
    QCOMPARE(b.customProperties(), kolab.customProperties());
}


void KCalConversionTest::testTodoConversion_data()
{
    QTest::addColumn<KCalCore::Todo>( "kcal" );
    QTest::addColumn<Kolab::Todo>( "kolab" );
    
    Kolab::cDateTime date(2011,2,2,12,11,10,true);
    Kolab::cDateTime date2(2011,2,2,12,12,10,true);
    
    {
        KCalCore::Todo kcal;
        kcal.setUid("uid");
        kcal.setDtStart(toDate(date));
        kcal.setDtDue(toDate(date2));
        kcal.setRelatedTo("uid2", KCalCore::Incidence::RelTypeParent);
        
        Kolab::Todo kolab;
        kolab.setUid("uid");
        kolab.setStart(date);
        kolab.setDue(date2);
        std::vector<std::string> relateds;
        relateds.push_back("uid2");
        kolab.setRelatedTo(relateds);

        QTest::newRow( "todo" ) << kcal << kolab;
    }
}


void KCalConversionTest::testTodoConversion()
{
    QFETCH(KCalCore::Todo, kcal);
    QFETCH(Kolab::Todo, kolab);
    
    const KCalCore::Todo::Ptr e = toKCalCore(kolab);
    
    QCOMPARE(e->uid(), kcal.uid());
    QCOMPARE(e->dtStart(), kcal.dtStart());
    QCOMPARE(e->dtDue(), kcal.dtDue());
    QCOMPARE(e->relatedTo(KCalCore::Incidence::RelTypeParent), kcal.relatedTo(KCalCore::Incidence::RelTypeParent));
   
    const Kolab::Todo &b = fromKCalCore(kcal);
    QCOMPARE(b.uid(), kolab.uid());
    QCOMPARE(b.start(), kolab.start());
    QCOMPARE(b.due(), kolab.due());
    QCOMPARE(b.relatedTo(), kolab.relatedTo());
}

void KCalConversionTest::testJournalConversion_data()
{
    QTest::addColumn<KCalCore::Journal>( "kcal" );
    QTest::addColumn<Kolab::Journal>( "kolab" );
    
    Kolab::cDateTime date(2011,2,2,12,11,10,true);
    Kolab::cDateTime date2(2011,2,2,12,12,10,true);
    
    {
        KCalCore::Journal kcal;
        kcal.setUid("uid");
        kcal.setDtStart(toDate(date));
        kcal.setSummary("summary");
        
        Kolab::Journal kolab;
        kolab.setUid("uid");
        kolab.setStart(date);
        kolab.setSummary("summary");
        
        QTest::newRow( "journal" ) << kcal << kolab;
    }
}


void KCalConversionTest::testJournalConversion()
{
    QFETCH(KCalCore::Journal, kcal);
    QFETCH(Kolab::Journal, kolab);
    
    const KCalCore::Journal::Ptr e = toKCalCore(kolab);
    
    QCOMPARE(e->uid(), kcal.uid());
    QCOMPARE(e->dtStart(), kcal.dtStart());
    QCOMPARE(e->summary(), kcal.summary());
    
    const Kolab::Journal &b = fromKCalCore(kcal);
    QCOMPARE(b.uid(), kolab.uid());
    QCOMPARE(b.start(), kolab.start());
    QCOMPARE(b.summary(), kolab.summary());
}

void KCalConversionTest::testContactConversion_data()
{
    QTest::addColumn<KABC::Addressee>( "kcal" );
    QTest::addColumn<Kolab::Contact>( "kolab" );
    
    Kolab::cDateTime date(2011,2,2,12,11,10,true);
    Kolab::cDateTime date2(2011,2,2,12,12,10,true);
    
    {
        KABC::Addressee kcal;
        kcal.setUid("uid");
        kcal.setFormattedName("name");
        
        Kolab::Contact kolab;
        kolab.setUid("uid");
        kolab.setName("name");
        
        QTest::newRow( "contact" ) << kcal << kolab;
    }
}


void KCalConversionTest::testContactConversion()
{
    QFETCH(KABC::Addressee, kcal);
    QFETCH(Kolab::Contact, kolab);
    
    const KABC::Addressee &e = toKABC(kolab);
    
    QCOMPARE(e.uid(), kcal.uid());
    QCOMPARE(e.formattedName(), kcal.formattedName());
    
    const Kolab::Contact &b = fromKABC(kcal);
    QCOMPARE(b.uid(), kolab.uid());
    QCOMPARE(b.name(), kolab.name());
}


// void KCalConversionTest::BenchmarkRoundtripKCAL()
// {
//     const Kolab::Event &event = Kolab::readEvent(TEST_DATA_PATH "/testfiles/icalEvent.xml", true);
//     std::string result = Kolab::writeEvent(event);
//     QBENCHMARK {
//         Kolab::Conversion::toKCalCore(Kolab::readEvent(result, false));
//     }
// }

QTEST_MAIN( KCalConversionTest )

#include "kcalconversiontest.moc"
