/*
 * Copyright (C) 2011  Christian Mollekopf <mollekopf@kolabsys.com>
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

#include "kcalconversion.h"

#include <kcalcore/recurrence.h>
#include <QtCore/QBitArray>
#include <QtCore/QVector>
#include <QtCore/QDebug>
#include <QUrl>
#include <vector>

#include "kolabformat/errorhandler.h"
#include "commonconversion.h"

namespace Kolab {
    namespace Conversion {

//The uid of a contact which refers to the uuid of a contact in the addressbook
#define CUSTOM_KOLAB_CONTACT_UUID "X_KOLAB_CONTACT_UUID"
#define CUSTOM_KOLAB_CONTACT_CUTYPE "X_KOLAB_CONTACT_CUTYPE"

KCalCore::Duration toDuration(const Kolab::Duration &d)
{
    int value = 0;
    if (d.hours() || d.minutes() || d.seconds()) {
        value = ((((d.weeks() * 7 + d.days()) * 24 + d.hours()) * 60 + d.minutes()) * 60 + d.seconds());
        if (d.isNegative()) {
            value = -value;
        }
        return KCalCore::Duration(value);
    }
    value = d.weeks() * 7 + d.days();
    if (d.isNegative()) {
        value = -value;
    }
    return KCalCore::Duration(value, KCalCore::Duration::Days);
}

Kolab::Duration fromDuration(const KCalCore::Duration &d)
{
    int value = d.value();
    bool isNegative = false;
    if (value < 0) {
        isNegative = true;
        value = -value;
    }
    //We don't know how the seconds/days were distributed before, so no point in distributing them (probably)
    if (d.isDaily()) {
        int days = value;
        return Kolab::Duration(days, 0, 0, 0, isNegative);
    }
    int seconds = value;
//         int minutes = seconds / 60;
//         seconds = seconds % 60;
//         int hours = minutes / 60;
//         minutes = minutes % 60;
    return Kolab::Duration(0, 0, 0, seconds, isNegative);

}

KCalCore::Incidence::Secrecy toSecrecy(Kolab::Classification c)
{
    switch(c) {
        case Kolab::ClassPublic:
            return KCalCore::Incidence::SecrecyPublic;
        case Kolab::ClassPrivate:
            return KCalCore::Incidence::SecrecyPrivate;
        case Kolab::ClassConfidential:
            return KCalCore::Incidence::SecrecyConfidential;
        default:
            Error() << "unhandled";
            Q_ASSERT(0);
    }
    return KCalCore::Incidence::SecrecyPublic;
}

Kolab::Classification fromSecrecy(KCalCore::Incidence::Secrecy c)
{
    switch(c) {
        case KCalCore::Incidence::SecrecyPublic:
            return Kolab::ClassPublic;
        case KCalCore::Incidence::SecrecyPrivate:
            return Kolab::ClassPrivate;
        case KCalCore::Incidence::SecrecyConfidential:
            return Kolab::ClassConfidential;
        default:
            Error() << "unhandled";
            Q_ASSERT(0);
    }
    return Kolab::ClassPublic;
}

int toPriority(int priority)
{
    //Same mapping
    return priority;
}

int fromPriority(int priority)
{
    //Same mapping
    return priority;
}

KCalCore::Incidence::Status toStatus(Kolab::Status s)
{
    switch (s) {
        case StatusUndefined:
            return  KCalCore::Incidence::StatusNone;
        case StatusNeedsAction:
            return  KCalCore::Incidence::StatusNeedsAction;
        case StatusCompleted:
            return  KCalCore::Incidence::StatusCompleted;
        case StatusInProcess:
            return  KCalCore::Incidence::StatusInProcess;
        case StatusCancelled:
            return  KCalCore::Incidence::StatusCanceled;
        case StatusTentative:
            return  KCalCore::Incidence::StatusTentative;
        case StatusConfirmed:
            return  KCalCore::Incidence::StatusConfirmed;
        case StatusDraft:
            return  KCalCore::Incidence::StatusDraft;
        case StatusFinal:
            return  KCalCore::Incidence::StatusFinal;
        default:
            Error() << "unhandled";
            Q_ASSERT(0);
    }
    return KCalCore::Incidence::StatusNone;
}

Kolab::Status fromStatus(KCalCore::Incidence::Status s)
{
    switch (s) {
        case KCalCore::Incidence::StatusNone:
            return StatusUndefined;
        case KCalCore::Incidence::StatusNeedsAction:
            return StatusNeedsAction;
        case KCalCore::Incidence::StatusCompleted:
            return StatusCompleted;
        case KCalCore::Incidence::StatusInProcess:
            return StatusInProcess;
        case KCalCore::Incidence::StatusCanceled:
            return StatusCancelled;
        case KCalCore::Incidence::StatusTentative:
            return StatusTentative;
        case KCalCore::Incidence::StatusConfirmed:
            return StatusConfirmed;
        case KCalCore::Incidence::StatusDraft:
            return StatusDraft;
        case KCalCore::Incidence::StatusFinal:
            return StatusFinal;
        default:
            Error() << "unhandled";
            Q_ASSERT(0);
    }
    return StatusUndefined;
}

KCalCore::Attendee::PartStat toPartStat(Kolab::PartStatus p)
{
    switch (p) {
        case PartNeedsAction:
            return KCalCore::Attendee::NeedsAction;
        case PartAccepted:
            return KCalCore::Attendee::Accepted;
        case PartDeclined:
            return KCalCore::Attendee::Declined;
        case PartTentative:
            return KCalCore::Attendee::Tentative;
        case PartDelegated:
            return KCalCore::Attendee::Delegated;
        default:
            Error() << "unhandled";
            Q_ASSERT(0);
    }
    return KCalCore::Attendee::NeedsAction;
}

Kolab::PartStatus fromPartStat(KCalCore::Attendee::PartStat p)
{
    switch (p) {
        case KCalCore::Attendee::NeedsAction:
            return PartNeedsAction;
        case KCalCore::Attendee::Accepted:
            return PartAccepted;
        case KCalCore::Attendee::Declined:
            return PartDeclined;
        case KCalCore::Attendee::Tentative:
            return PartTentative;
        case KCalCore::Attendee::Delegated:
            return PartDelegated;
        default:
            Error() << "unhandled";
            Q_ASSERT(0);
    }
    return PartNeedsAction;
}

KCalCore::Attendee::Role toRole(Kolab::Role r)
{
    switch (r) {
        case Required:
            return KCalCore::Attendee::ReqParticipant;
        case Chair:
            return KCalCore::Attendee::Chair;
        case Optional:
            return KCalCore::Attendee::OptParticipant;
        case NonParticipant:
            return KCalCore::Attendee::NonParticipant;
        default:
            Error() << "unhandled";
            Q_ASSERT(0);
    }
    return KCalCore::Attendee::ReqParticipant;
}

Kolab::Role fromRole(KCalCore::Attendee::Role r)
{
    switch (r) {
        case KCalCore::Attendee::ReqParticipant:
            return Required;
        case KCalCore::Attendee::Chair:
            return Chair;
        case KCalCore::Attendee::OptParticipant:
            return Optional;
        case KCalCore::Attendee::NonParticipant:
            return NonParticipant;
        default:
            Error() << "unhandled";
            Q_ASSERT(0);
    }
    return Required;
}

template <typename T>
QString getCustomProperty(const QString &id, const T &e)
{
    std::vector<CustomProperty> &props = e.customProperties();
    foreach (const CustomProperty &p, props) {
        if (fromStdString(p.identifier) == id) {
            return fromStdString(p.value);
        }
    }
}

template <typename T>
void setIncidence(KCalCore::Incidence &i, const T &e)
{
    if (!e.uid().empty()) {
        i.setUid(fromStdString(e.uid()));
    }
    
    i.setCreated(toDate(e.created()));
    i.setLastModified(toDate(e.lastModified()));
    i.setRevision(e.sequence());
    i.setSecrecy(toSecrecy(e.classification()));
    i.setCategories(toStringList(e.categories()));
    
    if (e.start().isValid()) {
        i.setDtStart(toDate(e.start()));
    }

    i.setSummary(fromStdString(e.summary())); //TODO detect richtext
    i.setDescription(fromStdString(e.description())); //TODO detect richtext
    i.setStatus(toStatus(e.status()));
    foreach (const Kolab::Attendee a, e.attendees()) {
        /*
         * KCalCore always sets a UID if empty, but that's just a pointer, and not the uid of a real contact.
         * Since that means the semantics of the two are different, we have to store the kolab uid as a custom property.
         */
        KCalCore::Attendee::Ptr attendee = KCalCore::Attendee::Ptr(new KCalCore::Attendee(fromStdString(a.contact().name()), 
                                                                                          fromStdString(a.contact().email()),
                                                                                          a.rsvp(),
                                                                                          toPartStat(a.partStat()),
                                                                                          toRole(a.role()) ));
        if (!a.contact().uid().empty()) { //TODO Identify contact from addressbook based on uid
            attendee->customProperties().setNonKDECustomProperty(CUSTOM_KOLAB_CONTACT_UUID, fromStdString(a.contact().uid()));
        }
        if (!a.delegatedTo().empty()) {
            if (a.delegatedTo().size() > 1) {
                WARNING("multiple delegatees are not supported");
            }
            attendee->setDelegate(toMailto(a.delegatedTo().front().email(), a.delegatedTo().front().name()).toString());
        }
        if (!a.delegatedFrom().empty()) {
            if (a.delegatedFrom().size() > 1) {
                WARNING("multiple delegators are not supported");
            }
            attendee->setDelegator(toMailto(a.delegatedFrom().front().email(), a.delegatedFrom().front().name()).toString());
        }
        if (!a.cutype() != CutypeIndividual) {
            attendee->customProperties().setNonKDECustomProperty(CUSTOM_KOLAB_CONTACT_CUTYPE, QString::number(a.cutype()));
        }
        i.addAttendee(attendee);
    }
    foreach (const Kolab::Attachment a, e.attachments()) {
        KCalCore::Attachment::Ptr ptr;
        if (!a.uri().empty()) {
            ptr = KCalCore::Attachment::Ptr(new KCalCore::Attachment(fromStdString(a.uri()), fromStdString(a.mimetype())));
        } else {
            ptr = KCalCore::Attachment::Ptr(new KCalCore::Attachment(QByteArray::fromRawData(a.data().c_str(), a.data().size()), fromStdString(a.mimetype())));
        }
        if (!a.label().empty()) {
            ptr->setLabel(fromStdString(a.label()));
        }
        i.addAttachment(ptr);
    }

    QMap<QByteArray, QString> props;
    foreach (const Kolab::CustomProperty &prop, e.customProperties()) {
        QString key;
        if (prop.identifier.compare(0, 5, "X-KDE")) {
            key.append(QString::fromLatin1("X-KOLAB-"));
        }
        key.append(fromStdString(prop.identifier));
        props.insert(key.toLatin1(), fromStdString(prop.value));
//         i.setCustomProperty("KOLAB", fromStdString(prop.identifier).toLatin1(), fromStdString(prop.value));
    }
    i.setCustomProperties(props);
}

