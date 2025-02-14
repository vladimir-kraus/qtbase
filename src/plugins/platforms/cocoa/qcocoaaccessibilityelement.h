// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QCOCOAACCESIBILITYELEMENT_H
#define QCOCOAACCESIBILITYELEMENT_H

#ifndef QT_NO_ACCESSIBILITY

#include <QtCore/qglobal.h>

#include <QtCore/private/qcore_mac_p.h>
#include <QtGui/qaccessible.h>

QT_DECLARE_NAMESPACED_OBJC_INTERFACE(QMacAccessibilityElement, NSObject <NSAccessibilityElement>
- (instancetype)initWithId:(QAccessible::Id)anId;
+ (instancetype)elementWithId:(QAccessible::Id)anId;
)

#endif // QT_NO_ACCESSIBILITY

#endif // QCOCOAACCESIBILITYELEMENT_H
