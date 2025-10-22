/**
 * @file Schedule.h
 * @brief 调度配置数据模型
 */

#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <QString>
#include <QTime>
#include <QVariantMap>

namespace ResticGUI {
namespace Models {

struct Schedule
{
    enum Type {
        None = 0,
        Manual,
        Minutely,
        Hourly,
        Daily,
        Weekly,
        Monthly,
        Custom
    };

    Type type = None;
    QTime time = QTime(0, 0);
    int dayOfWeek = 0;      // 0=周日
    int dayOfMonth = 1;
    QString cronExpression;

    // 条件
    bool requireAC = false;
    bool requireNetwork = false;
    bool requireIdle = false;

    bool isValid() const { return type != None; }
    QString description() const;
    QVariantMap toVariantMap() const;
    static Schedule fromVariantMap(const QVariantMap& map);
};

} // namespace Models
} // namespace ResticGUI

#endif // SCHEDULE_H