template <typename T, typename I>
void getIncidence(T &i, const I &e)
{
    i.setUid(toStdString(e.uid()));
    i.setCreated(fromDate(e.created()));
    i.setLastModified(fromDate(e.lastModified()));
    i.setSequence(e.revision());
    i.setClassification(fromSecrecy(e.secrecy()));
    i.setCategories(fromStringList(e.categories()));
    
    i.setStart(fromDate(e.dtStart()));
    i.setSummary(toStdString(e.summary()));
    i.setDescription(toStdString(e.description()));
    i.setStatus(fromStatus(e.status()));
    std::vector<Kolab::Attendee> attendees;
    foreach (const KCalCore::Attendee::Ptr ptr, e.attendees()) {
        const QString &uid = ptr->customProperties().nonKDECustomProperty(CUSTOM_KOLAB_CONTACT_UUID);
        Kolab::Attendee a(Kolab::ContactReference(toStdString(ptr->email()), toStdString(ptr->name()), toStdString(uid)));
        a.setRSVP(ptr->RSVP());
        a.setPartStat(fromPartStat(ptr->status()));
        a.setRole(fromRole(ptr->role()));
        if (!ptr->delegate().isEmpty()) {
            std::string name;
            const std::string &email = fromMailto(ptr->delegate(), name);
            a.setDelegatedTo(std::vector<Kolab::ContactReference>() << Kolab::ContactReference(email, name));
        }
        if (!ptr->delegator().isEmpty()) {
            std::string name;
            const std::string &email = fromMailto(ptr->delegator(), name);
            a.setDelegatedFrom(std::vector<Kolab::ContactReference>() << Kolab::ContactReference(email, name));
        }
        const QString &cutype = ptr->customProperties().nonKDECustomProperty(CUSTOM_KOLAB_CONTACT_CUTYPE);
        if (!cutype.isEmpty()) {
            a.setCutype(static_cast<Kolab::Cutype>(cutype.toInt()));
        }
        
        attendees.push_back(a);
    }
    i.setAttendees(attendees);
    std::vector<Kolab::Attachment> attachments;
    foreach (const KCalCore::Attachment::Ptr &ptr, e.attachments()) {
        Kolab::Attachment a;
        if (ptr->isUri()) {
            a.setUri(toStdString(ptr->uri()), toStdString(ptr->mimeType()));
        } else {
            a.setData(std::string(ptr->decodedData().data(), ptr->decodedData().size()), toStdString(ptr->mimeType()));
        }
        a.setLabel(toStdString(ptr->label()));
        attachments.push_back(a);
    }
    i.setAttachments(attachments);

    std::vector<Kolab::CustomProperty> customProperties;
    const QMap<QByteArray, QString> &props = e.customProperties();
    for (QMap<QByteArray, QString>::const_iterator it = props.begin(); it != props.end(); it++) {
        QString key = QString(it.key());
        customProperties.push_back(Kolab::CustomProperty(toStdString(key.remove("X-KOLAB-")), toStdString(it.value())));
    }
    i.setCustomProperties(customProperties);
}

