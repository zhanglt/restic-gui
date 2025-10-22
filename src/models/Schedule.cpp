#include "Schedule.h"

namespace ResticGUI {
namespace Models {

QString Schedule::description() const
{
    switch (type) {
    case Minutely: return "每分钟";
    case Hourly:  return "每小时";
    case Daily:   return QString("每天 %1").arg(time.toString("HH:mm"));
    case Weekly:  return QString("每周%1 %2").arg(dayOfWeek).arg(time.toString("HH:mm"));
    case Monthly: return QString("每月%1日 %2").arg(dayOfMonth).arg(time.toString("HH:mm"));
    case Custom:  return cronExpression;
    default:      return "手动";
    }
}

QVariantMap Schedule::toVariantMap() const
{
    QVariantMap map;
    map["type"] = static_cast<int>(type);
    map["time"] = time.toString();
    map["dayOfWeek"] = dayOfWeek;
    map["dayOfMonth"] = dayOfMonth;
    map["cronExpression"] = cronExpression;
    map["requireAC"] = requireAC;
    map["requireNetwork"] = requireNetwork;
    map["requireIdle"] = requireIdle;
    return map;
}

Schedule Schedule::fromVariantMap(const QVariantMap& map)
{
    Schedule schedule;
    schedule.type = static_cast<Type>(map.value("type", 0).toInt());
    schedule.time = QTime::fromString(map.value("time").toString());
    schedule.dayOfWeek = map.value("dayOfWeek", 0).toInt();
    schedule.dayOfMonth = map.value("dayOfMonth", 1).toInt();
    schedule.cronExpression = map.value("cronExpression").toString();
    schedule.requireAC = map.value("requireAC", false).toBool();
    schedule.requireNetwork = map.value("requireNetwork", false).toBool();
    schedule.requireIdle = map.value("requireIdle", false).toBool();
    return schedule;
}

} // namespace Models
} // namespace ResticGUI