int toWeekDay(Kolab::Weekday wday)
{
    switch (wday) {
        case Kolab::Monday:
            return  1;
        case Kolab::Tuesday:
            return 2;
        case Kolab::Wednesday:
            return 3;
        case Kolab::Thursday:
            return 4;
        case Kolab::Friday:
            return 5;
        case Kolab::Saturday:
            return 6;
        case Kolab::Sunday:
            return 7;
        default:
            Error() << "unhandled";
            Q_ASSERT(0);
    }
    return 1;
}

Kolab::Weekday fromWeekDay(int wday)
{
    switch (wday) {
        case 1:
            return Kolab::Monday;
        case 2:
            return Kolab::Tuesday;
        case 3:
            return Kolab::Wednesday;
        case 4:
            return Kolab::Thursday;
        case 5:
            return Kolab::Friday;
        case 6:
            return Kolab::Saturday;
        case 7:
            return Kolab::Sunday;
        default:
            Error() << "unhandled";
            Q_ASSERT(0);
    }
    return Kolab::Monday;
}

KCalCore::RecurrenceRule::PeriodType toRecurrenceType(Kolab::RecurrenceRule::Frequency freq)
{
    switch(freq) {
        case Kolab::RecurrenceRule::FreqNone:
            Warning() << "no recurrence?";
            break;
        case Kolab::RecurrenceRule::Yearly:
            return KCalCore::RecurrenceRule::rYearly;
        case Kolab::RecurrenceRule::Monthly:
            return KCalCore::RecurrenceRule::rMonthly;
        case Kolab::RecurrenceRule::Weekly:
            return KCalCore::RecurrenceRule::rWeekly;
        case Kolab::RecurrenceRule::Daily:
            return KCalCore::RecurrenceRule::rDaily;
        case Kolab::RecurrenceRule::Hourly:
            return KCalCore::RecurrenceRule::rHourly;
        case Kolab::RecurrenceRule::Minutely:
            return KCalCore::RecurrenceRule::rMinutely;
        case Kolab::RecurrenceRule::Secondly:
            return KCalCore::RecurrenceRule::rSecondly;
        default:
            Error() << "unhandled";
            Q_ASSERT(0);
    }
    return KCalCore::RecurrenceRule::rNone;
}

Kolab::RecurrenceRule::Frequency fromRecurrenceType(KCalCore::RecurrenceRule::PeriodType freq)
{
    switch(freq) {
        case KCalCore::RecurrenceRule::rNone:
            Warning() << "no recurrence?";
            break;
        case KCalCore::RecurrenceRule::rYearly:
            return Kolab::RecurrenceRule::Yearly;
        case KCalCore::RecurrenceRule::rMonthly:
            return Kolab::RecurrenceRule::Monthly;
        case KCalCore::RecurrenceRule::rWeekly:
            return Kolab::RecurrenceRule::Weekly;
        case KCalCore::RecurrenceRule::rDaily:
            return Kolab::RecurrenceRule::Daily;
        case KCalCore::RecurrenceRule::rHourly:
            return Kolab::RecurrenceRule::Hourly;
        case KCalCore::RecurrenceRule::rMinutely:
            return Kolab::RecurrenceRule::Minutely;
        case KCalCore::RecurrenceRule::rSecondly:
            return Kolab::RecurrenceRule::Secondly;
        default:
            Error() << "unhandled";
            Q_ASSERT(0);
    }
    return Kolab::RecurrenceRule::FreqNone;
}

KCalCore::RecurrenceRule::WDayPos toWeekDayPos(const Kolab::DayPos &dp)
{
    return KCalCore::RecurrenceRule::WDayPos(dp.occurence(), toWeekDay(dp.weekday()));
}

Kolab::DayPos fromWeekDayPos(const KCalCore::RecurrenceRule::WDayPos &dp)
{
    return Kolab::DayPos(dp.pos(), fromWeekDay(dp.day()));
}

template <typename T>
void setRecurrence(KCalCore::Incidence &e, const T &event)
{
    const Kolab::RecurrenceRule &rrule = event.recurrenceRule();
    if (rrule.isValid()) {
        KCalCore::Recurrence *rec = e.recurrence();
        
        KCalCore::RecurrenceRule *defaultRR = rec->defaultRRule(true);
        Q_ASSERT(defaultRR);
        
        defaultRR->setWeekStart(toWeekDay(rrule.weekStart()));
        defaultRR->setRecurrenceType(toRecurrenceType(rrule.frequency()));
        defaultRR->setFrequency(rrule.interval());
        
        if (rrule.end().isValid()) {
            rec->setEndDateTime(toDate(rrule.end())); //TODO date/datetime setEndDate(). With date-only the start date has to be taken into account.
        } else {
            rec->setDuration(rrule.count());
        }
        
        if (!rrule.bysecond().empty()) {
            defaultRR->setBySeconds(QVector<int>::fromStdVector(rrule.bysecond()).toList());
        }
        if (!rrule.byminute().empty()) {
            defaultRR->setByMinutes(QVector<int>::fromStdVector(rrule.byminute()).toList());
        }
        if (!rrule.byhour().empty()) {
            defaultRR->setByHours(QVector<int>::fromStdVector(rrule.byhour()).toList());
        }
        if (!rrule.byday().empty()) {
            QList<KCalCore::RecurrenceRule::WDayPos> daypos;
            foreach(const Kolab::DayPos &dp, rrule.byday()) {
                daypos.append(toWeekDayPos(dp));
            }
            defaultRR->setByDays(daypos);
        }
        if (!rrule.bymonthday().empty()) {
            defaultRR->setByMonthDays(QVector<int>::fromStdVector(rrule.bymonthday()).toList());
        }
        if (!rrule.byyearday().empty()) {
            defaultRR->setByYearDays(QVector<int>::fromStdVector(rrule.byyearday()).toList());
        }
        if (!rrule.byweekno().empty()) {
            defaultRR->setByWeekNumbers(QVector<int>::fromStdVector(rrule.byweekno()).toList());
        }
        if (!rrule.bymonth().empty()) {
            defaultRR->setByMonths(QVector<int>::fromStdVector(rrule.bymonth()).toList());
        }
    }
    foreach (const Kolab::cDateTime &dt, event.recurrenceDates()) {
        const KDateTime &date = toDate(dt);
        if (date.isDateOnly()) {
            e.recurrence()->addRDate(date.date());
        } else {
            e.recurrence()->addRDateTime(date);
        }
    }
    foreach (const Kolab::cDateTime &dt, event.exceptionDates()) {
        const KDateTime &date = toDate(dt);
        if (date.isDateOnly()) {
            e.recurrence()->addExDate(date.date());
        } else {
            e.recurrence()->addExDateTime(date);
        }
    }

}

template <typename T, typename I>
void getRecurrence(T &i, const I &e)
{
    if (!e.recurs()) {
        return;
    }
    KCalCore::Recurrence *rec = e.recurrence();
    KCalCore::RecurrenceRule *defaultRR = rec->defaultRRule(false);
    if (!defaultRR) {
        Warning() << "no recurrence";
        return;
    }
    Q_ASSERT(defaultRR);

    Kolab::RecurrenceRule rrule;
    rrule.setWeekStart(fromWeekDay(defaultRR->weekStart()));
    rrule.setFrequency(fromRecurrenceType(defaultRR->recurrenceType()));
    rrule.setInterval(defaultRR->frequency());
  
    if (defaultRR->duration() != 0) { //Inidcates if end date is set or not
        if (defaultRR->duration() > 0) {
            rrule.setCount(defaultRR->duration());
        }
    } else {
        rrule.setEnd(fromDate(defaultRR->endDt()));
    }
    
    rrule.setBysecond(defaultRR->bySeconds().toVector().toStdVector());
    rrule.setByminute(defaultRR->byMinutes().toVector().toStdVector());
    rrule.setByhour(defaultRR->byHours().toVector().toStdVector());
    
    std::vector<Kolab::DayPos> daypos;
    foreach (const KCalCore::RecurrenceRule::WDayPos &dp, defaultRR->byDays()) {
        daypos.push_back(fromWeekDayPos(dp));
    }
    rrule.setByday(daypos);
    
    rrule.setBymonthday(defaultRR->byMonthDays().toVector().toStdVector());
    rrule.setByyearday(defaultRR->byYearDays().toVector().toStdVector());
    rrule.setByweekno(defaultRR->byWeekNumbers().toVector().toStdVector());
    rrule.setBymonth(defaultRR->byMonths().toVector().toStdVector());
    i.setRecurrenceRule(rrule);
    
    std::vector<Kolab::cDateTime> rdates;
    foreach (const KDateTime &dt, rec->rDateTimes()) {
        rdates.push_back(fromDate(dt));
    }
    foreach (const QDate &dt, rec->rDates()) {
        rdates.push_back(fromDate(KDateTime(dt)));
    }
    i.setRecurrenceDates(rdates);
    
    std::vector<Kolab::cDateTime> exdates;
    foreach (const KDateTime &dt, rec->exDateTimes()) {
        exdates.push_back(fromDate(dt));
    }
    foreach (const QDate &dt, rec->exDates()) {
        exdates.push_back(fromDate(KDateTime(dt)));
    }
    i.setExceptionDates(exdates);
    
    if (!rec->exRules().empty()) {
        Warning() << "exrules are not supported";
    }
}

template <typename T>
void setTodoEvent(KCalCore::Incidence &i, const T &e)
{
    i.setPriority(toPriority(e.priority()));
    if (!e.location().empty()) {
        i.setLocation(fromStdString(e.location())); //TODO detect richtext
    }
    if (e.organizer().isValid()) {
        i.setOrganizer(KCalCore::Person::Ptr(new KCalCore::Person(fromStdString(e.organizer().name()), fromStdString(e.organizer().email())))); //TODO handle uid too
    }
    if (e.recurrenceID().isValid()) {
        i.setRecurrenceId(toDate(e.recurrenceID())); //TODO THISANDFUTURE
    }
    setRecurrence(i, e);
    foreach (const Kolab::Alarm a, e.alarms()) {
        KCalCore::Alarm::Ptr alarm = KCalCore::Alarm::Ptr(new KCalCore::Alarm(&i));
        switch (a.type()) {
            case Kolab::Alarm::EMailAlarm: {
                KCalCore::Person::List receipents;
                foreach (Kolab::ContactReference c ,a.attendees()) {
                    KCalCore::Person::Ptr person = KCalCore::Person::Ptr(new KCalCore::Person(fromStdString(c.name()), fromStdString(c.email())));
                    receipents.append(person);
                }
                alarm->setEmailAlarm(fromStdString(a.summary()), fromStdString(a.description()), receipents);
            }
                break;
            case Kolab::Alarm::DisplayAlarm:
                alarm->setDisplayAlarm(fromStdString(a.text()));
                break;
            case Kolab::Alarm::AudioAlarm:
                alarm->setAudioAlarm(fromStdString(a.audioFile().uri()));
                break;
            default:
                Error() << "invalid alarm";
        }

        if (a.start().isValid()) {
            alarm->setTime(toDate(a.start()));
        } else if (a.relativeStart().isValid()) {
            if (a.relativeTo() == Kolab::End) {
                alarm->setEndOffset(toDuration(a.relativeStart()));
            } else {
                alarm->setStartOffset(toDuration(a.relativeStart()));
            }
        }
        
        alarm->setSnoozeTime(toDuration(a.duration()));
        alarm->setRepeatCount(a.numrepeat());
        alarm->setEnabled(true);
        i.addAlarm(alarm);       
    }
}

template <typename T, typename I>
void getTodoEvent(T &i, const I &e)
{
    i.setPriority(fromPriority(e.priority()));
    i.setLocation(toStdString(e.location()));
    if (e.organizer() && !e.organizer()->email().isEmpty()) {
        i.setOrganizer(Kolab::ContactReference(Kolab::ContactReference::EmailReference, toStdString(e.organizer()->email()), toStdString(e.organizer()->name()))); //TODO handle uid too
    }
    i.setRecurrenceID(fromDate(e.recurrenceId()), false); //TODO THISANDFUTURE
    getRecurrence(i, e);
    std::vector <Kolab::Alarm> alarms;
    foreach (const KCalCore::Alarm::Ptr &a, e.alarms()) {
        Kolab::Alarm alarm;
        //TODO KCalCore disables alarms using KCalCore::Alarm::enabled() (X-KDE-KCALCORE-ENABLED) We should either delete the alarm, or store the attribute .
        //Ideally we would store the alarm somewhere and temporarily delete it, so we can restore it when parsing. For now we just remove disabled alarms.
        if (!a->enabled()) {
            Warning() << "skipping disabled alarm";
            continue;
        }
        switch (a->type()) {
            case KCalCore::Alarm::Display:
                alarm = Kolab::Alarm(toStdString(a->text()));
                break;
            case KCalCore::Alarm::Email: {
                std::vector<Kolab::ContactReference> receipents;
                foreach(const KCalCore::Person::Ptr &p, a->mailAddresses()) {
                    receipents.push_back(Kolab::ContactReference(toStdString(p->email()), toStdString(p->name())));
                }
                alarm = Kolab::Alarm(toStdString(a->mailSubject()), toStdString(a->mailText()), receipents);
            }
                break;
            case KCalCore::Alarm::Audio: {
                Kolab::Attachment audioFile;
                audioFile.setUri(toStdString(a->audioFile()), std::string());
                alarm = Kolab::Alarm(audioFile);
            }
                break;
            default:
                Error() << "unhandled alarm";
        }

        if (a->hasTime()) {
            alarm.setStart(fromDate(a->time()));
        } else if (a->hasStartOffset()) {
            alarm.setRelativeStart(fromDuration(a->startOffset()), Start);
        } else if (a->hasEndOffset()) {
            alarm.setRelativeStart(fromDuration(a->endOffset()), End);
        } else {
            Error() << "alarm trigger is missing";
            continue;
        }
        
        alarm.setDuration(fromDuration(a->snoozeTime()), a->repeatCount());
        
        alarms.push_back(alarm);
    }
    i.setAlarms(alarms);
}

KCalCore::Event::Ptr toKCalCore(const Kolab::Event &event)
{
    KCalCore::Event::Ptr e(new KCalCore::Event);
    setIncidence(*e, event);
    setTodoEvent(*e, event);
    if (event.end().isValid()) {
        e->setDtEnd(toDate(event.end()));
    }
    if (event.duration().isValid()) {
        e->setDuration(toDuration(event.duration()));
    }
    if (event.transparency()) {
        e->setTransparency(KCalCore::Event::Transparent);
    } else {
        e->setTransparency(KCalCore::Event::Opaque);
    }
    return e;
}

Event fromKCalCore(const KCalCore::Event &event)
{
    Event e;
    getIncidence(e, event);
    getTodoEvent(e, event);
    if (event.hasEndDate()) {
        e.setEnd(fromDate(event.dtEnd()));
    } else if (event.hasDuration()) {
        e.setDuration(fromDuration(event.duration()));
    }
    if (event.transparency() == KCalCore::Event::Transparent) {
        e.setTransparency(true);
    } else {
        e.setTransparency(false);
    }
    return e;
}



KCalCore::Todo::Ptr toKCalCore ( const Todo &todo )
{
    KCalCore::Todo::Ptr e(new KCalCore::Todo);
    setIncidence(*e, todo);
    setTodoEvent(*e, todo);
    if (todo.due().isValid()) {
        e->setDtDue(toDate(todo.due()));
    }
    if (!todo.relatedTo().empty()) {
        e->setRelatedTo(Conversion::fromStdString(todo.relatedTo().front()), KCalCore::Incidence::RelTypeParent);
        if (todo.relatedTo().size() > 1) {
            Error() << "only one relation support but got multiple";
        }
    }
    e->setPercentComplete(todo.percentComplete());
    return e;
}

Todo fromKCalCore ( const KCalCore::Todo &todo )
{
    Todo t;
    getIncidence(t, todo);
    getTodoEvent(t, todo);
    t.setDue(fromDate(todo.dtDue(true)));
    t.setPercentComplete(todo.percentComplete());
    const QString relatedTo = todo.relatedTo(KCalCore::Incidence::RelTypeParent);
    if (!relatedTo.isEmpty()) {
        std::vector<std::string> relateds;
        relateds.push_back(Conversion::toStdString(relatedTo));
        t.setRelatedTo(relateds);
    }
    return t;
}

KCalCore::Journal::Ptr toKCalCore ( const Journal &journal )
{
    KCalCore::Journal::Ptr e(new KCalCore::Journal);
    setIncidence(*e, journal);
    //TODO contacts
    return e;
}

Journal fromKCalCore ( const KCalCore::Journal &journal )
{
    Journal j;
    getIncidence(j, journal);
    //TODO contacts
    return j;
}



}
}
